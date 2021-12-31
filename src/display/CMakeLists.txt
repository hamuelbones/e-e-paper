if(SIMULATOR)
    target_include_directories(epaper_display PUBLIC .)
    target_sources(epaper_display PUBLIC display_buffer.c display_draw_geometry.c display_draw_image.c display_draw_text.c)
else()
    idf_component_register(
            SRCS "display_buffer.c" "display_draw_geometry.c" "display_draw_image.c" "display_draw_text.c"
            INCLUDE_DIRS "."
            REQUIRES font
    )
endif()