/**
 * @file main.cpp
 * @brief I2S audio receiver → Serial CSV + SD card WAV recording
 *
 * Receives I2S audio from a sender node and:
 *   1. Outputs CSV samples to Serial (for real-time analysis)
 *   2. Records a .wav file to a microSD card over SPI
 *
 * Original I2S-to-Serial code adapted from:
 *   https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-stream/streams-i2s-serial/README.md
 */

#include "AudioTools.h"

// define pins for SPI writing
#include "sd_card.h"

// defines a few helper functions for writing WAV files to SD card
#include "wav_writer.h"

// audio information config
// 44.1 kHz, stereo, 32-bit — must match the sender node.
AudioInfo StreamInfo(44100, 2, 32);

// csv output object
CsvOutput<int32_t> csvOutput(Serial);

// this is the key component that handles stream routing.
StreamCopy copier(multiOutput, i2sStream);

// input to stream copy function
// this represents the signal coming from microphone
I2SStream i2sStream;

// output of stream copy function
// this will fan out to both Serial CSV and WAV file
MultiOutput multiOutput;

// Track whether SD + WAV init succeeded so loop() can gracefully degrade
bool sdReady = false;

void printDebugInfo() {
    Serial.println("Debug Info:");
    // stream data
    Serial.print("StreamInfo - Sample Rate: "); Serial.print(StreamInfo.sample_rate);
    Serial.print(", Channels: ");              Serial.print(StreamInfo.channels);
    Serial.print(", Bits per Sample: ");       Serial.println(StreamInfo.bits_per_sample);
    // pin data
    Serial.print("BCLK: ");  Serial.println(cfg.pin_bck);
    Serial.print("LRCLK: "); Serial.println(cfg.pin_ws);
    Serial.print("DIN: ");   Serial.println(cfg.pin_data);
    Serial.println();
}

void setup() {

    delayMicroseconds(2000000);  // 2s delay — give sender time to boot

    Serial.begin(115200);
    Serial.println("Serial communication initialized");

    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);

    // ---- I2S configuration (receiver / master) ----
    // set up cfg object
    auto cfg = i2sStream.defaultConfig(RX_MODE);
    cfg.copyFrom(StreamInfo);
    cfg.i2s_format = I2S_STD_FORMAT;
    cfg.is_master  = true;
    cfg.use_apll   = false;
    cfg.pin_mck    = 3;
    // pass cfg object to i2sStream begin() call
    i2sStream.begin(cfg);

    // CSV output
    csvOutput.begin(StreamInfo);

    Serial.println("Setup complete, waiting for I2S sender...");
    delayMicroseconds(2000000);

    // ---- Wait for I2S sender ----
    // i gotta check this shit fr because even if nothing is connected it hallucinates a connection
    // idfk
    while (true) {
        if (i2sStream.isActive() && i2sStream.available() > 100) {
            Serial.println("I2S sender detected — YES, WE HAVE LIFTOFF!");

            AudioInfo receivedInfo = i2sStream.audioInfo();
            Serial.print("Received Audio Info - Sample Rate: "); Serial.print(receivedInfo.sample_rate);
            Serial.print(", Channels: ");                        Serial.print(receivedInfo.channels);
            Serial.print(", Bits per Sample: ");                 Serial.println(receivedInfo.bits_per_sample);
            break;
        }
        Serial.println("Waiting for I2S sender... (pray harder)");
        delayMicroseconds(500000);
    }

    // ---- SD card + WAV init ----
    sdReady = sd_card_init();
    if (sdReady) {
        sdReady = wav_writer_begin(StreamInfo, "/recording.wav");
    }

    // ---- Build the output pipeline ----
    if (sdReady) {
        // SD succeeded — route to both Serial CSV and WAV file
        multiOutput.add(csvOutput);
        multiOutput.add(wav_writer_stream());
        Serial.println("[MAIN] Output: Serial CSV + WAV file");
    } else {
        // SD failed — default to Serial-only
        multiOutput.add(csvOutput);
        Serial.println("[MAIN] Output: Serial CSV only (SD unavailable)");
    }
}

void loop() {
    copier.copy();
}