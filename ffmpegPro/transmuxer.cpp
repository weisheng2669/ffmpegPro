#include "transmuxer.h"

transmuxer::transmuxer() {

}
transmuxer::~transmuxer() {

}

int transmuxer::transport_file(const char* src_url, const char* dst_url) {
	//输出容器的格式
	AVOutputFormat* ofmt = NULL;
	AVBitStreamFilterContext* vbsf = NULL;
	//输入和输出的环境
	AVFormatContext* ifmt_ctx = NULL, * ofmt_ctx = NULL;
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
	//MP4中使用的是H264编码,而H.264编码有两种封装模式
	//一种是annexb模式,它是传统模式,有startcode,SPS和PPS在Element Stream中
	//另一种是mp4模式,一般MP4,MKV,AVI都没有startcode,SPS和PPS以及其他信息被封装在容器中
	//每一帧前面是这一帧的长度值,很多解码其只支持annxeb模式,因此需要对MP4模式做转换在FFMPEG中用h264_mp4toannexb就可以实现转换


	vbsf = av_bitstream_filter_init("h264_mp4toannexb");
	//打印信息
	av_dump_format(ifmt_ctx, 0, src_url, 0);
	//申请输出环境
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, dst_url);
	if (!ofmt_ctx) {
		printf("Could not create output context \n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	//输出格式
	ofmt = ofmt_ctx->oformat;
	for (int i = 0; i < ifmt_ctx->nb_streams; i++) {
		if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_index = i;
		}
		AVStream* in_stream = ifmt_ctx->streams[i];
		//初始化AVStream,将流封存到输出环境里面
		AVStream* out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
		if (!out_stream) {
			printf("Failed to create new Stream \n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}
		//拷贝输入视频码流的AVCodecContex的数值t到输出视频的AVCodecContext。
		if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
			printf("Failed to copy stream context!");
			goto end;
		}
		//默认加为0
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
		//转化dts和pts的数值
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
			(AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
			(AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.pos = -1;
		if (pkt.stream_index == video_index) {
			AVPacket fpkt = pkt;
			//把每个AVPacket中的数据（data字段）经过bitstream filter“过滤”一遍。关键函数是av_bitstream_filter_filter()。
			/*
	经过处理完成下面两个功能
	*每个AVPacket的data添加了H.264的NALU的起始码{0,0,0,1}

	*每个IDR帧数据前面添加了SPS和PPS

	v_bitstream_filter_filter()的输入数据和输出数据（分别对应第4,5,6,7个参数）
   */
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