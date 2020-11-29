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

struct VideoFramePair
{
	size_t dataSize1;
	size_t dataSize2;
	NDIlib_video_frame_v2_t videoFrame1;
	NDIlib_video_frame_v2_t videoFrame2;
};

struct AudioFrame
{
	size_t dataSize;
	NDIlib_audio_frame_v2_t audioFrame;
};


class FrameSender
{
public:
	FrameSender(const char* host, in_port_t videoPort, in_port_t audioPort);
	~FrameSender();

	void SendVideoFrame(VideoFramePair frame, uint8_t* data);
	void SendAudioFrame(NDIlib_audio_frame_v2_t* ndi_frame);

	void WaitForConfirmation();

private:
	sockpp::tcp_connector m_videoConn;
	sockpp::tcp_connector m_audioConn;
};


