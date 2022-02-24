
if(SIMULATOR)
    target_include_directories(epaper_display PUBLIC .)
    target_sources(epaper_display PUBLIC base64.c jwt.c file_utils.c toml_resources.c font_resources.c resources.c)
else()
    idf_component_register(
            SRCS "base64.c" "jwt.c" "file_utils.c" "toml_resources.c" "font_resources.c" "resources.c"
            INCLUDE_DIRS "."
            REQUIRES tomlc99 hw font
    )
endif()