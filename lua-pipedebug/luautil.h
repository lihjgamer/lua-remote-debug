#pragma once

struct lua_State;
struct lua_Debug;

void print_var(lua_State* L,int ci,int index,int &counter);

void print_table_key(lua_State* L,int ci,int index);
void print_table(lua_State* L,int ci,int index,int &counter);

int  search_local_var(lua_State* L,lua_Debug* ar,const char* var);
int  search_upval_var(lua_State*L,lua_Debug* ar,const char* var);
int  search_global_var(lua_State* L,lua_Debug* ar,const char* var);

