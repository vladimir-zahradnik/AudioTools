# AudioTools

AudioTools is very basic audio processing tool and player. As an input can be used any audio file supported by [libsndfile](http://www.mega-nerd.com/libsndfile/) library.

## Features

- Upmixing of input audio into 1 - 6 channels (including LFE)
- Creating LFE channel from input audio by applying low-pass filter on input audio (cut-off frequency is hardcoded to 120 Hz)
- Changing of playback speed by altering sampling frequency information
- Writing of modified audio into file
- Playing modified audio on-the-fly using Pulseaudio [simple API](http://freedesktop.org/software/pulseaudio/doxygen/simple.html).
