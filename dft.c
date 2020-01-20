#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <complex.h>
#include <math.h>

#include "types.h"
#include "wav.h"

/**
 * \brief print spectrum
 */
void spectrum_print(complex_t* spectrum, u32 spectrum_size)
{
	//calculate real spectrum size
	u32 spectrum_size_real = (u32)(ceil(spectrum_size / 2.0) + ((spectrum_size % 2 == 0) ? 1 : 0));

	printf("[\n");

	for(u32 i = 0; i < spectrum_size; i++)
	{
		//just print the first half
		if(i < spectrum_size_real)
		{
			printf("freq: %fpi [sim: %f]\n", carg(spectrum[i]) / M_PI, cabs(spectrum[i]));
		}
		//offset the second half into the first half and conjugate the complex number
		else
		{
			if(spectrum_size % 2 == 0)
			{
				printf("freq: %fpi [sim: %f]\n", -carg(spectrum[(spectrum_size_real - 1) - (i - (spectrum_size_real - 1))]) / M_PI, 
												  cabs(spectrum[(spectrum_size_real - 1) - (i - (spectrum_size_real - 1))]));
			}
			else
			{
				printf("freq: %fpi [sim: %f]\n", -carg(spectrum[(spectrum_size_real - 1) - (i - (spectrum_size_real))]) / M_PI, 
												  cabs(spectrum[(spectrum_size_real - 1) - (i - (spectrum_size_real))]));
			}
		}
	}

	printf("]\n");
}

/**
 * \brief print signal
 */
void signal_print(s16* signal, u32 num_samples)
{
	printf("[\n");
	
	for(u32 i = 0; i < num_samples; i++)
	{
		printf("%i, ", signal[i]);
	}

	printf("]\n");
}

/**
 * \brief calculate signal spectrum using discrete fourier transform
 */
complex_t* dft(s16* samples, u32 num_samples)
{

	//calculate real spectrum size, since the second half of a spectrum is just conjugate of the first half
	u32 spectrum_size_real = (u32)(ceil(num_samples / 2.0) + ((num_samples % 2 == 0) ? 1 : 0));

	//create zero buffer with only half number of samples
	complex_t* spectrum = calloc(spectrum_size_real, sizeof(complex_t));

	//calculate spectrum
	for(u32 f = 0; f < spectrum_size_real; f++)
	{
		//sum of samples multiplied by frequency -> we get what frequency is in the signal and how much it affects the signal
		for(u32 i = 0; i < num_samples; i++)
		{
			spectrum[f] += (complex_t)samples[i] * cexp(-((I * 2.0 * M_PI) / (complex_t)num_samples) * (complex_t)f * (complex_t)i);
		}
	}

	//return final buffer
	return spectrum;
}

/**
 * \brief calculate signal from spectrum
 */
s16* dft_rev(complex_t* spectrum, u32 spectrum_size)
{
	//calculate real spectrum size
	u32 spectrum_size_real = (u32)(ceil(spectrum_size / 2.0) + ((spectrum_size % 2 == 0) ? 1 : 0));

	//create signal buffer with full number of samples
	s16* samples = malloc(sizeof(s16) * spectrum_size);

	//calculate every sample
	for(u32 f = 0; f < spectrum_size; f++)
	{
		complex_t acc = 0.0;

		//signal sample -> sum of frequencies in specific time sample
		for(u32 i = 0; i < spectrum_size; i++)
		{
			//just sum the first half
			if(i < spectrum_size_real)
			{
				acc += spectrum[i] * cexp(((I * 2.0 * M_PI) / (complex_t)spectrum_size) * (complex_t)f * (complex_t)i);
			}
			//offset the second half into the first half and conjugate the complex number
			else
			{

				if(spectrum_size % 2 == 0)
				{
					acc += conj(spectrum[(spectrum_size_real - 1) - (i - (spectrum_size_real - 1))]) * cexp(((I * 2.0 * M_PI) / (complex_t)spectrum_size) * (complex_t)f * (complex_t)i);
				}
				else
				{
					acc += conj(spectrum[(spectrum_size_real - 1) - (i - (spectrum_size_real))]) * cexp(((I * 2.0 * M_PI) / (complex_t)spectrum_size) * (complex_t)f * (complex_t)i);
				}
			}
		}

		//the sum in the dft caused scaling issues
		//so we need to divide by the number of frequencies so we get the right scale
		acc = acc / (complex_t)spectrum_size;

		//round that bitch and store the sample
		samples[f] = round(creal(acc));
	}

	//return final buffer
	return samples;
}

int main(int argc, char* argv[])
{
	if(argc != 2) { fprintf(stderr, "usage: dft input.wav"); return 0; }

	wav_t* wav = wav_read(argv[1]);

	if(wav == NULL || wav->bits_per_sample != 16) { fprintf(stderr, "wat\n"); return 0; }

	u32 samples_num = wav->data_size / (wav->bits_per_sample / 8);

	complex_t** spec_array = malloc((u32)ceil(sizeof(complex_t*) * samples_num / 512));
	for(u32 i = 0; i < samples_num; i += 512)
	{
		if(i + 512 >= samples_num)
		{
			spec_array[i / 512] = dft((s16*)(wav->data) + i, samples_num - i);
			spectrum_print(spec_array[i / 512], samples_num - i);
		}
		else
		{
			spec_array[i / 512] = dft((s16*)(wav->data) + i, 512);
			spectrum_print(spec_array[i / 512], 512);
		}
	}

	/*s16** sig_array = malloc((u32)ceil(sizeof(s16*) * samples_num / 512));
	for(u32 i = 0; i < samples_num; i += 512)
	{
		if(i + 512 >= samples_num)
		{
			sig_array[i / 512] = dft_rev(spec_array[i / 512], samples_num - i);
		}
		else
		{
			sig_array[i / 512] = dft_rev(spec_array[i / 512], 512);
		}
	}

	s16* out_sig = malloc(samples_num * sizeof(s16));*/



	return 0;
}