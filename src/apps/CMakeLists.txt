
add_subdirectory(message_box)

if (SIMULATOR)
    target_include_directories(epaper_display PUBLIC .)
endif()