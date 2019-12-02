#include "encodepcmtoaac.h"

using namespace std;


encodepcmtoaac::encodepcmtoaac() {

}

encodepcmtoaac::~encodepcmtoaac() {

}

/* check that a given sample format is supported by the encoder */
static int check_sample_fmt(const AVCodec* codec, enum AVSampleFormat sample_fmt)
{
	const enum AVSampleFormat* p = codec->sample_fmts;
	while (*p != AV_SAMPLE_FMT_NONE) {
		if (*p == sample_fmt)
			return 1;
		p++;
	}
	return 0;
}
/* just pick the highest supported samplerate */
static int select_sample_rate(const AVCodec* codec)
{
	const int* p;
	int best_samplerate = 0;
	if (!codec->supported_samplerates)
		return 44100;
	p = codec->supported_samplerates;
	while (*p) {
		if (!best_samplerate || abs(44100 - *p) < abs(44100 - best_samplerate))
			best_samplerate = *p;
		p++;
	}
	return best_samplerate;
}
/* select layout with the highest channel count */
static int select_channel_layout(const AVCodec* codec)
{
	const uint64_t* p;
	uint64_t best_ch_layout = 0;
	int best_nb_channels = 0;
	if (!codec->channel_layouts)
		return AV_CH_LAYOUT_STEREO;
	p = codec->channel_layouts;
	while (*p) {
		int nb_channels = av_get_channel_layout_nb_channels(*p);
		if (nb_channels > best_nb_channels) {
			best_ch_layout = *p;
			best_nb_channels = nb_channels;
		}
		p++;
	}
	return best_ch_layout;
}
static void encode(AVCodecContext* ctx, AVFrame* frame, AVPacket* pkt,
	FILE* output)
{
	int ret;
	/* send the frame for encoding */
	ret = avcodec_send_frame(ctx, frame);
	if (ret < 0) {
		fprintf(stderr, "Error sending the frame to the encoder\n");
		exit(1);
	}
	/* read all the available output packets (in general there may be any
	 * number of them */
	while (ret >= 0) {
		ret = avcodec_receive_packet(ctx, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0) {
			fprintf(stderr, "Error encoding audio frame\n");
			exit(1);
		}
		fwrite(pkt->data, 1, pkt->size, output);
		av_packet_unref(pkt);
	}
}

int encodepcmtoaac::encode_audio(const char* in_file_url,FILE *output_file) {
	av_register_all();
	avcodec_register_all();
	const char* in_file_name;
	const AVCodec* encodec;
	AVCodecContext* encode_context = NULL;
	AVFrame* audio_frame,*temp_frame;
	AVPacket* audio_pkt;
	int i, j, k, ret_audio;
	FILE* f;
	uint16_t* samples;
	float t, tincr;
	SwrContext* swr;

	
	in_file_name = in_file_url;
	/* find the AAC encoder */
	encodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
	if (!encodec) {
		fprintf(stderr, "Codec not found\n");
		exit(1);
	}
	encode_context = avcodec_alloc_context3(encodec);
	if (!encode_context) {
		fprintf(stderr, "Could not allocate audio codec context\n");
		exit(1);
	}
	/* put sample parameters */
	encode_context->bit_rate = 64000;
	/* check that the encoder supports s16 pcm input */
	encode_context->sample_fmt = AV_SAMPLE_FMT_S16;
	/*if (!check_sample_fmt(encodec, encode_context->sample_fmt)) {
		fprintf(stderr, "Encoder does not support sample format %s",
			av_get_sample_fmt_name(encode_context->sample_fmt));
		exit(1);
	}*/
	swr = swr_alloc();
	av_opt_set_int(swr, "in_channel_layout", AV_CH_LAYOUT_MONO, 0);
	av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
	av_opt_set_int(swr, "in_sample_rate", 44100, 0);
	av_opt_set_int(swr, "out_sample_rate", 44100, 0);
	av_opt_set_sample_fmt(swr, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
	swr_init(swr);
	/* select other audio parameters supported by the encoder */
	encode_context->sample_rate = select_sample_rate(encodec);
	encode_context->channel_layout = select_channel_layout(encodec);
	encode_context->channels = av_get_channel_layout_nb_channels(encode_context->channel_layout);
	/* open it */
	if (avcodec_open2(encode_context, encodec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		exit(1);
	}
	f = fopen(in_file_name, "rb");
	if (!f) {
		fprintf(stderr, "Could not open %s\n", in_file_name);
		exit(1);
	}
	/* packet for holding encoded output */
	audio_pkt = av_packet_alloc();
	if (!audio_pkt) {
		fprintf(stderr, "could not allocate the packet\n");
		exit(1);
	}
	/* frame containing input raw audio */
	audio_frame = av_frame_alloc();
	temp_frame = av_frame_alloc();
	if (!audio_frame) {
		fprintf(stderr, "Could not allocate audio frame\n");
		exit(1);
	}
	audio_frame->nb_samples = encode_context->frame_size;
	audio_frame->format = encode_context->sample_fmt;
	audio_frame->channel_layout = encode_context->channel_layout;
	/* allocate the data buffers */
	ret_audio = av_frame_get_buffer(audio_frame, 0);
	if (ret_audio < 0) {
		fprintf(stderr, "Could not allocate audio data buffers\n");
		exit(1);
	}
	/* encode a single tone sound */
	t = 0;
	tincr = 2 * M_PI * 440.0 / encode_context->sample_rate;
	for (i = 0; i < 200; i++) {
		/* make sure the frame is writable -- makes a copy if the encoder
		 * kept a reference internally */
		ret_audio = av_frame_make_writable(audio_frame);
		if (ret_audio < 0)
			exit(1);
		samples = (uint16_t*)audio_frame->data[0];
		for (j = 0; j < encode_context->frame_size; j++) {
			samples[2 * j] = (int)(sin(t) * 10000);
			for (k = 1; k < encode_context->channels; k++)
				samples[2 * j + k] = samples[2 * j];
			t += tincr;
		}
		encode(encode_context, audio_frame, audio_pkt, output_file);
	}
	/* flush the encoder */
	encode(encode_context, NULL, audio_pkt, output_file);
	fclose(f);
	av_frame_free(&audio_frame);
	av_packet_free(&audio_pkt);
	avcodec_free_context(&encode_context);
	return 0;

}