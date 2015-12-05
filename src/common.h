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

#ifndef COMMON_H_
#define COMMON_H_

#define ARRAY_LEN(x)                     ((int) (sizeof (x) / sizeof (x [0])))
#define MAX(x,y)                         ((x) > (y) ? (x) : (y))
#define MIN(x,y)                         ((x) < (y) ? (x) : (y))


/* Boolean support */
/*
#if HAVE__BOOL
  #include <stdbool.h>
#else
  #ifndef bool
  #define bool int
  #endif
  #ifndef false
  #define false 0
  #endif
  #ifndef true
  #define true (!false)
  #endif
#endif
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/* create dynamic double array */
extern double * init_buffer_dbl(size_t size);

/* print time based on samples played */
char* show_time(int samplerate, int samples);

// check for NaN and Inf and replace by 0.0
double check_nan (double number);

#endif /* COMMON_H_ */
