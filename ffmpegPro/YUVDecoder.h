#pragma once
#include "common_relation.h"
class YUVDecoder
{
public:
	int flush_encoder(AVFormatContext* fmt_ctx, unsigned int stream_index);
	int decoder_yuv();

	
};

