cmake_minimum_required(VERSION 3.25)

# Demo executable
add_executable(epaper_demo demo.cpp)

target_link_libraries(epaper_demo
    PRIVATE
        epaper
)

# Bitmap example executable
add_executable(bitmap_example bitmap_example.cpp)

target_link_libraries(bitmap_example
    PRIVATE
        epaper
)

# Installation
install(TARGETS epaper_demo bitmap_example
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

