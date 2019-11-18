#include "Decoder.h"

AVFormatContext* outputFormatCtx;
AVOutputFormat* fmt;
AVStream* audio_st;
AVCodecContext* pCodecCtx;
AVCodec* pCodec;

uint8_t* frame_buf;
AVFrame* pFrame;
AVPacket pkt;

int got_frame = 0;



FILE* in_file = NULL;	                        //Raw PCM data
int framenum = 1000;                          //Audio frame number
const char* out_file = "D://test.aac";          //Output URL
int i;


Decoder::Decoder() {

}

Decoder::~Decoder() {

}

int Decoder::flush_encoder(AVFormatContext* fmt_ctx, unsigned int stream_index) {
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

void Decoder::init() {
	//1.注册
	av_register_all();
	//2.设置输出文件的信息格式
	outputFormatCtx = avformat_alloc_context();
	fmt = av_guess_format(NULL, out_file, NULL);
	outputFormatCtx->oformat = fmt;
}

int Decoder::openOutpuEnv() {
	//1.打开输出文件
	if (avio_open(&outputFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) {
		printf("Failed to open output file!\n");
		return -1;
	}
	//2.创建新的 输出流
	audio_st = avformat_new_stream(outputFormatCtx, 0);
	if (audio_st == NULL) {
		return -1;
	}

	//3.设置编码器环境
	pCodecCtx = audio_st->codec;
	pCodecCtx->codec_id = fmt->audio_codec;
	pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
	pCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
	pCodecCtx->sample_rate = 44100;
	pCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
	pCodecCtx->channels = av_get_channel_layout_nb_channels(pCodecCtx->channel_layout);
	pCodecCtx->bit_rate = 64000;

	//Show some information
	av_dump_format(outputFormatCtx, 0, out_file, 1);
	//4.根据codec的上下文查找编码器
	pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
	if (!pCodec) {
		printf("Can not find encoder!\n");
		return -1;
	}
	//5.找到编码器,打开编码器
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		printf("Failed to open encoder!\n");
		return -1;
	}

}

int Decoder::ReadFrameFromSource(){
	int size = 0;
	int ret = 0;
	in_file = fopen("D://test.pcm", "rb");
	pFrame = av_frame_alloc();
	pFrame->nb_samples = pCodecCtx->frame_size;
	pFrame->format = pCodecCtx->sample_fmt;

	size = av_samples_get_buffer_size(NULL, pCodecCtx->channels, pCodecCtx->frame_size, pCodecCtx->sample_fmt, 1);
	frame_buf = (uint8_t*)av_malloc(size);
	avcodec_fill_audio_frame(pFrame, pCodecCtx->channels, pCodecCtx->sample_fmt, (const uint8_t*)frame_buf, size, 1);

	//写头文件
	avformat_write_header(outputFormatCtx, NULL);

	av_new_packet(&pkt, size);

	for (i = 0; i < framenum; i++) {
		//Read PCM
		if (fread(frame_buf, 1, size, in_file) <= 0) {
			printf("Failed to read raw data! \n");
			return -1;
		}
		else if (feof(in_file)) {
			break;
		}
		pFrame->data[0] = frame_buf;  //PCM Data

		pFrame->pts = i * 100;
		got_frame = 0;
		//编码
		ret = avcodec_encode_audio2(pCodecCtx, &pkt, pFrame, &got_frame);
		if (ret < 0) {
			printf("Failed to encode!\n");
			return -1;
		}
		if (got_frame == 1) {
			printf("Succeed to encode 1 frame! \tsize:%5d\n", pkt.size);
			pkt.stream_index = audio_st->index;
			ret = av_write_frame(outputFormatCtx, &pkt);
			av_free_packet(&pkt);
		}
		//Flush Encoder
		ret = flush_encoder(outputFormatCtx, 0);
		if (ret < 0) {
			printf("Flushing encoder failed\n");
			return -1;
		}

		//Write Trailer
		av_write_trailer(outputFormatCtx);

		//Clean
		if (audio_st) {
			avcodec_close(audio_st->codec);
			av_free(pFrame);
			av_free(frame_buf);
		}
		avio_close(outputFormatCtx->pb);
		avformat_free_context(outputFormatCtx);

		fclose(in_file);

		return 0;

	}

}