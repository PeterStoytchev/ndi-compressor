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

struct FrameBuffer
{
	bool GrowIfNeeded(size_t potentialNewSize)
	{
		if (m_peakSize < potentialNewSize)
		{
			printf("[DebugLog][FrameBuffer] Increasing frame buffer size with id %i from %llu to %llu\n", m_id, m_peakSize, potentialNewSize);

			m_peakSize = potentialNewSize;
			m_buffer = (uint8_t*)realloc(m_buffer, m_peakSize);

			assert(m_buffer != nullptr, "[DebugLog][FrameBuffer] Failed to allocate more memory, probabbly becasue the system is out of RAM!");

			return true;
		}

		return false;
	}

	uint8_t* m_buffer = NULL;

private:
	size_t m_peakSize = 0;
	int m_id = rand();
};


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
	FrameBuffer m_globalFrameBuffer;

	sockpp::tcp_connector m_videoConn;
	sockpp::tcp_connector m_videoConnAux;
	sockpp::tcp_connector m_audioConn;
};