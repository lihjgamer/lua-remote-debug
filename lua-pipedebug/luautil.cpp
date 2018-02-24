#include "stdafx.h"
#include "luautil.h"

int  search_local_var(lua_State* L,lua_Debug* ar,const char* var)
{
	const char* name;
	for (int i = 1; (name = lua_getlocal(L,ar,i)) != NULL; ++i)
	{
		if(strcmp(name,var) == 0)
		{
			return i;
		}

		lua_pop(L,1);
	}

	return 0;
}

int  search_upval_var(lua_State*L,lua_Debug* ar,const char* var)
{

	return 0;
}

int  search_global_var(lua_State* L,lua_Debug* ar,const char* var)
{
	lua_getglobal(L,var);
	if(lua_type(L,-1) == LUA_TNIL)
	{
		lua_pop(L,1);
		return 0;
	}
	return 1;
}

void print_var(lua_State* L,int ci,int index,int &depth)
{
	switch(lua_type(L,index))
	{
	case LUA_TNIL:
		fprintf(stdout,"%s","nil");
		break;
	case LUA_TNUMBER:
		{
			if (lua_isinteger(L,index))
				fprintf(stdout,"%d",lua_tointeger(L,index));
			else
				fprintf(stdout,"%f",lua_tonumber(L,index));

		}
		break;
	case LUA_TSTRING:
		fprintf(stdout,"\"%s\"",lua_tostring(L,index));
		break;		
	case LUA_TBOOLEAN:
		fprintf(stdout,"%s",lua_toboolean(L,index)?"true":"false");
		break;
	case LUA_TTABLE:
		print_table(L,0,index,depth);
		break;
	case LUA_TFUNCTION:
		{
			lua_CFunction func = lua_tocfunction(L, index);
			if( func != NULL ) 
			{
				fprintf(stderr,"(C function)0x%p", func);
			}
			else 
			{
				fprintf(stderr,"(function)");
			}
		}
		break;
	case LUA_TUSERDATA:
		fprintf(stderr,"(user data)0x%p", lua_touserdata(L, index));
		break;
	default:
		fprintf(stdout,"%s",lua_typename(L,index));
	}

}

void print_table_key( lua_State* L,int ci,int index )
{
	int ncounter = 0;
	switch(lua_type(L,index))
	{
	case LUA_TTABLE:
		fprintf(stderr,"(table)");
		break;
	default:
		print_var(L,ci,index,ncounter);
		break;
	}
}

void print_table( lua_State* L,int ci,int index ,int &depth)
{	
	++depth;
	if (depth >= 5)
	{
		return;
	}

	fprintf(stderr,"{");
	index = lua_absindex(L,index);
	int ntop = lua_gettop(L);
	lua_pushnil(L);
	bool bstart = true;
	while (lua_next(L,index))
	{
		if (!bstart)
			fprintf(stderr,",");
		else
			bstart = false;

		print_table_key(L,-1,-2);
		fprintf(stderr,":");
		print_var(L,-1,-1,depth);

		lua_pop(L,1);
	}
	lua_settop(L,ntop);
	--depth;

	fprintf(stderr,"}");
}

