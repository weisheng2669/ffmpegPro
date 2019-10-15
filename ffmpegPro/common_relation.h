#pragma once
#ifndef VIDEOOP
#define VIDEOOP
extern "C" {
#include "libavutil/opt.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include"libavutil/samplefmt.h"
#include "libavutil/time.h"
#include "libavutil/fifo.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfiltergraph.h"
#include "libavformat/avio.h"
#include "libavfilter/buffersink.h"
#include"libavfilter/buffersrc.h"
#include"libswscale/swscale.h"
#include "libswresample/swresample.h"
}


#define LOGE(...)  av_log(NULL,AV_LOG_ERROR,__VA_ARGS__)
#define LOGI(...)  av_log(NULL,AV_LOG_INFO,__VA_ARGS__)

#endif  VIDEOOP// !VIDEOOP
