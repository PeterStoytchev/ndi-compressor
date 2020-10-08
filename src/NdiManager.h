#pragma once

#include <assert.h>
#include <iostream>

#include "Processing.NDI.Lib.h"

class NdiManager
{
public:
	NdiManager(const char* srcName, const char* dstName);
private:
	NDIlib_recv_instance_t m_recvInstance;
};