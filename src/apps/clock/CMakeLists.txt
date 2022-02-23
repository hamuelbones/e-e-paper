
if(SIMULATOR)
    target_include_directories(epaper_display PUBLIC .)
    target_sources(epaper_display PUBLIC clock_app.c)
else()
    idf_component_register(
            SRCS "clock_app.c"
            INCLUDE_DIRS "." "../"
            REQUIRES hw display tomlc99 font utils
    )
endif()