
if (SIMULATOR)
    add_subdirectory(message_box)
    target_include_directories(epaper_display PUBLIC .)
endif()