#pragma once

#define __PROFILE

#ifdef __PROFILE
#include "optik/optick.h"
#else
#define OPTICK_EVENT(name)
#define OPTICK_FRAME(name)
#define OPTICK_THREAD(name)
#endif