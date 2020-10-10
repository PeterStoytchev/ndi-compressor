#pragma once

#include <assert.h>
#include <cstdio>
#include <string>
#include <tuple>

#include "Processing.NDI.Lib.h"

class NdiManager
{
public:
	NdiManager(const char* srcName, const char* dstName);
	~NdiManager();

	void SendVideo(const NDIlib_video_frame_v2_t* p_video_data);
	void SendAudio(const NDIlib_audio_frame_v2_t* p_audio_data);

	void FreeVideo(const NDIlib_video_frame_v2_t* p_video_data);
	void FreeAudio(const NDIlib_audio_frame_v2_t* p_audio_data);

	void SendAndFreeVideo(NDIlib_video_frame_v2_t* p_video_data);
	void SendAndFreeAudio(const NDIlib_audio_frame_v2_t* p_audio_data);

	NDIlib_video_frame_v2_t* CaptureVideoFrame();
	NDIlib_audio_frame_v2_t* CaptureAudioFrame();

private:
	NDIlib_recv_instance_t m_recvInstance = NULL;
	NDIlib_send_instance_t m_sendInstance = NULL;
};