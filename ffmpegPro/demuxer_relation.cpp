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
	//1.注册API
	av_register_all();
	
	FILE* video_dst_file = fopen(output_v_fname, "wb");
	FILE* audio_dst_file = fopen(output_a_fname, "wb");
	//2.申请AVFormatContext
	AVFormatContext* fmt_ctx = NULL;
	//3.打开文件,分配AVFormatContext
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
	printf("视频流的个数%d\n",fmt_ctx->nb_streams);
	printf("视频的文件名%s\n",fileName);
	printf("视频的时长%d\n",fmt_ctx->duration);
	printf("视频的比特率%d\n",fmt_ctx->bit_rate);
	printf("视频的packet_size%d\n", fmt_ctx->packet_size);
	printf("视频的max_delay%d\n", fmt_ctx->max_delay);
	for (int index = 0; index < fmt_ctx->nb_streams; index++) {
		if (fmt_ctx->streams[index]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = index;
			printf("视频信息:\n");
			printf("index:%d\n", fmt_ctx->streams[index]->index);   //如果一个媒体文件既有音频，又有视频，则视频index的值一般为0。但该值不一定准确，所以还是得通过as->codecpar->codec_type判断是视频还是音频
			printf("视频帧率:%lffps\n", r2d(fmt_ctx->streams[index]->avg_frame_rate)); //视频帧率,单位为fps，表示每秒出现多少帧
			if (AV_CODEC_ID_MPEG4 == fmt_ctx->streams[index]->codecpar->codec_id) //视频压缩编码格式
			{
				printf("视频压缩编码格式:MPEG4\n");
			}
			printf("帧宽度:%d 帧高度:%d\n", fmt_ctx->streams[index]->codecpar->width, fmt_ctx->streams[index]->codecpar->height); //视频帧宽度和帧高度
			int DurationVideo = (fmt_ctx->streams[index]->duration) * r2d(fmt_ctx->streams[index]->time_base); //视频总时长，单位为秒。注意如果把单位放大为毫秒或者微妙，音频总时长跟视频总时长不一定相等的
			printf("视频总时长：%d时%d分%d秒\n", DurationVideo / 3600, (DurationVideo % 3600) / 60, (DurationVideo % 60)); //将视频总时长转换为时分秒的格式打印到控制台上
			printf("\n");

		}
		else if(fmt_ctx->streams[index]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
			audio_stream_index = index;
			printf("音频信息:\n");
			printf("index:%d\n", fmt_ctx->streams[index]->index);  //如果一个媒体文件既有音频，又有视频，则音频index的值一般为1。但该值不一定准确，所以还是得通过fmt_ctx->streams[index]->codecpar->codec_type判断是视频还是音频
			printf("音频采样率:%dHz\n", fmt_ctx->streams[index]->codecpar->sample_rate); //音频编解码器的采样率，单位为Hz
			if (AV_SAMPLE_FMT_FLTP == fmt_ctx->streams[index]->codecpar->format)   //音频采样格式
			{
				printf("音频采样格式:AV_SAMPLE_FMT_FLTP\n");
			}
			else if (AV_SAMPLE_FMT_S16P == fmt_ctx->streams[index]->codecpar->format)
			{
				printf("音频采样格式:AV_SAMPLE_FMT_S16P\n");
			}
			printf("音频信道数目:%d\n", fmt_ctx->streams[index]->codecpar->channels); //音频信道数目
			if (AV_CODEC_ID_AAC == fmt_ctx->streams[index]->codecpar->codec_id)   //音频压缩编码格式
			{
				printf("音频压缩编码格式:AAC\n");
				isaaccodec = 1;
			}
			else if (AV_CODEC_ID_MP3 == fmt_ctx->streams[index]->codecpar->codec_id)
			{
				printf("音频压缩编码格式:MP3\n");
			}
			int DurationAudio = (fmt_ctx->streams[index]->duration) * r2d(fmt_ctx->streams[index]->time_base); //音频总时长，单位为秒。注意如果把单位放大为毫秒或者微妙，音频总时长跟视频总时长不一定相等的
			printf("音频总时长：%d时%d分%d秒\n", DurationAudio / 3600, (DurationAudio % 3600) / 60, (DurationAudio % 60)); //将音频总时长转换为时分秒的格式打印到控制台上
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
				int chanCfg = 1;   //双音道
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