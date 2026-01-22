cmake_minimum_required(VERSION 3.25)

# Configure test driver selection
# Options: EPD27 (default, requires hardware), Mock (no hardware, for CI)
if(NOT DEFINED LIBEPAPER_DRIVER)
    set(LIBEPAPER_DRIVER "EPD27" CACHE STRING "Driver to use for tests")
endif()

message(STATUS "Test driver: ${LIBEPAPER_DRIVER}")

# Test executables - each test is a standalone program
set(TEST_PROGRAMS
    test_display_modes
    test_drawing_primitives
    test_styles
    test_capabilities
    test_orientations
    test_fonts
    test_bitmaps
    test_power_management
    test_auto_sleep
    test_edge_cases
    test_coordinate_transforms
    test_error_handling
    test_stress
    test_color_management
    test_mock_color_modes
    test_final
    test_pure_mock_hal
)

# Create executable for each test program
foreach(TEST_PROGRAM ${TEST_PROGRAMS})
    add_executable(${TEST_PROGRAM} ${TEST_PROGRAM}.cpp)

    target_link_libraries(${TEST_PROGRAM}
        PRIVATE
            libepaper
    )

    # Pass driver selection to test code
    # Map driver name to numeric constant for preprocessor comparison
    if(LIBEPAPER_DRIVER STREQUAL "EPD27")
        set(DRIVER_VALUE "DRIVER_EPD27")
    elseif(LIBEPAPER_DRIVER STREQUAL "Mock")
        set(DRIVER_VALUE "DRIVER_MOCK")
    else()
        message(FATAL_ERROR "Unknown LIBEPAPER_DRIVER: ${LIBEPAPER_DRIVER}")
    endif()

    target_compile_definitions(${TEST_PROGRAM}
        PRIVATE
            TEST_DRIVER=${DRIVER_VALUE}
    )

    # Fix RPATH for Ninja generator
    set_target_properties(${TEST_PROGRAM} PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
        BUILD_WITH_INSTALL_RPATH TRUE
    )

    # Install test executables (optional)
    install(TARGETS ${TEST_PROGRAM}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
    )
endforeach()

# Create umbrella target to build all tests
add_custom_target(tests DEPENDS ${TEST_PROGRAMS})

# Install test script
install(PROGRAMS run_all_tests.sh
    DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)

# Install test documentation
install(FILES README.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/tests
)
