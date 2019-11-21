#include "demuxer_relation.h"


static double r2d(AVRational r)
{
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}
demuxer_relation::demuxer_relation() {

}

demuxer_relation::~demuxer_relation() {

}

int demuxer_relation::demuxer(const char* input_fname)
{  
	int audio_stream_index = -1, video_stream_index = -1,isaaccodec = -1;
	const char* output_v_fname = "D:\\audioAndvideo\\muxer\\test_de.h264";
	const char* output_a_fname = "D:\\audioAndvideo\\muxer\\test_de.aac";
	//1.ע��API
	av_register_all();
	
	FILE* video_dst_file = fopen(output_v_fname, "wb");
	FILE* audio_dst_file = fopen(output_a_fname, "wb");
	//2.����AVFormatContext
	AVFormatContext* fmt_ctx = NULL;
	//3.���ļ�,����AVFormatContext
	int ret = avformat_open_input(&fmt_ctx, input_fname, NULL, NULL);
	if (ret < 0 || &fmt_ctx == NULL) {
		fprintf(stderr,"open file failed");
		return -1;
	}
	printf("1.file opened \n");
	ret = avformat_find_stream_info(fmt_ctx, NULL);
	if(ret<0){
		fprintf(stderr, "find stream info failed\n");
		return -1;
	}
	printf("2.find stream success!\n");
	char* fileName = fmt_ctx->filename;
	printf("��Ƶ���ĸ���%d\n",fmt_ctx->nb_streams);
	printf("��Ƶ���ļ���%s\n",fileName);
	printf("��Ƶ��ʱ��%d\n",fmt_ctx->duration);
	printf("��Ƶ�ı�����%d\n",fmt_ctx->bit_rate);
	printf("��Ƶ��packet_size%d\n", fmt_ctx->packet_size);
	printf("��Ƶ��max_delay%d\n", fmt_ctx->max_delay);
	for (int index = 0; index < fmt_ctx->nb_streams; index++) {
		if (fmt_ctx->streams[index]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = index;
			printf("��Ƶ��Ϣ:\n");
			printf("index:%d\n", fmt_ctx->streams[index]->index);   //���һ��ý���ļ�������Ƶ��������Ƶ������Ƶindex��ֵһ��Ϊ0������ֵ��һ��׼ȷ�����Ի��ǵ�ͨ��as->codecpar->codec_type�ж�����Ƶ������Ƶ
			printf("��Ƶ֡��:%lffps\n", r2d(fmt_ctx->streams[index]->avg_frame_rate)); //��Ƶ֡��,��λΪfps����ʾÿ����ֶ���֡
			if (AV_CODEC_ID_MPEG4 == fmt_ctx->streams[index]->codecpar->codec_id) //��Ƶѹ�������ʽ
			{
				printf("��Ƶѹ�������ʽ:MPEG4\n");
			}
			printf("֡���:%d ֡�߶�:%d\n", fmt_ctx->streams[index]->codecpar->width, fmt_ctx->streams[index]->codecpar->height); //��Ƶ֡��Ⱥ�֡�߶�
			int DurationVideo = (fmt_ctx->streams[index]->duration) * r2d(fmt_ctx->streams[index]->time_base); //��Ƶ��ʱ������λΪ�롣ע������ѵ�λ�Ŵ�Ϊ�������΢���Ƶ��ʱ������Ƶ��ʱ����һ����ȵ�
			printf("��Ƶ��ʱ����%dʱ%d��%d��\n", DurationVideo / 3600, (DurationVideo % 3600) / 60, (DurationVideo % 60)); //����Ƶ��ʱ��ת��Ϊʱ����ĸ�ʽ��ӡ������̨��
			printf("\n");

		}
		else if(fmt_ctx->streams[index]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
			audio_stream_index = index;
			printf("��Ƶ��Ϣ:\n");
			printf("index:%d\n", fmt_ctx->streams[index]->index);  //���һ��ý���ļ�������Ƶ��������Ƶ������Ƶindex��ֵһ��Ϊ1������ֵ��һ��׼ȷ�����Ի��ǵ�ͨ��fmt_ctx->streams[index]->codecpar->codec_type�ж�����Ƶ������Ƶ
			printf("��Ƶ������:%dHz\n", fmt_ctx->streams[index]->codecpar->sample_rate); //��Ƶ��������Ĳ����ʣ���λΪHz
			if (AV_SAMPLE_FMT_FLTP == fmt_ctx->streams[index]->codecpar->format)   //��Ƶ������ʽ
			{
				printf("��Ƶ������ʽ:AV_SAMPLE_FMT_FLTP\n");
			}
			else if (AV_SAMPLE_FMT_S16P == fmt_ctx->streams[index]->codecpar->format)
			{
				printf("��Ƶ������ʽ:AV_SAMPLE_FMT_S16P\n");
			}
			printf("��Ƶ�ŵ���Ŀ:%d\n", fmt_ctx->streams[index]->codecpar->channels); //��Ƶ�ŵ���Ŀ
			if (AV_CODEC_ID_AAC == fmt_ctx->streams[index]->codecpar->codec_id)   //��Ƶѹ�������ʽ
			{
				printf("��Ƶѹ�������ʽ:AAC\n");
				isaaccodec = 1;
			}
			else if (AV_CODEC_ID_MP3 == fmt_ctx->streams[index]->codecpar->codec_id)
			{
				printf("��Ƶѹ�������ʽ:MP3\n");
			}
			int DurationAudio = (fmt_ctx->streams[index]->duration) * r2d(fmt_ctx->streams[index]->time_base); //��Ƶ��ʱ������λΪ�롣ע������ѵ�λ�Ŵ�Ϊ�������΢���Ƶ��ʱ������Ƶ��ʱ����һ����ȵ�
			printf("��Ƶ��ʱ����%dʱ%d��%d��\n", DurationAudio / 3600, (DurationAudio % 3600) / 60, (DurationAudio % 60)); //����Ƶ��ʱ��ת��Ϊʱ����ĸ�ʽ��ӡ������̨��
			printf("\n");
		}
	}
	printf("video stream index %d,audio stream index %d", video_stream_index, audio_stream_index);
	//
	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	
	AVBitStreamFilterContext* bsfc = av_bitstream_filter_init("h264_mp4toannexb");

	AVCodecContext* videocodecctx = fmt_ctx->streams[video_stream_index]->codec;
	AVCodec* videodecode = avcodec_find_decoder(videocodecctx->codec_id);

	AVCodecContext* audioCodecCtx = fmt_ctx->streams[audio_stream_index]->codec;
	AVCodec* audiodecode = avcodec_find_decoder(audioCodecCtx->codec_id);

	AVFrame picture;

	while (av_read_frame(fmt_ctx, &pkt) >= 0)
	{
		if (pkt.stream_index == audio_stream_index)
		{
			if (isaaccodec == 1)
			{
				char aac_adts_header[7] = { 0 };

				
				int profile = 2;   //AAC LC
				int chanCfg = 1;   //˫����
				int freqIdx = 3;   //48000HZ

				aac_adts_header[0] = (char)0xFF;      // 11111111     = syncword
				aac_adts_header[1] = (char)0xF1;      // 1111 1 00 1  = syncword MPEG-2 Layer CRC
				aac_adts_header[2] = (char)(((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
				aac_adts_header[3] = (char)(((chanCfg & 3) << 6) + ((7 + pkt.size) >> 11));
				aac_adts_header[4] = (char)(((7 + pkt.size) & 0x7FF) >> 3);
				aac_adts_header[5] = (char)((((7 + pkt.size) & 7) << 5) + 0x1F);
				aac_adts_header[6] = (char)0xFC;

				fwrite(aac_adts_header, 1, 7, audio_dst_file);
			}
			fwrite(pkt.data, 1, pkt.size, audio_dst_file);
			printf("audio pts is %fn", pkt.pts * av_q2d(fmt_ctx->streams[audio_stream_index]->time_base));
		}
		else if (pkt.stream_index == video_stream_index) {
			int a = av_bitstream_filter_filter(bsfc, videocodecctx, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);
			fwrite(pkt.data, 1, pkt.size,video_dst_file);
			//int gotfinished = 0;
			//if (avcodec_decode_video2(videocodecctx, &picture, &gotfinished, &pkt) < 0)
			//{
			//	printf("decode video errorn");
			//}
			//if (picture.key_frame)
			//{
			//	//printf("key framen");
			//}
			//else {
			//	//printf("frame num is %dn",picture.pict_type);
			//}

			//printf("video pts is %f, %dn",picture.pkt_dts * av_q2d(fmtctx->streams[videoindex]->codec->time_base),picture.pts );
		}
		av_packet_unref(&pkt);
	}

	//printf("Demuxing succeeded.\n");

end:
	avformat_close_input(&fmt_ctx);
	fclose(video_dst_file);
	fclose(audio_dst_file);

	return 0;
}