//
// Created by Samuel Jones on 3/11/22.
//

#ifndef EPAPER_DISPLAY_EPAPER_HAL_DEFINITIONS_H
#define EPAPER_DISPLAY_EPAPER_HAL_DEFINITIONS_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    EPAPER_WAIT_UNTIL_READY = 0,
    EPAPER_RESET,
    EPAPER_SEND_COMMAND,
    EPAPER_WRITE_OLD_DATA,
    EPAPER_WRITE_NEW_DATA,
    EPAPER_WAIT_TIME,
    EPAPER_COMMAND_MAX,
} EPAPER_HAL_OP_ID;

typedef struct {
    EPAPER_HAL_OP_ID id;
    const uint8_t* data;
    size_t length;
} EPAPER_HAL_OP;

#define EPAPER_RESET_OP(hold_time) {EPAPER_RESET, NULL, hold_time}
#define EPAPER_WAIT_TIME_OP(time) {EPAPER_WAIT_TIME, NULL, time}
#define EPAPER_WAIT_UNTIL_READY_OP() {EPAPER_WAIT_UNTIL_READY, NULL, 0}
#define EPAPER_COMMAND_OP_ARRAY(array) EPAPER_COMMAND_OP_PTR(array, sizeof(array))
#define EPAPER_COMMAND_OP_PTR(ptr, length) {EPAPER_SEND_COMMAND, ptr, length}
#define EPAPER_END_OP() {EPAPER_COMMAND_MAX, NULL, 0}
#define EPAPER_WRITE_OLD_DATA_OP() {EPAPER_WRITE_OLD_DATA, NULL, 0}
#define EPAPER_WRITE_NEW_DATA_OP() {EPAPER_WRITE_NEW_DATA, NULL, 0}


typedef struct {
    int spi_speed_hz;
    int width;
    int height;
    EPAPER_HAL_OP * full_refresh_operation;
} EPAPER_SPI_HAL_CONFIG;

#endif //EPAPER_DISPLAY_EPAPER_HAL_DEFINITIONS_H
