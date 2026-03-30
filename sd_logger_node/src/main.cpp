/**
 * @file main.cpp
 * @brief I2S audio receiver → Serial CSV + SD card WAV recording
 *
 * Receives I2S audio from a sender node and simultaneously:
 *   1. Outputs CSV samples to Serial (for real-time analysis)
 *   2. Records a .wav file to a microSD card over SPI
 *
 * Original I2S-to-Serial code adapted from:
 *   https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-stream/streams-i2s-serial/README.md
 */

#include "AudioTools.h"
#include "sd_card.h"
#include "wav_writer.h"

// ---------------------------------------------------------------------------
//  Audio Configuration
// ---------------------------------------------------------------------------
// 44.1 kHz, stereo, 32-bit — must match the sender node.
AudioInfo info(44100, 2, 32);

// ---------------------------------------------------------------------------
//  I2S Input
// ---------------------------------------------------------------------------
I2SStream i2sStream;

// ---------------------------------------------------------------------------
//  Output: Serial CSV (existing behaviour)
// ---------------------------------------------------------------------------
CsvOutput<int32_t> csvOutput(Serial);

// ---------------------------------------------------------------------------
//  Output: MultiOutput — fans I2S data to BOTH serial CSV and WAV file
// ---------------------------------------------------------------------------
MultiOutput multiOutput;

// ---------------------------------------------------------------------------
//  Stream Copier — single copier drives the whole pipeline
// ---------------------------------------------------------------------------
StreamCopy copier(multiOutput, i2sStream);

// Track whether SD + WAV init succeeded so loop() can gracefully degrade
bool sdReady = false;

// ---------------------------------------------------------------------------
//  setup()
// ---------------------------------------------------------------------------
void setup() {

    delayMicroseconds(2000000);  // 2s delay — give sender time to boot

    Serial.begin(115200);
    Serial.println("Serial communication initialized");

    delayMicroseconds(2000000);  // Another 2s — patience is non-negotiable

    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);

    // ---- I2S configuration (receiver / master) ----
    auto cfg = i2sStream.defaultConfig(RX_MODE);
    cfg.copyFrom(info);
    cfg.i2s_format = I2S_STD_FORMAT;
    cfg.is_master  = true;
    cfg.use_apll   = false;
    cfg.pin_mck    = 3;
    i2sStream.begin(cfg);

    // CSV output
    csvOutput.begin(info);

    // Pin check (debug aid)
    Serial.print("BCLK: ");  Serial.println(cfg.pin_bck);
    Serial.print("LRCLK: "); Serial.println(cfg.pin_ws);
    Serial.print("DIN: ");   Serial.println(cfg.pin_data);
    Serial.println();

    Serial.println("Setup complete, waiting for I2S sender...");
    delayMicroseconds(2000000);

    // ---- Wait for I2S sender ----
    while (true) {
        if (i2sStream.isActive() && i2sStream.available() > 100) {
            Serial.println("I2S sender detected — YES, WE HAVE LIFTOFF!");

            AudioInfo receivedInfo = i2sStream.audioInfo();
            Serial.print("Received Audio Info - Sample Rate: "); Serial.print(receivedInfo.sample_rate);
            Serial.print(", Channels: ");                        Serial.print(receivedInfo.channels);
            Serial.print(", Bits per Sample: ");                 Serial.println(receivedInfo.bits_per_sample);
            Serial.println("BCLK PIN: " + String(cfg.pin_bck) +
                           ", LRCLK PIN: " + String(cfg.pin_ws) +
                           ", DIN PIN: " + String(cfg.pin_data));
            break;
        }
        Serial.println("Waiting for I2S sender... (pray harder)");
        delayMicroseconds(500000);
    }

    // ---- SD card + WAV init ----
    sdReady = sd_card_init();
    if (sdReady) {
        sdReady = wav_writer_begin(info, "/recording.wav");
    }

    // ---- Build the output pipeline ----
    if (sdReady) {
        // Fan out to both Serial CSV and WAV file
        multiOutput.add(csvOutput);
        multiOutput.add(wav_writer_stream());
        Serial.println("[MAIN] Output: Serial CSV + WAV file");
    } else {
        // SD failed — fall back to Serial-only (original behaviour)
        multiOutput.add(csvOutput);
        Serial.println("[MAIN] Output: Serial CSV only (SD unavailable)");
    }
}

// ---------------------------------------------------------------------------
//  loop()
// ---------------------------------------------------------------------------
void loop() {
    copier.copy();
}