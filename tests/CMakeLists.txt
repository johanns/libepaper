cmake_minimum_required(VERSION 3.25)

# Test executables - each test is a standalone program
set(TEST_PROGRAMS
    test_display_modes
    test_drawing_primitives
    test_orientations
    test_fonts
    test_bitmaps
    test_power_management
    test_auto_sleep
    test_edge_cases
    test_coordinate_transforms
    test_error_handling
    test_stress
)

# Create executable for each test program
foreach(TEST_PROGRAM ${TEST_PROGRAMS})
    add_executable(${TEST_PROGRAM} ${TEST_PROGRAM}.cpp)

    target_link_libraries(${TEST_PROGRAM}
        PRIVATE
            epaper
    )

    # Install test executables (optional)
    install(TARGETS ${TEST_PROGRAM}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
    )
endforeach()
