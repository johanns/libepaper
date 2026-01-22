#include <array>
#include <cstddef>
#include <cstdlib>
#include <expected>
#include <iostream>
#include <span>
#include <string_view>

#include "epaper/color/color.hpp"
#include "epaper/core/device.hpp"
#include "epaper/core/display.hpp"
#include "epaper/core/errors.hpp"
#include "epaper/core/framebuffer.hpp"
#include "epaper/core/types.hpp"
#include "epaper/drivers/mock_driver.hpp"
#include "epaper/graphics/font.hpp"
#include "epaper/io/image_io.hpp"

using namespace epaper;

namespace {

constexpr std::size_t RGB_CHANNELS = 3U;

constexpr std::size_t MIN_WIDTH = 120U;
constexpr std::size_t MIN_HEIGHT = 80U;

// Test regions layout
constexpr std::size_t RECT_LEFT = 5U;
constexpr std::size_t RECT_TOP = 5U;
constexpr std::size_t RECT_RIGHT = 15U;
constexpr std::size_t RECT_BOTTOM = 15U;

constexpr std::size_t LINE_START_X = 20U;
constexpr std::size_t LINE_START_Y = 5U;
constexpr std::size_t LINE_END_X = 35U;
constexpr std::size_t LINE_END_Y = 15U;

constexpr std::size_t DOTTED_LINE_START_X = 40U;
constexpr std::size_t DOTTED_LINE_START_Y = 5U;
constexpr std::size_t DOTTED_LINE_END_X = 55U;
constexpr std::size_t DOTTED_LINE_END_Y = 15U;
constexpr std::size_t DOTTED_LINE_SAMPLE_X = DOTTED_LINE_START_X;
constexpr std::size_t DOTTED_LINE_SAMPLE_Y = DOTTED_LINE_START_Y;

constexpr std::size_t CIRCLE_CENTER_X = 10U;
constexpr std::size_t CIRCLE_CENTER_Y = 30U;
constexpr std::size_t CIRCLE_RADIUS = 5U;
constexpr std::size_t CIRCLE_SAMPLE_X = CIRCLE_CENTER_X + CIRCLE_RADIUS;
constexpr std::size_t CIRCLE_SAMPLE_Y = CIRCLE_CENTER_Y;

constexpr std::size_t FILLED_CIRCLE_CENTER_X = 30U;
constexpr std::size_t FILLED_CIRCLE_CENTER_Y = 30U;
constexpr std::size_t FILLED_CIRCLE_RADIUS = 5U;

constexpr std::size_t TEXT_X = 60U;
constexpr std::size_t TEXT_Y = 8U;

constexpr std::size_t PATTERN_LEFT = 5U;
constexpr std::size_t PATTERN_TOP = 50U;
constexpr std::size_t PATTERN_SIZE = 16U;
constexpr std::size_t PATTERN_CELL_SIZE = 4U;

constexpr std::size_t PLANE_COUNT_ONE = 1U;
constexpr std::size_t PLANE_COUNT_TWO = 2U;

constexpr std::size_t COLORS_BW = 2U;
constexpr std::size_t COLORS_BWR = 3U;
constexpr std::size_t COLORS_BWY = 3U;
constexpr std::size_t COLORS_SPECTRA6 = 6U;
constexpr std::size_t COLORS_GRAYSCALE_AVAILABLE = 2U;

enum class FramebufferKind { Mono, TwoPlane };

struct ModeCase {
  DisplayMode mode;
  FramebufferKind framebuffer_kind;
  bool is_color;
  std::size_t expected_planes;
  std::size_t expected_available_colors;
  std::string_view name;
};

constexpr std::array<ModeCase, 5U> MODE_CASES = {
    ModeCase{.mode = DisplayMode::BlackWhite,
             .framebuffer_kind = FramebufferKind::Mono,
             .is_color = false,
             .expected_planes = PLANE_COUNT_ONE,
             .expected_available_colors = COLORS_BW,
             .name = "BlackWhite"},
    ModeCase{.mode = DisplayMode::Grayscale4,
             .framebuffer_kind = FramebufferKind::Mono,
             .is_color = false,
             .expected_planes = PLANE_COUNT_ONE,
             .expected_available_colors = COLORS_GRAYSCALE_AVAILABLE,
             .name = "Grayscale4"},
    ModeCase{.mode = DisplayMode::BWR,
             .framebuffer_kind = FramebufferKind::TwoPlane,
             .is_color = true,
             .expected_planes = PLANE_COUNT_TWO,
             .expected_available_colors = COLORS_BWR,
             .name = "BWR"},
    ModeCase{.mode = DisplayMode::BWY,
             .framebuffer_kind = FramebufferKind::TwoPlane,
             .is_color = true,
             .expected_planes = PLANE_COUNT_TWO,
             .expected_available_colors = COLORS_BWY,
             .name = "BWY"},
    ModeCase{.mode = DisplayMode::Spectra6,
             .framebuffer_kind = FramebufferKind::Mono,
             .is_color = true,
             .expected_planes = PLANE_COUNT_ONE,
             .expected_available_colors = COLORS_SPECTRA6,
             .name = "Spectra6"},
};

struct DrawColors {
  Color background;
  Color rectangle;
  Color line;
  Color dotted_line;
  Color circle;
  Color filled_circle;
  Color text;
  Color pattern_primary;
  Color pattern_secondary;
};

struct PixelExpectation {
  std::size_t x;
  std::size_t y;
  Color color;
};

[[nodiscard]] constexpr auto colors_for_mode(DisplayMode mode) -> DrawColors {
  switch (mode) {
  case DisplayMode::BlackWhite:
    return DrawColors{.background = Color::White,
                      .rectangle = Color::Black,
                      .line = Color::Black,
                      .dotted_line = Color::Black,
                      .circle = Color::Black,
                      .filled_circle = Color::Black,
                      .text = Color::Black,
                      .pattern_primary = Color::Black,
                      .pattern_secondary = Color::White};
  case DisplayMode::Grayscale4:
    return DrawColors{.background = Color::White,
                      .rectangle = Color::Gray1,
                      .line = Color::Gray2,
                      .dotted_line = Color::Black,
                      .circle = Color::Black,
                      .filled_circle = Color::Gray2,
                      .text = Color::Gray1,
                      .pattern_primary = Color::Gray2,
                      .pattern_secondary = Color::Gray1};
  case DisplayMode::BWR:
    return DrawColors{.background = Color::White,
                      .rectangle = Color::Red,
                      .line = Color::Black,
                      .dotted_line = Color::Red,
                      .circle = Color::Red,
                      .filled_circle = Color::Black,
                      .text = Color::Red,
                      .pattern_primary = Color::Red,
                      .pattern_secondary = Color::Black};
  case DisplayMode::BWY:
    return DrawColors{.background = Color::White,
                      .rectangle = Color::Yellow,
                      .line = Color::Black,
                      .dotted_line = Color::Yellow,
                      .circle = Color::Yellow,
                      .filled_circle = Color::Black,
                      .text = Color::Yellow,
                      .pattern_primary = Color::Yellow,
                      .pattern_secondary = Color::Black};
  case DisplayMode::Spectra6:
    return DrawColors{.background = Color::White,
                      .rectangle = Color::Red,
                      .line = Color::Blue,
                      .dotted_line = Color::Green,
                      .circle = Color::Green,
                      .filled_circle = Color::Yellow,
                      .text = Color::Yellow,
                      .pattern_primary = Color::Red,
                      .pattern_secondary = Color::Blue};
  }
  return DrawColors{.background = Color::White,
                    .rectangle = Color::Black,
                    .line = Color::Black,
                    .dotted_line = Color::Black,
                    .circle = Color::Black,
                    .filled_circle = Color::Black,
                    .text = Color::Black,
                    .pattern_primary = Color::Black,
                    .pattern_secondary = Color::White};
}

[[nodiscard]] constexpr auto expected_rgb(Color color) -> RGB {
  switch (color) {
  case Color::Black:
    return colors::Black;
  case Color::White:
    return colors::White;
  case Color::Red:
    return colors::Red;
  case Color::Green:
    return colors::Green;
  case Color::Blue:
    return colors::Blue;
  case Color::Yellow:
    return colors::Yellow;
  case Color::Gray1:
    return colors::LightGray;
  case Color::Gray2:
    return colors::DarkGray;
  }
  return colors::White;
}

[[nodiscard]] auto ensure_dimensions(const Display<MockDriver, MonoFramebuffer> &display)
    -> std::expected<void, Error> {
  if (display.width() < MIN_WIDTH || display.height() < MIN_HEIGHT) {
    return std::unexpected(make_error(ErrorCode::InvalidDimensions, "display too small for color drawing"));
  }
  return {};
}

[[nodiscard]] auto ensure_dimensions(const Display<MockDriver, TwoPlaneFramebuffer> &display)
    -> std::expected<void, Error> {
  if (display.width() < MIN_WIDTH || display.height() < MIN_HEIGHT) {
    return std::unexpected(make_error(ErrorCode::InvalidDimensions, "display too small for color drawing"));
  }
  return {};
}

template <typename DisplayT>
[[nodiscard]] auto draw_primitives(DisplayT &display, const DrawColors &colors) -> std::expected<void, Error> {
  display.clear(colors.background);

  // Filled rectangle
  display.draw(display.rectangle()
                   .top_left(RECT_LEFT, RECT_TOP)
                   .bottom_right(RECT_RIGHT, RECT_BOTTOM)
                   .color(colors.rectangle)
                   .border_width(DotPixel::Pixel1x1)
                   .fill(DrawFill::Full)
                   .build());

  // Solid line
  display.draw(display.line()
                   .from(LINE_START_X, LINE_START_Y)
                   .to(LINE_END_X, LINE_END_Y)
                   .color(colors.line)
                   .width(DotPixel::Pixel1x1)
                   .style(LineStyle::Solid)
                   .build());

  // Dotted line
  display.draw(display.line()
                   .from(DOTTED_LINE_START_X, DOTTED_LINE_START_Y)
                   .to(DOTTED_LINE_END_X, DOTTED_LINE_END_Y)
                   .color(colors.dotted_line)
                   .width(DotPixel::Pixel1x1)
                   .style(LineStyle::Dotted)
                   .build());

  // Outline circle
  display.draw(display.circle()
                   .center(CIRCLE_CENTER_X, CIRCLE_CENTER_Y)
                   .radius(CIRCLE_RADIUS)
                   .color(colors.circle)
                   .fill(DrawFill::Empty)
                   .build());

  // Filled circle
  display.draw(display.circle()
                   .center(FILLED_CIRCLE_CENTER_X, FILLED_CIRCLE_CENTER_Y)
                   .radius(FILLED_CIRCLE_RADIUS)
                   .color(colors.filled_circle)
                   .fill(DrawFill::Full)
                   .build());

  // Text
  display.draw(display.text("COLOR")
                   .at(TEXT_X, TEXT_Y)
                   .font(&Font::font12())
                   .foreground(colors.text)
                   .background(colors.background)
                   .build());

  // Checkerboard pattern
  for (std::size_t y = 0U; y < PATTERN_SIZE; y += PATTERN_CELL_SIZE) {
    for (std::size_t x = 0U; x < PATTERN_SIZE; x += PATTERN_CELL_SIZE) {
      const bool is_primary = ((x / PATTERN_CELL_SIZE) + (y / PATTERN_CELL_SIZE)) % 2U == 0U;
      const auto color = is_primary ? colors.pattern_primary : colors.pattern_secondary;
      display.draw(
          display.rectangle()
              .top_left(PATTERN_LEFT + x, PATTERN_TOP + y)
              .bottom_right(PATTERN_LEFT + x + PATTERN_CELL_SIZE - 1U, PATTERN_TOP + y + PATTERN_CELL_SIZE - 1U)
              .color(color)
              .border_width(DotPixel::Pixel1x1)
              .fill(DrawFill::Full)
              .build());
    }
  }

  return {};
}

template <typename DisplayT>
[[nodiscard]] auto expected_pixels_for_mode(DisplayMode mode) -> std::array<PixelExpectation, 7U> {
  const auto colors = colors_for_mode(mode);
  return {PixelExpectation{.x = RECT_LEFT + 2U, .y = RECT_TOP + 2U, .color = colors.rectangle},
          PixelExpectation{.x = LINE_START_X, .y = LINE_START_Y, .color = colors.line},
          PixelExpectation{.x = DOTTED_LINE_SAMPLE_X, .y = DOTTED_LINE_SAMPLE_Y, .color = colors.dotted_line},
          PixelExpectation{.x = CIRCLE_SAMPLE_X, .y = CIRCLE_SAMPLE_Y, .color = colors.circle},
          PixelExpectation{.x = FILLED_CIRCLE_CENTER_X, .y = FILLED_CIRCLE_CENTER_Y, .color = colors.filled_circle},
          PixelExpectation{.x = PATTERN_LEFT + 1U, .y = PATTERN_TOP + 1U, .color = colors.pattern_primary},
          PixelExpectation{
              .x = PATTERN_LEFT + PATTERN_CELL_SIZE + 1U, .y = PATTERN_TOP + 1U, .color = colors.pattern_secondary}};
}

template <typename DisplayT>
[[nodiscard]] auto verify_get_pixel(DisplayT &display, std::span<const PixelExpectation> expectations)
    -> std::expected<void, Error> {
  for (const auto &expectation : expectations) {
    auto actual = display.get_pixel(expectation.x, expectation.y);
    if (actual != expectation.color) {
      std::string msg = "get_pixel mismatch at (" + std::to_string(expectation.x) + "," +
                        std::to_string(expectation.y) + ") expected " +
                        std::to_string(static_cast<int>(expectation.color)) + " got " +
                        std::to_string(static_cast<int>(actual));
      return std::unexpected(make_error(ErrorCode::InvalidMode, msg));
    }
  }
  return {};
}

template <typename DisplayT>
[[nodiscard]] auto verify_rgb(DisplayT &display, std::span<const PixelExpectation> expectations)
    -> std::expected<void, Error> {
  const auto rgb = ImageIO::framebuffer_to_rgb(display.framebuffer());
  const std::size_t width = display.width();
  const std::size_t height = display.height();
  const std::size_t expected_size = width * height * RGB_CHANNELS;

  if (rgb.size() != expected_size) {
    return std::unexpected(make_error(ErrorCode::InvalidDimensions, "RGB buffer size mismatch"));
  }

  for (const auto &expectation : expectations) {
    const std::size_t base = (expectation.y * width + expectation.x) * RGB_CHANNELS;
    const auto expected = expected_rgb(expectation.color);

    if (rgb.at(base) != expected.r) {
      return std::unexpected(make_error(ErrorCode::InvalidFormat, "RGB red channel mismatch"));
    }
    if (rgb.at(base + 1U) != expected.g) {
      return std::unexpected(make_error(ErrorCode::InvalidFormat, "RGB green channel mismatch"));
    }
    if (rgb.at(base + 2U) != expected.b) {
      return std::unexpected(make_error(ErrorCode::InvalidFormat, "RGB blue channel mismatch"));
    }
  }

  return {};
}

template <typename FramebufferT>
[[nodiscard]] auto run_mode(Device &device, const ModeCase &mode_case) -> std::expected<void, Error> {
  auto display_result = create_display<MockDriver, FramebufferT>(device, mode_case.mode);
  if (!display_result) {
    return std::unexpected(display_result.error());
  }
  auto &display = display_result.value();

  if (display.is_color() != mode_case.is_color) {
    return std::unexpected(make_error(ErrorCode::InvalidMode, "is_color mismatch"));
  }
  if (display.get_num_planes() != mode_case.expected_planes) {
    return std::unexpected(make_error(ErrorCode::InvalidMode, "plane count mismatch"));
  }

  const auto available_colors = display.available_colors();
  if (available_colors.size() != mode_case.expected_available_colors) {
    return std::unexpected(make_error(ErrorCode::InvalidMode, "available_colors size mismatch"));
  }

  if (auto res = ensure_dimensions(display); !res) {
    return std::unexpected(res.error());
  }

  const auto colors = colors_for_mode(mode_case.mode);
  const auto expectations = expected_pixels_for_mode<decltype(display)>(mode_case.mode);

  if (auto res = draw_primitives(display, colors); !res) {
    return std::unexpected(res.error());
  }
  if (auto res = verify_get_pixel(display, expectations); !res) {
    return std::unexpected(res.error());
  }
  if (auto res = verify_rgb(display, expectations); !res) {
    return std::unexpected(res.error());
  }

  if (auto res = display.refresh(); !res) {
    return std::unexpected(res.error());
  }

  return {};
}

} // namespace

auto main() -> int {
  std::cout << "=======================================\n";
  std::cout << "  MockDriver Color Modes Test\n";
  std::cout << "=======================================\n";

  Device device;
  if (auto result = device.init(); !result) {
    std::cerr << "Device initialization failed: " << result.error().what() << "\n";
    return EXIT_FAILURE;
  }

  for (const auto &mode_case : MODE_CASES) {
    std::cout << "Testing mode: " << mode_case.name << "\n";

    std::expected<void, Error> result{};
    switch (mode_case.framebuffer_kind) {
    case FramebufferKind::Mono: {
      result = run_mode<MonoFramebuffer>(device, mode_case);
      break;
    }
    case FramebufferKind::TwoPlane: {
      result = run_mode<TwoPlaneFramebuffer>(device, mode_case);
      break;
    }
    }

    if (!result) {
      std::cerr << "Mode test failed: " << mode_case.name << " -> " << result.error().what() << "\n";
      return EXIT_FAILURE;
    }
  }

  std::cout << "All MockDriver color mode tests passed.\n";
  return EXIT_SUCCESS;
}
