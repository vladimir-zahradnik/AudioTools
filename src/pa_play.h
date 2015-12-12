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

#ifndef PA_PLAY_H_
#define PA_PLAY_H_

#include "dsp.h"
#include "audiotools.h"
#include <pulse/simple.h>
#include <pulse/error.h>

// initialize PA Simple API
pa_simple *at_pulse_init(int channels, int samplerate);

#endif /* PA_PLAY_H_ */
