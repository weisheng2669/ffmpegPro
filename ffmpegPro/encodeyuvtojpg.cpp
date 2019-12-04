#include "encodeyuvtojpg.h"

int ret_yuv_jpg = 0;
encodeyuvtojpg::encodeyuvtojpg() {

}

encodeyuvtojpg::~encodeyuvtojpg() {

}

void init_op() {
	av_register_all();
	avcodec_register_all();
}

int initOutput(AVFormatContext** ofmt_ctx, const char* dst_url) {
	return avformat_alloc_output_context2(ofmt_ctx, NULL, "singlejpeg", dst_url);
}

int openOutput(AVFormatContext** ofmt_ctx, const char* dst_url) {
	return avio_open2(&(*ofmt_ctx)->pb, dst_url, AVIO_FLAG_WRITE, NULL, NULL);
}

void initcodecContext(AVCodecContext** out_codec_ctx,AVStream* video_stream) {
	AVCodec* picCodec;
	picCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
	//2.初始化(*encodeContext)环境
	(*out_codec_ctx) = avcodec_alloc_context3(picCodec);
	(*out_codec_ctx)->codec_id = picCodec->id;
	(*out_codec_ctx)->time_base.num = 1;
	(*out_codec_ctx)->time_base.den = 30;
	(*out_codec_ctx)->pix_fmt = *picCodec->pix_fmts;
	(*out_codec_ctx)->width = 640;
	(*out_codec_ctx)->height = 360;
}

int findAndOpenEncode(AVCodecContext* out_codec_ctx,AVCodec **codec) {
	(*codec) = avcodec_find_encoder(out_codec_ctx->codec_id);
	if ((*codec) == NULL) {
		LOGE("codec not find");
		return -1;
	}
	return avcodec_open2(out_codec_ctx, (*codec), NULL);
}


int flush_encoder(AVFormatContext* fmt_ctx, unsigned int stream_index) {
	int got_frame;
	AVPacket enc_pkt;
	if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
		CODEC_CAP_DELAY))
		return 0;
	while (1) {
		enc_pkt.data = NULL;
		enc_pkt.size = 0;
		av_init_packet(&enc_pkt);
		ret_yuv_jpg = avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec, &enc_pkt,
			NULL, &got_frame);
		av_frame_free(NULL);
		if (ret_yuv_jpg < 0)
			break;
		if (!got_frame) {
			ret_yuv_jpg = 0;
			break;
		}
		printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
		/* mux encoded frame */
		ret_yuv_jpg = av_write_frame(fmt_ctx, &enc_pkt);
		if (ret_yuv_jpg < 0)
			break;
	}
	return ret_yuv_jpg;
}

int encodeyuvtojpg::encode(const char* src_url, const char* dst_url) {

	AVFormatContext* ofmt_ctx = nullptr;
	AVOutputFormat* ofmt = nullptr;
	AVCodecContext* out_codec_ctx = nullptr;
	AVStream* video_stream = nullptr;
	AVCodec* codec = nullptr;
	AVFrame* frame = nullptr;
	int size = 0,yuv_size = 0;
	uint8_t* picture_buf = nullptr;
	AVPacket pkt;
	FILE* in_file = fopen(src_url, "rb");

	//1.初始化
	init_op();

	//2.分配AVFormatContext
	if ((ret_yuv_jpg = initOutput(&ofmt_ctx, dst_url)) < 0) {
		LOGE("output init failed \n");
		return ret_yuv_jpg;
	}
	ofmt = ofmt_ctx->oformat;

	//3.打开输出文件
	if ((ret_yuv_jpg = openOutput(&ofmt_ctx, dst_url)) < 0) {
		LOGE("output open failed \n");
		return ret_yuv_jpg;
	}

    //4.添加新流
	if ((video_stream = avformat_new_stream(ofmt_ctx, 0))== NULL) {
		LOGE("video stream new failed \n");
		return -1;
	}

	//5.初始化codecContext
	initcodecContext(&out_codec_ctx,video_stream);

	//6.查找并打开编码器
	if ((ret_yuv_jpg = findAndOpenEncode(out_codec_ctx, &codec)) < 0) {
		LOGE("find open encode failed \n");
		return -1;
	}

	//输出编码器环境
	av_dump_format(ofmt_ctx, 0, dst_url, 1);
	size = avpicture_get_size(out_codec_ctx->pix_fmt,
		out_codec_ctx->width, out_codec_ctx->height);

	//7.帧初始化
	frame = av_frame_alloc();
	frame->width = out_codec_ctx->width;
	frame->height = out_codec_ctx->height;
	frame->format = out_codec_ctx->pix_fmt;
	picture_buf = (uint8_t*)av_malloc(size);
	avpicture_fill((AVPicture*)frame, picture_buf, out_codec_ctx->pix_fmt,
		out_codec_ctx->width, out_codec_ctx->height);

	//8.写入Frame

	// 8.1 写入头部
	if ((ret_yuv_jpg = avformat_write_header(ofmt_ctx, NULL)) < 0) {
		LOGE("write head failed \n");
		return ret_yuv_jpg;
	}
	// 8.2 写入文件
	yuv_size = out_codec_ctx->width * out_codec_ctx->height;
	av_new_packet(&pkt, size);
	if ((ret_yuv_jpg = fread(picture_buf, 1, yuv_size * 3 / 2, in_file)) <= 0){
		LOGE("Failed to read raw data! \n");
		return ret_yuv_jpg;
	}
	frame->data[0] = picture_buf;
	frame->data[1] = picture_buf + yuv_size;
	frame->data[2] = picture_buf + yuv_size * 5 / 4;
	frame->pts = 0;
	int got_frame = 0;
	if ((ret_yuv_jpg = avcodec_encode_video2(out_codec_ctx, &pkt, frame, &got_frame)) < 0) {
		LOGE("编码失败");
	}
	if (got_frame == 1) {
		printf("Succeed to encode  size:%5d\n",pkt.size);
		pkt.stream_index = video_stream->index;
		ret_yuv_jpg = av_interleaved_write_frame(ofmt_ctx, &pkt);
		if (ret_yuv_jpg < 0) {
			LOGE("write failed\n");
			return -1;
		}
		av_free_packet(&pkt);
	}
	else {
		if (flush_encoder(ofmt_ctx, 0) < 0) {
			LOGE("Flushing encoder failed\n");
			return -1;
		}
	}
	//8.3 写入尾部
	ret_yuv_jpg = av_write_trailer(ofmt_ctx);
	av_frame_free(&frame);

	//9.关闭资源
	avcodec_close(out_codec_ctx);
	if (ofmt_ctx != nullptr) {
		for (int i = 0; i < ofmt_ctx->nb_streams; i++) {
			AVCodecContext* codec_ctx = ofmt_ctx->streams[i]->codec;
			avcodec_close(codec_ctx);
		}
		avformat_close_input(&ofmt_ctx);
	}

}
