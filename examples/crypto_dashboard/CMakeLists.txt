cmake_minimum_required(VERSION 3.20)

project(crypto_dashboard
    VERSION 1.0.0
    DESCRIPTION "Cryptocurrency Dashboard for e-Paper Display"
    LANGUAGES CXX)

# Require C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required dependencies
find_package(CURL REQUIRED)
find_package(nlohmann_json 3.0.0 REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/types.cpp
    src/http_client.cpp
    src/crypto_api.cpp
    src/dashboard_renderer.cpp
)

# Create executable
add_executable(crypto_dashboard ${SOURCES})

# Include directories
target_include_directories(crypto_dashboard
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include  # For epaper headers
)

# Link libraries
target_link_libraries(crypto_dashboard
    PRIVATE
        epaper
        CURL::libcurl
        nlohmann_json::nlohmann_json
)

# Compiler warnings
target_compile_options(crypto_dashboard
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Installation
install(TARGETS crypto_dashboard
    RUNTIME DESTINATION bin
)

# Print configuration
message(STATUS "Crypto Dashboard Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  CURL Found: ${CURL_FOUND}")
message(STATUS "  nlohmann_json Found: ${nlohmann_json_FOUND}")

