#pragma once

// 返回值表示是否 继续执行交互
typedef int (*command_processor)(std::string &command,void* ud);

struct command_handler
{
	const char* command_name;
	const char* command_shortname;
	command_processor handler;
	const char* helper;
};

// debug 
int t_step(std::string& params,void* ud = NULL);
int t_continue(std::string& params,void* ud = NULL);
int t_print(std::string& params,void* ud = NULL);
int t_help(std::string& params,void* ud = NULL);

extern command_handler debug_command_filter[];

// net
int t_break(std::string &params,void* ud = NULL);
int t_delete_break(std::string &params,void* ud = NULL);
int t_clear(std::string &params,void* ud = NULL);

extern command_handler net_command_filter[];
