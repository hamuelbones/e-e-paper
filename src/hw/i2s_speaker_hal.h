//
// Created by Samuel Jones on 3/26/22.
//

#include <stdbool.h>

#ifndef EPAPER_DISPLAY_I2S_SPEAKER_HAL_H
#define EPAPER_DISPLAY_I2S_SPEAKER_HAL_H

void speaker_play_wav(const char* filename, bool (*until)(void*), void** arg);

#endif //EPAPER_DISPLAY_I2S_SPEAKER_HAL_H
