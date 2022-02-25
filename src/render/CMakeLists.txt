if(SIMULATOR)
    target_include_directories(epaper_display PUBLIC .)
    target_sources(epaper_display PUBLIC render_toml.c)
else()
    idf_component_register(
            SRCS "render_toml.c"
            INCLUDE_DIRS "."
            REQUIRES font display tomlc99
    )
endif()