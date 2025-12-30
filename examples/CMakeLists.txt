cmake_minimum_required(VERSION 3.25)

# Find required libraries
find_package(CURL REQUIRED)
find_package(nlohmann_json 3.0.0 REQUIRED)

# Add example subdirectories
add_subdirectory(bmp_debug_example)
add_subdirectory(crypto_dashboard)
