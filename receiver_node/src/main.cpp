/**
 * @file streams-i2s-serial.ino
 * Original code adapted from: https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-stream/streams-i2s-serial/README.md
 */

#include "AudioTools.h"

// BCLK: 14
// LRCLK: 15
// DIN: 32

// Audio configuration: 44.1kHz, stereo, 32-bit
AudioInfo info(44100, 2, 32);
I2SStream i2sStream; // I2S input stream
CsvOutput<int32_t> csvOutput(Serial); // Output samples to Serial as CSV
StreamCopy copier(csvOutput, i2sStream); // Stream copier: copy I2S to Serial

/**
 * @brief Setup I2S receiver and Serial communication
 * Initializes serial, configures I2S in master mode, waits for sender to start.
 */
void setup(void) {

    delayMicroseconds(2000000);  // 2 second delay — give sender time to boot

    Serial.begin(115200);
    Serial.println("Serial communication initialized");

    delayMicroseconds(2000000);  // Another 2-second delay — patience is non-negotiable

    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);
    
    auto cfg = i2sStream.defaultConfig(RX_MODE);
    cfg.copyFrom(info);
    cfg.i2s_format = I2S_STD_FORMAT; // Standard I2S format (standard, not LSB/MSB)
    cfg.is_master = true;            // Receiver is clock master — must match sender's expectations
    cfg.use_apll = false;            // Use internal clock, not APLL (avoid phase drift)
    cfg.pin_mck = 3;                 // MCLK output on GPIO3 — ensure receiver syncs properly
    i2sStream.begin(cfg);

    // Configure CSV output for 32-bit signed integer format
    csvOutput.begin(info);

    // TEMPORARY PIN CHECK — help debug wiring issues
    Serial.print("BCLK: "); Serial.println(cfg.pin_bck);
    Serial.print("LRCLK: "); Serial.println(cfg.pin_ws);
    Serial.print("DIN: "); Serial.println(cfg.pin_data);

    Serial.println();
    Serial.println("Setup complete, waiting for I2S sender...");
    delayMicroseconds(2000000);  // Final delay — we ain't rushing Jack

    // Wait for I2S data — we're waiting for the damn sender to wake up
    while (true) {
        if (i2sStream.isActive()) { // Require >100 samples for confidence
            Serial.println("I2S sender detected — YES, WE HAVE LIFTOFF!");

            AudioInfo receivedInfo = i2sStream.audioInfo();
            Serial.print("Received Audio Info - Sample Rate: "); Serial.print(receivedInfo.sample_rate);
            Serial.print(", Channels: "); Serial.print(receivedInfo.channels);
            Serial.print(", Bits per Sample: "); Serial.println(receivedInfo.bits_per_sample);
            Serial.println("BCLK PIN: " + String(cfg.pin_bck) + ", LRCLK PIN: " + String(cfg.pin_ws) + ", DIN PIN: " + String(cfg.pin_data));

            break;
        }
        Serial.println("Waiting for I2S sender... (pray harder)");
        delayMicroseconds(500000); // Half-second delays — no need to spam the log
    }
}

/**
 * @brief Main loop: continuously copy bufferred I2S samples to Serial
 * No processing — just pipe raw samples to Serial for analysis.
 *   Also, don't touch this. It's delicate.
 */
void loop() {
    copier.copy();
}