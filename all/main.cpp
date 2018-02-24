#include <stdio.h>
#include <stdlib.h>

#include "LuaTest.h"
// #define WIN32_LEAN_AND_MEAN
// #include <Windows.h>
// #include <winsock2.h>
// #pragma comment(lib,"ws2_32.lib")


#define TESTCASE(C,FUNC,Enable)	\
	{\
		if(Enable)\
		{\
			C::FUNC();\
		}\
	}

int main()
{
	TESTCASE(CLuaTest,Test,true);
	return 0;
}
