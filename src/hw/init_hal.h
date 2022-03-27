//
// Created by Samuel Jones on 12/29/21.
//

#ifndef EPAPER_DISPLAY_INIT_HAL_H
#define EPAPER_DISPLAY_INIT_HAL_H


#if (!HARDWARE_VER)

#define GPIO_CHG_EN (GPIO_NUM_4)
#define GPIO_CHG_PGOOD (GPIO_NUM_7)
#define GPIO_CHG_STAT1 (GPIO_NUM_5)
#define GPIO_CHG_STAT2 (GPIO_NUM_6)

#define SPI_SCK (GPIO_NUM_0)
#define SPI_MOSI (GPIO_NUM_1)
#define SPI_MISO (GPIO_NUM_2)

#define BUTTON_0 (GPIO_NUM_10)

#elif (HARDWARE_VER == 2)

#define BUTTON_0 (GPIO_NUM_0)
#define BUTTON_1 (GPIO_NUM_1)
#define BUTTON_2 (GPIO_NUM_2)
#define BUTTON_3 (GPIO_NUM_3)

#define BATT_MEAS (GPIO_NUM_4)

#define SPI_SCK (GPIO_NUM_7)
#define SPI_MOSI (GPIO_NUM_5)
#define SPI_MISO (GPIO_NUM_6)

#endif


void app_hal_init(void);

void app_hal_reboot(void);

const char* app_hal_version(void);

const char* app_hal_name(void);

void app_yield_spi_bus(void);

void app_resume_spi_bus(void);

#endif //EPAPER_DISPLAY_INIT_HAL_H
