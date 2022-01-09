
if(SIMULATOR)
    target_include_directories(epaper_display PUBLIC .)
    target_sources(epaper_display PUBLIC hal_simulator.c filesystem_hal_simulator.c)
else()
    idf_component_register(
            SRCS "epaper_hal_esp32c.c" "wifi_hal_esp32c.c" "init_hal_esp32c.c" "filesystem_hal_esp32c_sd.c"
            INCLUDE_DIRS "."
            REQUIRES spi_flash nvs_flash
    )
endif()