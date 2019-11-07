#include "decodeaac.h"

int decodeaac() {
	//ע�����еĹ���
	av_register_all();
	const char* in_file = "D:\\audioAndvideo\\test_de.aac";
	const char* out_file = "D:\\audioAndvideo\\test_de.pcm";
	AVFormatContext* fmt_ctx = NULL;
	AVCodecContext* cod_ctx = NULL;
	AVCodec* cod = NULL;

	//����һ��avformat
	fmt_ctx = avformat_alloc_context();
	if (fmt_ctx == NULL)
		printf("alloc fail");

	//���ļ������װ
	if (avformat_open_input(&fmt_ctx, in_file, NULL, NULL) != 0)
		printf("open fail");

	//�����ļ����������Ϣ
	if (avformat_find_stream_info(fmt_ctx, NULL) < 0)
		printf("find stream fail");

	//�����ʽ��Ϣ
	av_dump_format(fmt_ctx, 0, in_file, 0);

	//���ҽ�����Ϣ
	int stream_index = -1;
	for (int i = 0; i < fmt_ctx->nb_streams; i++)
		if (fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			stream_index = i;
			break;
		}

	if (stream_index == -1)
		printf("find stream fail");

	//���������
	cod_ctx = fmt_ctx->streams[stream_index]->codec;
	cod = avcodec_find_decoder(cod_ctx->codec_id);

	if (cod == NULL)
		printf("find codec fail");

	if (avcodec_open2(cod_ctx, cod, NULL) < 0)
		printf("can't open codec");

	FILE* out_fb = NULL;
	out_fb = fopen(out_file, "wb");

	//����packet,���ڴ洢����ǰ������
	AVPacket* packet = (AVPacket*)malloc(sizeof(AVPacket));
	av_init_packet(packet);

	//����ת��������ز���
	//�����Ĳ��ַ�ʽ
	uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
	//��������
	int out_nb_samples = 1024;
	//������ʽ
	enum AVSampleFormat  sample_fmt = AV_SAMPLE_FMT_FLTP;
	//������
	int out_sample_rate = 48000;
	//ͨ����
	int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
	printf("%d\n", out_channels);
	//����buffer
	int buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, sample_fmt, 1);


	//ע��Ҫ��av_malloc
	uint8_t* buffer = (uint8_t*)av_malloc(MAX_AUDIO_FRAME_SIZE * 2);


	//����Frame�����ڴ洢����������
	AVFrame* frame = av_frame_alloc();

	int got_picture;

	int64_t in_channel_layout = av_get_default_channel_layout(cod_ctx->channels);
	//��ת����
	struct SwrContext* convert_ctx = swr_alloc();
	//����ת�����
	convert_ctx = swr_alloc_set_opts(convert_ctx, out_channel_layout, sample_fmt, out_sample_rate, \
		in_channel_layout, cod_ctx->sample_fmt, cod_ctx->sample_rate, 0, NULL);
	//��ʼ��ת����
	swr_init(convert_ctx);

	//whileѭ����ÿ�ζ�ȡһ֡����ת��

	while (av_read_frame(fmt_ctx, packet) >= 0) {

		if (packet->stream_index == stream_index) {

			//��������
			if (avcodec_decode_audio4(cod_ctx, frame, &got_picture, packet) < 0) {
				printf("decode error");
				return -1;
			}

			if (got_picture > 0) {
				//ת��
				swr_convert(convert_ctx, &buffer, MAX_AUDIO_FRAME_SIZE, (const uint8_t * *)frame->data, frame->nb_samples);

				printf("pts:%10lld\t packet size:%d\n", packet->pts, packet->size);

				fwrite(buffer, 1, buffer_size, out_fb);
			}
			got_picture = 0;
		}

		av_free_packet(packet);
	}

	swr_free(&convert_ctx);

	fclose(out_fb);

	return 0;
}