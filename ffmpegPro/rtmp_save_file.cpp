#include "rtmp_save_file.h"
using namespace std;

//输入环境的上下文
AVFormatContext* inputContext = nullptr;
//输出环境的上下文
AVFormatContext* outputContext;

RTMP_SAVE::RTMP_SAVE() {

}

RTMP_SAVE::~RTMP_SAVE() {

}

//表示int返回的输出
int ret = 0;
//获取输入的上下文环境,流信息等

int RTMP_SAVE::openInputEnv(string inputUrl) {
	//给输入环境分配空间
	inputContext = avformat_alloc_context();
	ret = avformat_open_input(&inputContext, inputUrl.c_str(), nullptr, nullptr);
	if (ret < 0) {
		LOGE("输入打开失败!");
		return ret;
	}
	//查找视频的流信息
	ret = avformat_find_stream_info(inputContext, nullptr);
	if (ret < 0) {
		LOGE("流信息查找失败!");
	}
	else {
		LOGI("打开文件成功");
	}
	return ret;
}

//创建输出的上下文
int RTMP_SAVE::openOutpuEnv(string outUrl) {
	ret = avformat_alloc_output_context2(&outputContext, nullptr, "mpegts", outUrl.c_str());
	if (ret < 0) {
		LOGE("打开输出上下文失败");
		goto ERROR;
	}
	//avio 写入磁盘
	ret = avio_open2(&outputContext->pb, outUrl.c_str(), AVIO_FLAG_WRITE, nullptr, nullptr);
	//复制input的流信息
	for (int i = 0; i < inputContext->nb_streams; i++) {
		AVStream* stream = avformat_new_stream(outputContext, inputContext->streams[i]->codec->codec);
		ret = avcodec_copy_context(stream->codec, inputContext->streams[i]->codec);
		if (ret < 0) {
			LOGE("复制流信息错误");
			goto ERROR;
		}
	}
	ret = avformat_write_header(outputContext, nullptr);
	if (ret < 0) {
		LOGE("写头部出现问题");
		goto ERROR;
	}
	return ret;
ERROR:
	if (outputContext) {
		for (int i = 0; i < outputContext->nb_streams; i++) {
			avcodec_close(outputContext->streams[i]->codec);
		}
		avformat_close_input(&outputContext);
	}
	return ret;
}
//读出Packet
shared_ptr<AVPacket> RTMP_SAVE::ReadPackectFromSource() {
	shared_ptr<AVPacket> packet(static_cast<AVPacket*>(av_malloc(sizeof(AVPacket))), [&](AVPacket* p) { av_packet_free(&p); av_freep(&p); });
	av_init_packet(packet.get());
	//从输入上下文中的到一个压缩数据
	int ret = av_read_frame(inputContext, packet.get());
	if (ret >= 0)
	{
		return packet;
	}
	else
	{
		return nullptr;
	}
}




void av_packet_rescale_ts(AVPacket* pkt, AVRational src_tb, AVRational dst_tb) {
	if (pkt->pts != AV_NOPTS_VALUE)
		pkt->pts = av_rescale_q(pkt->pts, src_tb, dst_tb);
	if (pkt->dts != AV_NOPTS_VALUE)
		pkt->dts = av_rescale_q(pkt->dts, src_tb, dst_tb);
	if (pkt->duration > 0)
		pkt->duration = av_rescale_q(pkt->duration, src_tb, dst_tb);
}

//写入文件
int RTMP_SAVE::WritePacket(shared_ptr<AVPacket> packet) {
	auto inputStream = inputContext->streams[packet->stream_index];
	auto outputStream = outputContext->streams[packet->stream_index];
	av_packet_rescale_ts(packet.get(), inputStream->time_base, outputStream->time_base);
	//写入数据
	av_interleaved_write_frame(outputContext, packet.get());
	return 0;
}

void RTMP_SAVE::init() {
	av_register_all();
	avfilter_register_all();
	avformat_network_init();
}
