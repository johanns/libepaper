cmake_minimum_required(VERSION 3.25)
find_package(Threads REQUIRED)

add_executable(recover_device main.cpp)

target_link_libraries(recover_device PRIVATE
    libepaper
    Threads::Threads
)

target_compile_features(recover_device PRIVATE cxx_std_23)

set_target_properties(recover_device PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples/recovery"
)
