#include "encodeyuvtojpg.h"

int ret = 0;
encodeyuvtojpg::encodeyuvtojpg() {

}

encodeyuvtojpg::~encodeyuvtojpg() {

}

void init() {
	av_register_all();
	avcodec_register_all();
}

int initOutput(AVFormatContext* ofmt_ctx, const char* dst_url) {
	return avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, dst_url);
}

int openOutput(AVFormatContext* ofmt_ctx, const char* dst_url) {
	return avio_open2(&ofmt_ctx->pb, dst_url, AVIO_FLAG_WRITE, NULL, NULL);
}

void initcodecContext(AVCodecContext* out_codec_ctx,AVStream* video_stream) {
	out_codec_ctx = video_stream->codec;
	out_codec_ctx->codec_id = AV_CODEC_ID_MJPEG;
	out_codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
	out_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	out_codec_ctx->width = 1920;
	out_codec_ctx->height = 1080;
	out_codec_ctx->bit_rate = 400000;
	out_codec_ctx->gop_size = 30;
}

int findAndOpenEncode(AVCodecContext* out_codec_ctx,AVCodec *codec) {
	codec = avcodec_find_encoder(out_codec_ctx->codec_id);
	if (codec == NULL) {
		LOGE("codec not find");
		return -1;
	}
	return avcodec_open2(out_codec_ctx, codec, NULL);

}
void initFrame(AVCodecContext* out_codec_ctx,AVFrame* frame, uint8_t* picture_buf, int size) {
	frame = av_frame_alloc();
	frame->width = out_codec_ctx->width;
	frame->height = out_codec_ctx->height;
	frame->format = out_codec_ctx->pix_fmt;
	picture_buf = (uint8_t*)av_malloc(size);
	avpicture_fill((AVPicture*)frame, picture_buf, out_codec_ctx->pix_fmt,
		out_codec_ctx->width, out_codec_ctx->height);
}

int encodeyuvtojpg::encode(const char* src_url, const char* dst_url) {

	AVFormatContext* ofmt_ctx = nullptr;
	AVOutputFormat* ofmt = nullptr;
	AVCodecContext* out_codec_ctx = nullptr;
	AVStream* video_stream = nullptr;
	AVCodec* codec;
	AVFrame* frame;
	int size = 0,yuv_size = 0;
	uint8_t* picture_buf = nullptr;
	AVPacket pkt;
	FILE* in_file = fopen(src_url, "rb");

	//1.初始化
	init();
	//2.分配AVFormatContext
	if ((ret = initOutput(ofmt_ctx, dst_url)) < 0) {
		LOGE("output init failed \n");
		return ret;
	}
	ofmt = ofmt_ctx->oformat;
	//3.打开输出文件
	if ((ret = openOutput(ofmt_ctx, dst_url)) < 0) {
		LOGE("output open failed \n");
		return ret;
	}
    //4.添加新流
	if ((video_stream = avformat_new_stream(ofmt_ctx, 0))== NULL) {
		LOGE("video stream new failed \n");
		return -1;
	}
	//5.初始化codecContext
	initcodecContext(out_codec_ctx,video_stream);
	//6.查找并打开编码器
	if ((ret = findAndOpenEncode(out_codec_ctx, codec)) < 0) {
		LOGE("find open encode failed \n");
		return -1;
	}
	//输出编码器环境
	av_dump_format(ofmt_ctx, 0, dst_url, 1);
	size = avpicture_get_size(out_codec_ctx->pix_fmt,
		out_codec_ctx->width, out_codec_ctx->height);
	//7.帧初始化
	initFrame(out_codec_ctx,frame,picture_buf,size);
	//8.写入Frame
	// 8.1 写入头部
	if ((ret = avformat_write_header(ofmt_ctx, NULL)) < 0) {
		LOGE("write head failed \n");
		return ret;
	}
	// 8.2 写入文件
	yuv_size = out_codec_ctx->width * out_codec_ctx->height;
	av_new_packet(&pkt, size);
	if ((ret = fread(picture_buf, 1, yuv_size * 3 / 2, in_file)) <= 0){
		LOGE("Failed to read raw data! \n");
		return ret;
	}
	frame->data[0] = picture_buf;
	frame->data[1] = picture_buf + yuv_size;
	frame->data[2] = picture_buf + yuv_size * 5 / 4;



	


}
