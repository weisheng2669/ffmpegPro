#include "encodeyuvtojpg.h"

int ret = 0,video_index=-1;
AVFormatContext* ifmt_ctx = NULL, * ofmt_ctx;
AVIOContext* ifmt;
AVPacket* pkt = av_packet_alloc();
encodeyuvtojpg::encodeyuvtojpg() {

}
encodeyuvtojpg::~encodeyuvtojpg() {

}
void init() {
	av_register_all();
	avfilter_register_all();
}
int openInput() {
	
	//�����������Ļ���
	ret = avformat_open_input(&ifmt_ctx, "D:\\audioAndvideo\\encode\\test.mp4", NULL, NULL);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "open file failed \ n");
	}
	else {
		if ((ret = avformat_find_stream_info(ifmt_ctx, nullptr)) < 0) {
			LOGE("find stream info failed\n");
		}
		else {
			for (int i = 0; i < ifmt_ctx->nb_streams; i++) {
				if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
					video_index = i;
					LOGI("open file success\n");
					break;
				}
			}
			if (video_index == -1) {
				LOGE("can't find video Stream \n");
				ret = -1;
			}
		}
		return ret;
	}
}
int openOutput() {
	//1.�����ڴ�
	ret = avformat_alloc_output_context2(&ofmt_ctx, nullptr, "singlejpeg", "D:\\audioAndvideo\\encode\\test.jpg");
	if (ret < 0) {
		LOGE("open output file failed \n");
		goto error;
	}
	//2.�������������
	if ((ret = avio_open2(&ofmt_ctx->pb, "D:\\audioAndvideo\\encode\\test.jpg", AVIO_FLAG_WRITE, NULL, NULL))<0) {
		LOGE("open avio failed \n");
		goto error;
	}
	//3.��������Ϣ
	for (int i = 0; i < ifmt_ctx->nb_streams; i++) {
		//3.1 �ҵ�video����Ϣ
		if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			//3.2 ��out_stream���뵽���������ȥ
			AVStream* out_stream = avformat_new_stream(ofmt_ctx, ifmt_ctx->streams[i]->codec->codec);
			//3.3copy stream��Ϣ
			if (ret = avcodec_copy_context(out_stream->codec, ifmt_ctx->streams[i]->codec)) {
				LOGE("copy codec error \n");
				goto error;
			}
		}
	}
	return ret;

error:
	//��������Ĵ�ʱ
	if (ofmt_ctx) {
		for (int i = 0; i < ofmt_ctx->nb_streams; i++) {
			avcodec_close(ofmt_ctx->streams[i]->codec);
		}
		avformat_close_input(&ofmt_ctx);
	}
	return ret;

}
int init_decode_context(AVStream* in_stream) {
	//1.�ҵ�codec��id
	auto codecId = in_stream->codecpar->codec_id;
	//2.�ҵ�codec
	auto codec = avcodec_find_decoder(codecId);
	if (!codec) {
		return -1;
	}
	//3.��ʼ��codec
	/*
	param 1 in_stream->codec Ҫ��ʼ����������
	param 2 codec Ҫ��ʼ���Ľ�����
	param 3 NULL options
	*/
	if ((ret = avcodec_open2(in_stream->codec, codec, NULL)) < 0) {
		LOGE("open decodec failed \n");
	}
	return ret;
}
int init_encode_context(AVStream *in_stream,AVCodecContext **encodeContext) {
	//1.ע�����Codec
	AVCodec* picCodec;
	picCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
	//2.��ʼ��(*encodeContext)����
	(*encodeContext) = avcodec_alloc_context3(picCodec);
	(*encodeContext)->codec_id = picCodec->id;
	(*encodeContext)->time_base.num = in_stream->codec->time_base.num;
	(*encodeContext)->time_base.den = in_stream->codec->time_base.den;
	(*encodeContext)->pix_fmt = *picCodec->pix_fmts;
	(*encodeContext)->width = in_stream->codec->width;
	(*encodeContext)->height = in_stream->codec->height;

	if ((ret = avcodec_open2((*encodeContext), picCodec, NULL)) < 0) {
		LOGE("open encodec failed \n");
	}
	return ret;
}
shared_ptr<AVPacket> read_video_frame() {
	shared_ptr<AVPacket> packet(static_cast<AVPacket*> (av_malloc(sizeof(AVPacket))),
		[&](AVPacket* p) {av_packet_free(&p); av_freep(&p); });
	av_init_packet(packet.get());
	ret = av_read_frame(ifmt_ctx, packet.get());
	if (ret >= 0) {
		return packet;
	}
	else {
		return nullptr;
	}

}
int decodetoyuv(AVStream* in_stream, AVPacket* pkt, AVFrame *frame) {
	int gotFrame = 0;
	auto hr = avcodec_decode_video2(in_stream->codec, frame, &gotFrame, pkt);
	if (hr >= 0&&gotFrame!=0 ) {
		return 0;
	}
	return -1;
}
shared_ptr<AVPacket> encodetojpg(AVCodecContext* encodeContext, AVFrame* frame) {
	int gotoutput = 0;
	shared_ptr<AVPacket> pkt(static_cast<AVPacket*> (av_malloc(sizeof(AVPacket))),
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
int writetofile(shared_ptr<AVPacket> packet) {
	auto in_stream = ifmt_ctx->streams[packet->stream_index];
	auto out_stream = ofmt_ctx->streams[packet->stream_index];
	printf("data size = %d", packet.get()->size);
	return av_interleaved_write_frame(ofmt_ctx, packet.get());
}

void closeInput() {
	if (ifmt_ctx != NULL) {
		avformat_close_input(&ifmt_ctx);
	}
}
void closeOutput() {
	if (ofmt_ctx != NULL) {
		
		for (int i = 0; i < ofmt_ctx->nb_streams; i++) {
			AVCodecContext* codecContext = ofmt_ctx->streams[i]->codec;
			avcodec_close(codecContext);
		}
		avformat_close_input(&ofmt_ctx);
	}
}


int encodeyuvtojpg::encode() {

	//1.ע��
	init();

	//2.�������ļ�
	if ((ret = openInput()) < 0) {
		goto error;
	}
	else {
		//3.������ļ�
		if ((ret = openOutput()) < 0) {
			goto error;
		}
		AVCodecContext* encodeContext = NULL;
		//4.��ʼ������������
		init_decode_context(ifmt_ctx->streams[video_index]);
		//5.��ȡһ֡
		AVFrame* frame = av_frame_alloc();
		//6.��ʼ������������
		init_encode_context(ifmt_ctx->streams[video_index], &encodeContext);
		//7.��ȡһ֡��Ƶ֡
		while (true) {
			auto pkt = read_video_frame();
			if (pkt && pkt->stream_index == video_index) {
				//8.�Ƚ���Ϊyuv����
				if ((ret = decodetoyuv(ifmt_ctx->streams[video_index], pkt.get(), frame)) >= 0) {
					//9.����Ϊjpg����
					auto packetEncode = encodetojpg(encodeContext, frame);

					if (packetEncode) {
						//10.д���ļ�
						if ((ret = avformat_write_header(ofmt_ctx, NULL)) < 0) {
							LOGE("write file header failed\n");
							goto error;
						}
						if ((ret = writetofile(packetEncode)) < 0) {
							LOGE("write to file failed\n");
							goto error;
						}

						if ((ret = av_write_trailer(ofmt_ctx)) < 0) {
							LOGE("write file tailer failed\n");
							goto error;
						}
						break;
					}
				}
			}
			
			
		}
		
		LOGI("д��ͼƬ�ɹ� \n");
		av_frame_free(&frame);
		avcodec_close(encodeContext);
		closeInput();
		closeOutput();
		

	}
	return ret;
error:
	closeInput();
	closeOutput();
	return ret;

}