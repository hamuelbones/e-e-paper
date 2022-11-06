
if(SIMULATOR)
    target_include_directories(epaper_display PUBLIC .)
    target_sources(epaper_display PUBLIC
            epaper_hal_simulator.c
            epapers/7p5inch_800x480_bw.c epapers/2p9inch_296x128_bw.c epapers/epapers.c
            filesystem_hal_simulator.c
            cryptography_hal_simulator.c
            init_hal_simulator.c
            time_hal_simulator.c
            wifi_hal_simulator.c)
else()
    idf_component_register(
            SRCS "epaper_hal_esp32c.c" "epapers/7p5inch_800x480_bw.c" "epapers/2p9inch_296x128_bw.c" "epapers/epapers.c"
                 "wifi_hal_esp32c.c"
                 "init_hal_esp32c.c"
                 "i2s_microphone_hal_esp32c.c"
                 "i2s_speaker_hal_esp32c.c"
                 "filesystem_hal_esp32c_sd.c"
                 "cryptography_hal_esp32c.c"
                 "time_hal_esp32c.c"
            INCLUDE_DIRS "."
            REQUIRES spi_flash nvs_flash fatfs esp_littlefs esp_http_client esp_https_ota app_update lwip mbedtls esp_pm freertos driver esp_wifi esp_event esp_hw_support
    )
endif()