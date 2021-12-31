//
// Created by Samuel Jones on 12/29/21.
//

#include <stdint.h>
#include <stddef.h>

#ifndef EPAPER_DISPLAY_EPAPER_HAL_H
#define EPAPER_DISPLAY_EPAPER_HAL_H

void EPAPER_Init(void);
void EPAPER_RenderBuffer(const uint8_t *buffer, const uint8_t *old_buffer, size_t buffer_size);

#endif //EPAPER_DISPLAY_EPAPER_HAL_H
