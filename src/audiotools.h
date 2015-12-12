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

#ifndef AUDIOTOOLS_H_
#define AUDIOTOOLS_H_

#include "common.h"
#include "dsp.h"

typedef struct AT_INFO {
    int out_channels;        // no. of channels for output audio
    bool lfe_only;            // LFE output only
    const char *in_file;    // input filename
    const char *out_file;    // output filename
    int frame_duration;    // frame duration
    int overlap;            // overlap
    double volume;            // volume setting
    double playback_speed;  // tempo setting
} AT_INFO;

// getters for AT_INFO
int at_get_out_channels(void);

bool at_get_lfe_only_setting(void);

int at_get_frame_duration(void);

int at_get_overlap(void);

double at_get_volume(void);

double at_get_playback_speed(void);

const char *at_get_out_file(void);

// putters for AT_INFO
void at_set_out_channels(int channels);

void at_set_frame_duration(int duration);

// parse input arguments
void at_parse_input_args(AT_INFO *info, SF_INFO *sfinfo, bool verbose);

#endif /* AUDIOTOOLS_H_ */
