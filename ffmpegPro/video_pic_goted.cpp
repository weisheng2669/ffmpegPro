#include "video_pic_goted.h"

int ret;
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
int init() {
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
	return ret;
}
int open_output(const char* dst_url) {
	ret = avformat_alloc_output_context2(&ofmt_ctx, nullptr, "singlejpeg", dst_url);
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
}
