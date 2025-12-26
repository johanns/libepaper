cmake_minimum_required(VERSION 3.25)

# Find required libraries
find_package(CURL REQUIRED)
find_package(nlohmann_json 3.0.0 REQUIRED)

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

# Crypto dashboard executable
add_executable(crypto_dashboard crypto_dashboard.cpp)

target_link_libraries(crypto_dashboard
    PRIVATE
        epaper
        CURL::libcurl
        nlohmann_json::nlohmann_json
)

# Installation
install(TARGETS epaper_demo bitmap_example crypto_dashboard
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

