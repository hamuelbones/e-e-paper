
if(SIMULATOR)
    set(CMAKE_VERBOSE_MAKEFILE ON)
    add_executable(epaper_display epaper_display_main.c epaper_display_simulator_main.c wifi_task.c)
    target_include_directories(epaper_display PUBLIC .)

    # Use the package PkgConfig to detect GTK+ headers/library files
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    target_link_libraries(epaper_display PRIVATE ${GTK3_LIBRARIES})
    # Add other flags to the compiler
    target_compile_definitions(epaper_display PRIVATE ${GTK3_CFLAGS_OTHER})
    # Setup CMake to use GTK+, tell the compiler where to look for headers
    # and to the linker where to look for libraries
    target_include_directories(epaper_display PRIVATE ${GTK3_INCLUDE_DIRS})
    target_link_directories(epaper_display PRIVATE ${GTK3_LIBRARY_DIRS})

    pkg_check_modules(OPENSSL REQUIRED openssl)
    target_link_libraries(epaper_display PRIVATE ${OPENSSL_LIBRARIES})
    target_compile_definitions(epaper_display PRIVATE ${OPENSSL_CFLAGS_OTHER})
    target_include_directories(epaper_display PRIVATE ${OPENSSL_INCLUDE_DIRS})
    target_link_directories(epaper_display PRIVATE ${OPENSSL_LIBRARY_DIRS})


    add_subdirectory(display)
    add_subdirectory(freertos)
    add_subdirectory(font)
    add_subdirectory(hw)
    add_subdirectory(lib)
    add_subdirectory(render)
    add_subdirectory(utils)
else()
    idf_component_register(
            SRCS "epaper_display_main.c" "wifi_task.c"
            INCLUDE_DIRS "."
            REQUIRES font display hw tomlc99 utils freertos resources render
    )
endif()
