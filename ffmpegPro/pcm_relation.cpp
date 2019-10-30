#include "pcm_relation.h"

using namespace std;

/**
 * Split Left and Right channel of 16LE PCM file.
 * @param url  Location of PCM file.
 *
 */
int simplest_pcm16le_split(const char* url) {
	
	FILE* fp = fopen(url, "rb+");
	FILE* fp1 = fopen("D:/audioAndvideo/output_l.pcm", "wb+");
	FILE* fp2 = fopen("D:/audioAndvideo/output_r.pcm", "wb+");
	if (fp != NULL) {
		printf("文件打开");
	}
	else {
		printf("文件没打开");
		return -1;
	}

	unsigned char *sample = (unsigned char *)malloc(4);
	int ret;
	while ((ret = fread(sample,1,4,fp))!=0) {
		//L
		fwrite(sample, 1, 2, fp1);
		//R
		fwrite(sample + 2, 1, 2, fp2);
	}
	printf("读写完成");
	free(sample);
	fclose(fp);
	fclose(fp1);
	fclose(fp2);
	return 0;
}

/**
 * Halve volume of Left channel of 16LE PCM file
 * @param url  Location of PCM file.
 */
int simplest_pcm16le_halfvolumeleft(const char* url) {
	FILE* fp = fopen(url, "rb+");
	FILE* fp1 = fopen("D:\\audioAndvideo\\output_halfleft.pcm", "wb+");

	int cnt = 0;

	unsigned char* sample = (unsigned char*)malloc(4);

	while (!feof(fp)) {
		short* samplenum = NULL;
		fread(sample, 1, 4, fp);

		samplenum = (short*)sample;
		*samplenum = *samplenum /8;
		//L
		fwrite(sample, 1, 2, fp1);
		//R
		fwrite(sample + 2, 1, 2, fp1);

		cnt++;
	}
	printf("Sample Cnt:%d\n", cnt);

	free(sample);
	fclose(fp);
	fclose(fp1);
	return 0;
}

/**
 * Re-sample to double the speed of 16LE PCM file
 * @param url  Location of PCM file.
 */
int simplest_pcm16le_doublespeed(const char* url) {

	FILE* fp = fopen(url, "rb+");
	FILE* fp1 = fopen("D:/audioAndvideo/output_doublespeed.pcm", "wb+");

	int cnt = 0;

	unsigned char* sample = (unsigned char*)malloc(4);

	while (!feof(fp)) {

		fread(sample, 1, 4, fp);

		if (cnt % 2 != 0) {
			//L
			fwrite(sample, 1, 2, fp1);
			//R
			fwrite(sample + 2, 1, 2, fp1);
		}
		cnt++;
	}
	printf("Sample Cnt:%d\n", cnt);

	free(sample);
	fclose(fp);
	fclose(fp1);
	return 0;
}