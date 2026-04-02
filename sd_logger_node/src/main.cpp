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
// CHANGED TO 8khz for testing purposes (BROWNOUT)
AudioInfo StreamInfo(8000, 2, 32);

// input to stream copy function
// this represents the signal coming from microphone
I2SStream i2sStream;

// csv output object
CsvOutput<int32_t> csvOutput(Serial);

// output of stream copy function
// this will fan out to both Serial CSV and WAV file
MultiOutput multiOutput;

// this is the key component that handles stream routing.
// NOTE: must be declared AFTER i2sStream and multiOutput
StreamCopy copier(multiOutput, i2sStream);

// Track whether recording is finished
bool recordingDone = false;

// Debug helper — call AFTER cfg is created in setup()
template <typename Cfg>
void printDebugInfo(const Cfg &cfg) {
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

// ---------------------------------------------------------------------------
//  Record a fixed-length WAV clip to the SD card.
//  Blocks for `durationMs` milliseconds, copying I2S data into the WAV file.
//  After the duration elapses, the file is finalized and closed.
// ---------------------------------------------------------------------------
void record_wav_clip(unsigned long durationMs) {
    // Build a temporary pipeline: I2S → WAV file only (no CSV spam)
    MultiOutput wavOnly;
    wavOnly.add(wav_writer_stream());
    StreamCopy wavCopier(wavOnly, i2sStream);

    Serial.printf("[REC] Recording %lu seconds to SD card...\n", durationMs / 1000);

    unsigned long startTime = millis();

    while ((millis() - startTime) < durationMs) {
        wavCopier.copy();
    }

    // Finalize — flushes buffer and writes correct WAV header size
    wav_writer_end();

    Serial.println("[REC] =============================");
    Serial.println("[REC] file saved");
    Serial.println("[REC] =============================");
}

void setup() {

    delayMicroseconds(2000000);  // 2s delay — give sender time to boot

    Serial.begin(921600);
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

    printDebugInfo(cfg);

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

    // ---- SD card + WAV recording (15-second clip) ----
    bool sdReady = sd_card_init();
    if (sdReady) {
        sdReady = wav_writer_begin(StreamInfo, "/recording.wav");
    }
    if (sdReady) {
        record_wav_clip(15000);  // 15 seconds
        recordingDone = true;
    } else {
        Serial.println("[MAIN] SD unavailable — skipping WAV recording.");
    }

    // ---- After recording, set up CSV-only output for loop() ----
    multiOutput.add(csvOutput);
    Serial.println("[MAIN] Entering CSV streaming mode.");
}

void loop() {
    copier.copy();
}