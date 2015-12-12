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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include "dsp.h"
#include "common.h"
#include "audiotools.h"
#include "pa_play.h"

sf_count_t at_audio_processor(SNDFILE *infile, SNDFILE *outfile) {
    sf_count_t count = 0, frames_read = 0;
    SF_INFO info;
    pa_simple *pa_server = NULL;
    size_t noverlap, nslide;
    double *multi_data, *prev_multi_data;
    size_t window_size = 0;
    int fft_size = 0;
    int pa_error;
    float *pulse_data = NULL;

    sf_command(infile, SFC_GET_CURRENT_SF_INFO, &info, sizeof(info));

    // compute correction in output sampling frequency
    int input_samplerate = info.samplerate;
    int output_samplerate = (int) (floor(info.samplerate * at_get_playback_speed()));
    info.samplerate = output_samplerate;

    // get max. amount of output channels needed -- because of memory allocation
    int max_channel_count = info.channels > at_get_out_channels() ? info.channels : at_get_out_channels();

    // calculate optimal values for window size and FFT size
    at_calc_window_and_fft_size(&window_size, &fft_size, at_get_frame_duration(), input_samplerate);

    noverlap = (size_t) floor(window_size * at_get_overlap() / 100);
    nslide = window_size - noverlap;

    multi_data = init_buffer_dbl(window_size * max_channel_count);
    prev_multi_data = init_buffer_dbl(noverlap * max_channel_count);

    // output file not specified, initialize sound server
    if (at_get_out_file() == NULL) {
        pa_server = at_pulse_init(at_get_out_channels(), output_samplerate);
        pulse_data = malloc(sizeof(*pulse_data) * nslide * max_channel_count);
        if (pulse_data == NULL) {
            puts("malloc() failed. Exiting.");
            exit(1);
        }
    }

    // initialize FFT library
    at_fftw_init(fft_size);

    // internal representation for separated audio channels in time domain
    audio_container_t *audio_data_td = at_allocate_buffer(at_get_out_channels(), window_size, input_samplerate);

    // internal representation for separated audio channels in frequency domain
    audio_container_t *audio_data_fft = at_allocate_buffer(at_get_out_channels(), (size_t) fft_size, input_samplerate);

    // internal representation for data required for add-and-overlap method of audio reconstruction
    audio_container_t *audio_data_old = at_allocate_buffer(at_get_out_channels(), nslide, input_samplerate);

    /* Implementation of Add-And-Overlap method for joining of adjacent audio frames;
     * overlap of frames is specified as a parameter in range <0 - 99>, default value
     * is overlap equal to 50 percent.
     */

    do {
        if (frames_read == 0) {
            if ((count = sf_readf_double(infile, multi_data, (sf_count_t) window_size)) <= 0)
                exit(1);
            memcpy((void *) prev_multi_data, (void *) (multi_data + nslide * info.channels),
                   sizeof(*multi_data) * noverlap * info.channels);
        }
        else {
            count = sf_readf_double(infile, (multi_data + noverlap * info.channels), (sf_count_t) nslide);
            memcpy((void *) multi_data, (void *) prev_multi_data, sizeof(*prev_multi_data) * noverlap * info.channels);
            memcpy((void *) prev_multi_data, (void *) (multi_data + nslide * info.channels),
                   sizeof(*multi_data) * noverlap * info.channels);
        }

        frames_read += count;

        // print time into console
        printf("Time: %s", show_time(input_samplerate, (int) frames_read));
        puts("\033[1A");

        // separate channels to at_container struct
        at_separate_channels(multi_data, audio_data_td, info.channels);

        // basic channel interleaving to create multichannel matrix
        at_interleave_audio(audio_data_td, info.channels, fft_size);

        // if volume change was set, apply new volume setting
        if (at_get_volume() != 1.0)
            at_audio_gain(audio_data_td, at_get_volume());

        // apply window function to data
        apply_window(audio_data_td, window_size);

        // FFT transform
        for (int i = 0; i < MAX_CHANNELS; i++)
            at_compute_fft(audio_data_td->channel[i], window_size, audio_data_fft->channel[i]);

        // inverse FFT transform
        for (int i = 0; i < MAX_CHANNELS; i++) {
            at_compute_ifft(audio_data_fft->channel[i], window_size, audio_data_td->channel[i]);

            // overlap
            for (int j = 0; j < nslide; j++) {
                audio_data_td->channel[i][j] += audio_data_old->channel[i][j];
                audio_data_old->channel[i][j] = audio_data_td->channel[i][j + noverlap];

            }
        }


        // combine channels from at_container struct
        at_combine_channels(multi_data, audio_data_td, at_get_out_channels());

        // check if output file was specified
        if (at_get_out_file())
            sf_writef_double(outfile, multi_data, (sf_count_t) nslide);
        else {
            // convert data from double into float
            for (int i = 0; i < nslide * max_channel_count; i++) {
                pulse_data[i] = (float) multi_data[i];
            }

            /* play content of buffer via PA server */
            if (pa_simple_write(pa_server, pulse_data, sizeof(float) * nslide * max_channel_count, &pa_error) < 0) {
                fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(pa_error));
                exit(1);
            }

        }
    } while (count > 0);

    /* Make sure that every single sample was played */
    if (pa_server != NULL && pa_simple_drain(pa_server, &pa_error) < 0) {
        fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa_strerror(pa_error));
        exit(1);
    }

    // insert new line
    puts("\n");

    // free memory
    free(multi_data);
    free(prev_multi_data);
    at_free_buffer(audio_data_td);
    at_free_buffer(audio_data_old);
    at_free_buffer(audio_data_fft);
    sf_close(outfile);
    sf_close(infile);
    at_fftw_free();

    if (pa_server != NULL) {
        pa_simple_free(pa_server);
    }

    if (pulse_data != NULL)
        free(pulse_data);

    return count;

}

audio_container_t *at_allocate_buffer(int channels, size_t size, int samplerate) {
    if (size <= 0) {
        puts("Size of audio buffer not defined, exiting.");
        exit(1);
    }

    audio_container_t *buffer = malloc(sizeof(*buffer) * size);

    if (buffer == NULL) {
        fprintf(stdout, "\nError: malloc() failed: %s\n", strerror(errno));
        exit(1);
    }

    memset(buffer, 0, sizeof(*buffer));

    buffer->length = size;
    buffer->used_channels = channels;
    buffer->samplerate = samplerate;

    for (int i = 0; i < MAX_CHANNELS; ++i) {
        buffer->channel[i] = init_buffer_dbl(size);
    }

    return buffer;
}

int at_free_buffer(audio_container_t *buffer) {
    for (int i = 0; i < MAX_CHANNELS; ++i)
        free(buffer->channel[i]);

    free(buffer);

    return 0;
}

/* separate_channels */
int at_separate_channels(double *multi_data, audio_container_t *container, int input_channels) {
    if (input_channels > MAX_CHANNELS) {
        puts("Processing of multichannel audio with more that 6 channels is not supported.");
        exit(1);
    }

    int i, j;

    for (i = 0; i < input_channels; i++) {
        for (j = 0; j < container->length; j++) {
            container->channel[i][j] = multi_data[j * input_channels + i];
        }
    }

    return 0;
}

/* combine_channels */
int at_combine_channels(double *multi_data, audio_container_t *container, int output_channels) {
    if (output_channels > MAX_CHANNELS) {
        puts("Processing of multichannel audio with more that 6 channels is not supported.");
        exit(1);
    }

    double *tmp = NULL;

    // if LFE only is enabled, map LFE to FL channel (first available)
    if (at_get_lfe_only_setting() == true) {
        tmp = container->channel[FL];
        container->channel[FL] = container->channel[LFE];
        container->channel[LFE] = tmp;
    }

    // fix proper storage of 4-channel audio to be 2F/2R format
    if (output_channels == 4) {
        tmp = container->channel[C];
        container->channel[C] = container->channel[SL];
        container->channel[SL] = tmp;
        tmp = container->channel[LFE];
        container->channel[LFE] = container->channel[SR];
        container->channel[SR] = tmp;
    }

    // fix proper storage of 5-channel audio to be 3F/2R format
    if (output_channels == 5) {
        tmp = container->channel[LFE];
        container->channel[LFE] = container->channel[SL];
        container->channel[SL] = tmp;
    }


    int i, j;

    for (i = 0; i < output_channels; i++) {
        for (j = 0; j < container->length; j++) {
            multi_data[j * output_channels + i] = container->channel[i][j];
        }
    }

    return 0;
}

// simple audio upmix
void at_interleave_audio(audio_container_t *container, int input_channels, int fft_size) {
    // input is mono file, copy left channel to right
    if (input_channels == 1) {
        memcpy(container->channel[FR], container->channel[FL], container->length);
        input_channels++;
    }

    /* input is stereo or quadraphonic file (FL, FR, SL, SR), create center
     * channel with combination of left and right channel; decrease
     * volume by 50 percent */
    if (input_channels == 2 || input_channels == 4) {
        for (int i = 0; i < container->length; i++) {
            container->channel[C][i] = (container->channel[FL][i] + container->channel[FR][i]) / 4.0;
        }
        if (input_channels == 2)
            input_channels++;
    }

    /* input is 3-channel file (FL, FR, C); create 5-channel file with SL and SR channels */
    if (input_channels == 3) {
        for (int i = 0; i < container->length; i++) {
            container->channel[SL][i] = container->channel[FL][i] * 0.2;
            container->channel[SR][i] = container->channel[FR][i] * 0.2;
        }
        input_channels += 2;
    }

    /* create mono file from all these channels and save this into LFE buffer */
    if (input_channels == 5) {
        for (int i = 0; i < container->length; i++) {
            container->channel[LFE][i] = (container->channel[FL][i] + container->channel[FR][i] +
                                          container->channel[C][i] + container->channel[SL][i] +
                                          container->channel[SR][i]) / 5.0;
        }
        at_create_lfe(container, container->samplerate, fft_size);
    }
}

/* multiply audio_container data with some gain */
void at_audio_gain(audio_container_t *container, double gain) {
    // if volume settings were set
    if (gain != 1.0) {

        for (int ch = 0; ch < 6; ch++)
            for (int i = 0; i < container->length; i++)
                container->channel[ch][i] *= gain;
    }
}


/* apply_window */
int apply_window(audio_container_t *container, size_t datalen) {

    for (int i = 0; i < MAX_CHANNELS; i++) {
        for (size_t j = 0; j < datalen; j++) {
            container->channel[i][j] *= (j <= datalen - 1) ?
                                        0.54 - 0.46 * cos(2 * M_PI * j / (datalen - 1)) : 0;
        }
    }

    return 0;
}

// create LFE channel
void at_create_lfe(audio_container_t *container, int sampling_freq, int fft_size) {
    // create temporary variables
    double *buffer = init_buffer_dbl((size_t) fft_size);
    double *y_ps = init_buffer_dbl((size_t) fft_size / 2 + 1); /* power spectrum */
    double *y_phase = init_buffer_dbl((size_t) fft_size / 2 + 1); /* phase */

    // FFT
    at_compute_fft(container->channel[LFE], container->length, buffer);

    calc_magnitude(buffer, fft_size, y_ps);

    calc_phase(buffer, fft_size, y_phase);

    // calculate cut-off sample
    int n_freq = (int) ceil((2 * CUTOFF_FREQ * ((fft_size / 2.0) - 1) / sampling_freq));

    for (int i = n_freq; i < fft_size / 2 + 1; i++)
        y_ps[i] = 0;

    // recreate frequency spectrum from magnitude and phase
    calc_fft_complex_data(y_ps, y_phase, fft_size, buffer);

    // IFFT
    at_compute_ifft(buffer, container->length, container->channel[LFE]);

    // cleanup
    free(buffer);
    free(y_ps);
    free(y_phase);

}
