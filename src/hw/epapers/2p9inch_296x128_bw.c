//
// Created by Samuel Jones on 3/11/22.
//

#include "2p9inch_296x128_bw.h"

//
// Created by Samuel Jones on 3/11/22.
//


const uint8_t epaper2p9bw_power_setting[] = {0x01, 0x17, 0x17, 0x3f, 0x3f, 0x11};
const uint8_t epaper2p9bw_vcom_dc_setting[] = {0x82, 0x24};
const uint8_t epaper2p9bw_booster_setting[] = {0x06, 0x27, 0x27, 0x2f, 0x17};
const uint8_t epaper2p9bw_power_on[] =  {0x04};
const uint8_t epaper2p9bw_panel_setting[] = {0x00, 0x3f};
const uint8_t epaper2p9bw_tres_setting[] = {0x61, 0x03, 0x20, 0x01, 0xe0};
const uint8_t epaper2p9bw_other_setting[] = {0x15, 0x00};
const uint8_t epaper2p9bw_vcom_data_interval_setting[] = {0x50, 0x10, 0x00};
const uint8_t epaper2p9bw_tcon_setting[] = {0x60, 0x22};
const uint8_t epaper2p9bw_resolution_setting[] = {0x65, 0x00, 0x00, 0x00};
const uint8_t epaper2p9bw_lut20[] = {0x20,
                                     0x0,	0xF,	0xF,	0x0,	0x0,	0x1,
                                     0x0,	0xF,	0x1,	0xF,	0x1,	0x2,
                                     0x0,	0xF,	0xF,	0x0,	0x0,	0x1,
                                     0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
                                     0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
                                     0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
                                     0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
};
const uint8_t epaper2p9bw_lut21[] = {0x21,
                                     0x10,	0xF,	0xF,	0x0,	0x0,	0x1,
                                     0x84,	0xF,	0x1,	0xF,	0x1,	0x2,
                                     0x20,	0xF,	0xF,	0x0,	0x0,	0x1,
                                     0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
                                     0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
                                     0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
                                     0x0,	0x0,	0x0,	0x0,	0x0,	0x0};

const uint8_t epaper2p9bw_lut22[] = {0x22,
                                     0x10,	0xF,	0xF,	0x0,	0x0,	0x1,
                                     0x84,	0xF,	0x1,	0xF,	0x1,	0x2,
                                     0x20,	0xF,	0xF,	0x0,	0x0,	0x1,
                                     0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
                                     0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
                                     0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
                                     0x0,	0x0,	0x0,	0x0,	0x0,	0x0};

const uint8_t epaper2p9bw_lut23[] = {0x23,
                                     0x80,	0xF,	0xF,	0x0,	0x0,	0x1,
                                     0x84,	0xF,	0x1,	0xF,	0x1,	0x2,
                                     0x40,	0xF,	0xF,	0x0,	0x0,	0x1,
                                     0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
                                     0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
                                     0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
                                     0x0,	0x0,	0x0,	0x0,	0x0,	0x0
};
const uint8_t epaper2p9bw_lut24[] =  {0x24,
                                      0x80,	0xF,	0xF,	0x0,	0x0,	0x1,
                                      0x84,	0xF,	0x1,	0xF,	0x1,	0x2,
                                      0x40,	0xF,	0xF,	0x0,	0x0,	0x1,
                                      0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
                                      0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
                                      0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
                                      0x0,	0x0,	0x0,	0x0,	0x0,	0x0
};
const uint8_t epaper2p9bw_old_data_command[] = {0x13};
const uint8_t epaper2p9bw_new_data_command[] = {0x13};
const uint8_t epaper2p9bw_refresh_command[] = {0x12};
const uint8_t epaper2p9bw_power_off_command[] = {0x02};
const uint8_t epaper2p9bw_deep_sleep_command[] =  {0x07, 0xa5};

const EPAPER_HAL_OP epaper2p9bw_full_refresh[] = {
        EPAPER_RESET_OP(5),
        EPAPER_WAIT_TIME_OP(20),
        EPAPER_WAIT_UNTIL_READY_OP(),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_power_setting),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_vcom_dc_setting),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_booster_setting),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_power_on),
        EPAPER_WAIT_TIME_OP(10),
        EPAPER_WAIT_UNTIL_READY_OP(),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_panel_setting),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_tres_setting),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_other_setting),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_vcom_data_interval_setting),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_tcon_setting),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_resolution_setting),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_lut20),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_lut21),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_lut22),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_lut23),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_lut24),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_old_data_command),
        EPAPER_WRITE_OLD_DATA_OP(),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_new_data_command),
        EPAPER_WRITE_NEW_DATA_OP(),
        EPAPER_WAIT_TIME_OP(10),
        EPAPER_WAIT_UNTIL_READY_OP(),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_refresh_command),
        EPAPER_WAIT_TIME_OP(10),
        EPAPER_WAIT_UNTIL_READY_OP(),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_power_off_command),
        EPAPER_COMMAND_OP_ARRAY(epaper2p9bw_deep_sleep_command),
        EPAPER_END_OP()
};

const EPAPER_SPI_HAL_CONFIG g_2p9inch_296x128_bw = {
        .spi_speed_hz = 10*1000*1000,
        .width = 296,
        .height = 128,
        .full_refresh_operation = (EPAPER_HAL_OP*) epaper2p9bw_full_refresh,
};