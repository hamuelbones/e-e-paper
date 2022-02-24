//
// Created by Samuel Jones on 1/22/22.
//
// toml_resources - loading and unloading toml files into RAM

#ifndef EPAPER_DISPLAY_TOML_RESOURCES_H
#define EPAPER_DISPLAY_TOML_RESOURCES_H

void* toml_load(const char* file_name);
void toml_unload(void* table);

#endif //EPAPER_DISPLAY_TOML_RESOURCES_H
