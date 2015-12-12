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
#include "pa_play.h"

pa_simple *at_pulse_init(int channels, int samplerate) {
    int error;

    /* The Sample format to use */
    pa_sample_spec ss = {
            .rate = (uint32_t) samplerate,
            .format = PA_SAMPLE_FLOAT32LE,
            .channels = (uint8_t) channels
    };


    /* Channel map */
    static pa_channel_map channel_map;

    // Connection to server
    pa_simple *s = NULL;

    // init channel map
    pa_channel_map_init(&channel_map);

    // if playing LFE or mono file
    if (channels == 1) {
        channel_map.channels = 1;

        // LFE specified
        if (at_get_lfe_only_setting() == true) {
            channel_map.map[0] = PA_CHANNEL_POSITION_LFE;
        }
        else
            channel_map.map[0] = PA_CHANNEL_POSITION_LEFT;
    }

    // 2-channel audio
    if (channels == 2)
        pa_channel_map_init_stereo(&channel_map);

    // 3-channel audio: FL, FR, C
    if (channels == 3) {
        pa_channel_map_init(&channel_map);
        channel_map.channels = 3;
        channel_map.map[0] = PA_CHANNEL_POSITION_FRONT_LEFT;
        channel_map.map[1] = PA_CHANNEL_POSITION_FRONT_RIGHT;
        channel_map.map[2] = PA_CHANNEL_POSITION_FRONT_CENTER;
    }

    // 4-channel audio: FL, FR, SL, SR
    if (channels == 4) {
        pa_channel_map_init(&channel_map);
        channel_map.channels = 4;
        channel_map.map[0] = PA_CHANNEL_POSITION_FRONT_LEFT;
        channel_map.map[1] = PA_CHANNEL_POSITION_FRONT_RIGHT;
        channel_map.map[2] = PA_CHANNEL_POSITION_REAR_LEFT;
        channel_map.map[3] = PA_CHANNEL_POSITION_REAR_RIGHT;
    }

    // 5-channel audio: FL, FR, C, SL, SR
    if (channels == 5) {
        pa_channel_map_init(&channel_map);
        channel_map.channels = 5;
        channel_map.map[0] = PA_CHANNEL_POSITION_FRONT_LEFT;
        channel_map.map[1] = PA_CHANNEL_POSITION_FRONT_RIGHT;
        channel_map.map[2] = PA_CHANNEL_POSITION_FRONT_CENTER;
        channel_map.map[3] = PA_CHANNEL_POSITION_REAR_LEFT;
        channel_map.map[4] = PA_CHANNEL_POSITION_REAR_RIGHT;
    }

    // 6-channel audio: FL, FR, C, LFE, SL, SR
    if (channels == 6) {
        pa_channel_map_init(&channel_map);
        channel_map.channels = 6;
        channel_map.map[0] = PA_CHANNEL_POSITION_FRONT_LEFT;
        channel_map.map[1] = PA_CHANNEL_POSITION_FRONT_RIGHT;
        channel_map.map[2] = PA_CHANNEL_POSITION_FRONT_CENTER;
        channel_map.map[3] = PA_CHANNEL_POSITION_LFE;
        channel_map.map[4] = PA_CHANNEL_POSITION_REAR_LEFT;
        channel_map.map[5] = PA_CHANNEL_POSITION_REAR_RIGHT;
    }

    /* Create a new playback stream */
    if (!(s = pa_simple_new(NULL, "Audio Toolkit", PA_STREAM_PLAYBACK, NULL,
                            "audio stream", &ss, &channel_map, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        exit(1);
    }

    // init completed
    return s;
}
