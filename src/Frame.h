#pragma once

#include <libavformat/avformat.h>

#define FRAME_BATCH_SIZE 30

struct FrameBuffer
{
	size_t totalDataSize = 0;

	size_t encodedVideoSizes[FRAME_BATCH_SIZE];
	AVPacket* encodedVideoPtrs[FRAME_BATCH_SIZE];
	NDIlib_video_frame_v2_t ndiVideoFrames[FRAME_BATCH_SIZE];

	size_t totalAudioSize = 0;
	NDIlib_audio_frame_v2_t ndiAudioFrames[FRAME_BATCH_SIZE];

	uint8_t* packedData = nullptr;

	void PackData()
	{
		//compute total buffer size
		totalDataSize = sizeof(FrameBuffer);
		for (int i = 0; i < FRAME_BATCH_SIZE; i++) 
		{ 
			totalDataSize += encodedVideoSizes[i];

			size_t audioFrameSize = sizeof(float) * ndiAudioFrames[i].no_samples * ndiAudioFrames[i].no_channels;
			totalDataSize += audioFrameSize;
		}

		packedData = (uint8_t*)malloc(totalDataSize); //data buffer used to hold all frame data
		memcpy(packedData, (uint8_t*)&totalDataSize, sizeof(FrameBuffer));

		size_t localSize = sizeof(FrameBuffer);

		//copy data into the buffer
		for (int i = 0; i < FRAME_BATCH_SIZE; i++)
		{
			memcpy(packedData + localSize, encodedVideoPtrs[i]->data, encodedVideoSizes[i]);
			localSize += encodedVideoSizes[i];
		}

		for (int i = 0; i < FRAME_BATCH_SIZE; i++)
		{
			size_t frameSize = sizeof(float) * ndiAudioFrames[i].no_samples * ndiAudioFrames[i].no_channels;
			memcpy(packedData + localSize, ndiAudioFrames[i].p_data, frameSize);
			localSize += frameSize;
		}
	}

};