
#include "epapers.h"
#include "7p5inch_800x480_bw.h"
#include <string.h>

typedef enum {
    DISPLAY_7P5INCH_800X400_BW = 0,
    DISPLAY_MAX
} SUPPORTED_DISPLAYS;

static const char *EPAPER_DISPLAY_NAMES[DISPLAY_MAX] = {
        "DISPLAY_7P5INCH_800X400_BW",
};

static const EPAPER_SPI_HAL_CONFIG *EPAPER_DISPLAY_CONFIGS[DISPLAY_MAX] = {
        &g_7p5inch_800x480_bw,
};


const EPAPER_SPI_HAL_CONFIG * epaper_get_config_for_name(const char* name) {
    for (int i=0; i<DISPLAY_MAX; i++) {
        if (0 == strcmp(name, EPAPER_DISPLAY_NAMES[i])) {
            return EPAPER_DISPLAY_CONFIGS[i];
        }
    }
    return NULL;
}
