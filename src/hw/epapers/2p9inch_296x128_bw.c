//
// Created by Samuel Jones on 3/11/22.
//
// based on datasheet

#include "2p9inch_296x128_bw.h"


const uint8_t epaper2p9bw_booster_soft_start[] = {0x06, 0x17, 0x17, 0x17};
const uint8_t epaper2p9bw_power_setting[] = {0x01, 0x03, 0x00, 0x2b, 0x2b, 0x09};
const uint8_t epaper2p9bw_power_on[] = {0x04};
// Check pin
const uint8_t epaper2p9bw_panel_setting[] = {0x00,0x1f};
const uint8_t epaper2p9bw_pll_control[] = {0x30,0x3a};
const uint8_t epaper2p9bw_resolution[] = {0x61,0x80,0x01,0x28};
const uint8_t epaper2p9bw_vcm_dc[] = {0x82,0x12};
const uint8_t epaper2p9bw_vcom_data_interval[] = {0x50,0x97};
const uint8_t epaper2p9bw_lut_full[] = {0x20, 0xAA, 0x55, 0xAA, 0x11, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xFF, 0xFF, 0x1F, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const uint8_t epaper2p9bw_lut_partial[] = {0x10, 0x18, 0x18, 0x08, 0x18, 0x18,
        0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x13, 0x14, 0x44, 0x12,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


const uint8_t epaper2p9bw_old_data_command[] = {0x10};
const uint8_t epaper2p9bw_new_data_command[] = {0x13};

const uint8_t epaper2p9bw_refresh[] = {0x12};
// busy pin
const uint8_t epaper2p9bw_vcom_data_interval_off[] = {0x50, 0x00};
const uint8_t epaper2p9bw_power_off[] = {0x02};
const uint8_t epaper2p9bw_deep_sleep[] = {0x07, 0xa5};

const EPAPER_HAL_OP epaper2p9bw_full_refresh[] = {
        EPAPER_RESET_OP(2),
        EPAPER_WAIT_TIME_OP(50),
        EPAPER_WAIT_UNTIL_READY_OP(),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_booster_soft_start),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_power_setting),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_power_on),
        EPAPER_WAIT_UNTIL_READY_OP(),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_panel_setting),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_pll_control),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_resolution),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_vcm_dc),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_vcom_data_interval),
        //EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_lut_full),

        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_old_data_command),
        EPAPER_WRITE_NEW_DATA_OP(),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_new_data_command),
        EPAPER_WRITE_NEW_DATA_OP(),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_refresh),
        EPAPER_WAIT_TIME_OP(10),
        EPAPER_WAIT_UNTIL_READY_OP(),
        //EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_vcom_data_interval_off),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_power_off),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_deep_sleep),
        EPAPER_END_OP()
};

const EPAPER_SPI_HAL_CONFIG g_2p9inch_296x128_bw = {
        .spi_speed_hz = 10*1000*1000,
        .width = 296,
        .height = 128,
        .bw_inverted = true,
        .xy_flipped = true,
        .full_refresh_operation = (EPAPER_HAL_OP*) epaper2p9bw_full_refresh,
};