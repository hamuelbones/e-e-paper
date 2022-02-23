//
// Created by Samuel Jones on 2/18/22.
//

#include "time_hal.h"
#include "nvs.h"

void time_backup_volatile(time_t time) {
    // Handled internally...
}
time_t time_get_volatile(void) {
    // Handled internally...
    time_t raw_time;
    time ( &raw_time );
    return raw_time;
}

void time_backup_nonvolatile(time_t time) {

    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }

    nvs_set_u32(my_handle, "unix_time", time);
    nvs_commit(my_handle);
    nvs_close(my_handle);
}

time_t time_get_nonvolatile(void) {

    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return 0;
    }
    time_t time = 0;

    nvs_get_u32(my_handle, "unix_time", (uint32_t*)&time);
    nvs_close(my_handle);
    return time;
}