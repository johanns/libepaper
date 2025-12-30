cmake_minimum_required(VERSION 3.25)

# BMP Debug Example
# Demonstrates saving framebuffer to BMP for debugging layouts

add_executable(bmp_debug_example
  src/main.cpp
)

target_link_libraries(bmp_debug_example PRIVATE
  epaper
)
