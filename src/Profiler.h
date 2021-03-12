#pragma once

//profiling macro
#ifdef __PROFILE
#include "../optik/optick.h"
#else
#define OPTICK_EVENT(name)
#define OPTICK_FRAME(name)
#define OPTICK_THREAD(name)
#endif


//this is just a debugging macro that wraps around printf, nothin fancy
#ifdef __DEBUGLOG
#define DEBUG_LOG(x) printf(x);
#else
#define DEBUG_LOG(x)
#endif