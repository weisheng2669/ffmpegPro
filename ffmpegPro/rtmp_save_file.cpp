#include "rtmp_save_file.h"
using namespace std;

//���뻷����������
AVFormatContext* inputContext = nullptr;
//���������������
AVFormatContext* outputContext;

RTMP_SAVE::RTMP_SAVE() {

}

RTMP_SAVE::~RTMP_SAVE() {

}

//��ʾint���ص����
int ret = 0;
//��ȡ����������Ļ���,����Ϣ��

int RTMP_SAVE::openInputEnv(string inputUrl) {
	//�����뻷������ռ�
	inputContext = avformat_alloc_context();
	ret = avformat_open_input(&inputContext, inputUrl.c_str(), nullptr, nullptr);
	if (ret < 0) {
		LOGE("�����ʧ��!");
		return ret;
	}
	//������Ƶ������Ϣ
	ret = avformat_find_stream_info(inputContext, nullptr);
	if (ret < 0) {
		LOGE("����Ϣ����ʧ��!");
	}
	else {
		LOGI("���ļ��ɹ�");
	}
	return ret;
}

//���������������
int RTMP_SAVE::openOutpuEnv(string outUrl) {
	ret = avformat_alloc_output_context2(&outputContext, nullptr, "mpegts", outUrl.c_str());
	if (ret < 0) {
		LOGE("�����������ʧ��");
		goto ERROR;
	}
	//avio д�����
	ret = avio_open2(&outputContext->pb, outUrl.c_str(), AVIO_FLAG_WRITE, nullptr, nullptr);
	//����input������Ϣ
	for (int i = 0; i < inputContext->nb_streams; i++) {
		AVStream* stream = avformat_new_stream(outputContext, inputContext->streams[i]->codec->codec);
		ret = avcodec_copy_context(stream->codec, inputContext->streams[i]->codec);
		if (ret < 0) {
			LOGE("��������Ϣ����");
			goto ERROR;
		}
	}
	ret = avformat_write_header(outputContext, nullptr);
	if (ret < 0) {
		LOGE("дͷ����������");
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
//����Packet
shared_ptr<AVPacket> RTMP_SAVE::ReadPackectFromSource() {
	shared_ptr<AVPacket> packet(static_cast<AVPacket*>(av_malloc(sizeof(AVPacket))), [&](AVPacket* p) { av_packet_free(&p); av_freep(&p); });
	av_init_packet(packet.get());
	//�������������еĵ�һ��ѹ������
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

//д���ļ�
int RTMP_SAVE::WritePacket(shared_ptr<AVPacket> packet) {
	auto inputStream = inputContext->streams[packet->stream_index];
	auto outputStream = outputContext->streams[packet->stream_index];
	av_packet_rescale_ts(packet.get(), inputStream->time_base, outputStream->time_base);
	//д������
	av_interleaved_write_frame(outputContext, packet.get());
	return 0;
}

void RTMP_SAVE::init() {
	av_register_all();
	avfilter_register_all();
	avformat_network_init();
}
