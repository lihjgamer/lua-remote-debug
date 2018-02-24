#pragma once


const int max_pipe_buffer_size = 4096;

enum E_COMMAND_HANDLER_RET
{
	E_DO_NONE,
	E_DO_CONTINUE,
	E_DO_BREAK,
};

enum E_NET_CONFIG
{
	E_NET_PACK_SIZE   = 512,
	E_NET_BUFFER_SIZE = 4096,
};


struct LPack{
	char buf[E_NET_PACK_SIZE + 1];
};

struct LPackHeader
{
	int size;
	int version;

	int GetSize() const
	{
		return size;
	}

	const char* GetMPack() const
	{
		return (const char*)(this + 1);
	}

	int GetMPackSize() const
	{
		return size - sizeof(*this);
	}
};

struct LNetPack
{
	enum {
		E_NET_HEADER_SIZE = sizeof(LPackHeader),
	};

	union InnerStruct
	{
		// 最后一个字符做字符串结束标记
		char netpack[E_NET_PACK_SIZE + 1];
		LPackHeader netheader;
	}data;


	int GetPackSize() const
	{
		return data.netheader.size;
	}

	const char* GetPackBody() const
	{
		return (const char*)(data.netpack + E_NET_HEADER_SIZE);
	}

	int GetPackBodySize() const
	{
		return data.netheader.size - E_NET_HEADER_SIZE;
	}

	const char operator[](int bodyindex)
	{
		return data.netpack[E_NET_HEADER_SIZE + bodyindex];
	}
};

struct l_break 
{
	l_break*    next;
	int         id;
	int         line;
	std::string name;

	l_break()
		:next(NULL)
		, id(-1)
		, line(-1)
	{
	}
};

struct ldb 
{
	HANDLE			pipe_handle;
	HANDLE			thread_handle;
	lua_State		*L;

	CAtomic<bool>	exit_worker_thread;
	CAtomic<bool>	client_connected;

	int						generator_break_id;
	CCS						break_mutex;
	l_break*				breaks;
	bool					step_break_flag;

	int						net_buffer_use_bytes;
	char					net_buffer[E_NET_BUFFER_SIZE];


	HANDLE					msg_queue_event;
	CCS						msg_queue_mutex;
	std::list<LNetPack*>	msg_queue;

	void*					ud;

	ldb()
		: pipe_handle(NULL)
		, thread_handle(NULL)
		, L(NULL)
		, exit_worker_thread(false)
		, client_connected(false)
		, breaks(NULL)
		, net_buffer_use_bytes(0)
		, generator_break_id(0)
		, ud(NULL)
		, step_break_flag(false)
	{
		msg_queue_event = CreateEvent(NULL,TRUE,FALSE,NULL);
	}

	void Init()
	{
		net_buffer_use_bytes = 0;

		client_connected.store(false);
		ResetEvent(msg_queue_event);

		l_break* unrelease_breaks = NULL;
		{
			CAutoLock lock(break_mutex);
			unrelease_breaks = breaks;
			breaks = NULL;
		}

		while(unrelease_breaks)
		{
			l_break* be_freed = unrelease_breaks;
			unrelease_breaks = unrelease_breaks->next;
			delete be_freed;
		}

		std::list<LNetPack*> unrelease_packs;
		{
			CAutoLock lock(msg_queue_mutex);
			unrelease_packs.swap(msg_queue);
		}

		for (;!unrelease_packs.empty();)
		{
			LNetPack* be_freed_pack = unrelease_packs.front();
			unrelease_packs.pop_front();
			delete be_freed_pack;
		}
	}
};

extern ldb g_db;

#ifdef __cplusplus
extern "C"{
#endif
	// lua_hook 
	extern void lua_debughook(lua_State *L,lua_Debug *ar);
#ifdef __cplusplus
};
#endif
