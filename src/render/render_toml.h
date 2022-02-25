//
// Created by Samuel Jones on 2/24/22.
//

#ifndef EPAPER_DISPLAY_RENDER_TOML_H
#define EPAPER_DISPLAY_RENDER_TOML_H

#include "toml.h"
#include "display_buffer.h"

void render_toml(toml_table_t *render, DISPLAY_COORD dimensions);

#endif //EPAPER_DISPLAY_RENDER_TOML_H
