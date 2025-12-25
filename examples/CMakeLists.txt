cmake_minimum_required(VERSION 3.25)

# Demo executable
add_executable(epaper_demo demo.cpp)

target_link_libraries(epaper_demo
    PRIVATE
        epaper
)

# Installation
install(TARGETS epaper_demo
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

