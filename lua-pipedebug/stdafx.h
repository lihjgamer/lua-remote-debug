// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <process.h>
#include <string>
#include <list>

#include "util.h"
#include "lock.h"

#include "lua.hpp"


#include "pipedef.h"
//#include "commandhandler.h"

#define EXPORT_API_PIPE

#ifdef EXPORT_API_PIPE
#define API_PIPE __declspec(dllexport)
#else
#define API_PIPE __declspec(dllimport)
#endif


// TODO: reference additional headers your program requires here
