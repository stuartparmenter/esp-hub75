// SPDX-FileCopyrightText: 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT

// @file scan_patterns.hpp
// @brief Scan pattern coordinate remapping

// Based on https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA

#pragma once

#include "hub75_types.h"
#include "hub75_config.h"

namespace hub75 {

// Coordinate pair
struct Coords {
  uint16_t x;
  uint16_t y;
};

// Scan pattern coordinate remapping
// Transforms logical pixel coordinates to physical shift register positions
class ScanPatternRemap {
 public:
  // Remap coordinates based on scan pattern
  // @param c Input coordinates
  // @param pattern Scan pattern type
  // @param panel_width Width of single panel
  // @return Remapped coordinates
  static HUB75_CONST HUB75_IRAM constexpr Coords remap(Coords c, ScanPattern pattern, uint16_t panel_width) {
    switch (pattern) {
      case ScanPattern::STANDARD_TWO_SCAN:
        // No transformation needed
        return c;

      case ScanPattern::FOUR_SCAN_16PX_HIGH: {
        if ((c.y & 4) == 0) {
          c.x += (((c.x / panel_width) + 1) * panel_width);
        } else {
          c.x += ((c.x / panel_width) * panel_width);
        }
        c.y = (c.y >> 3) * 4 + (c.y & 0b00000011);
        return c;
      }

      case ScanPattern::FOUR_SCAN_32PX_HIGH: {
        if ((c.y & 8) == 0) {
          c.x += (((c.x / panel_width) + 1) * panel_width);
        } else {
          c.x += ((c.x / panel_width) * panel_width);
        }
        c.y = (c.y >> 4) * 8 + (c.y & 0b00000111);
        return c;
      }

      case ScanPattern::FOUR_SCAN_64PX_HIGH: {
        // Extra remapping for 64px high panels
        if ((c.y & 8) != ((c.y & 16) >> 1)) {
          c.y = (((c.y & 0b11000) ^ 0b11000) + (c.y & 0b11100111));
        }
        if ((c.y & 8) == 0) {
          c.x += (((c.x / panel_width) + 1) * panel_width);
        } else {
          c.x += ((c.x / panel_width) * panel_width);
        }
        c.y = (c.y >> 4) * 8 + (c.y & 0b00000111);
        return c;
      }

      default:
        return c;
    }
  }
};

// ============================================================================
// Compile-Time Validation
// ============================================================================

namespace {  // Anonymous namespace for compile-time validation

// Validate standard scan is identity transform
consteval bool test_standard_scan_identity() {
  constexpr Coords input = {32, 16};
  constexpr uint16_t panel_width = 64;
  constexpr Coords output = ScanPatternRemap::remap(input, ScanPattern::STANDARD_TWO_SCAN, panel_width);
  return (output.x == input.x) && (output.y == input.y);
}

// Validate four-scan doesn't overflow coordinates
consteval bool test_four_scan_no_overflow() {
  constexpr Coords input = {63, 63};
  constexpr uint16_t panel_width = 64;

  constexpr Coords out16 = ScanPatternRemap::remap(input, ScanPattern::FOUR_SCAN_16PX_HIGH, panel_width);
  constexpr Coords out32 = ScanPatternRemap::remap(input, ScanPattern::FOUR_SCAN_32PX_HIGH, panel_width);
  constexpr Coords out64 = ScanPatternRemap::remap(input, ScanPattern::FOUR_SCAN_64PX_HIGH, panel_width);

  return (out16.x < 256 && out16.y < 256) && (out32.x < 256 && out32.y < 256) && (out64.x < 256 && out64.y < 256);
}

static_assert(test_standard_scan_identity(), "Standard scan must be identity transform");
static_assert(test_four_scan_no_overflow(), "Four-scan patterns produce out-of-bounds coordinates");

}  // namespace

}  // namespace hub75
