#pragma once

#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>

#include "Processing.NDI.Lib.h"

#include "Encoder.h"
#include "Frame.h"
#include "NdiManager.h"
#include "FrameSender.h"

class FrameWrangler
{
public:
	FrameWrangler(NdiManager* ndiManager, FrameSender* frameSender, EncoderSettings encSettings);
	~FrameWrangler();

	void Stop();

	void Ndi();
	void Main();

private:
	Encoder* m_encoder;
	NdiManager* m_ndiManager;
	FrameSender* m_frameSender;
	
	std::vector<NDIlib_video_frame_v2_t*> m_frameQueue;
	std::vector<NDIlib_video_frame_v2_t*> m_encodingQueue;
	std::vector<VideoPkt> m_sendingQueue;

	std::mutex m_ndiMutex;

	std::thread ndiHandler;
	std::thread mainHandler;

	bool m_isLocked = false;
	std::atomic<bool> m_exit = false;
};