#pragma once

#include <queue>
#include <mutex>
#include <atomic>
#include <thread>

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

	void HandleNdi();
	void HandleSending();
	void HandleEncoding();

private:
	Encoder* m_encoder;
	NdiManager* m_ndiManager;
	FrameSender* m_frameSender;

	std::thread ndiHandler;
	std::thread sendingHandler;
	std::thread encodingHandler;
	
	std::queue<VideoFrame> m_ndiQueue;
	std::queue<VideoFrame> m_encodingQueue;

	std::mutex m_ndiMutex;
	std::mutex m_encodingMutex;

	std::atomic<bool> m_exit = false;

	uint8_t* bsBuffer = (uint8_t*)malloc(2);

	uint64_t lastid = 0;
};