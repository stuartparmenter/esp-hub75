/**
 * SPDX-FileCopyrightText: 2025 Stuart Parmenter
 * SPDX-License-Identifier: MIT
 *
 * @file pins_example.h
 * @brief Well-commented HUB75 pin configuration examples
 *
 * Copy this file to your project and modify for your specific board/panel.
 *
 * IMPORTANT: GPIO pin availability varies by ESP32 variant!
 * - ESP32: Avoid GPIO 6-11 (connected to flash), GPIO 34-39 (input-only)
 * - ESP32-S2: Avoid GPIO 26-32 (connected to flash/PSRAM)
 * - ESP32-S3: Avoid GPIO 26-37 (connected to flash/PSRAM)
 * - ESP32-C6: Most GPIOs can be used, check datasheet for restrictions
 * - ESP32-P4: Check datasheet for GPIO restrictions
 */

#pragma once

#include <cstdio>
#include "hub75.h"

// ============================================================================
// Example 1: Typical 64x64 panel (1/32 scan) on ESP32
// ============================================================================

static inline hub75_config_t getDefaultConfig_ESP32() {
  hub75_config_t config = HUB75_CONFIG_DEFAULT();

  // Panel dimensions
  config.width = 64;
  config.height = 64;
  config.scan_pattern = HUB75_SCAN_1_32;  // 64 rows = 1/32 scan

  // Data pins - Upper half (R1/G1/B1)
  config.pins.r1 = 25;
  config.pins.g1 = 26;
  config.pins.b1 = 27;

  // Data pins - Lower half (R2/G2/B2)
  config.pins.r2 = 14;
  config.pins.g2 = 12;
  config.pins.b2 = 13;

  // Address lines (A, B, C, D, E)
  config.pins.a = 23;
  config.pins.b = 19;
  config.pins.c = 5;
  config.pins.d = 17;
  config.pins.e = 18;  // Required for 64-row panels (1/32 scan)

  // Control signals
  config.pins.lat = 4;   // Latch
  config.pins.oe = 15;   // Output Enable (active low)
  config.pins.clk = 16;  // Clock

  // Performance settings
  config.output_clock_speed = 20000000;  // 20 MHz
  config.bit_depth = 8;                  // 8-bit BCM
  config.min_refresh_rate = 60;          // 60 Hz minimum

  // Features
  config.double_buffer = false;
  config.temporal_dither = false;
  config.gamma_mode = HUB75_GAMMA_CIE1931;
  config.brightness = 255;

  return config;
}

// ============================================================================
// Example 2: 32x32 panel (1/16 scan) on ESP32
// ============================================================================

static inline hub75_config_t getDefaultConfig_32x32() {
  hub75_config_t config = HUB75_CONFIG_DEFAULT();

  config.width = 32;
  config.height = 32;
  config.scan_pattern = HUB75_SCAN_1_16;  // 32 rows = 1/16 scan

  // Same pin mapping as 64x64, but E pin not needed
  config.pins.r1 = 25;
  config.pins.g1 = 26;
  config.pins.b1 = 27;
  config.pins.r2 = 14;
  config.pins.g2 = 12;
  config.pins.b2 = 13;
  config.pins.a = 23;
  config.pins.b = 19;
  config.pins.c = 5;
  config.pins.d = 17;
  config.pins.e = -1;  // Not used for 32-row panels
  config.pins.lat = 4;
  config.pins.oe = 15;
  config.pins.clk = 16;

  return config;
}

// ============================================================================
// Example 3: ESP32-S3 with double buffering and dithering
// ============================================================================

static inline hub75_config_t getDefaultConfig_ESP32S3() {
  hub75_config_t config = HUB75_CONFIG_DEFAULT();

  config.width = 64;
  config.height = 64;
  config.scan_pattern = HUB75_SCAN_1_32;

  // ESP32-S3 can use different pins (avoid 26-37 for flash/PSRAM)
  config.pins.r1 = 1;
  config.pins.g1 = 2;
  config.pins.b1 = 3;
  config.pins.r2 = 4;
  config.pins.g2 = 5;
  config.pins.b2 = 6;
  config.pins.a = 7;
  config.pins.b = 8;
  config.pins.c = 9;
  config.pins.d = 10;
  config.pins.e = 11;
  config.pins.lat = 12;
  config.pins.oe = 13;
  config.pins.clk = 14;

  // Enable advanced features (S3 has more RAM)
  config.double_buffer = true;
  config.temporal_dither = true;
  config.bit_depth = 10;  // Higher bit depth for smoother gradients

  return config;
}

// ============================================================================
// Example 4: ESP32-C6 with PARLIO (simplest configuration)
// ============================================================================

static inline hub75_config_t getDefaultConfig_ESP32C6() {
  hub75_config_t config = HUB75_CONFIG_DEFAULT();

  config.width = 64;
  config.height = 64;
  config.scan_pattern = HUB75_SCAN_1_32;

  // ESP32-C6 PARLIO pins - check datasheet for valid PARLIO GPIO groups
  config.pins.r1 = 0;
  config.pins.g1 = 1;
  config.pins.b1 = 2;
  config.pins.r2 = 3;
  config.pins.g2 = 4;
  config.pins.b2 = 5;
  config.pins.a = 6;
  config.pins.b = 7;
  config.pins.c = 8;
  config.pins.d = 9;
  config.pins.e = 10;
  config.pins.lat = 11;
  config.pins.oe = 12;
  config.pins.clk = 13;

  // PARLIO is very efficient
  config.output_clock_speed = 40000000;  // 40 MHz possible with PARLIO

  return config;
}

// ============================================================================
// Example 5: Chained panels (128x64 = two 64x64 panels side-by-side)
// ============================================================================

static inline hub75_config_t getDefaultConfig_Chained() {
  hub75_config_t config = HUB75_CONFIG_DEFAULT();

  // Virtual dimensions (chained panels)
  config.width = 128;  // 2 panels × 64 width
  config.height = 64;
  config.scan_pattern = HUB75_SCAN_1_32;

  // Same physical pin mapping as single panel
  config.pins.r1 = 25;
  config.pins.g1 = 26;
  config.pins.b1 = 27;
  config.pins.r2 = 14;
  config.pins.g2 = 12;
  config.pins.b2 = 13;
  config.pins.a = 23;
  config.pins.b = 19;
  config.pins.c = 5;
  config.pins.d = 17;
  config.pins.e = 18;
  config.pins.lat = 4;
  config.pins.oe = 15;
  config.pins.clk = 16;

  return config;
}

// ============================================================================
// Example 6: ESP32-S3 with 2x 64x64 chained panels (16-bit bus mode)
// ============================================================================

static inline hub75_config_t getUserConfig_ESP32S3_Dual64x64() {
  hub75_config_t config = HUB75_CONFIG_DEFAULT();

  // Panel configuration (2x 64x64 panels = 128x64 virtual display)
  config.width = 64;  // Single panel width
  config.height = 64;
  config.chain_length = 2;                // 2 panels chained horizontally
  config.scan_pattern = HUB75_SCAN_1_32;  // 64 rows = 1/32 scan

  // User's exact pin mapping (ESP32-S3)
  config.pins.r1 = 1;
  config.pins.g1 = 5;
  config.pins.b1 = 6;
  config.pins.r2 = 7;
  config.pins.g2 = 13;
  config.pins.b2 = 9;
  config.pins.a = 16;
  config.pins.b = 48;
  config.pins.c = 47;
  config.pins.d = 21;
  config.pins.e = 38;
  config.pins.lat = 8;
  config.pins.oe = 4;
  config.pins.clk = 18;

  // Performance settings (from user's config)
  config.output_clock_speed = 20000000;  // 20 MHz (HZ_20M)
  config.latch_blanking = 1;
  config.clk_phase_inverted = false;

  // Recommended settings for dual panels
  config.bit_depth = 8;  // Start with 8-bit, can increase to 10-12 later
  config.min_refresh_rate = 60;
  config.double_buffer = false;  // Start simple, enable if needed
  config.temporal_dither = false;
  config.gamma_mode = HUB75_GAMMA_CIE1931;
  config.brightness = 255;

  return config;
}

// ============================================================================
// Helper: Print pin configuration (for debugging)
// ============================================================================

static inline void printPinConfig(const hub75_pins_t &pins) {
  printf("HUB75 Pin Configuration:\n");
  printf("  Data (Upper): R1=%d, G1=%d, B1=%d\n", pins.r1, pins.g1, pins.b1);
  printf("  Data (Lower): R2=%d, G2=%d, B2=%d\n", pins.r2, pins.g2, pins.b2);
  printf("  Address: A=%d, B=%d, C=%d, D=%d, E=%d\n", pins.a, pins.b, pins.c, pins.d, pins.e);
  printf("  Control: LAT=%d, OE=%d, CLK=%d\n", pins.lat, pins.oe, pins.clk);
}
