
target_sources(epaper_display PUBLIC croutine.c event_groups.c list.c queue.c stream_buffer.c tasks.c timers.c
        Posix/port.c MemMang/heap_4.c Posix/utils/wait_for_event.c)
target_include_directories(epaper_display PUBLIC ./include ./include/freertos ./Posix)