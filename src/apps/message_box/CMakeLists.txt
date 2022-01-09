
if(SIMULATOR)
    target_include_directories(epaper_display PUBLIC .)
    target_sources(epaper_display PUBLIC message_box_app.c)
else()
    idf_component_register(
            SRCS "message_box_app.c"
            INCLUDE_DIRS "."
    )
endif()