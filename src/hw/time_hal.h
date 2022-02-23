
#include <time.h>
#include <sys/time.h>

/*
 * For both the simulator target and ESP SDK, you can use the following stlib functions:
 *  gettimeofday
 *  settimeofday
 *  time
 *  asctime
 *  clock
 *  ctime
 *  difftime
 *  gmtime
 *  localtime
 *  mktime
 *  strftime
 *  adjtime
 */

// Saves time to RAM or registers. Lightweight and can be called frequently.
void time_backup_volatile(time_t time);
time_t time_get_volatile(void);

// Saves time to flash. Relatively heavyweight, so should be done once every minute or less.
// Should be used to restore if the volatile time is not valid.
void time_backup_nonvolatile(time_t time);
time_t time_get_nonvolatile(void);