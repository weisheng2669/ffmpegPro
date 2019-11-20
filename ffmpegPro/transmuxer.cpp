#include "transmuxer.h"

transmuxer::transmuxer() {

}
transmuxer::~transmuxer() {

}

int transmuxer::transport_file(const char* src_url, const char* dst_url) {
	//��������ĸ�ʽ
	AVOutputFormat* ofmt = NULL;
	AVBitStreamFilterContext* vbsf = NULL;
	//���������Ļ���
	AVFormatContext* ifmt_ctx = NULL, * ofmt_ctx = NULL;
	AVPacket pkt;

	int ret, i;
	int frame_index = 0, video_index = -1;
	av_register_all();
	//��������
	if ((ret = avformat_open_input(&ifmt_ctx, src_url, 0, 0)) < 0) {
		printf("Could not open input");
		goto end;
	}
	//���һ�����Ϣ
	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		printf("find video info failed");
		goto end;
	}
	//ת��ģʽ
	//MP4��ʹ�õ���H264����,��H.264���������ַ�װģʽ
	//һ����annexbģʽ,���Ǵ�ͳģʽ,��startcode,SPS��PPS��Element Stream��
	//��һ����mp4ģʽ,һ��MP4,MKV,AVI��û��startcode,SPS��PPS�Լ�������Ϣ����װ��������
	//ÿһ֡ǰ������һ֡�ĳ���ֵ,�ܶ������ֻ֧��annxebģʽ,�����Ҫ��MP4ģʽ��ת����FFMPEG����h264_mp4toannexb�Ϳ���ʵ��ת��


	vbsf = av_bitstream_filter_init("h264_mp4toannexb");
	//��ӡ��Ϣ
	av_dump_format(ifmt_ctx, 0, src_url, 0);
	//�����������
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, dst_url);
	if (!ofmt_ctx) {
		printf("Could not create output context \n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	//�����ʽ
	ofmt = ofmt_ctx->oformat;
	for (int i = 0; i < ifmt_ctx->nb_streams; i++) {
		if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_index = i;
		}
		AVStream* in_stream = ifmt_ctx->streams[i];
		//��ʼ��AVStream,������浽�����������
		AVStream* out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
		if (!out_stream) {
			printf("Failed to create new Stream \n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}
		//����������Ƶ������AVCodecContex����ֵt�������Ƶ��AVCodecContext��
		if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
			printf("Failed to copy stream context!");
			goto end;
		}
		//Ĭ�ϼ�Ϊ0
		out_stream->codec->codec_tag = 0;
		if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
			out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}
	//��ʾ�����������Ϣ
	av_dump_format(ofmt_ctx, 0, dst_url, 1);
	if (!(ofmt_ctx->flags & AVFMT_NOFILE)) {
		ret = avio_open(&ofmt_ctx->pb, dst_url, AVIO_FLAG_WRITE);
		if (ret < 0) {
			printf("Could not open output file %s", dst_url);
			goto end;
		}
	}
	//д�ļ�ͷ
	if (avformat_write_header(ofmt_ctx, NULL) < 0) {
		printf("Error occured when opening output file");
		goto end;
	}
	//ѭ��д��
	while (1) {
		AVStream* in_stream, * out_stream;
		ret = av_read_frame(ifmt_ctx, &pkt);
		if (ret < 0)
			break;
		in_stream = ifmt_ctx->streams[pkt.stream_index];
		out_stream = ofmt_ctx->streams[pkt.stream_index];
		//ת��dts��pts����ֵ
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
			(AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
			(AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.pos = -1;
		if (pkt.stream_index == video_index) {
			AVPacket fpkt = pkt;
			//��ÿ��AVPacket�е����ݣ�data�ֶΣ�����bitstream filter�����ˡ�һ�顣�ؼ�������av_bitstream_filter_filter()��
			/*
	�����������������������
	*ÿ��AVPacket��data�����H.264��NALU����ʼ��{0,0,0,1}

	*ÿ��IDR֡����ǰ�������SPS��PPS

	v_bitstream_filter_filter()���������ݺ�������ݣ��ֱ��Ӧ��4,5,6,7��������
   */
			int a = av_bitstream_filter_filter(vbsf, out_stream->codec, NULL, &fpkt.data,
				&fpkt.size, pkt.data, pkt.size, pkt.flags & AV_PKT_FLAG_KEY);
			pkt.data = fpkt.data;
			pkt.size = fpkt.size;
		}
		//дPacket
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