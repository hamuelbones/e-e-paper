
if(SIMULATOR)
    target_include_directories(epaper_display PUBLIC .)
    target_sources(epaper_display PUBLIC toml.c)
else()
    idf_component_register(
            SRCS "toml.c"
            INCLUDE_DIRS "."
    )
endif()