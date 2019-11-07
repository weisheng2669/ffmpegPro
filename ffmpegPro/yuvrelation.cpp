#include "yuvrelation.h"

/**
 * Split Y, U, V planes in YUV420P file.
 * @param url  Location of Input YUV file.
 * @param w    Width of Input YUV file.
 * @param h    Height of Input YUV file.
 * @param num  Number of frames to process.
 *
 */
int simplest_yuv420_split(char* url, int w, int h, int num) {
	FILE* fp = fopen(url, "rb+");
	FILE* fp1 = fopen("D:\\audioAndvideo\\output_420_y.y", "wb+");
	FILE* fp2 = fopen("D:\\audioAndvideo\\output_420_u.y", "wb+");
	FILE* fp3 = fopen("D:\\audioAndvideo\\output_420_v.y", "wb+");

	unsigned char* pic = (unsigned char*)malloc(w * h * 3 / 2);

	for (int i = 0; i < num; i++) {

		fread(pic, 1, w * h * 3 / 2, fp);
		//Y
		fwrite(pic, 1, w * h, fp1);
		//U
		fwrite(pic + w * h, 1, w * h / 4, fp2);
		//V
		fwrite(pic + w * h * 5 / 4, 1, w * h / 4, fp3);
	}

	free(pic);
	fclose(fp);
	fclose(fp1);
	fclose(fp2);
	fclose(fp3);

	return 0;
}


int decodeyuv420(char* url) {
	//1.注册组件->编码器 解码器等
	av_register_all();
	//2.初始化封装格式上下文->视频编码->处理为视频压缩格式
	AVFormatContext* avformat_context = avformat_alloc_context();
	//3.得到视频压缩数据的类型
	return 0;


}