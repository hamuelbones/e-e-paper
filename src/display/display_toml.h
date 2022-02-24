//
// Created by Samuel Jones on 2/23/22.
//

#ifndef EPAPER_DISPLAY_DISPLAY_TOML_H
#define EPAPER_DISPLAY_DISPLAY_TOML_H

#include "display_buffer.h"
#include "toml.h"

void display_toml(toml_table_t *render_root, DISPLAY_COORD upper_left, DISPLAY_COORD lower_right);

#endif //EPAPER_DISPLAY_DISPLAY_TOML_H
