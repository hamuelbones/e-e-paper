
if(SIMULATOR)
    target_include_directories(epaper_display PUBLIC .)
    target_sources(epaper_display PUBLIC base64.c jwt.c)
else()
    idf_component_register(
            SRCS "base64.c" "jwt.c"
            INCLUDE_DIRS "."
    )
endif()