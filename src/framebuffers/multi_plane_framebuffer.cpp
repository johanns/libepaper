#include "epaper/color/color_manager.hpp"
#include "epaper/core/framebuffer.hpp"
#include "epaper/internal/internal.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace epaper {
namespace {
constexpr std::size_t BYTE_BITS = 8;
} // namespace

template <internal::PlaneCount PlaneCount>
MultiPlaneFramebuffer<PlaneCount>::MultiPlaneFramebuffer(std::size_t width, std::size_t height, DisplayMode mode)
    : width_(width), height_(height), stride_((width + BYTE_BITS - 1) / BYTE_BITS), mode_(mode) {
  static_assert(PLANE_COUNT == internal::PLANE_COUNT_TWO, "Only two-plane color framebuffers are supported.");

  const std::size_t plane_size = stride_ * height_;
  for (auto &plane : planes_) {
    plane.assign(plane_size, std::byte{0xFF});
  }
  clear(Color::White);
}

template <internal::PlaneCount PlaneCount> auto MultiPlaneFramebuffer<PlaneCount>::width() const -> std::size_t {
  return width_;
}

template <internal::PlaneCount PlaneCount> auto MultiPlaneFramebuffer<PlaneCount>::height() const -> std::size_t {
  return height_;
}

template <internal::PlaneCount PlaneCount> auto MultiPlaneFramebuffer<PlaneCount>::mode() const -> DisplayMode {
  return mode_;
}

template <internal::PlaneCount PlaneCount>
auto MultiPlaneFramebuffer<PlaneCount>::data() const -> std::span<const std::byte> {
  return planes_.front();
}

template <internal::PlaneCount PlaneCount>
auto MultiPlaneFramebuffer<PlaneCount>::get_planes() const -> std::vector<std::span<const std::byte>> {
  std::vector<std::span<const std::byte>> result;
  result.reserve(PLANE_COUNT);
  for (const auto &plane : planes_) {
    result.push_back(plane);
  }
  return result;
}

template <internal::PlaneCount PlaneCount>
auto MultiPlaneFramebuffer<PlaneCount>::set_pixel(std::size_t x, std::size_t y, Color color, Orientation orientation)
    -> void {
  auto [px, py] = internal::transform_coordinates(x, y, width_, height_, orientation);
  if (px >= width_ || py >= height_) {
    return;
  }

  const std::size_t index = (py * stride_) + (px / BYTE_BITS);
  const auto bit = static_cast<std::uint8_t>(1U << (BYTE_BITS - 1U - (px % BYTE_BITS)));

  const auto rgb = ColorManager::to_rgb(color);

  if (mode_ == DisplayMode::BWR) {
    const auto dev_color = ColorManager::convert<DisplayMode::BWR>(rgb);
    const bool bw_on = dev_color.get_bw_bit();
    const bool col_on = dev_color.get_color_bit();

    if (bw_on) {
      planes_[0][index] |= std::byte{bit};
    } else {
      planes_[0][index] &= std::byte{static_cast<std::uint8_t>(~bit)};
    }

    if (col_on) {
      planes_[1][index] |= std::byte{bit};
    } else {
      planes_[1][index] &= std::byte{static_cast<std::uint8_t>(~bit)};
    }
    return;
  }

  if (mode_ == DisplayMode::BWY) {
    const auto dev_color = ColorManager::convert<DisplayMode::BWY>(rgb);
    const bool bw_on = dev_color.get_bw_bit();
    const bool col_on = dev_color.get_color_bit();

    if (bw_on) {
      planes_[0][index] |= std::byte{bit};
    } else {
      planes_[0][index] &= std::byte{static_cast<std::uint8_t>(~bit)};
    }

    if (col_on) {
      planes_[1][index] |= std::byte{bit};
    } else {
      planes_[1][index] &= std::byte{static_cast<std::uint8_t>(~bit)};
    }
    return;
  }

  std::unreachable();
}

template <internal::PlaneCount PlaneCount>
auto MultiPlaneFramebuffer<PlaneCount>::get_pixel(std::size_t x, std::size_t y, Orientation orientation) const
    -> Color {
  auto [px, py] = internal::transform_coordinates(x, y, width_, height_, orientation);
  if (px >= width_ || py >= height_) {
    return Color::White;
  }

  const std::size_t index = (py * stride_) + (px / BYTE_BITS);
  const auto bit = static_cast<std::uint8_t>(1U << (BYTE_BITS - 1U - (px % BYTE_BITS)));
  const bool bw_bit = (static_cast<std::uint8_t>(planes_[0][index]) & bit) != 0U;
  const bool col_bit = (static_cast<std::uint8_t>(planes_[1][index]) & bit) != 0U;

  if (!col_bit) {
    return (mode_ == DisplayMode::BWY) ? Color::Yellow : Color::Red;
  }
  return bw_bit ? Color::White : Color::Black;
}

template <internal::PlaneCount PlaneCount> auto MultiPlaneFramebuffer<PlaneCount>::clear(Color color) -> void {
  const auto rgb = ColorManager::to_rgb(color);

  if (mode_ == DisplayMode::BWR) {
    const auto dev_color = ColorManager::convert<DisplayMode::BWR>(rgb);
    const std::byte bw_byte = dev_color.get_bw_bit() ? std::byte{0xFF} : std::byte{0x00};
    const std::byte col_byte = dev_color.get_color_bit() ? std::byte{0xFF} : std::byte{0x00};

    std::ranges::fill(planes_[0], bw_byte);
    std::ranges::fill(planes_[1], col_byte);
    return;
  }

  if (mode_ == DisplayMode::BWY) {
    const auto dev_color = ColorManager::convert<DisplayMode::BWY>(rgb);
    const std::byte bw_byte = dev_color.get_bw_bit() ? std::byte{0xFF} : std::byte{0x00};
    const std::byte col_byte = dev_color.get_color_bit() ? std::byte{0xFF} : std::byte{0x00};

    std::ranges::fill(planes_[0], bw_byte);
    std::ranges::fill(planes_[1], col_byte);
    return;
  }

  std::unreachable();
}

template class MultiPlaneFramebuffer<internal::PlaneCount::Two>;

} // namespace epaper
