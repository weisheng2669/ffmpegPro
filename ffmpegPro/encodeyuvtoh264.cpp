#include "encodeyuvtoh264.h"

int ret_h264;
encodeyuvtoh264::encodeyuvtoh264(){

}

encodeyuvtoh264::~encodeyuvtoh264() {

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
		ret_h264 = avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec, &enc_pkt,
			NULL, &got_frame);
		av_frame_free(NULL);
		if (ret_h264 < 0)
			break;
		if (!got_frame) {
			ret_h264 = 0;
			break;
		}
		printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
		/* mux encoded frame */
		ret_h264 = av_write_frame(fmt_ctx, &enc_pkt);
		if (ret_h264 < 0)
			break;
	}
	return ret_h264;
}

/*
@param src_url 源文件地址
@param dst_url 输出文件地址
*/
int encodeyuvtoh264::encodeframetoh264(const char* src_url,const char* dst_url) {

	AVFormatContext* ofmt_ctx_h264 = nullptr;
	AVOutputFormat* ofmt_h264 = nullptr;
	AVStream* video_stream_h264 = nullptr;
	AVCodecContext* out_codec_ctx = nullptr;
	AVCodec* out_codec = nullptr;

	uint8_t* picture_buf = nullptr;
	uint8_t* picture_buf_num = nullptr;
	AVFrame* picture = nullptr;
	int size;
	AVPacket pkt;
	int frame_num = 0 ;

	int y_size;
	int got_picture;
	int framecnt = 0;
	//注册
	av_register_all();
	avcodec_register_all();

	//打开文件
	FILE* in_file = fopen(src_url, "rb");
	FILE* in_file_num = fopen(src_url, "rb");
	if (!in_file) {
		LOGE("file not opened \n");
		return -1;
	}

	

	//初始化AVFormatContext
	if ((ret_h264 = avformat_alloc_output_context2(&ofmt_ctx_h264, NULL, NULL, dst_url)) < 0) {
		LOGE("open file context failed\n");
		return -1;
	}
	ofmt_h264 = ofmt_ctx_h264->oformat;

	if ((ret_h264 = avio_open2(&ofmt_ctx_h264->pb, dst_url, AVIO_FLAG_WRITE, NULL,NULL)) < 0) {
		LOGE("open file failed\n");
		return -1;
	}
	//初始化视频码流
	video_stream_h264 = avformat_new_stream(ofmt_ctx_h264, 0);

	if (video_stream_h264 == NULL) {
		LOGE("video stream is null");
		return -1;
	}
	video_stream_h264->time_base.num = 1;
	video_stream_h264->time_base.den = 60;

	out_codec_ctx = video_stream_h264->codec;
	out_codec_ctx->codec_id = ofmt_h264->video_codec;
	out_codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
	out_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	out_codec_ctx->width = 640;
	out_codec_ctx->height = 360;
	out_codec_ctx->time_base.num = 1;
	out_codec_ctx->time_base.den = 25;
	out_codec_ctx->bit_rate = 4000000;
	out_codec_ctx->gop_size = 100;

	if (out_codec_ctx->codec_id == AV_CODEC_ID_H264) {
		out_codec_ctx->refs = 3;
		out_codec_ctx->qmin = 10;
		out_codec_ctx->qmax = 51;
		out_codec_ctx->qcompress = 0.6;
	}
	else if (out_codec_ctx->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
		out_codec_ctx->max_b_frames = 2;
	}
	else if (out_codec_ctx->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
		out_codec_ctx->mb_decision = 2;
	}
	out_codec = avcodec_find_encoder(out_codec_ctx->codec_id);
	if (!out_codec) {
		LOGE("no encodec \n");
		return -1;
	}
	if ((ret_h264 = avcodec_open2(out_codec_ctx, out_codec, NULL)) < 0) {
		LOGE("codec open \n");
		return -1;
	}
	
	
	av_dump_format(ofmt_ctx_h264, 0, dst_url, 1);
	picture = av_frame_alloc();
	picture->width = out_codec_ctx->width;
	picture->height = out_codec_ctx->height;
	picture->format = out_codec_ctx->pix_fmt;
	size = avpicture_get_size(out_codec_ctx->pix_fmt, out_codec_ctx->width, out_codec_ctx->height);
	picture_buf = (uint8_t*)av_malloc(size);
	//分配Frame的格式
	avpicture_fill((AVPicture*)picture, picture_buf, out_codec_ctx->pix_fmt, 
		out_codec_ctx->width, out_codec_ctx->height);
	avformat_write_header(ofmt_ctx_h264, NULL);
	y_size = out_codec_ctx->width * out_codec_ctx->height;
	av_new_packet(&pkt, size );

	picture_buf_num = (uint8_t*)av_malloc(size);
	while (fread(picture_buf_num, 1, y_size * 3 / 2, in_file_num) > 0) {
		frame_num++;
	}
	fclose(in_file_num);
	printf("num = %d", frame_num);


	for (int i = 0; i < frame_num; i++) {
		//Read raw YUV data
		if (fread(picture_buf, 1, y_size * 3 / 2, in_file) <= 0) {
			printf("Failed to read raw data! \n");
			return -1;
		}
		else if (feof(in_file)) {
			break;
		}
		picture->data[0] = picture_buf;              // Y
		picture->data[1] = picture_buf + y_size;      // U 
		picture->data[2] = picture_buf + y_size * 5 / 4;  // V
		//PTS
		//pFrame->pts=i;
		picture->pts = i * (video_stream_h264->time_base.den) / ((video_stream_h264->time_base.num) * 25);
		got_picture = 0;
		//Encode
		ret_h264 = avcodec_encode_video2(out_codec_ctx, &pkt, picture, &got_picture);
		if (ret_h264 < 0) {
			printf("Failed to encode! \n");
			return -1;
		}
		
		if (got_picture == 1) {
			printf("Succeed to encode index:%5d \t frame: %5d\tsize:%5d\n", i,framecnt, pkt.size);
			framecnt++;
			pkt.stream_index = video_stream_h264->index;
			ret_h264 = av_write_frame(ofmt_ctx_h264, &pkt);
			av_free_packet(&pkt);
		}
		
	}
	//Flush Encoder
	ret_h264 = flush_encoder(ofmt_ctx_h264, 0);
	if (ret_h264 < 0) {
		printf("Flushing encoder failed\n");
		return -1;
	}

	//Write file trailer
	av_write_trailer(ofmt_ctx_h264);

	//Clean
	if (video_stream_h264) {
		avcodec_close(video_stream_h264->codec);
		av_free(picture);
		av_free(picture_buf);
	}
	avio_close(ofmt_ctx_h264->pb);
	avformat_free_context(ofmt_ctx_h264);

	fclose(in_file);

	return 0;

}