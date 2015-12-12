include_directories(${PROJECT_SOURCE_DIR}/src)
link_directories(${PROJECT_BINARY_DIR}/src)

# The version number
set (AudioTools_VERSION_MAJOR 0)
set (AudioTools_VERSION_MINOR 1)

# Pass detected settings to the source code
configure_file (
        "${PROJECT_SOURCE_DIR}/src/config.h.in"
        "${PROJECT_BINARY_DIR}/config.h"
)

# Path to generated config.h file
include_directories(${PROJECT_BINARY_DIR})

set(SOURCE_FILES
        audiotools.c
        audiotools.h
        common.c
        common.h
        dsp.c
        dsp.h
        fft.c
        fft.h
        pa_play.c
        pa_play.h)

add_executable(audiotools ${SOURCE_FILES})

# Link to sndfile fftw3 and GNU Math library
target_link_libraries(audiotools ${CORELIBS})
