
#include "muxer.h"

muxer::muxer() {

}
muxer::~muxer() {

}
int muxer::muxer_file(const char* src_url_v, const char* src_url_a, const char* dst_url) {
	//1.注册
	av_register_all();
	//2.输入输出上下文
	AVFormatContext* video_ifmt_ctx = NULL, * audio_ifmt_ctx = NULL, * ofmt_ctx = NULL;

	//3.输出Format
	AVOutputFormat* ofmt = NULL;
	AVPacket pkt;
	int ret, i;
	int video_index_in = -1, video_index_out = -1;
	int audio_index_in = -1, audio_index_out = -1;
	int frame_index = 0;
	int64_t cur_pts_v = 0, cur_pts_a = 0;

	//4.打开文件
	//音频
	if ((ret = avformat_open_input(&audio_ifmt_ctx, src_url_a, 0, 0)) < 0) {
		printf("Could not open audio file! \n");
		goto end;
	}
	if ((ret = avformat_find_stream_info(audio_ifmt_ctx, 0)) < 0) {
		printf("Failed to retrieve audio input stream information \n");
		goto end;
	}
	//视频
	if ((ret = avformat_open_input(&video_ifmt_ctx, src_url_v, 0, 0)) < 0) {
		printf("Could not open video file! \n");
		goto end;
	}
	if ((ret = avformat_find_stream_info(video_ifmt_ctx, 0)) < 0) {
		printf("Failed to retrieve video input stream information \n");
		goto end;
	}
	av_dump_format(audio_ifmt_ctx, 0, src_url_a, 0);
	av_dump_format(video_ifmt_ctx, 0, src_url_v, 0);
	//5.为输出分配上下文
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, dst_url);
	if (!ofmt_ctx) {
		printf("Could not create output context \n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt = ofmt_ctx->oformat;

	//6.复制信息
	//视频


	for (i = 0; i < video_ifmt_ctx->nb_streams; i++) {
		//Create output AVStream according to input AVStream
		if (video_ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			AVStream* in_stream = video_ifmt_ctx->streams[i];
			//Add a new stream to a media file.将out_stream加到ofmt_ctx中
			AVStream* out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
			video_index_in = i;
			if (!out_stream) {
				printf("Failed allocating output stream\n");
				ret = AVERROR_UNKNOWN;
				goto end;
			}
			video_index_out = out_stream->index;
			//Copy the settings of AVCodecContext
			if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
				printf("Failed to copy context from input to output stream codec context\n");
				goto end;
			}
			out_stream->codec->codec_tag = 0;
			if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
				out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
			break;
		}
	}
	//音频
	for (i = 0; i < audio_ifmt_ctx->nb_streams; i++) {
		//Create output AVStream according to input AVStream
		if (audio_ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			AVStream* in_stream = audio_ifmt_ctx->streams[i];
			//Add a new stream to a media file.将out_stream加到ofmt_ctx中
			AVStream* out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
			audio_index_in = i;
			if (!out_stream) {
				printf("Failed allocating output stream\n");
				ret = AVERROR_UNKNOWN;
				goto end;
			}
			audio_index_out = out_stream->index;
			//Copy the settings of AVCodecContext
			if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
				printf("Failed to copy context from input to output stream codec context\n");
				goto end;
			}
			out_stream->codec->codec_tag = 0;
			if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
				out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
			break;
		}
	}
	//输出输出文件的信息
	av_dump_format(ofmt_ctx, 0, dst_url, 1);

	//打开输出文件
	if (!(ofmt->flags & AVFMT_NOFILE)) {
		if (avio_open(&ofmt_ctx->pb, dst_url, AVIO_FLAG_WRITE) < 0) {
			printf("Could not open output file '%s'", dst_url);
			goto end;
		}
	}
	//写输出文件的文件头
	if (avformat_write_header(ofmt_ctx, NULL) < 0) {
		printf("Error occurred when opening output file\n");
		goto end;
	}

	//FIX
#if USE_H264BSF
	AVBitStreamFilterContext* h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
#endif
#if USE_AACBSF
	AVBitStreamFilterContext* aacbsfc = av_bitstream_filter_init("aac_adtstoasc");
#endif
	while (1) {
		AVFormatContext* ifmt_ctx;
		int stream_index = 0;
		AVStream* in_stream, * out_stream;

		//Get an AVPacket
		if (av_compare_ts(cur_pts_v, video_ifmt_ctx->streams[video_index_in]->time_base,
			cur_pts_a, audio_ifmt_ctx->streams[audio_index_in]->time_base) <= 0) {
			ifmt_ctx = video_ifmt_ctx;
			stream_index = video_index_out;

			if (av_read_frame(ifmt_ctx, &pkt) >= 0) {
				do {
					in_stream = ifmt_ctx->streams[pkt.stream_index];
					out_stream = ofmt_ctx->streams[stream_index];

					if (pkt.stream_index == video_index_in) {
						//FIX：No PTS (Example: Raw H.264)
						//Simple Write PTS
						if (pkt.pts == AV_NOPTS_VALUE) {
							//Write PTS
							AVRational time_base1 = in_stream->time_base;
							//Duration between 2 frames (us)
							int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
							//Parameters
							pkt.pts = (double)(frame_index * calc_duration) / (double)(av_q2d(time_base1) * AV_TIME_BASE);
							pkt.dts = pkt.pts;
							pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1) * AV_TIME_BASE);
							frame_index++;
						}

						cur_pts_v = pkt.pts;
						break;
					}
				} while (av_read_frame(ifmt_ctx, &pkt) >= 0);
			}
			else {
				break;
			}
		}
		else {
			ifmt_ctx = audio_ifmt_ctx;
			stream_index = audio_index_out;
			if (av_read_frame(ifmt_ctx, &pkt) >= 0) {
				do {
					in_stream = ifmt_ctx->streams[pkt.stream_index];
					out_stream = ofmt_ctx->streams[stream_index];

					if (pkt.stream_index == audio_index_in) {

						//FIX：No PTS
						//Simple Write PTS
						if (pkt.pts == AV_NOPTS_VALUE) {
							//Write PTS
							AVRational time_base1 = in_stream->time_base;
							//Duration between 2 frames (us)
							int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
							//Parameters
							pkt.pts = (double)(frame_index * calc_duration) / (double)(av_q2d(time_base1) * AV_TIME_BASE);
							pkt.dts = pkt.pts;
							pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1) * AV_TIME_BASE);
							frame_index++;
						}
						cur_pts_a = pkt.pts;

						break;
					}
				} while (av_read_frame(ifmt_ctx, &pkt) >= 0);
			}
			else {
				break;
			}

		}

		//FIX:Bitstream Filter
#if USE_H264BSF
		av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
#if USE_AACBSF
		av_bitstream_filter_filter(aacbsfc, out_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif


		//Convert PTS/DTS
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		pkt.stream_index = stream_index;

		printf("Write 1 Packet. size:%5d\tpts:%lld\n", pkt.size, pkt.pts);
		//Write
		if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
			printf("Error muxing packet\n");
			break;
		}
		av_free_packet(&pkt);

	}
	//Write file trailer
	av_write_trailer(ofmt_ctx);
#if USE_H264BSF
	av_bitstream_filter_close(h264bsfc);
#endif
#if USE_AACBSF
	av_bitstream_filter_close(aacbsfc);
#endif


end:
	avformat_close_input(&video_ifmt_ctx);
	avformat_close_input(&audio_ifmt_ctx);
	/* close output */
	if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);
	if (ret < 0 && ret != AVERROR_EOF) {
		printf("Error occurred.\n");
		return -1;
	}
	return 0;


}
