#pragma once

#define __PROFILE

#ifdef __PROFILE
#include "optik/optick.h"
#define PROFILE_FUNC() OPTICK_EVENT()
#define PROFILE_FRAME(name) OPTICK_FRAME(name)
#define PROFILE_THREAD() OPTICK_THREAD()
#else
#define PROFILE_FUNC()
#define PROFILE_FRAME(name)
#define PROFILE_THREAD()
#endif