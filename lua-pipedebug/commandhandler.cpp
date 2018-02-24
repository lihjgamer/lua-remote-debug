#include "StdAfx.h"
#include "commandhandler.h"
#include "luautil.h"

command_handler debug_command_filter[] =
{
	{"help"		,"h"		,&t_help		,"help     | h "},
	{"step"		,"s"		,&t_step		,"step     | s"},
	{"continue"	,"c"		,&t_continue	,"continue | c "},
	{"print"	,"p"		,&t_print		,"print ...| p ..."},
	{ NULL, NULL, NULL, NULL }
};

// ÍøÂç¹ýÂËµÄÃüÁî
command_handler net_command_filter[] =
{ 
	{ "break"	,"b"	,&t_break			,"break function | b function | break file:line | b file:line" },
	{ "delete"	,"d"	,&t_delete_break	,"delete id      | d id" },
	{ "clear"	,"c"	,&t_clear			,"clear          | c" },
	{ NULL, NULL, NULL, NULL }
};

int t_step(std::string& params,void* ud)
{
	ldb* l_db = (ldb*)ud;
	lua_State* L = l_db->L;
	lua_Debug* ar = (lua_Debug*)l_db->ud;

	l_db->step_break_flag = true;

	lua_sethook(L,lua_gethook(L),lua_gethookmask(L)|LUA_MASKLINE,0);						

	return E_DO_BREAK;
}

int t_continue(std::string& params,void* ud)
{
	ldb* l_db = (ldb*)ud;
	lua_State* L = l_db->L;
	lua_Debug* ar = (lua_Debug*)l_db->ud;

	l_db->step_break_flag = false;

	return E_DO_BREAK;
}

int t_print(std::string& params,void* ud)
{
	ldb* l_db = (ldb*)ud;
	if(l_db == NULL) return E_DO_CONTINUE;
	lua_Debug* ar = (lua_Debug*)l_db->ud;
	int beginpos = 0;

	std::string var = parse_segment(params,beginpos);
	while (!var.empty())
	{
		if (search_local_var(l_db->L,ar,var.c_str()))
		{
			int ncounter = 0;
			print_var(l_db->L,-1,-1,ncounter);
			lua_pop(l_db->L,1);
			fprintf(stderr,"\n");
		}
		else if(search_global_var(l_db->L,NULL,var.c_str()))
		{
			int ncounter = 0;
			print_var(l_db->L,-1,-1,ncounter);
			lua_pop(l_db->L,1);

			fprintf(stderr,"\n");
		}
		else
		{
			fprintf(stdout,"not find value : %s\n",var.c_str());	
		}

		if(beginpos >= params.length())
			break;

		var = parse_segment(params,beginpos);
	}

	return E_DO_CONTINUE;
}

int t_help(std::string& params,void* ud)
{
	for(int i = 0; net_command_filter[i].command_name != NULL; ++i)
	{
		fprintf(stderr,"%s\n",net_command_filter[i].helper);
	}

	for(int i = 0; debug_command_filter[i].command_name != NULL; ++i)
	{
		fprintf(stderr,"%s\n",debug_command_filter[i].helper);
	}

	return E_DO_CONTINUE;
}

int t_break(std::string &params,void* ud)
{
	int nbegin = 0;
	std::string breakinfo = parse_segment(params,nbegin);
	if (breakinfo.empty())
	{
		return E_DO_NONE;
	}

	std::string::size_type pos = breakinfo.find(':');

	std::string name;
	int         line;
	int			mask;
	// function
	if (pos == std::string::npos)
	{
		name = breakinfo;
		line = -1;			
		mask = LUA_MASKCALL;
	}
	else
	{
		name = breakinfo.substr(0, pos);
		try{
			line = atoi(breakinfo.substr(pos + 1).c_str());
			mask = LUA_MASKLINE;
		}
		catch (...)
		{
			line = -1;
		}
	}

	if (pos != std::string::npos && line <= 0)
	{
		return E_DO_NONE;
	}

	l_break* new_break = new l_break;
	new_break->id = g_db.generator_break_id++;
	new_break->name = name;
	new_break->line = line;

	{
		CAutoLock lock(g_db.break_mutex);
		new_break->next = g_db.breaks;
		g_db.breaks = new_break;
	}

	lua_sethook(g_db.L, lua_debughook, lua_gethookmask(g_db.L) | mask, 0);

	return E_DO_NONE;
}

int t_delete_break(std::string &params,void* ud)
{
	int nbegin = 0;
	std::string breakid = parse_segment(params,nbegin);
	if(breakid.empty())	return E_DO_NONE;

	try{

		int id = atoi(breakid.c_str());

		CAutoLock lock(g_db.break_mutex);
		l_break** slow_pointer = &g_db.breaks;
		l_break* fast_pointer = g_db.breaks;
		for (; fast_pointer != NULL;fast_pointer = fast_pointer->next)
		{
			if (fast_pointer->id == id)
			{
				*slow_pointer = fast_pointer->next;
				delete fast_pointer;
				break;
			}

			slow_pointer = &fast_pointer->next;
		}

		if (g_db.breaks == NULL)
		{
			SetEvent(g_db.msg_queue_event);
		}
	}
	catch (...)
	{
		// do nothing
	}

	return E_DO_NONE;
}

int t_clear(std::string &params,void* ud)
{
	(void)params;

	l_break* fast_pointer = NULL;
	l_break* delete_pointer = NULL;
	{
		CAutoLock lock(g_db.break_mutex);
		fast_pointer = g_db.breaks;
		g_db.breaks = NULL;
	}

	lua_sethook(g_db.L, NULL, 0, 0);
	SetEvent(g_db.msg_queue_event);

	for (; fast_pointer != NULL; )
	{
		delete_pointer = fast_pointer;
		fast_pointer = fast_pointer->next;
		delete delete_pointer;
		delete_pointer = NULL;
	}

	return E_DO_NONE;
}
