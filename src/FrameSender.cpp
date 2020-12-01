#include "FrameSender.h"

FrameSender::FrameSender(const char* host, in_port_t videoPort, in_port_t audioPort)
{
	sockpp::socket_initializer sockInit;

	m_videoConn.connect({host, videoPort});
	if (!m_videoConn) 
	{
		printf("Error creating video connection to to %s over port %u!\nError: %s.\n", host, videoPort, m_videoConn.last_error_str());
		assert(0);
	}

	m_videoConnAux.connect({ host, videoPort });
	if (!m_videoConnAux)
	{
		printf("Error creating video connection aux to to %s over port %u!\nError: %s.\n", host, videoPort, m_videoConnAux.last_error_str());
		assert(0);
	}

	std::thread aux([this] 
		{
			SendVideoFrameAux();
		});
	aux.detach();

	printf("Video connection created to: %s at port %u.\n", host, videoPort);

	m_audioConn.connect({ host, audioPort });
	if (!m_audioConn)
	{
		printf("Error creating audio connection to to %s over port %u!\nError: %s.\n", host, audioPort, m_audioConn.last_error_str());
		assert(0);
	}

	printf("Audio connection created to: %s at port %u.\n", host, audioPort);
}

FrameSender::~FrameSender()
{
	m_videoConn.close();
	m_videoConnAux.close();
	m_audioConn.close();
}


void FrameSender::SendVideoFrame(VideoFrame frame, uint8_t* data)
{
	processed = false;

	if (m_videoConn.write_n(&frame, sizeof(frame)) != sizeof(frame))
	{
		printf("Failed to write video frame details!\nError: %s\n", m_videoConn.last_error_str().c_str());
	}

	if (!frame.isSingle)
	{
		auxData = data;
		auxSize1 = frame.buf1;
		auxSize2 = frame.buf2;

		{
			std::lock_guard<std::mutex> lk(m);
			ready = true;
		}

		cv.notify_one();
	}


	if (m_videoConn.write_n(data, frame.buf1) != frame.buf1)
	{
		printf("Failed to write video data!\nError: %s\n", m_videoConn.last_error_str().c_str());
	}

	if (!frame.isSingle)
	{
		std::unique_lock<std::mutex> lk(m);
		cv.wait(lk, [this] {return processed; });
	}
}

void FrameSender::SendVideoFrameAux()
{
	while (true)
	{
		std::unique_lock<std::mutex> lk(m);
		cv.wait(lk, [this] {return ready; });

		if (m_videoConnAux.write_n(auxData + auxSize1, auxSize2) != auxSize2)
		{
			printf("THREAD 2: Failed to write video data!\nError: %s\n", m_videoConnAux.last_error_str().c_str());
		}

		processed = true;
		ready = false;

		lk.unlock();
		cv.notify_one();
	}
}

void FrameSender::SendAudioFrame(NDIlib_audio_frame_v2_t* ndi_frame)
{
	AudioFrame frame;
	frame.audioFrame = *ndi_frame;
	frame.dataSize = sizeof(float) * ndi_frame->no_samples * ndi_frame->no_channels;

	if (m_audioConn.write_n(&frame, sizeof(frame)) != sizeof(frame))
	{
		printf("Failed to write audio frame details!\nError: %s\n", m_audioConn.last_error_str());
	}

	if (m_audioConn.write_n(ndi_frame->p_data, frame.dataSize) != frame.dataSize)
	{
		printf("Failed to write audio data!\nError: %s\n", m_audioConn.last_error_str());
	}
}

void FrameSender::WaitForConfirmation()
{
	char c = 0;
	if (m_videoConn.read_n(&c, sizeof(c)) == -1)
	{
		printf("Failed to receve confirmation!\nError: %s\n", m_videoConn.last_error_str().c_str());
	}
}
