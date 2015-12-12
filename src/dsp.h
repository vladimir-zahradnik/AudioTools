/*
** Copyright (C) 2013 Vladimir Zahradnik <vladimir.zahradnik@gmail.com>
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 2 or version 3 of the
** License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DSP_H_
#define DSP_H_

#include <sndfile.h>
#include "fft.h"

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

#define MAX_CHANNELS      6                    // maximum count of channels for input/output audio

#define CUTOFF_FREQ        120                    // defined cutoff frequency for simple low-pass filter for LFE


enum channel_map {
    FL = 0,        // Front-Left or Mono Channel
    FR = 1,        // Front-Right
    C = 2,        // Center
    LFE = 3,    // Low-Frequency Effects Channel
    SL = 4,        // Surround-Left
    SR = 5        // Surround-Right
};

typedef struct audio_container_t {
    double *channel[MAX_CHANNELS];    // data samples for each channel
    size_t length;                    // size of an array
    int used_channels;                // number of actually used arrays for storing channels
    int samplerate;                    // sample rate
} audio_container_t;

extern sf_count_t at_audio_processor(SNDFILE *infile, SNDFILE *outfile);

extern audio_container_t *at_allocate_buffer(int channels, size_t size, int samplerate);

extern int at_free_buffer(audio_container_t *buffer);

/* separate_channels */
int at_separate_channels(double *multi_data, audio_container_t *container, int input_channels);

/* combine_channels_double */
int at_combine_channels(double *multi_data, audio_container_t *container, int output_channels);

/* simple audio upmix */
void at_interleave_audio(audio_container_t *container, int input_channels, int fft_size);

/* multiply audio_container data with some gain */
void at_audio_gain(audio_container_t *container, double gain);

/* apply_window */
int apply_window(audio_container_t *container, size_t datalen);

/* create LFE channel */
void at_create_lfe(audio_container_t *container, int sampling_freq, int fft_size);

#endif /* DSP_H_ */
