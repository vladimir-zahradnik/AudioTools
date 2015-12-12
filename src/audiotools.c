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
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include "audiotools.h"

/* Print usage */
static void help(const char *argv0);

/* Print status info */
void at_print_status_info(SF_INFO sfinfo);

/* Basic information about runtime variables */
static AT_INFO info;

/* Main function */
int main(int argc, char **argv) {
    /* command line rules */
    enum audiotools_args_t {
        ARG_OUT_CHANNELS,
        ARG_LFE,
        ARG_FRAME_DURATION,
        ARG_OVERLAP,
        ARG_VERBOSITY,
        ARG_VOLUME,
        ARG_PLAYBACK_SPEED
    };

    // verbose output
    static bool verbose = false;

    memset(&info, 0, sizeof(info));
    info.overlap = -1;
    info.volume = 1.0;

    /* options for getopt library */
    static const struct option long_options[] = {
            {"help",           no_argument,       NULL, 'h'},
            {"version",        no_argument,       NULL, 'v'},
            {"verbose",        no_argument,       NULL, ARG_VERBOSITY},
            {"output",         required_argument, NULL, 'o'},
            {"channels",       required_argument, NULL, ARG_OUT_CHANNELS},
            {"lfe-only",       no_argument,       NULL, ARG_LFE},
            {"volume",         required_argument, NULL, ARG_VOLUME},
            {"frame-dur",      required_argument, NULL, ARG_FRAME_DURATION},
            {"overlap",        required_argument, NULL, ARG_OVERLAP},
            {"playback-speed", required_argument, NULL, ARG_PLAYBACK_SPEED},
            {NULL,             no_argument,       NULL, 0}
    };

    int opt = 0;
    int long_index = 0;

    /* if no argument was specified, print basic menu */
    if (argc == 1) {
        help(argv[0]);
        exit(1);
    }

    while ((opt = getopt_long(argc, argv, "hvo:", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'h': // help
                help(argv[0]);
                return 0;
            case 'v': // version
                printf("AudioTools %s\n", PACKAGE_VERSION);
                return 0;
            case 'o': // output file specified
                info.out_file = optarg;
                break;
            case ARG_OUT_CHANNELS: // optional, output channels specified
                info.out_channels = atoi(optarg);
                break;
            case ARG_LFE: // optional, LFE frequency output only
                info.lfe_only = true;
                break;
            case ARG_FRAME_DURATION:  // optional, specifies frame duration in milliseconds //
                info.frame_duration = atoi(optarg);
                break;
            case ARG_OVERLAP: // optional, specifies frame overlap
                info.overlap = atoi(optarg);
                break;
            case ARG_VERBOSITY: // verbose mode
                verbose = true;
                break;
            case ARG_VOLUME:    // volume setting, range <0 - 2>
                info.volume = atof(optarg);
                break;
            case ARG_PLAYBACK_SPEED:    // playback speed setting, range <0.5 - 1.5>
                info.playback_speed = atof(optarg);
                break;
            default:
                break;
        }
    }

    // update input file name info
    info.in_file = argv[optind++];
    if (info.in_file == NULL) {
        puts("Please specify an input file to process.");
        exit(1);
    }

    // process files
    SNDFILE *infile = NULL, *outfile = NULL;
    SF_INFO sfinfo;

    memset(&sfinfo, 0, sizeof(sfinfo));

    // open input file
    if ((infile = sf_open(info.in_file, SFM_READ, &sfinfo)) == NULL) {
        fprintf(stderr, "Error: Unable to open input file '%s': %s\n", info.in_file, sf_strerror(NULL));
        exit(1);
    }

    // set output format for audio
    if (info.out_file != NULL && strrchr(info.in_file, '.') != NULL
        && strrchr(info.out_file, '.') != NULL) {
        char ext_input[10];
        char ext_output[10];

        char *tmp = strrchr(info.in_file, '.');
        tmp++;

        strncpy(ext_input, tmp, 10);

        tmp = strrchr(info.out_file, '.');
        tmp++;

        strncpy(ext_output, tmp, 10);

        // convert both extensions to lowercase
        for (int i = 0; i < strlen(ext_input); i++) {
            ext_input[i] = (char) toupper(ext_input[i]);
            i++;
        }

        for (int i = 0; i < strlen(ext_output); i++) {
            ext_output[i] = (char) toupper(ext_output[i]);
            i++;
        }

        // extensions for input and output do not match
        if (strcmp(ext_input, ext_output) != 0) {
            // output is WAV
            if (strcmp(ext_output, "WAV") == 0 || strcmp(ext_output, "WAVE") == 0) {
                sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
            }
            // output is FLAC
            if (strcmp(ext_output, "FLAC") == 0) {
                sfinfo.format = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;
            }
            if (strcmp(ext_output, "OGG") == 0) {
                sfinfo.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;
            }
        }

    }

    // parse input
    at_parse_input_args(&info, &sfinfo, verbose);

    if (verbose)
        at_print_status_info(sfinfo);

    // set output channels to specified number
    sfinfo.channels = info.out_channels;

    // open output file, if specified
    if (info.out_file && (outfile = sf_open(info.out_file, SFM_WRITE, &sfinfo)) == NULL) {
        fprintf(stderr, "Error: Unable to open output file '%s': %s\n", info.out_file, sf_strerror(NULL));
        sf_close(infile);
        exit(1);
    }

    // main processing loop
    at_audio_processor(infile, outfile);

    // exit
    sf_close(infile);
    sf_close(outfile);

    if (verbose)
        puts("End of processing.");

    return EXIT_SUCCESS;
}

/* Print usage */
static void help(const char *argv0) {

    printf(
            "\nAudio Tools\n"
                    "--------------------------\n\n"
                    "Usage: %s [options] file\n\n"
                    "  -h, --help                  Show this help and quit\n"
                    "  -v, --version               Show version and quit\n"

                    "      --verbose               Enable verbose operations\n\n"

                    "  -o, --output                Output file name\n\n"

                    "                              If this switch is omitted,\n"
                    "                              processed audio is played via default sound card instead\n\n"

                    "      --channels              Specify number of channels for output\n\n"

                    "                              If specified, output audio will be downmixed or upmixed\n"
                    "                              to desired number of channels\n"
                    "                              Valid values: <1 - 6> channels\n\n"

                    "      --lfe-only              Create only a LFE channel as a output using cutoff frequency 100 Hz\n"
                    "                              --channels switch is ignored\n\n"

                    "      --volume                Increase or decrease audio volume.\n"
                    "                              Range <0.0 - 2.0> - where 0.0 means silence\n"
                    "                              and 2.0 means boost of volume 2-times.\n"
                    "                              Note: By setting the value too high, clipping may occur.\n\n"

                    "      --frame-dur             Duration of speech frame in milliseconds,\n"
                    "                              range <10 - 30> milliseconds\n\n"

                    "      --overlap               Overlap of adjacent frames given in percents,\n"
                    "                              range <0 - 99>, where '0' means no overlap\n\n"

                    "      --playback-speed        This option overrides sampling frequency settings in input file\n"
                    "                              and enables to control playback speed of a recording.\n"
                    "                              Range <0.5 - 1.5>\n\n"

                    "Supported formats for input audio:\n"
                    "----------------------------------\n"
                    "WAV, AIFF, AU, SND, VOC, W64, FLAC, OGG\n\n"

                    "Supported formats for output audio:\n"
                    "-----------------------------------\n"
                    "WAV, FLAC, OGG\n\n"
                    "For detailed information regarding format support see documentation to a library\n"
                    "libsndfile at < http://www.mega-nerd.com/libsndfile/#Features >\n\n", argv0);
}

int at_get_out_channels(void) {
    return info.out_channels;
}

void at_set_out_channels(int channels) {
    info.out_channels = channels;
}

bool at_get_lfe_only_setting(void) {
    return info.lfe_only;
}

int at_get_frame_duration(void) {
    return info.frame_duration;
}

int at_get_overlap(void) {
    return info.overlap;
}

const char *at_get_out_file(void) {
    return info.out_file;
}

void at_print_status_info(SF_INFO sfinfo) {
    printf("-----------------------------------------\n");
    printf("I N F O R M A T I O N :\n");
    printf("-----------------------------------------\n");
    printf("Input File: %s\n", info.in_file);
    printf("Duration: %s\n", show_time(sfinfo.samplerate, sfinfo.frames));
    printf("Sample rate of input audio: %d Hz\n", sfinfo.samplerate);
    printf("Input channels: %d\n", sfinfo.channels);

    if (info.out_file)
        printf("Output File: %s\n", info.out_file);
    printf("Playback speed of output audio: %.2f X\n", info.playback_speed);
    printf("Sample rate of output audio: %d Hz\n", (int) floor(sfinfo.samplerate * info.playback_speed));

    char *ch_out;
    if (info.lfe_only == true)
        ch_out = "LFE output only";
    else if (info.out_channels > sfinfo.channels)
        ch_out = "upmix";
    else if (info.out_channels == sfinfo.channels)
        ch_out = "no upmix/downmix";
    else
        ch_out = "downmix";

    printf("Output channels: %d (%s)\n", info.out_channels, ch_out);
    printf("Volume: %.3f\n", info.volume);
    printf("Frame Duration: %d ms\n", info.frame_duration);
    printf("Overlap: %d %%\n", info.overlap);
    printf("-----------------------------------------\n");
}

void at_parse_input_args(AT_INFO *info, SF_INFO *sfinfo, bool verbose) {

    // check for frame duration value
    if (!info->frame_duration || info->frame_duration > 30 || info->frame_duration < 10) {
        if (verbose && info->frame_duration)
            puts("Value for frame duration is out of range. Setting to defaults (20 ms).");
        info->frame_duration = 20;    // default frame duration 20 ms
    }

    // check for overlap value
    if (!info->overlap || info->overlap < 0 || info->overlap > 99) {
        if (verbose && info->overlap > 99)
            puts("Value for overlap is out of range. Setting to defaults (50 %).");
        info->overlap = 50;
    }

    // check for output channels parameter
    if (!(info->out_channels) || info->out_channels > 6) {
        if (verbose && info->out_channels > 6)
            puts("Value for output channels is out of range. Setting to same value as input.");
        info->out_channels = sfinfo->channels;
    }

    // check for LFE only parameter
    if (info->lfe_only == true) {
        info->out_channels = 1;
    }

    // check for volume parameter
    if (info->volume > 2.0 || info->volume < 0) {
        puts("Volume setting is out of range. Setting do defaults (no change in volume).");
        info->volume = 1.0;
    }

    // check for playback speed settings
    if (info->playback_speed > 1.5 || info->playback_speed < 0.5) {
        puts("Playback speed setting is out of range. Setting do defaults.");
        info->playback_speed = 1.0;
    }

    // insert empty line
    puts("");

}

void at_set_frame_duration(int duration) {
    info.frame_duration = duration;
}

// get audio volume settings
double at_get_volume(void) {
    return info.volume;
}

// get playback speed setting
double at_get_playback_speed(void) {
    return info.playback_speed;
}
