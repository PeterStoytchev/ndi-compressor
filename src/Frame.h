#pragma once

#include <libavformat/avformat.h>

#include <assert.h>

#define FRAME_BATCH_SIZE 30
#define FRAME_BATCH_SIZE_AUDIO 24

struct FrameBuffer
{
	size_t totalDataSize = 0;

	size_t encodedVideoSizes[FRAME_BATCH_SIZE];
	AVPacket* encodedVideoPtrs[FRAME_BATCH_SIZE];
	NDIlib_video_frame_v2_t ndiVideoFrames[FRAME_BATCH_SIZE];

	size_t totalAudioSize = 0;
	NDIlib_audio_frame_v2_t ndiAudioFrames[FRAME_BATCH_SIZE_AUDIO];

	uint8_t* packedData = nullptr;

	void PackData()
	{
		//compute total buffer size
		totalDataSize = sizeof(FrameBuffer);
		for (int i = 0; i < FRAME_BATCH_SIZE; i++) 
		{ 
			assert(encodedVideoSizes[i] != 666);
			totalDataSize += encodedVideoSizes[i];
		}

		for (int i = 0; i < FRAME_BATCH_SIZE_AUDIO; i++)
		{
			size_t audioFrameSize = sizeof(float) * ndiAudioFrames[i].no_samples * ndiAudioFrames[i].no_channels;
			totalDataSize += audioFrameSize;
		}

		packedData = (uint8_t*)malloc(totalDataSize); //data buffer used to hold all frame data
		memcpy(packedData, (uint8_t*)&totalDataSize, sizeof(FrameBuffer));

		size_t localSize = sizeof(FrameBuffer);

		//copy video data into the buffer
		for (int i = 0; i < FRAME_BATCH_SIZE; i++)
		{
			memcpy(packedData + localSize, encodedVideoPtrs[i]->data, encodedVideoSizes[i]);
			localSize += encodedVideoSizes[i];
		}

		//copy audio data into the buffer
		for (int i = 0; i < FRAME_BATCH_SIZE_AUDIO; i++)
		{
			size_t frameSize = sizeof(float) * ndiAudioFrames[i].no_samples * ndiAudioFrames[i].no_channels;
			memcpy(packedData + localSize, ndiAudioFrames[i].p_data, frameSize);
			localSize += frameSize;
		}
	}

};