#include "epaper/device.hpp"
#include "epaper/draw.hpp"
#include "epaper/drivers/epd27.hpp"
#include "epaper/font.hpp"
#include "epaper/screen.hpp"
#include <cstdlib>
#include <expected>
#include <iostream>
#include <thread>

using namespace epaper;

auto demo_black_white(Screen& screen, Draw& draw) -> void {
    std::cout << "Running Black & White demo...\n";

    // Clear screen
    screen.clear(Color::White);

    // Draw points
    draw.draw_point(10, 80, Color::Black, DotPixel::Pixel1x1);
    draw.draw_point(10, 90, Color::Black, DotPixel::Pixel2x2);
    draw.draw_point(10, 100, Color::Black, DotPixel::Pixel3x3);

    // Draw lines
    draw.draw_line(20, 70, 70, 120, Color::Black,
                  DotPixel::Pixel1x1, LineStyle::Solid);
    draw.draw_line(70, 70, 20, 120, Color::Black,
                  DotPixel::Pixel1x1, LineStyle::Solid);

    // Draw rectangles
    draw.draw_rectangle(20, 70, 70, 120, Color::Black,
                       DotPixel::Pixel1x1, DrawFill::Empty);
    draw.draw_rectangle(80, 70, 130, 120, Color::Black,
                       DotPixel::Pixel1x1, DrawFill::Full);

    // Draw circles
    draw.draw_circle(45, 95, 20, Color::Black,
                    DotPixel::Pixel1x1, DrawFill::Empty);
    draw.draw_circle(105, 95, 20, Color::White,
                    DotPixel::Pixel1x1, DrawFill::Full);

    // Draw dotted line
    draw.draw_line(85, 95, 125, 95, Color::Black,
                  DotPixel::Pixel1x1, LineStyle::Dotted);
    draw.draw_line(105, 75, 105, 115, Color::Black,
                  DotPixel::Pixel1x1, LineStyle::Dotted);

    // Draw text
    draw.draw_string(10, 0, "Waveshare", Font::font16(),
                    Color::Black, Color::White);
    draw.draw_string(10, 20, "Hello World", Font::font12(),
                    Color::White, Color::Black);

    // Draw numbers
    draw.draw_number(10, 33, 123456789, Font::font12(),
                    Color::Black, Color::White);
    draw.draw_number(10, 50, 987654321, Font::font16(),
                    Color::White, Color::Black);

    // Refresh display
    std::cout << "Refreshing display...\n";
    screen.refresh();

    std::cout << "Black & White demo complete.\n";
}

auto demo_grayscale(Screen& screen, Draw& draw) -> void {
    std::cout << "Running Grayscale demo...\n";

    // Clear screen
    screen.clear(Color::White);

    // Draw points in different gray levels
    draw.draw_point(10, 80, Color::Gray1, DotPixel::Pixel1x1);
    draw.draw_point(10, 90, Color::Gray1, DotPixel::Pixel2x2);
    draw.draw_point(10, 100, Color::Gray1, DotPixel::Pixel3x3);

    // Draw lines
    draw.draw_line(20, 70, 70, 120, Color::Gray1,
                  DotPixel::Pixel1x1, LineStyle::Solid);
    draw.draw_line(70, 70, 20, 120, Color::Gray1,
                  DotPixel::Pixel1x1, LineStyle::Solid);

    // Draw rectangles with different fills
    draw.draw_rectangle(20, 70, 70, 120, Color::Gray1,
                       DotPixel::Pixel1x1, DrawFill::Empty);
    draw.draw_rectangle(80, 70, 130, 120, Color::Gray1,
                       DotPixel::Pixel1x1, DrawFill::Full);

    // Draw circles
    draw.draw_circle(45, 95, 20, Color::Gray1,
                    DotPixel::Pixel1x1, DrawFill::Empty);
    draw.draw_circle(105, 95, 20, Color::Gray2,
                    DotPixel::Pixel1x1, DrawFill::Full);

    // Draw text with gray levels
    draw.draw_string(10, 0, "Waveshare", Font::font16(),
                    Color::Gray1, Color::White);
    draw.draw_string(10, 20, "Hello World", Font::font12(),
                    Color::Gray2, Color::White);

    // Draw numbers
    draw.draw_number(10, 33, 123456789, Font::font12(),
                    Color::Gray1, Color::Gray2);
    draw.draw_number(10, 50, 987654321, Font::font16(),
                    Color::White, Color::Gray1);

    // Draw decimal number
    draw.draw_decimal(10, 130, 3.14159, 3, Font::font12(),
                     Color::Gray1, Color::White);

    // Refresh display
    std::cout << "Refreshing display...\n";
    screen.refresh();

    std::cout << "Grayscale demo complete.\n";
}

auto main() -> int {
    try {
        std::cout << "Modern C++ E-Paper Display Demo\n";
        std::cout << "================================\n\n";

        // Initialize BCM2835 device
        std::cout << "Initializing BCM2835 device...\n";
        Device device;

        if (auto result = device.init(); !result) {
            std::cerr << "Failed to initialize device: "
                     << to_string(result.error()) << "\n";
            return EXIT_FAILURE;
        }

        std::cout << "Device initialized successfully.\n\n";

        // Create EPD27 driver
        std::cout << "Creating 2.7\" e-paper driver...\n";
        EPD27 epd27(device);

        // Black & White Mode Demo
        std::cout << "\n--- Black & White Mode ---\n";
        if (auto result = epd27.init(DisplayMode::BlackWhite); !result) {
            std::cerr << "Failed to initialize display: "
                     << to_string(result.error()) << "\n";
            return EXIT_FAILURE;
        }

        std::cout << "Display initialized (B/W mode).\n";
        std::cout << "Display size: " << epd27.width() << "x"
                 << epd27.height() << " pixels\n\n";

        // Clear the display
        std::cout << "Clearing display...\n";
        epd27.clear();
        std::cout << "Display cleared.\n\n";

        // Create screen and drawing interface
        Screen screen_bw(epd27, Orientation::Landscape90);
        Draw draw_bw(screen_bw);

        // Run black & white demo
        demo_black_white(screen_bw, draw_bw);

        // Wait before switching modes
        std::cout << "\nWaiting 3 seconds...\n";
        std::this_thread::sleep_for(std::chrono::seconds(3));

        // Grayscale Mode Demo
        std::cout << "\n--- 4-Level Grayscale Mode ---\n";
        if (auto result = epd27.init(DisplayMode::Grayscale4); !result) {
            std::cerr << "Failed to initialize grayscale mode: "
                     << to_string(result.error()) << "\n";
            return EXIT_FAILURE;
        }

        std::cout << "Display initialized (Grayscale mode).\n\n";

        // Create screen for grayscale
        Screen screen_gray(epd27);
        Draw draw_gray(screen_gray);

        // Run grayscale demo
        demo_grayscale(screen_gray, draw_gray);

        // Wait before sleep
        std::cout << "\nWaiting 3 seconds before sleep...\n";
        std::this_thread::sleep_for(std::chrono::seconds(3));

        // Put display to sleep
        std::cout << "\nPutting display to sleep...\n";
        epd27.sleep();

        std::cout << "\nDemo completed successfully!\n";
        return EXIT_SUCCESS;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown error occurred.\n";
        return EXIT_FAILURE;
    }
}

