#include "FrameSender.h"
#include "Profiler.h"

FrameSender::FrameSender(const char* host, in_port_t videoPort, in_port_t audioPort)
{
	sockpp::socket_initializer sockInit;

	m_videoConn.connect({host, videoPort});
	if (!m_videoConn) 
	{
		printf("Error creating video connection to to %s over port %u!\nError: %s.\n", host, videoPort, m_videoConn.last_error_str().c_str());
		assert(0);
	}

	printf("Video connection created to: %s at port %u.\n", host, videoPort);

	m_audioConn.connect({ host, audioPort });
	if (!m_audioConn)
	{
		printf("Error creating audio connection to to %s over port %u!\nError: %s.\n", host, audioPort, m_audioConn.last_error_str().c_str());
		assert(0);
	}

	printf("Audio connection created to: %s at port %u.\n", host, audioPort);
}

FrameSender::~FrameSender()
{
	m_videoConn.close();
	m_audioConn.close();
}


void FrameSender::SendVideoFrame(VideoPkt* frame)
{
	PROFILE_FUNC();
	
	size_t dataSize = 0;
	size_t videoPktNetSize = offsetof(VideoPkt, encodedDataPackets);
	size_t totalSize = dataSize + videoPktNetSize;

	for (int i = 0; i < 30; i++) { dataSize += frame->frameSizes[i]; }

	//TODO: make this a global buffer so that we dont have to allocate memory every time
	uint8_t* frameData = (uint8_t*)malloc(totalSize);
	
	memcpy(frameData, frame, videoPktNetSize);

	for (int i = 0; i < 30; i++)
	{
		memcpy(frameData + videoPktNetSize, frame->encodedDataPackets[i]->data, frame->encodedDataPackets[i]->size);
		videoPktNetSize += frame->encodedDataPackets[i]->size;
	}

	if (m_videoConn.write_n(&totalSize, sizeof(size_t)) != sizeof(size_t))
	{
		printf("Failed to write video frame size!\nError: %s\n", m_videoConn.last_error_str().c_str());
	}

	if (m_videoConn.write_n(frameData, totalSize) != totalSize)
	{
		printf("Failed to write video data!\nError: %s\n", m_videoConn.last_error_str().c_str());
	}

	free(frameData);
}

void FrameSender::SendAudioFrame(NDIlib_audio_frame_v2_t* ndi_frame)
{
	PROFILE_FUNC();

	AudioFrame frame;
	frame.audioFrame = *ndi_frame;
	frame.dataSize = sizeof(float) * ndi_frame->no_samples * ndi_frame->no_channels;

	if (m_audioConn.write_n(&frame, sizeof(frame)) != sizeof(frame))
	{
		printf("Failed to write audio frame details!\nError: %s\n", m_audioConn.last_error_str().c_str());
	}

	if (m_audioConn.write_n(ndi_frame->p_data, frame.dataSize) != frame.dataSize)
	{
		printf("Failed to write audio data!\nError: %s\n", m_audioConn.last_error_str().c_str());
	}
}

void FrameSender::WaitForConfirmation()
{
	PROFILE_FUNC();
	
	char c = 0;
	if (m_videoConn.read_n(&c, sizeof(c)) == -1)
	{
		printf("Failed to receve confirmation!\nError: %s\n", m_videoConn.last_error_str().c_str());
	}
}
