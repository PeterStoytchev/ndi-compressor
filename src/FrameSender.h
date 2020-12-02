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

struct VideoFrame
{
	size_t buf1 = 0;
	size_t buf2 = 0;
	bool isSingle = false;
	NDIlib_video_frame_v2_t videoFrame;
	std::chrono::time_point<std::chrono::steady_clock> frameStart;
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

	void SendVideoFrame(VideoFrame frame, uint8_t* data);
	void SendVideoFrameAux();
	void SendAudioFrame(NDIlib_audio_frame_v2_t* ndi_frame);

	void WaitForConfirmation();

private:
	sockpp::tcp_connector m_videoConn;
	sockpp::tcp_connector m_videoConnAux;
	sockpp::tcp_connector m_audioConn;

	uint8_t* auxData;
	size_t auxSize1;
	size_t auxSize2;

	std::mutex m;
	std::condition_variable cv;
	bool ready = false;
	bool processed = false;
};


