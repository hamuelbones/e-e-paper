
#include "time_hal.h"


void time_backup_volatile(time_t time) {
}

time_t time_get_volatile(void) {
    time_t raw_time;
    time ( &raw_time );
    return raw_time;
}

void time_backup_nonvolatile(time_t time) {

}
time_t time_get_nonvolatile(void) {
    time_t raw_time;
    time ( &raw_time );
    return raw_time;
}