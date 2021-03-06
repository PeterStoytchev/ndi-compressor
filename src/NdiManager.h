#pragma once

#include <assert.h>
#include <cstdio>
#include <string>
#include <tuple>

#include "Processing.NDI.Lib.h"

#include "Profiler.h"

class NdiManager
{
public:
	NdiManager(const char* srcName, const char* dstName);
	~NdiManager();

	void FreeVideo(const NDIlib_video_frame_v2_t* p_video_data);
	void FreeAudio(const NDIlib_audio_frame_v2_t* p_audio_data);

	NDIlib_video_frame_v2_t CaptureVideoFrame();
	NDIlib_audio_frame_v2_t CaptureAudioFrame();

private:
	NDIlib_recv_instance_t m_recvInstance = NULL;
	NDIlib_send_instance_t m_sendInstance = NULL;
};