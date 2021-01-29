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


void FrameSender::SendVideoFrame(std::vector<VideoPkt>& frames)
{
	OPTICK_EVENT();

	//compute total buffer size
	size_t dataSize = FRAME_BATCH_SIZE * sizeof(VideoPkt);
	for (int i = 0; i < FRAME_BATCH_SIZE; i++) { dataSize += frames[i].frameSize; }

	m_globalFrameBuffer.GrowIfNeeded(dataSize); //grow the buffer if needed

	//copy the video pkt data into the buffer
	size_t localSize = 0;
	for (VideoPkt pkt : frames)
	{
		memcpy(m_globalFrameBuffer.m_buffer + localSize, &pkt, sizeof(VideoPkt));
		localSize += sizeof(VideoPkt);
	}

	//copy data into the buffer
	for (int i = 0; i < FRAME_BATCH_SIZE; i++)
	{
		memcpy(m_globalFrameBuffer.m_buffer + localSize, frames[i].encodedDataPacket->data, frames[i].encodedDataPacket->size);
		localSize += frames[i].encodedDataPacket->size;
	}

	//write the size of the buffer
	if (m_videoConn.write_n(&dataSize, sizeof(size_t)) != sizeof(size_t))
	{
		printf("Failed to write video frame size!\nError: %s\n", m_videoConn.last_error_str().c_str());
	}

	//write the buffer
	if (m_videoConn.write_n(m_globalFrameBuffer.m_buffer, dataSize) != dataSize)
	{
		printf("Failed to write video data!\nError: %s\n", m_videoConn.last_error_str().c_str());
	}
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
