#include "Encoder.h"

#include <assert.h>

Encoder::Encoder(EncoderSettings settings)
{
	errorBuf = (char*)malloc(500);

    m_settings = settings;

	frame = av_frame_alloc();
	pkt = av_packet_alloc();

	codec = avcodec_find_encoder_by_name(settings.encoderName.c_str());
	
	codecContext = avcodec_alloc_context3(codec);

	codecContext->width = m_settings.xres;
	codecContext->bit_rate = m_settings.bitrateMbps * 1000000;
	codecContext->height = m_settings.yres;
	codecContext->time_base = { 1, m_settings.fps };
	codecContext->gop_size = m_settings.gop_size;
	codecContext->pix_fmt = m_settings.pix_fmt;
	codecContext->max_b_frames = m_settings.max_b_frames;

	codecContext->slices = settings.thread_count;
	codecContext->thread_count = settings.thread_count;


	swsContext = sws_getContext(m_settings.xres, m_settings.yres, AV_PIX_FMT_UYVY422, m_settings.xres, m_settings.yres, m_settings.pix_fmt, SWS_POINT | SWS_BITEXACT, 0, 0, 0);

	frame->format = codecContext->pix_fmt;

	frame->width = codecContext->width;
	frame->height = codecContext->height;

	for (std::pair<std::string, std::string> pair : m_settings.priv_data)
	{
		av_opt_set(codecContext->priv_data, pair.first.c_str(), pair.second.c_str(), 0);
	}

	if (avcodec_open2(codecContext, codec, NULL) < 0)
	{
		printf("Could not open codec!\n");
		assert(0);
	}

	ret = av_frame_get_buffer(frame, 0);
}

std::tuple<size_t, uint8_t*> Encoder::Encode(NDIlib_video_frame_v2_t* ndi_frame)
{
	ret = av_frame_make_writable(frame);

	uint8_t* data[1] = { ndi_frame->p_data };
	int linesize[1] = { m_settings.xres * 2 };

	uint8_t* outData[3] = { frame->data[0], frame->data[1], frame->data[2] };
	int outLinesize[3] = { m_settings.xres, m_settings.xres / 2, m_settings.xres / 2 };

	sws_scale(swsContext, data, linesize, 0, m_settings.yres, outData, outLinesize);

	frame->pts = i;

	if ((ret = avcodec_send_frame(codecContext, frame)) < 0)
	{
		LOG_ERR(ret);

		printf("Could not send_frame!\n");
		assert(0);
	}

	ret = avcodec_receive_packet(codecContext, pkt);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
	{
		printf("Buffering frame... send empty!\n");
		return std::make_tuple(0, nullptr);
	}
	else if (ret < 0)
	{
		LOG_ERR(ret);

		printf("Could not receive_packet!\n");
		assert(0);
	}

	i++;

	return std::make_tuple(pkt->size, pkt->data);
}

