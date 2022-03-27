//
// Created by Samuel Jones on 12/29/21.
//

#include <stdint.h>
#include <stddef.h>
#include "epaper_hal_definitions.h"

#ifndef EPAPER_DISPLAY_EPAPER_HAL_H
#define EPAPER_DISPLAY_EPAPER_HAL_H

void epaper_init(const EPAPER_SPI_HAL_CONFIG* config);
const EPAPER_SPI_HAL_CONFIG* epaper_config(void);
void epaper_render_buffer(const EPAPER_SPI_HAL_CONFIG* config, const uint8_t *buffer, const uint8_t *old_buffer, size_t buffer_size);
void epaper_deinit(void);

#endif //EPAPER_DISPLAY_EPAPER_HAL_H
