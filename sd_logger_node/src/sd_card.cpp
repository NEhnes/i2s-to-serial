/**
 * @file sd_card.cpp
 * @brief SPI-based microSD card initialization implementation
 */

#include "sd_card.h"

bool sd_card_init() {
    // Initialize the SPI bus with our pin assignments
    SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);

    // Mount the SD card on the CS pin
    // SD.begin() returns false if the card is missing, not formatted, or
    // if the SPI wiring is wrong.
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("[SD] ERROR: SD card mount failed!");
        Serial.println("[SD]   - Is a FAT32-formatted card inserted?");
        Serial.println("[SD]   - Check SPI wiring (CS, MOSI, MISO, SCK).");
        return false;
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("[SD] Card mounted — %llu MB total\n", cardSize);

    uint64_t freeBytes = SD.totalBytes() - SD.usedBytes();
    Serial.printf("[SD] Free space: %llu MB\n", freeBytes / (1024 * 1024));

    return true;
}
