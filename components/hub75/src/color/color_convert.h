// SPDX-FileCopyrightText: 2025 Stuart Parmenter
// SPDX-License-Identifier: MIT
//
// @file color_convert.h
// @brief RGB565 scaling utilities for color conversion
//
// Provides functions to scale RGB565 color components (5-bit and 6-bit)
// to 8-bit values for display on HUB75 panels.

#pragma once

#include "hub75_config.h"
#include "hub75_types.h"  // For Hub75PixelFormat, Hub75ColorOrder
#include <stdint.h>
#include <stddef.h>  // For size_t

namespace hub75 {

// ============================================================================
// RGB565 Scaling Utilities
// ============================================================================

/**
 * @brief Scale 5-bit color value to 8-bit (for RGB565 red/blue channels)
 *
 * Formula: (val5 << 3) | (val5 >> 2)
 * Replicates MSBs into LSBs for uniform distribution (31 -> 255)
 *
 * @param val5 5-bit value (0-31)
 * @return 8-bit value (0-255)
 */
HUB75_CONST HUB75_IRAM inline constexpr uint8_t scale_5bit_to_8bit(uint8_t val5) { return (val5 << 3) | (val5 >> 2); }

/**
 * @brief Scale 6-bit color value to 8-bit (for RGB565 green channel)
 *
 * Formula: (val6 << 2) | (val6 >> 4)
 * Replicates MSBs into LSBs for uniform distribution (63 -> 255)
 *
 * @param val6 6-bit value (0-63)
 * @return 8-bit value (0-255)
 */
HUB75_CONST HUB75_IRAM inline constexpr uint8_t scale_6bit_to_8bit(uint8_t val6) { return (val6 << 2) | (val6 >> 4); }

// ============================================================================
// Pixel Format Extraction
// ============================================================================

/**
 * @brief Extract RGB888 values from various pixel formats
 *
 * Handles RGB565, RGB888, and RGB888_32 formats with endianness and color order.
 * This is a hot-path function used in draw_pixels() implementations.
 *
 * @param buffer Source pixel buffer
 * @param pixel_idx Index of pixel to extract (not byte offset)
 * @param format Pixel format (RGB565, RGB888, or RGB888_32)
 * @param color_order Color component order (RGB or BGR, for RGB888_32 only)
 * @param big_endian True if buffer is big-endian
 * @param r8 Output: 8-bit red component (0-255)
 * @param g8 Output: 8-bit green component (0-255)
 * @param b8 Output: 8-bit blue component (0-255)
 */
HUB75_IRAM inline void extract_rgb888_from_format(const uint8_t *buffer, size_t pixel_idx, Hub75PixelFormat format,
                                                  Hub75ColorOrder color_order, bool big_endian, uint8_t &r8,
                                                  uint8_t &g8, uint8_t &b8) {
  switch (format) {
    case Hub75PixelFormat::RGB565: {
      // 16-bit RGB565
      const uint8_t *p = buffer + (pixel_idx * 2);
      uint16_t rgb565;
      if (big_endian) {
        rgb565 = (uint16_t(p[0]) << 8) | p[1];
      } else {
        rgb565 = (uint16_t(p[1]) << 8) | p[0];
      }

      // Extract RGB565 components
      const uint8_t r5 = (rgb565 >> 11) & 0x1F;
      const uint8_t g6 = (rgb565 >> 5) & 0x3F;
      const uint8_t b5 = rgb565 & 0x1F;

      // Scale to 8-bit using color conversion helpers
      r8 = scale_5bit_to_8bit(r5);
      g8 = scale_6bit_to_8bit(g6);
      b8 = scale_5bit_to_8bit(b5);
      break;
    }

    case Hub75PixelFormat::RGB888: {
      // 24-bit packed RGB
      const uint8_t *p = buffer + (pixel_idx * 3);
      r8 = p[0];
      g8 = p[1];
      b8 = p[2];
      break;
    }

    case Hub75PixelFormat::RGB888_32: {
      // 32-bit RGB with padding
      const uint8_t *p = buffer + (pixel_idx * 4);
      if (color_order == Hub75ColorOrder::RGB) {
        if (big_endian) {
          // Big-endian xRGB: [x][R][G][B]
          r8 = p[1];
          g8 = p[2];
          b8 = p[3];
        } else {
          // Little-endian xRGB stored as BGRx: [B][G][R][x]
          r8 = p[2];
          g8 = p[1];
          b8 = p[0];
        }
      } else {  // BGR
        if (big_endian) {
          // Big-endian xBGR: [x][B][G][R]
          r8 = p[3];
          g8 = p[2];
          b8 = p[1];
        } else {
          // Little-endian xBGR stored as RGBx: [R][G][B][x]
          r8 = p[0];
          g8 = p[1];
          b8 = p[2];
        }
      }
      break;
    }
  }
}

// ============================================================================
// Compile-Time Validation
// ============================================================================

namespace {  // Anonymous namespace for compile-time validation

// Validate RGB565 5-bit to 8-bit scaling produces exact 255
consteval bool test_rgb565_5bit_scaling() {
  constexpr uint8_t r8_max = scale_5bit_to_8bit(31);
  return r8_max == 255;
}

// Validate RGB565 6-bit to 8-bit scaling produces exact 255
consteval bool test_rgb565_6bit_scaling() {
  constexpr uint8_t g8_max = scale_6bit_to_8bit(63);
  return g8_max == 255;
}

static_assert(test_rgb565_5bit_scaling(), "RGB565 5-bit scaling does not produce exact 255 for max value (31)");
static_assert(test_rgb565_6bit_scaling(), "RGB565 6-bit scaling does not produce exact 255 for max value (63)");

}  // namespace

}  // namespace hub75
