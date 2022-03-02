//
// Created by Samuel Jones on 3/1/22.
//

#ifndef EPAPER_DISPLAY_SYSTEM_RESOURCES_H
#define EPAPER_DISPLAY_SYSTEM_RESOURCES_H

void* system_load(const char* file_name);
void system_unload(void* table);
void* system_get_element(void* table, const char *key);

#endif //EPAPER_DISPLAY_SYSTEM_RESOURCES_H
