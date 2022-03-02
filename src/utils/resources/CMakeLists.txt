

if(SIMULATOR)
    target_include_directories(epaper_display PUBLIC .)
    target_sources(epaper_display PUBLIC toml_resources.c font_resources.c resources.c clock_resources.c system_resources.c)
else()
    idf_component_register(
    SRCS "toml_resources.c" "font_resources.c" "resources.c" "clock_resources.c" "system_resources.c"
    INCLUDE_DIRS "."
    REQUIRES tomlc99 hw font freertos
    )
endif()