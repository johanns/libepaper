cmake_minimum_required(VERSION 3.20)
project(libepaper VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source files
set(SOURCES
    src/device.cpp
    src/font.cpp
    src/framebuffers/mono_framebuffer.cpp
    src/framebuffers/multi_plane_framebuffer.cpp
    src/image_io.cpp
    third_party/fonts/font8.c
    third_party/fonts/font12.c
    third_party/fonts/font16.c
    third_party/fonts/font20.c
    third_party/fonts/font24.c
)

# Main Library (header-only + font sources)
add_library(libepaper ${SOURCES})
set_target_properties(libepaper PROPERTIES LINKER_LANGUAGE CXX)

# Include directories
target_include_directories(libepaper PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Dependencies
# We use libgpiod for GPIO (HAL)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)
target_link_libraries(libepaper PUBLIC ${GPIOD_LIBRARIES})
target_include_directories(libepaper PUBLIC ${GPIOD_INCLUDE_DIRS})

# Options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
