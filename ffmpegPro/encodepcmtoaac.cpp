#include "encodepcmtoaac.h"
#include <stdlib.h>
#define _CRT_SECURE_NO_WARNINGS 
using namespace std;

encodepcmtoaac::encodepcmtoaac() {

}
encodepcmtoaac::~encodepcmtoaac() {

}
int flush_encoder_audio(AVFormatContext* fmt_ctx, unsigned int stream_index) {
	int ret;
	int got_frame;
	AVPacket enc_pkt;
	if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
		CODEC_CAP_DELAY))
		return 0;
	while (1) {
		enc_pkt.data = NULL;
		enc_pkt.size = 0;
		av_init_packet(&enc_pkt);
		ret = avcodec_encode_audio2(fmt_ctx->streams[stream_index]->codec, &enc_pkt,
			NULL, &got_frame);
		av_frame_free(NULL);
		if (ret < 0)
			break;
		if (!got_frame) {
			ret = 0;
			break;
		}
		printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
		/* mux encoded frame */
		ret = av_write_frame(fmt_ctx, &enc_pkt);
		if (ret < 0)
			break;
	}
	return ret;
}

int encodepcmtoaac::encode_audio(const char* src_url, const char* dst_url)
{
	AVFormatContext* ofmt_ctx_encode_audio = nullptr;
	AVOutputFormat* ofmt_encode_audio = nullptr;
	AVStream* stream_encode_audio = 0;
	AVCodecContext* codec_ctx_encode_audio = nullptr;
	AVCodec* codec_encode_audio = nullptr;

	uint8_t* frame_buf_encode_audio;
	AVFrame* frame_encode_audio;
	AVPacket pkt_encode_audio;

	int got_frame_encode_audio = 0;
	int ret_encode_audio;

	//1.ע��
	av_register_all();
	avcodec_register_all();

	//2.��ʼ�����������
	if ((ret_encode_audio = avformat_alloc_output_context2(&ofmt_ctx_encode_audio, NULL, NULL, dst_url)) < 0) {
		LOGE("init AVFormat failed\n");
		return ret_encode_audio;
	}
	//3.�����������
	if ((ret_encode_audio = avio_open2(&ofmt_ctx_encode_audio->pb, dst_url, AVIO_FLAG_WRITE, NULL,NULL)) < 0) {
		LOGE("open output file failed\n");
		return ret_encode_audio;
	}
    //4.new Stream
	if (!(stream_encode_audio = avformat_new_stream(ofmt_ctx_encode_audio, 0))) {
		LOGE("open stream failed\n");
		return -1;
	}
	//5.����codec_context(д���ļ�����������)
	codec_ctx_encode_audio = stream_encode_audio->codec;
	codec_ctx_encode_audio->codec_id = AV_CODEC_ID_AAC;
	codec_ctx_encode_audio->codec_type = AVMEDIA_TYPE_AUDIO;
	codec_ctx_encode_audio->sample_fmt = AV_SAMPLE_FMT_FLTP;
	codec_ctx_encode_audio->sample_rate = 44100; //����Ƶ��
	codec_ctx_encode_audio->channels = 1;  //���� 
	codec_ctx_encode_audio->channel_layout = AV_CH_LAYOUT_MONO;
	codec_ctx_encode_audio->bit_rate = 64000; //����

	av_dump_format(ofmt_ctx_encode_audio, 0, dst_url, 1);
	//6.�ҵ�������
	if (!(codec_encode_audio = avcodec_find_encoder(codec_ctx_encode_audio->codec_id))) {
		LOGE("can not find encoder \n");
		return -1;
	}
	//7.�򿪱�����
	if ((ret_encode_audio = avcodec_open2(codec_ctx_encode_audio, codec_encode_audio, NULL)) < 0) {
		LOGE("Failed to open encoder \n");
		return -1;
	}
	
	//8.д�ļ�
	//8.1д�ļ�ͷ
	if ((ret_encode_audio = avformat_write_header(ofmt_ctx_encode_audio, NULL)) < 0) {
		LOGE("write file header failed \n");
		return ret_encode_audio;
	}
	//8.2ѭ��д֡
	//8.2.1�ز�����ʼ��
	SwrContext* swr_ctx = NULL;
	swr_ctx = swr_alloc_set_opts(
		swr_ctx,  //�ز�������

		codec_ctx_encode_audio->channel_layout,  //�ز�������ͨ����ʽ
		codec_ctx_encode_audio->sample_fmt, //�ز�������λ��
		codec_ctx_encode_audio->sample_rate, //�ز�������Ƶ��

		AV_CH_LAYOUT_MONO,  //ԭʼ������ͨ����ʽ
		AV_SAMPLE_FMT_S16,//ԭʼ�Ĳ���λ��
		44100, //ԭʼ�Ĳ���Ƶ��

		0,0);
	if (!swr_ctx) {
		LOGE("swr_alloc_set_opts error");
		return -1;
	}
	if ((ret_encode_audio = swr_init(swr_ctx)) < 0) {
		LOGE("swr init failed \n");
		return ret_encode_audio;
	}

	//8.3 ����Frame
	frame_encode_audio = av_frame_alloc();
	frame_encode_audio->format = AV_SAMPLE_FMT_FLTP;
	frame_encode_audio->channels = 1;
	frame_encode_audio->channel_layout = AV_CH_LAYOUT_MONO;
	frame_encode_audio->nb_samples = 1024;
	
	if ((ret_encode_audio = av_frame_get_buffer(frame_encode_audio, 0)) < 0) {
		LOGE("get buffer failed\n");
		return ret_encode_audio;
	}
	//�����read_sizeΪÿ֡�Ĳ���Ƶ��* ������(����˫��������2)*��ʽλ��(�����ʽs16,��ô����2(Byte))
	int read_size = frame_encode_audio->nb_samples * 2 * 1;  
	frame_buf_encode_audio = (uint8_t*)av_malloc(read_size);
	FILE* fp = fopen(src_url, "rb");
	av_init_packet(&pkt_encode_audio);
	for (;;) {
		int len = fread(frame_buf_encode_audio, 1, read_size, fp);
		if (len <= 0) {
			break;
		}
		const uint8_t* data[1];
		data[0] = frame_buf_encode_audio;
		
		len = swr_convert(swr_ctx, 
			frame_encode_audio->data,   //���֡��data
			frame_encode_audio->nb_samples, //���֡�Ĳ���Ƶ��

			data,   //��������
			frame_encode_audio->nb_samples //����֡�Ĳ���Ƶ��
		);
		if (len <= 0) {
			break;
		}
		got_frame_encode_audio = 0;
		if ((ret_encode_audio = avcodec_encode_audio2(codec_ctx_encode_audio, &pkt_encode_audio,
			frame_encode_audio, &got_frame_encode_audio)) < 0) {
			LOGE("encode failed\n");
			return ret_encode_audio;
		}
		if (got_frame_encode_audio == 1) {
			pkt_encode_audio.stream_index = 0;
			ret_encode_audio = av_interleaved_write_frame(ofmt_ctx_encode_audio, &pkt_encode_audio);
			av_free_packet(&pkt_encode_audio);
		}
	}
	if ((ret_encode_audio = flush_encoder_audio(ofmt_ctx_encode_audio, 0)) < 0) {
		LOGE("Flushing encoder failed\n");
		return ret_encode_audio;
	}
	av_write_trailer(ofmt_ctx_encode_audio);
	fclose(fp);
	if (stream_encode_audio) {
		avcodec_close(stream_encode_audio->codec);
		av_free(frame_encode_audio);
		av_free(frame_buf_encode_audio);
	}
	avio_close(ofmt_ctx_encode_audio->pb);
	avformat_free_context(ofmt_ctx_encode_audio);

}