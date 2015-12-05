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

#ifndef FFT_H_
#define FFT_H_

#include <math.h>
#include <string.h>

#define FFT_MAX           2048              // maximum size of FFT transform
#define WINDOW_MAX        FFT_MAX/2         // maximum size of window

#define OPTIMAL_FFT_SIZE(x)              (2 * pow(2, ceil(log2(x))))

#define WINDOW_SIZE(x,y)                 (floor(((x) * (y) / 1000)))

/* Calculate window size and optimal FFT size; FFT size needs to be <= than FFT_MAX (2048 samples)
 * In a case that calculated FFT size is greater than 2048, frame_duration is decreased, until
 * this criteria is met.
 */
extern int at_calc_window_and_fft_size (int *wsize, int *fft_len, int frame_dur, int srate);

// initialize FFTW library
extern int at_fftw_init(int size);

// free FFTW allocated memory
extern int at_fftw_free(void);

// get size of a FFT
int at_fftw_get_size(void);

// calculate forward FFT transform
int at_compute_fft(double *time_data_in, int window_size, double *fft_data_out);

// calculate inverse FFT transform
int at_compute_ifft(double *fft_data_in, int window_size, double *time_data_out);

/* calc_magnitude */
void calc_magnitude (const double * freq, int fft_size, double * magnitude);

/* calc_phase */
void calc_phase (const double * freq, int fft_size, double * phase);

/* calc_power_spectrum */
double calc_power_spectrum (const double * magnitude, int fft_size, double * power_spectrum);

/* calc_fft_complex_data */
void calc_fft_complex_data (const double * magnitude, const double * phase, int fft_size, double * freq);

/* multiply_fft_spec_with_gain */
void multiply_fft_spec_with_gain (const double * gain, int fft_size, double * freq);

/* complex argument of FFT spectrum */
double complex_argument (const double real, const double imag);


#endif /* FFT_H_ */
