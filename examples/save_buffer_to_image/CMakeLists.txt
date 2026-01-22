cmake_minimum_required(VERSION 3.25)

# Save Buffer to Image Example
# Demonstrates saving framebuffer to PNG for debugging layouts

add_executable(save_buffer_to_image
  src/main.cpp
)

target_link_libraries(save_buffer_to_image PRIVATE
  libepaper
)
