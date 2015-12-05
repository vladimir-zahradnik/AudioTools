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
#include <errno.h>
#include <math.h>
#include "common.h"

double* init_buffer_dbl(size_t size)
{
  double *ptr = (double *) malloc (sizeof(*ptr) * size);

  if (ptr == NULL)
  {
     fprintf(stdout, "\nError: malloc failed: %s\n", strerror(errno));
     exit (1);
  }
  /* initialize array to zero */
  memset ((void *) ptr, 0, sizeof(*ptr) * size);

  return (ptr);
}

char* show_time(int samplerate, int samples)
{
   static char time_buff [15];
   int hours, minutes;
   double seconds;

   seconds = (double) samples / samplerate;

   minutes = seconds / 60;
   seconds -= minutes * 60;

   hours = minutes / 60;
   minutes -= hours * 60;

   sprintf(time_buff, "%02d:%02d:%06.3f", hours, minutes, seconds);
   return time_buff;
}

// check for NaN and Inf and replace by 0.0
double check_nan (double number)
{
   if (isnan(number) || isinf(number))
    return 0.0;

   return number;
}
