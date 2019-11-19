#include "transmuxer.h"

transmuxer::transmuxer() {

}
transmuxer::~transmuxer() {

}

int transmuxer::transport_file(char* src_url, char* dst_url) {
	AVOutputFormat* ofmt = NULL;
	AVBitStreamFilterContext* vbsf = NULL;
	AVFormatContext* ifmt_ctx, * ofmt_ctx;
	AVPacket pkt;

	int ret, i;
	int frame_index = 0, video_index = -1;
	av_register_all();
	//打开上下文
	if ((ret = avformat_open_input(&ifmt_ctx, src_url, 0, 0)) < 0) {
		printf("Could not open input");
		goto end;
	}
	//查找基本信息
	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		printf("find video info failed");
		goto end;
	}
	//转换模式
	vbsf = av_bitstream_filter_init("h264_mp4toannexb");
	//打印信息
	av_dump_format(ifmt_ctx, 0, src_url, 0);
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, dst_url);
	if (!ofmt_ctx) {
		printf("Could not create output context \n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt = ofmt_ctx->oformat;
	for (int i = 0; i < ifmt_ctx->nb_streams; i++) {
		if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_index = i;
		}
		AVStream* in_stream = ifmt_ctx->streams[i];
		//初始化AVStream
		AVStream* out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
		if (!out_stream) {
			printf("Failed to create new Stream \n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}
		//复制AVCodecContext的设置属性
		if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
			printf("Failed to copy stream context!");
			goto end;
		}
		out_stream->codec->codec_tag = 0;
		if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
			out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}
	//显示输出环境的信息
	av_dump_format(ofmt_ctx, 0, dst_url, 1);
	if (!(ofmt_ctx->flags & AVFMT_NOFILE)) {
		ret = avio_open(&ofmt_ctx->pb, dst_url, AVIO_FLAG_WRITE);
		if (ret < 0) {
			printf("Could not open output file %s", dst_url);
			goto end;
		}
	}
	//写文件头
	if (avformat_write_header(ofmt_ctx, NULL) < 0) {
		printf("Error occured when opening output file");
		goto end;
	}
	//循环写入
	while (1) {
		AVStream* in_stream, * out_stream;
		ret = av_read_frame(ifmt_ctx, &pkt);
		if (ret < 0)
			break;
		in_stream = ifmt_ctx->streams[pkt.stream_index];
		out_stream = ofmt_ctx->streams[pkt.stream_index];
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
			(AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
			(AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.pos = -1;
		if (pkt.stream_index == video_index) {
			AVPacket fpkt = pkt;
			int a = av_bitstream_filter_filter(vbsf, out_stream->codec, NULL, &fpkt.data,
				&fpkt.size, pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);
			pkt.data = fpkt.data;
			pkt.size = fpkt.size;
		}
		//写Packet
		if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
			printf("Error write packet");
			break;
		}
		printf("Write %8d frames to output file \n", frame_index);
		av_packet_unref(&pkt);
		frame_index++;
	}
	av_write_trailer(ofmt_ctx);

end:
	avformat_close_input(&ifmt_ctx);
	if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);
	system("pause");
	return 0;


}