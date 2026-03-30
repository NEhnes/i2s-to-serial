/**
 * @file sd_card.h
 * @brief SPI-based microSD card initialization for ESP32-WROOM
 *
 * ============================================================================
 *  ESP32-WROOM GPIO Pin Reference for SPI (SD Card Adapter)
 * ============================================================================
 *
 *  The ESP32-WROOM-32 has two usable SPI peripherals for user devices:
 *
 *    VSPI (default, recommended):
 *      MOSI = GPIO 23
 *      MISO = GPIO 19
 *      SCK  = GPIO 18
 *      CS   = GPIO 5   (any free GPIO works for CS)
 *
 *    HSPI (alternative):
 *      MOSI = GPIO 13
 *      MISO = GPIO 12   ⚠ GPIO 12 is a strapping pin — pull HIGH at boot
 *                          can prevent boot. Avoid if possible.
 *      SCK  = GPIO 14
 *      CS   = GPIO 15   (any free GPIO works for CS)
 *
 *  GPIOs you CANNOT use for SPI (or anything user-facing):
 *    GPIO 0   — Strapping pin (boot mode). Has internal pull-up.
 *    GPIO 1   — TX0 (default Serial output). Messing with this kills Serial.
 *    GPIO 2   — Strapping pin (must be LOW for flash boot). Sometimes OK for
 *               output but risky.
 *    GPIO 3   — RX0 (default Serial input). Same deal as GPIO 1.
 *    GPIO 6–11 — Connected to the on-board SPI flash. NEVER touch these.
 *    GPIO 12  — Strapping pin (MTDI). Controls flash voltage at boot.
 *               Can brick your board if pulled HIGH unexpectedly.
 *    GPIO 34–39 — Input-only GPIOs. Cannot drive MOSI, SCK, or CS.
 *
 *  Safe GPIOs for CS (directly usable, accent the below):
 *    GPIO 4, 5, 13, 14, 15, 16, 17, 21, 22, 25, 26, 27, 32, 33
 *    (subtract any pins you're already using for I2S)
 *
 *  Your current I2S pins (claimed by receiver):
 *    GPIO 3  — MCK (conflicts with RX0 — worth reconsidering later)
 *    GPIO 14 — BCLK   (default, check your wiring)
 *    GPIO 15 — LRCLK  (default, check your wiring)
 *    GPIO 32 — DIN    (default, check your wiring)
 *
 * ============================================================================
 */

#pragma once

#include <SPI.h>
#include <SD.h>

// ---------------------------------------------------------------------------
//  SPI Pin Definitions — Change these to match your wiring.
//  Defaults are VSPI bus pins on ESP32-WROOM-32.
// ---------------------------------------------------------------------------
constexpr int SD_CS_PIN   = 5;   // Chip Select — any free GPIO
constexpr int SD_MOSI_PIN = 23;  // Master Out Slave In
constexpr int SD_MISO_PIN = 19;  // Master In Slave Out
constexpr int SD_SCK_PIN  = 18;  // Serial Clock

/**
 * @brief Initialize the SD card over SPI.
 *
 * Sets up the SPI bus with the pins defined above and mounts the SD card.
 * The card must be FAT32 formatted.
 *
 * @return true  SD card mounted successfully.
 * @return false SD card init failed (check wiring / card format).
 */
bool sd_card_init();
