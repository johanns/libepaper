#!/bin/bash

# run_all_tests.sh - Run all libepaper manual tests sequentially
#
# This script runs all test programs in sequence with optional pauses between tests.
# No sudo required - make sure you're in the gpio and spi groups.
#
# Usage:
#   ./run_all_tests.sh              # Run all tests with pauses
#   ./run_all_tests.sh --auto       # Run all tests without pauses
#   ./run_all_tests.sh --skip TEST  # Skip specific test
#   ./run_all_tests.sh TEST_NAME    # Run only specific test

set -e  # Exit on error

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test programs in order
TESTS=(
    "test_display_modes"
    "test_drawing_primitives"
    "test_orientations"
    "test_fonts"
    "test_bitmaps"
    "test_power_management"
    "test_auto_sleep"
    "test_edge_cases"
    "test_coordinate_transforms"
    "test_error_handling"
    "test_stress"
)

# Configuration
AUTO_MODE=false
SKIP_TEST=""
SINGLE_TEST=""
PAUSE_DURATION=3

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --auto)
            AUTO_MODE=true
            shift
            ;;
        --skip)
            SKIP_TEST="$2"
            shift 2
            ;;
        --help)
            echo "Usage: ./run_all_tests.sh [OPTIONS] [TEST_NAME]"
            echo ""
            echo "Options:"
            echo "  --auto           Run all tests without pauses"
            echo "  --skip TEST      Skip specific test"
            echo "  --help           Show this help message"
            echo ""
            echo "Available tests:"
            for test in "${TESTS[@]}"; do
                echo "  - $test"
            done
            echo ""
            echo "Examples:"
            echo "  ./run_all_tests.sh                    # Run all tests with pauses"
            echo "  ./run_all_tests.sh --auto             # Run all without pauses"
            echo "  ./run_all_tests.sh --skip test_stress # Skip stress test"
            echo "  ./run_all_tests.sh test_fonts         # Run only font test"
            echo ""
            echo "Note: Make sure you're in the gpio and spi groups (no sudo required)"
            exit 0
            ;;
        *)
            # Assume it's a test name
            SINGLE_TEST="$1"
            shift
            ;;
    esac
done

# Check if user is in required groups
if ! groups | grep -q "\bgpio\b"; then
   echo -e "${YELLOW}Warning: You may not be in the gpio group${NC}"
   echo "Add yourself with: sudo usermod -a -G gpio,spi $USER"
   echo "Then log out and back in."
   echo ""
fi
if ! groups | grep -q "\bspi\b"; then
   echo -e "${YELLOW}Warning: You may not be in the spi group${NC}"
   echo "Add yourself with: sudo usermod -a -G gpio,spi $USER"
   echo "Then log out and back in."
   echo ""
fi

# Function to print section header
print_header() {
    echo ""
    echo -e "${BLUE}=======================================${NC}"
    echo -e "${BLUE}  $1${NC}"
    echo -e "${BLUE}=======================================${NC}"
    echo ""
}

# Function to print test status
print_status() {
    local status=$1
    local message=$2

    case $status in
        "pass")
            echo -e "${GREEN}✓ PASSED${NC}: $message"
            ;;
        "fail")
            echo -e "${RED}✗ FAILED${NC}: $message"
            ;;
        "skip")
            echo -e "${YELLOW}⊘ SKIPPED${NC}: $message"
            ;;
        "info")
            echo -e "${BLUE}ℹ INFO${NC}: $message"
            ;;
    esac
}

# Function to pause between tests
pause_between_tests() {
    if [[ "$AUTO_MODE" == false ]]; then
        echo ""
        echo -e "${YELLOW}Press Enter to continue to next test...${NC}"
        read
    else
        echo ""
        echo -e "${YELLOW}Waiting $PAUSE_DURATION seconds before next test...${NC}"
        sleep $PAUSE_DURATION
    fi
}

# Function to run a single test
run_test() {
    local test_name=$1
    local test_path="./$test_name"

    # Check if test should be skipped
    if [[ "$test_name" == "$SKIP_TEST" ]]; then
        print_status "skip" "$test_name"
        return 0
    fi

    # Check if test executable exists
    if [[ ! -f "$test_path" ]]; then
        print_status "fail" "$test_name - Executable not found"
        return 1
    fi

    # Check if test is executable
    if [[ ! -x "$test_path" ]]; then
        print_status "fail" "$test_name - Not executable (run: chmod +x $test_path)"
        return 1
    fi

    print_header "Running: $test_name"

    # Run the test
    if $test_path; then
        print_status "pass" "$test_name"
        return 0
    else
        print_status "fail" "$test_name"
        return 1
    fi
}

# Main execution
main() {
    print_header "libepaper Manual Test Suite"

    echo "Test suite version: 1.0"
    echo "Date: $(date)"
    echo ""

    if [[ "$AUTO_MODE" == true ]]; then
        echo -e "${YELLOW}Running in AUTO mode (no manual pauses)${NC}"
    fi

    if [[ -n "$SKIP_TEST" ]]; then
        echo -e "${YELLOW}Skipping test: $SKIP_TEST${NC}"
    fi

    if [[ -n "$SINGLE_TEST" ]]; then
        echo -e "${YELLOW}Running single test: $SINGLE_TEST${NC}"
    fi

    echo ""

    # Statistics
    local total_tests=0
    local passed_tests=0
    local failed_tests=0
    local skipped_tests=0

    # Determine which tests to run
    local tests_to_run=()
    if [[ -n "$SINGLE_TEST" ]]; then
        tests_to_run=("$SINGLE_TEST")
    else
        tests_to_run=("${TESTS[@]}")
    fi

    # Run tests
    for test in "${tests_to_run[@]}"; do
        ((total_tests++))

        if [[ "$test" == "$SKIP_TEST" ]]; then
            ((skipped_tests++))
            print_status "skip" "$test"
            continue
        fi

        if run_test "$test"; then
            ((passed_tests++))
        else
            ((failed_tests++))

            # Ask if user wants to continue after failure
            if [[ "$AUTO_MODE" == false ]]; then
                echo ""
                echo -e "${YELLOW}Test failed. Continue with remaining tests? (y/n)${NC}"
                read -r response
                if [[ ! "$response" =~ ^[Yy]$ ]]; then
                    echo "Test suite aborted."
                    break
                fi
            fi
        fi

        # Pause between tests (except after last test)
        if [[ "$test" != "${tests_to_run[-1]}" ]]; then
            pause_between_tests
        fi
    done

    # Print summary
    print_header "Test Suite Summary"

    echo "Total tests:   $total_tests"
    echo -e "Passed tests:  ${GREEN}$passed_tests${NC}"
    if [[ $failed_tests -gt 0 ]]; then
        echo -e "Failed tests:  ${RED}$failed_tests${NC}"
    else
        echo "Failed tests:  0"
    fi
    if [[ $skipped_tests -gt 0 ]]; then
        echo -e "Skipped tests: ${YELLOW}$skipped_tests${NC}"
    else
        echo "Skipped tests: 0"
    fi

    echo ""

    if [[ $failed_tests -eq 0 ]]; then
        echo -e "${GREEN}✓ All tests passed!${NC}"
        exit 0
    else
        echo -e "${RED}✗ Some tests failed.${NC}"
        exit 1
    fi
}

# Run main function
main
