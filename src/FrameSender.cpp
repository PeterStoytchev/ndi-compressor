#include "FrameSender.h"
#include "Profiler.h"

FrameSender::FrameSender(const char* host, in_port_t videoPort)
{
	sockpp::socket_initializer sockInit;

	m_conn.connect({host, videoPort});
	if (!m_conn) 
	{
		printf("Error creating video connection to to %s over port %u!\nError: %s.\n", host, videoPort, m_conn.last_error_str().c_str());
		assert(0);
	}

	printf("Connection created to: %s at port %u.\n", host, videoPort);
}

FrameSender::~FrameSender()
{
	m_conn.close();
}

void FrameSender::SendFrameBuffer(FrameBuffer* buffer)
{
	OPTICK_EVENT();

	buffer->PackData();

	//write buffer size
	if (m_conn.write_n(&(buffer->totalDataSize), sizeof(size_t)) != sizeof(size_t))
	{
		printf("Failed to write video frame details!\nError: %s\n", m_conn.last_error_str().c_str());
	}
	
	//write buffer data
	if (m_conn.write_n(buffer->packedData, buffer->totalDataSize) != buffer->totalDataSize)
	{
		printf("Failed to write video data!\nError: %s\n", m_conn.last_error_str().c_str());
	}
}

void FrameSender::WaitForConfirmation()
{
	OPTICK_EVENT();
	
	char c = 0;
	if (m_conn.read_n(&c, sizeof(c)) == -1)
	{
		printf("Failed to receve confirmation!\nError: %s\n", m_conn.last_error_str().c_str());
	}
}