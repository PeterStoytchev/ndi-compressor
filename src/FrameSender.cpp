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

	printf("Video connections created to: %s at port %u.\n", host, videoPort);

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


void FrameSender::SendVideoFrame(FrameBuffer* buffer)
{
	OPTICK_EVENT();

	//compute total buffer size
	size_t dataSize = 0;
	for (int i = 0; i < FRAME_BATCH_SIZE; i++) { dataSize += buffer->encodedFrameSizes[i]; }
	buffer->totalDataSize = dataSize;

	uint8_t* frameData = (uint8_t*)malloc(dataSize); //data buffer used to hold all frame data

	size_t localSize = 0;
	//copy data into the buffer
	for (int i = 0; i < FRAME_BATCH_SIZE; i++)
	{
		memcpy(frameData + localSize, buffer->encodedFramePtrs[i]->data, buffer->encodedFrameSizes[i]);
		localSize += buffer->encodedFrameSizes[i];
	}

	//write buffer details
	if (m_videoConn.write_n(buffer, sizeof(FrameBuffer)) != sizeof(FrameBuffer))
	{
		printf("Failed to write video frame details!\nError: %s\n", m_videoConn.last_error_str().c_str());
	}
	
	//write buffer data
	if (m_videoConn.write_n(frameData, dataSize) != dataSize)
	{
		printf("Failed to write video data!\nError: %s\n", m_videoConn.last_error_str().c_str());
	}

	free(frameData);
}

void FrameSender::SendAudioFrame(NDIlib_audio_frame_v2_t* ndi_frame)
{
	OPTICK_EVENT();

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
	OPTICK_EVENT();
	
	char c = 0;
	if (m_videoConn.read_n(&c, sizeof(c)) == -1)
	{
		printf("Failed to receve confirmation!\nError: %s\n", m_videoConn.last_error_str().c_str());
	}
}