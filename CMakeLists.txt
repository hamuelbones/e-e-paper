# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.20)

if(SIMULATOR)
    project(epaper_display)
    add_subdirectory(src)
else()
    set(EXTRA_COMPONENT_DIRS src src/display src/font src/hw src/lib/tomlc99 src/utils src/utils/resources src/render)
    set(IDF_TARGET esp32c3)
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(epaper_display)
endif()