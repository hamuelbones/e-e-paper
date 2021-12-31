if(SIMULATOR)
    target_include_directories(epaper_display PUBLIC .)
    target_sources(epaper_display PUBLIC fonts.c)
else()
    idf_component_register(
            SRCS "fonts.c"
            INCLUDE_DIRS "."
    )
endif()
