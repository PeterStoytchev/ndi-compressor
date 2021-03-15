#pragma once

#include <assert.h>
#include <future>

#include "Processing.NDI.Lib.h"

#include "sockpp/tcp_connector.h"

#include "Frame.h"


class FrameSender
{
public:
	FrameSender(const char* host, in_port_t videoPort);
	~FrameSender();

	void SendFrameBuffer(FrameBuffer* buffer);

	void WaitForConfirmation();
private:
	sockpp::tcp_connector m_conn;
};