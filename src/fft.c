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

#include <fftw3.h>
#include <stdlib.h>
#include "fft.h"
#include "audiotools.h"

// pointer to a block in memory where FFT transformation will be done
static double *buffer = NULL;

// FFT plan
static fftw_plan fft_forw;

// IFFT plan
static fftw_plan fft_back;

// size of a FFT
static int fft_size = 0;

// initialize FFTW library
int at_fftw_init(int size) {
    if (size == 0) {
        puts("FFT size is invalid. Exiting.");
        exit(1);
    }
    fft_size = size;

    buffer = init_buffer_dbl((size_t) fft_size);
    if (buffer == NULL) {
        puts("Unable to create a FFT plan. Exiting.");
        exit(1);
    }
    fft_forw = fftw_plan_r2r_1d(fft_size, buffer, buffer, FFTW_R2HC, FFTW_MEASURE);
    fft_back = fftw_plan_r2r_1d(fft_size, buffer, buffer, FFTW_HC2R, FFTW_MEASURE);

    return 0;
}

// free FFTW allocated memory
int at_fftw_free(void) {
    free(buffer);
    fftw_destroy_plan(fft_forw);
    fftw_destroy_plan(fft_back);

    return 0;
}

// get size of a FFT
int at_fftw_get_size(void) {
    return fft_size;
}

// calculate forward FFT transform
int at_compute_fft(double *time_data_in, size_t window_size, double *fft_data_out) {
    // initialize FFT array to zero values
    memset(buffer, 0, sizeof(*buffer) * fft_size);

    // copy time domain data into FFT buffer
    memcpy(buffer, time_data_in, sizeof(*buffer) * window_size);

    // FFT
    fftw_execute(fft_forw);

    // copy FFT data from buffer into destination array
    memcpy(fft_data_out, buffer, sizeof(*buffer) * fft_size);

    return 0;
}

// calculate inverse FFT transform
int at_compute_ifft(double *fft_data_in, size_t window_size, double *time_data_out) {
    // copy FFT data back into buffer
    memcpy(buffer, fft_data_in, sizeof(*buffer) * fft_size);

    // proceed with inverse FFT transform
    fftw_execute(fft_back);

    // copy time domain data into destination array and normalize FFT
    for (int i = 0; i < window_size; i++)
        time_data_out[i] = check_nan(buffer[i] / (double) fft_size);

    return 0;
}


/* Calculate window size and optimal FFT size; FFT size needs to be <= than FFT_MAX (2048 samples)
 * In a case that calculated FFT size is greater than 2048, frame_duration is decreased, until
 * this criteria is met.
 */
int at_calc_window_and_fft_size(size_t *wsize, int *fft_len, int frame_dur, int srate) {
    // make a copy of variables before calculation
    size_t window_size = 0;
    int fft_size = *fft_len;

    do {

        /* compute window size and make it even */
        window_size = WINDOW_SIZE (frame_dur, srate);
        (window_size % 2 != 0) ? window_size++ : window_size;

        /* if computed optimal FFT size is greater than FFT_MAX, decrease
         * frame_duration by one millisecond - repeated until FFT size <= FFT_MAX */
        if (fft_size == 0 || fft_size > FFT_MAX) {
            fft_size = OPTIMAL_FFT_SIZE(window_size);

            if (fft_size > FFT_MAX)
                frame_dur--;
        }

    } while (fft_size > FFT_MAX);

    // update real variables
    *wsize = window_size;
    *fft_len = fft_size;
    at_set_frame_duration(frame_dur);

    return 0;
}

/* calc_magnitude */
void calc_magnitude(const double *freq, int fft_size, double *magnitude) {
    int i;

    for (i = 0; i <= fft_size / 2; i++) {
        if (i == 0 || (i == fft_size / 2 && (fft_size % 2 == 0))) /* fft_size is even */
            magnitude[i] = sqrt(freq[i] * freq[i]);
        else
            magnitude[i] = sqrt(freq[i] * freq[i] + freq[fft_size - i] * freq[fft_size - i]);
    }

    return;
}

/* calc_phase */
void calc_phase(const double *freq, int fft_size, double *phase) {
    int i;

    for (i = 0; i <= fft_size / 2; i++) {
        if (i == 0 || (i == fft_size / 2 && (fft_size % 2 == 0))) /* fft_size is even */
            phase[i] = complex_argument(freq[i], 0.0);
        else
            phase[i] = complex_argument(freq[i], freq[fft_size - i]);
    }
    return;
}

/* calc_power_spectrum */
double calc_power_spectrum(const double *magnitude, int fft_size, double *power_spectrum) {
    double norm_ps = 0;
    int i;

    for (i = 0; i <= fft_size / 2; i++) {
        power_spectrum[i] = magnitude[i] * magnitude[i];
        norm_ps += power_spectrum[i];
    }

    return norm_ps;
}

/* calc_fft_complex_data */
void calc_fft_complex_data(const double *magnitude, const double *phase, int fft_size, double *freq) {
    int i;

    for (i = 0; i <= fft_size / 2; i++) {
        if (i == 0 || (i == fft_size / 2 && (fft_size % 2 == 0))) /* fft_size is even */
            freq[i] = magnitude[i];
        else {
            freq[i] = magnitude[i] * cos(phase[i]);
            freq[fft_size - i] = magnitude[i] * sin(phase[i]);
        }
    }

    return;
}

/* multiply_fft_spec_with_gain */
void multiply_fft_spec_with_gain(const double *gain, int fft_size, double *freq) {
    int i;

    for (i = 0; i <= fft_size / 2; i++) {
        if (i == 0 || (i == fft_size / 2 && (fft_size % 2 == 0))) /* fft_size is even */
            freq[i] *= gain[i];
        else {
            freq[i] *= gain[i];
            freq[fft_size - i] *= gain[i];
        }
    }

    return;
}

/* complex argument of FFT spectrum */
double complex_argument(const double real, const double imag) {
    if (real > 0)
        return (atan(imag / real));
    if ((real < 0) && (imag >= 0))
        return (atan(imag / real) + M_PI);
    if ((real < 0) && (imag < 0))
        return (atan(imag / real) - M_PI);
    if ((real == 0) && (imag > 0))
        return (M_PI / 2);
    if ((real == 0) && (imag < 0))
        return (-M_PI / 2);

    return 0;
}
