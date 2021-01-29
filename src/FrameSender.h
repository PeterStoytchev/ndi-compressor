#pragma once

#include <assert.h>
#include <future>

#include "Processing.NDI.Lib.h"

#ifdef _DEBUG
#pragma comment(lib, "sockpp-debug")
#endif // _DEBUG

#ifndef _DEBUG
#pragma comment(lib, "sockpp-release")
#endif

#include "sockpp/tcp_connector.h"
#include "sockpp/version.h"

#include "Frame.h"

class FrameSender
{
public:
	FrameSender(const char* host, in_port_t videoPort, in_port_t audioPort);
	~FrameSender();

	void SendVideoFrame(std::vector<VideoPkt>& frames);
	void SendAudioFrame(NDIlib_audio_frame_v2_t* ndi_frame);

	void WaitForConfirmation();

private:
	size_t m_maxFrameBufferSize = 0;
	uint8_t* m_globalFrameBuffer = NULL;

	sockpp::tcp_connector m_videoConn;
	sockpp::tcp_connector m_videoConnAux;
	sockpp::tcp_connector m_audioConn;
};