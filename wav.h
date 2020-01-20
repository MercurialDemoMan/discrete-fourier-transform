#ifndef WAV_H
#define WAV_H

#include "types.h"

/**
 * \brief wav structure
 */
typedef struct
{
	u16 compression;
	u16 channels;

	u32 sample_rate;
	u32 byte_rate;

	u16 block_align;
	u16 bits_per_sample;

	u32   data_size;
	void* data;
} wav_t;

/**
 * \brief create default wav structure
 * compression     = 1
 * channels        = 1
 * sample_rate     = 16000
 * byte_rate       = 16000 * channels * bits_per_sample / 2
 * block_align     = channels * bits_per_sample / 2
 * bits_per_sample = 16
 * data_size       = 0
 * data            = NULL
 */
wav_t* wav_create();
/**
 * \brief read wav data from file
 */
wav_t* wav_read(const char* file_name);
/**
 * \brief write wav data into file
 */
void   wav_write(wav_t* wav, const char* file_name);
/**
 * \brief deallocate wav structure
 */
void   wav_destroy(wav_t* wav);

#endif