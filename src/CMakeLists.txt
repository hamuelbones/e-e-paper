idf_component_register(
        SRCS "epaper_display_main.c" "bluetooth_task.c" "display_task.c" "wifi_task.c"
        INCLUDE_DIRS "."
        REQUIRES font display spi_flash nvs_flash
)

