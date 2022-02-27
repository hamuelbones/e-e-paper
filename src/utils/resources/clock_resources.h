//
// Created by Samuel Jones on 2/26/22.
//

#ifndef EPAPER_DISPLAY_CLOCK_RESOURCES_H
#define EPAPER_DISPLAY_CLOCK_RESOURCES_H

void* clock_load(const char* dummy);
void clock_unload(void* dummy);
void* clock_get_element(void* dummy, const char *key);


#endif //EPAPER_DISPLAY_CLOCK_RESOURCES_H
