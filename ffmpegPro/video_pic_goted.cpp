#include "video_pic_goted.h"
using namespace std;
int ret,video_index=-1;
AVFormatContext *ifmt_ctx = nullptr,*ofmt_ctx = nullptr;
int64_t lastReadPacketTime; 


video_pic_goted::video_pic_goted() {

}
video_pic_goted::~video_pic_goted() {

}

static int interrupt_cb(void* ctx) {
	int timeout = 3;
	if (av_gettime() - lastReadPacketTime > timeout * 1000 * 1000) {
		return -1;
	}
	return 0;
}
void init() {
	av_register_all();
	avfilter_register_all();
	avformat_network_init();
	av_log_set_level(AV_LOG_INFO);
}
int open_input(const char* src_url) {
	ifmt_ctx = avformat_alloc_context();
	lastReadPacketTime = av_gettime();
	printf("lastReadpacketTime = %64d", lastReadPacketTime);
	ifmt_ctx->interrupt_callback.callback = interrupt_cb;
	ret = avformat_open_input(&ifmt_ctx, src_url, nullptr, nullptr);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Input file open input failed \n");
		return ret;
	}
	ret = avformat_find_stream_info(ifmt_ctx, nullptr);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "stream info find failed \n");
	}
	else {
		av_log(NULL, AV_LOG_FATAL, "open file successed !\n");
	}
	for (int i = 0; i < ifmt_ctx->nb_streams; i++) {
		if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_index = i;
			break;
		}
	}

	return ret;
}

int open_output(const char* dst_url) {
	ret = avformat_alloc_output_context2(&ofmt_ctx, nullptr, "singlejpeg", dst_url);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "open output context failed \n");
		goto error;
	}
	ret = avio_open2(&ofmt_ctx->pb, dst_url, AVIO_FLAG_WRITE, nullptr, nullptr);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "open avio failed \n");
		goto error;
	}
	for (int i = 0; i < ifmt_ctx->nb_streams; i++) {
		if (ifmt_ctx->streams[i]->codec->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO) {
			continue;
		}
		AVStream* out_stream = avformat_new_stream(ofmt_ctx, ifmt_ctx->streams[i]->codec->codec);
		ret = avcodec_copy_context(out_stream->codec, ifmt_ctx->streams[i]->codec);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "copy codecContext failed \n");
			goto error;
		}
	}
	ret = avformat_write_header(ofmt_ctx, nullptr);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "format write header failed \n");
		goto error;
	}
	av_log(NULL, AV_LOG_FATAL, "open output file success success! \n");
	return ret;

error:
	if (ofmt_ctx) {
		for (int i = 0; i < ofmt_ctx->nb_streams; i++) {
			avcodec_close(ofmt_ctx->streams[i]->codec);
		}
		avformat_close_input(&ofmt_ctx);
	}
	return ret;
}
int init_decode_context(AVStream* in_stream) {
	auto codeId = in_stream->codec->codec_id;
	auto codec = avcodec_find_decoder(codeId);
	if (!codec) {
		av_log(NULL, AV_LOG_ERROR, "open codec failed! \n");
		return -1;
	}
	int ret = avcodec_open2(in_stream->codec, codec, NULL);
	return ret;
}
int init_encode_context_codec(AVStream* in_stream, AVCodecContext** encodeContext) {
	AVCodec* picCodec;
	picCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
	(*encodeContext) = avcodec_alloc_context3(picCodec);
	(*encodeContext)->codec_id = picCodec->id;
	(*encodeContext)->time_base.num = in_stream->codec->time_base.num;
	(*encodeContext)->time_base.den = in_stream->codec->time_base.den;
	(*encodeContext)->pix_fmt = *picCodec->pix_fmts;
	(*encodeContext)->width = in_stream->codec->width;
	(*encodeContext)->height = in_stream->codec->height;
	ret = avcodec_open2((*encodeContext), picCodec, nullptr);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "open encode codec failed! \n");
		return ret;
	}
	else {
		return 1;
	}
}
shared_ptr<AVPacket> read_frame_fromsource() {
	shared_ptr<AVPacket> packet(static_cast<AVPacket*> (av_malloc(sizeof(AVPacket))), 
		[&](AVPacket* p) {av_packet_free(&p); av_freep(&p); });
	av_init_packet(packet.get());
	lastReadPacketTime = av_gettime();
	ret = av_read_frame(ifmt_ctx, packet.get());
	if (ret >=  0) {
		return packet;
	}
	else {
		return nullptr;
	}

}
bool decode(AVStream* inputStream, AVPacket* packet, AVFrame* frame) {
	int gotFrame = 0;
	auto hr = avcodec_decode_video2(inputStream->codec, frame, &gotFrame, packet);
	if (hr >= 0 && gotFrame!=0) {
		return true;
	}
	return false;
}
shared_ptr<AVPacket> encode(AVCodecContext* encodeContext, AVFrame* frame) {
	int gotoutput = 0;
	std::shared_ptr<AVPacket> pkt(static_cast<AVPacket*> (av_malloc(sizeof(AVPacket))),
		[&](AVPacket* p) {av_packet_free(&p); av_freep(&p); });
	av_init_packet(pkt.get());
	pkt->data = NULL;
	pkt->size = 0;
	ret = avcodec_encode_video2(encodeContext, pkt.get(), frame, &gotoutput);
	if (ret >= 0 && gotoutput) {
		return pkt;

	}
	else {
		return nullptr;
	}
}
int WritePacket(shared_ptr<AVPacket> packet) {
	auto in_stream = ifmt_ctx->streams[packet->stream_index];
	auto out_stream = ofmt_ctx->streams[packet->stream_index];
	return av_interleaved_write_frame(ofmt_ctx, packet.get());
}
void close_input() {
	if (ifmt_ctx != nullptr) {
		avformat_close_input(&ifmt_ctx);
	}
}
void close_output() {
	if (ofmt_ctx != nullptr) {
		ret = av_write_trailer(ofmt_ctx);
		for (int i = 0; i < ofmt_ctx->nb_streams; i++) {
			AVCodecContext* codec_ctx = ofmt_ctx->streams[i]->codec;
			avcodec_close(codec_ctx);
		}
		avformat_close_input(&ofmt_ctx);
	}
}
int video_pic_goted::got_pic_from_video(const char* src_url,const char* dst_url) {
	//1.注册所有的链接
	init();
	//2.读取实时流的信息
	ret = open_input(src_url);
	if (ret >= 0) {
		//3.打开输出上下文
		ret = open_output(dst_url);
	}
	if (ret < 0) {
		goto error;
	}
	else {

		AVCodecContext* encodeContext = nullptr;
		//4.初始化解码环境
		if (video_index == -1) {
			printf("video index not found \n");
			goto error;
		}
		init_decode_context(ifmt_ctx->streams[video_index]);
		AVFrame* video_frame = av_frame_alloc();
		//5.编码器环境
		init_encode_context_codec(ifmt_ctx->streams[video_index], &encodeContext);
		while (true) {
			auto pkt = read_frame_fromsource();
			if (pkt && pkt->stream_index == 0) {
				if (decode(ifmt_ctx->streams[video_index], pkt.get(), video_frame)) {
					auto packetEncode = encode(encodeContext, video_frame);
					if (packetEncode) {
						printf("data = %8d", packetEncode->size);
						ret = WritePacket(packetEncode);
						if (ret >= 0) {
							printf("图片捕获成功!\n");
							break;
						}
					}
				}
			}
		}
		printf("Got pic !\n");
		av_frame_free(&video_frame);
		avcodec_close(encodeContext);

	}
	return 0;

error:
	close_input();
	close_output();
	return 0;

}
