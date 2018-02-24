#include "LuaTest.h"
#include "lua.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
CLuaTest::CLuaTest(void)
{
}

CLuaTest::~CLuaTest(void)
{
}

void CLuaTest::Initialize()
{
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	
	int nret = luaL_dofile(L,"main.lua");
	if (nret != LUA_OK)
	{
		fprintf(stderr,"lua error %s\n",lua_tostring(L,-1));
		lua_close(L);
		return;
	}

	while (true)
	{
		lua_getglobal(L,"update");
		lua_pcall(L,0,0,0);
		Sleep(100);
	}

	lua_close(L);
}

void CLuaTest::Test()
{
	CLuaTest instance;
	instance.Initialize();
}
