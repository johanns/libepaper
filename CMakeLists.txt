cmake_minimum_required(VERSION 3.25)
project(libepaper VERSION 1.0.0 LANGUAGES CXX C)

# Export compile commands for IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# C11 for font files
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Werror=return-type
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Debug>:-g>
    )
endif()

# Find libgpiod library
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBGPIOD REQUIRED libgpiod)
if(NOT LIBGPIOD_FOUND)
    message(FATAL_ERROR "libgpiod not found. Install with: sudo apt-get install libgpiod-dev")
endif()

# Library sources
set(EPAPER_SOURCES
    src/device.cpp
    src/display.cpp
    src/drivers/epd27.cpp
    src/font.cpp
)

# Font C files
set(FONT_SOURCES
    fonts/font8.c
    fonts/font12.c
    fonts/font16.c
    fonts/font20.c
    fonts/font24.c
)

# Create static library
add_library(libepaper STATIC
    ${EPAPER_SOURCES}
    ${FONT_SOURCES}
)

target_include_directories(libepaper
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/fonts
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
        ${LIBGPIOD_INCLUDE_DIRS}
)

target_link_libraries(libepaper
    PUBLIC
        ${LIBGPIOD_LIBRARIES}
        m
)

# Public headers for installation
set(EPAPER_PUBLIC_HEADERS
    include/epaper/device.hpp
    include/epaper/display.hpp
)

# Set library properties
# Keep output artifact name as libepaper.* even though target is libepaper
set_target_properties(libepaper PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${EPAPER_PUBLIC_HEADERS}"
    OUTPUT_NAME epaper
)

# Add examples subdirectory
add_subdirectory(examples)
add_subdirectory(tests)

# Installation rules
include(GNUInstallDirs)

install(TARGETS libepaper
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/epaper
)

install(DIRECTORY include/epaper
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

# Find Doxygen
find_package(Doxygen)

if(DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()
