#pragma once

#include <assert.h>
#include <cstdio>
#include <string>

#include "Processing.NDI.Lib.h"

class NdiManager
{
public:
	NdiManager(const char* srcName, const char* dstName);
	~NdiManager();

	NDIlib_recv_instance_t* getRecever();
	NDIlib_send_instance_t* getSender();

private:
	NDIlib_recv_instance_t m_recvInstance;
	NDIlib_send_instance_t m_sendInstance;
};