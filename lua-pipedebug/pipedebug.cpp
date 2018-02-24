#include "stdafx.h"
#include "luautil.h"
#include "commandhandler.h"

#ifdef __cplusplus
extern "C"{
#endif

    struct ldb g_db;
	
    bool filter_debug_command(std::string& command,bool &bcontinue)
    {		
		int nbegin = 0;
		std::string command_name = parse_segment(command,nbegin);
        
        for (int i = 0; debug_command_filter[i].command_name != NULL;++i)
        {
			if (debug_command_filter[i].command_shortname == command_name || debug_command_filter[i].command_name == command_name)
			{
				std::string params = command.substr(nbegin);
				bcontinue = ((*debug_command_filter[i].handler)(params,&g_db) != E_DO_BREAK);
				return true;
			}
        }

        return false;
    }

    void lua_debughook(lua_State* L, lua_Debug* ar)
    {		
        // 进入中断
        bool  b_break = false;
		b_break = g_db.step_break_flag;
		if (ar->event != LUA_HOOKCALL && ar->event != LUA_HOOKLINE && ar->event != LUA_HOOKTAILCALL)
		{
			return;
		}

		// 仅 lua 调试
		lua_getinfo(L, "S", ar);
		if(ar->what == NULL || strcmp(ar->what,"Lua") != 0)
		{
			return;
		}		

        {
            CAutoLock lock(g_db.break_mutex);
            if (!b_break && g_db.breaks == NULL)
            {
                return;
            }

			lua_getinfo(L, "ln", ar);

            l_break* begin = g_db.breaks;
            for (; b_break || begin != NULL;begin= begin->next)
            {
				// 首次 表示单步调试
                if (b_break 
					|| (ar->event == LUA_HOOKLINE && ar->currentline == begin->line && ar->source[0] == '@' && strcmp(ar->short_src,begin->name.c_str()) == 0))
                {
					b_break = true;
					fprintf(stderr,"==break : %s:%d \nlua debug> "
							,ar->short_src
							,ar->currentline);
					break;
                }  						
				else if(ar->event != LUA_HOOKLINE && ar->name != NULL && begin->line == -1 && !begin->name.empty() && strcmp(ar->name,begin->name.c_str()) == 0)
				{
					b_break = true;
					fprintf(stderr,"== function : [%s:%d] %s\nlua debug> "
						,ar->short_src
						,ar->currentline
						,ar->name);						
					break;
				}
            }
        }

		g_db.step_break_flag = false;
        std::list<LNetPack*> msg_queue;
        while (b_break&&g_db.client_connected.load())
        {
            if (msg_queue.empty())
            {
				if (WaitForSingleObject(g_db.msg_queue_event,1000) == WAIT_TIMEOUT)
				{
					continue;
				}
				
				// 重置
				ResetEvent(g_db.msg_queue_event);

				{
					CAutoLock lock(g_db.msg_queue_mutex);
					msg_queue.splice(msg_queue.end(), g_db.msg_queue);
				}

				// 可能是请空操作发送的信号
				if (msg_queue.empty())
				{
					fprintf(stderr,"\nquit debug\n\n");

					return;
				}
            }
            
            LNetPack* net_pack = msg_queue.front();
            msg_queue.pop_front();

			int ntop = lua_gettop(g_db.L);
            std::string command(net_pack->GetPackBody(), net_pack->GetPackBodySize());
			delete net_pack;
			lua_writestringerror("%s \n", command.c_str());

			g_db.ud = (void*)ar;
            if (!filter_debug_command(command,b_break))
            {				
                if(luaL_loadbuffer(g_db.L,command.c_str(),command.length(),"=lua debug") 
					|| lua_pcall(g_db.L,0,0,0))
				{
					lua_writestringerror("%s \n",lua_tostring(g_db.L,-1));
				}
            }
			g_db.ud = NULL;

			lua_settop(g_db.L,ntop);
			b_break ? fprintf(stderr,"%s ", "lua debug>") : 0;
        }
	
		{
			while(!msg_queue.empty())
			{
				LNetPack* unrelease_pack = msg_queue.front();
				msg_queue.pop_front();
				delete unrelease_pack;
			}
		}
    }



	bool filter_net_pack(LNetPack* net_pack)
    {
        std::string _parse(net_pack->GetPackBody(), net_pack->GetPackBodySize());

		int nbegin = 0;
        std::string cmd = parse_segment(_parse,nbegin);
        int i = 0;
        for (;  
			net_command_filter[i].command_name != NULL;
            ++i)
        {
			if(g_db.L == NULL) return true;
			
			if(net_command_filter[i].command_shortname == cmd || net_command_filter[i].command_name == cmd)
			{
				std::string _params = _parse.substr(nbegin);
				(*net_command_filter[i].handler)(_params,NULL);
				return true;
			}
        }

        return false;
    }

    bool parse_net_pack(ldb* l_db, int from,int &command_pack_count)
    {
        if (l_db->net_buffer_use_bytes - from <= sizeof(LPackHeader))
        {
            if (from > 0)
            {
                memmove_s(l_db->net_buffer, sizeof(l_db->net_buffer), l_db->net_buffer + from, l_db->net_buffer_use_bytes - from);
                l_db->net_buffer_use_bytes = l_db->net_buffer_use_bytes - from;
            }
            return from > 0 ? true : false;
        }

        LPackHeader* net_pack_header = (LPackHeader*)(l_db->net_buffer + from);
        if (net_pack_header->size > E_NET_PACK_SIZE)
        {
            return false;
        }

        if (net_pack_header->size > l_db->net_buffer_use_bytes - from)
        {
            if (from > 0)
            {
                memmove_s(l_db->net_buffer, sizeof(l_db->net_buffer), l_db->net_buffer + from, l_db->net_buffer_use_bytes - from);
                l_db->net_buffer_use_bytes = l_db->net_buffer_use_bytes - from;
            }
            return from > 0 ? true : false;
        }

        LNetPack* copy_net_pack = new LNetPack;
        memcpy_s(copy_net_pack, sizeof(LNetPack), net_pack_header, net_pack_header->size);
        copy_net_pack->data.netpack[net_pack_header->size] = '\0';  // 字符串结束标记
        from += net_pack_header->size;

        // 过滤掉特定的协议
        if(filter_net_pack(copy_net_pack))
        {
			delete copy_net_pack;
        }
        else
        {
			CAutoLock lock(l_db->msg_queue_mutex);
			l_db->msg_queue.push_back(copy_net_pack);
			++command_pack_count;
        }

        return parse_net_pack(l_db, from,command_pack_count);
    }
    unsigned int _stdcall worker_thread(void* p)
    {
        ldb* l_db = (ldb*)p;
        HANDLE pipe = (HANDLE)l_db->pipe_handle;               
        DWORD dwRecvBytes;

        while (!l_db->exit_worker_thread.load())
        {
            BOOL res = ConnectNamedPipe(pipe, NULL);
            if (!res && GetLastError() != ERROR_PIPE_CONNECTED)
            {
                Sleep(100);
                continue;
            }

            l_db->client_connected.store(true);
			
            l_db->net_buffer_use_bytes = 0;
            for (; !l_db->exit_worker_thread.load() && l_db->client_connected.load();)
            {
                BOOL bRes = ReadFile(pipe, l_db->net_buffer + l_db->net_buffer_use_bytes, E_NET_PACK_SIZE - l_db->net_buffer_use_bytes, &dwRecvBytes, NULL);
                if (!bRes && GetLastError() == ERROR_NO_DATA)
                {
					Sleep(10);
                    continue;
                }
                else if ((bRes && dwRecvBytes == 0) || !bRes)
                {                    
                    break;
                }

                l_db->net_buffer_use_bytes += (int)dwRecvBytes;

				int n_command_pack_count = 0;
                parse_net_pack(l_db, 0,n_command_pack_count);
				if(n_command_pack_count > 0)
                {
					SetEvent(l_db->msg_queue_event);
                }                
			}

			l_db->Init();

			// 退出时发送信号
			SetEvent(l_db->msg_queue_event);
            DisconnectNamedPipe(pipe);            

        }
        return 0;
    }

    int start(lua_State* L)
    {
		if(g_db.exit_worker_thread.load())
		{
			lua_pushboolean(L,1);
			lua_pushstring(L,"debug has already startup");
			return 2;
		}

        luaL_checktype(L, 1, LUA_TSTRING);
        const char* name = lua_tostring(L, 1);

        char pipeName[256] = "\\\\.\\pipe\\";
        memcpy_s(pipeName + strlen(pipeName), 256 - strlen(pipeName), name, strlen(name) + 1);
        pipeName[255] = '\0';

		HANDLE pipe = CreateNamedPipe(pipeName, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_NOWAIT | PIPE_REJECT_REMOTE_CLIENTS, 1, (DWORD)max_pipe_buffer_size, (DWORD)max_pipe_buffer_size, 0, NULL);
        if (pipe == INVALID_HANDLE_VALUE)
        {
            lua_pushboolean(L, 0);
            lua_pushstring(L, "create pipe failed");
            return 2;
        }

        g_db.pipe_handle = pipe;
        g_db.thread_handle = (HANDLE)_beginthreadex(NULL, 0, worker_thread, &g_db, 0, NULL);
        if (g_db.thread_handle == INVALID_HANDLE_VALUE)
        {
            CloseHandle(pipe);
            lua_pushboolean(L, 0);
            lua_pushstring(L, "create worker_thread failed");
            return 2;
        }
		
		g_db.L = L;

        lua_pushboolean(L, 1);
        lua_pushstring(L, "success");
        return 2;
    }

    int stop(lua_State *L)
    {
        g_db.exit_worker_thread.store(true);
        g_db.client_connected.store(false);

        WaitForSingleObject(g_db.thread_handle, 5000);

        CloseHandle(g_db.thread_handle);
	    CloseHandle(g_db.pipe_handle);
        
        lua_pushboolean(L, 1);
        return 1;
    }

    API_PIPE int luaopen_pipe(lua_State* L)
    {
        luaL_Reg reg[] =
        {
            { "start", &start},
            { "stop", &stop },
            {NULL,NULL},
        };

        luaL_newlib(L, reg);

        return 1;
    }
#ifdef __cplusplus
}
#endif
