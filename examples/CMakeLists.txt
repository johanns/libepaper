cmake_minimum_required(VERSION 3.25)

# Find required libraries
find_package(CURL REQUIRED)
find_package(nlohmann_json 3.0.0 REQUIRED)

# Add example subdirectories
add_subdirectory(save_buffer_to_image)
add_subdirectory(crypto_dashboard)
add_subdirectory(recovery)

# Create umbrella target to build all examples
add_custom_target(examples DEPENDS save_buffer_to_image crypto_dashboard recover_device)
