#include <stdio.h>
#include <stdlib.h>

#include "wav.h"

//if you are using big endian machine, get a real computer
static u32 fget32_le(FILE* in)
{
	u32 b;
	fread(&b, sizeof(u32), 1, in);
	return b;
}
static u16 fget16_le(FILE* in)
{
	u16 b;
	fread(&b, sizeof(u16), 1, in);
	return b;
}
/**
 * \brief create default wav structure
 * compression     = 1
 * channels        = 1
 * sample_rate     = 44100
 * byte_rate       = 44100 * channels * bits_per_sample / 2
 * block_align     = channels * bits_per_sample / 2
 * bits_per_sample = 16
 * data_size       = 0
 * data            = NULL
 */
wav_t* wav_create()
{
	wav_t* wav = malloc(sizeof(wav_t));

	wav->compression     = 1;
	wav->channels        = 1;
	wav->sample_rate     = 16000;
	wav->byte_rate       = 16000 * 1 * 16 / 2;
	wav->block_align     = 1 * 16 / 2;
	wav->bits_per_sample = 16;
	wav->data_size       = 0;
	wav->data            = NULL;

	return wav;
}
/**
 * \brief read wav data from file
 */
wav_t* wav_read(const char* file_path)
{
	//open file
	FILE* in = fopen(file_path, "rb");
	if(in == NULL) { fprintf(stderr, "wav_read() error: cannot open input file\n"); return NULL; }

	//check riff container header
	if(fgetc(in) != 'R') { fprintf(stderr, "wav_read() error: invalid riff header\n"); return NULL; }
	if(fgetc(in) != 'I') { fprintf(stderr, "wav_read() error: invalid riff header\n"); return NULL; }
	if(fgetc(in) != 'F') { fprintf(stderr, "wav_read() error: invalid riff header\n"); return NULL; }
	if(fgetc(in) != 'F') { fprintf(stderr, "wav_read() error: invalid riff header\n"); return NULL; }

	//read file size
	u32 file_size = fget32_le(in);

	//check wave container header
	if(fgetc(in) != 'W') { fprintf(stderr, "wav_read() error: invalid wave header\n"); return NULL; }
	if(fgetc(in) != 'A') { fprintf(stderr, "wav_read() error: invalid wave header\n"); return NULL; }
	if(fgetc(in) != 'V') { fprintf(stderr, "wav_read() error: invalid wave header\n"); return NULL; }
	if(fgetc(in) != 'E') { fprintf(stderr, "wav_read() error: invalid wave header\n"); return NULL; }

	//check format chunk header
	if(fgetc(in) != 'f') { fprintf(stderr, "wav_read() error: invalid format header\n"); return NULL; }
	if(fgetc(in) != 'm') { fprintf(stderr, "wav_read() error: invalid format header\n"); return NULL; }
	if(fgetc(in) != 't') { fprintf(stderr, "wav_read() error: invalid format header\n"); return NULL; }
	if(fgetc(in) != ' ') { fprintf(stderr, "wav_read() error: invalid format header\n"); return NULL; }

	//read format chunk size
	u32 fmt_size     = fget32_le(in);
	//read format type -> 1: pcm, others: compression
	u16 fmt_type     = fget16_le(in);
	//read number of channels
	u16 fmt_channels = fget16_le(in);
	//read sample rate
	u32 fmt_sample_rate = fget32_le(in);
	//read byte rate
	u32 fmt_byte_rate   = fget32_le(in);
	//read block alignment
	u16 fmt_block_align    = fget16_le(in);
	//read bits per sample
	u16 fmt_bits_per_sample = fget16_le(in);

	//check the next header
	int next_chunk;

	//check header loop
	CHECK_HEADER:

	next_chunk = fgetc(in);

	//read list chunk
	if(next_chunk == 'L')
	{
		if(fgetc(in) != 'I') { fprintf(stderr, "wav_read() error: invalid list header\n"); return NULL; }
		if(fgetc(in) != 'S') { fprintf(stderr, "wav_read() error: invalid list header\n"); return NULL; }
		if(fgetc(in) != 'T') { fprintf(stderr, "wav_read() error: invalid list header\n"); return NULL; }
	
		u32 list_size = fget32_le(in);

		//skip useless shit
		for(u32 i = 0; i < list_size; i++)
		{
			fgetc(in);
		}

		goto CHECK_HEADER;
	}
	//read data chunk
	else if(next_chunk == 'd')
	{
		if(fgetc(in) != 'a') { fprintf(stderr, "wav_read() error: invalid data header\n"); return NULL; }
		if(fgetc(in) != 't') { fprintf(stderr, "wav_read() error: invalid data header\n"); return NULL; }
		if(fgetc(in) != 'a') { fprintf(stderr, "wav_read() error: invalid data header\n"); return NULL; }
	}
	else
	{
		fprintf(stderr, "wav_read() error: unknown chunk header\n");
	}

	//read data chunk size
	u32 data_size = fget32_le(in);

	//read data buffer
	void* buffer = malloc(data_size);
	if(fread(buffer, sizeof(u8), data_size, in) != data_size)
	{
		fprintf(stderr, "wav_read() error: invalid data size\n");
		free(buffer);
		return NULL;
	}

	//close the input file
	fclose(in);

	//create and initialize wav structure
	wav_t* wav = malloc(sizeof(wav));

	wav->compression     = fmt_type;
	wav->channels        = fmt_channels;
	wav->sample_rate     = fmt_sample_rate;
	wav->byte_rate       = fmt_byte_rate;
	wav->block_align     = fmt_block_align;
	wav->bits_per_sample = fmt_bits_per_sample;
	wav->data_size       = data_size;
	wav->data            = buffer;

	return wav;
}
/**
 * \brief write wav data into file
 */
void wav_write(wav_t* wav, const char* file_name)
{
	//open output file
	FILE* out = fopen(file_name, "wb");
	if(out == NULL) { return; }

	//write riff header
	fwrite("RIFF", sizeof(char), 4, out);
	//write file size - 8
	u32 size = 4 + 4 + 4 + 2 + 2 + 4 + 4 + 2 + 2 + 4 + 4 + wav->data_size;
	fwrite(&size, sizeof(u32), 1, out);
	//write wave header
	fwrite("WAVE", sizeof(char), 4, out);
	//write format content
	fwrite("fmt ", sizeof(char), 4, out);
	fwrite("\x10\x00\x00\x00",    sizeof(u32), 1, out);
	fwrite(&wav->compression,     sizeof(u16), 1, out);
	fwrite(&wav->channels,        sizeof(u16), 1, out);
	fwrite(&wav->sample_rate,     sizeof(u32), 1, out);
	fwrite(&wav->byte_rate,       sizeof(u32), 1, out);
	fwrite(&wav->block_align,     sizeof(u16), 1, out);
	fwrite(&wav->bits_per_sample, sizeof(u16), 1, out);
	//write data content
	fwrite("data",          sizeof(char), 4, out);
	fwrite(&wav->data_size, sizeof(u32), 1, out);
	fwrite(wav->data,       (wav->bits_per_sample / 8), wav->data_size / (wav->bits_per_sample / 8), out);
}
/**
 * \brief deallocate wav structure
 */
void wav_destroy(wav_t* wav)
{
	free(wav->data);
	free(wav);
}



















