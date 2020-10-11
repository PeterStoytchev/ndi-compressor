#pragma once

#include <assert.h>

#include "Processing.NDI.Lib.h"

#ifdef _DEBUG
#pragma comment(lib, "sockpp-debug")
#endif // _DEBUG

#ifndef _DEBUG
#pragma comment(lib, "sockpp-release")
#endif

#include "sockpp/tcp_connector.h"
#include "sockpp/version.h"

class FrameSender
{
public:
	FrameSender(const char* host, in_port_t videoPort, in_port_t audioPort);
	~FrameSender();

	void SendVideoFrame(NDIlib_video_frame_v2_t* ndi_frame, uint8_t* data, size_t dataSize);
	void SendAudioFrame(NDIlib_audio_frame_v2_t* ndi_frame);

	void WaitForConfirmation();

private:
	sockpp::tcp_connector m_videoConn;
	sockpp::tcp_connector m_audioConn;
};


struct VideoFrame
{
	size_t dataSize;
	NDIlib_video_frame_v2_t videoFrame;
};

struct AudioFrame
{
	size_t dataSize;
	NDIlib_audio_frame_v2_t audioFrame;
};