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
// WHY THE FUCK DOES THIS CHANGE WAV LNEGTH?
// 16000 Hz, stereo, 32-bit — must match the sender node.
AudioInfo StreamInfo(16000, 2, 32);

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

    digitalWrite(LED_BUILTIN, HIGH);  // turn on LED to indicate recording status

    while ((millis() - startTime) < durationMs) {

        if (i2sStream.available() == 0) {
            Serial.println("BUFFER UNDERRUN");    // added to diagnose shortened WAV file issue
        }

        wavCopier.copy();
    }

    digitalWrite(LED_BUILTIN, LOW);   // turn off LED to indicate recording is done

    // Finalize — flushes buffer and writes correct WAV header size
    wav_writer_end();

    Serial.println("[REC] =============================");
    Serial.println("[REC] file saved");
    Serial.println("[REC] =============================");
}

// ---------------------------------------------------------------------------
// Helper function to generate a unique WAV filename for each recording.
// Checks for existing files named recording_1.wav, recording_2.wav, etc. up
// to recording_100.wav, and returns the first available filename.
// If all 100 filenames are taken, returns recording_ERROR.wav and logs an error.
// ---------------------------------------------------------------------------
String getWavHeader() {
    for (int i = 1; i <= 100; i++) {
        String filename = String("/recording_") + String(i, DEC) + String("_04-05-26.wav");
        if (!SD.exists(filename)) {
            return filename;
        }
    }
    Serial.println("ERROR: Recording limit (100) exceeded");
    return "/recording_ERROR.wav";
}

void setup() {

    delayMicroseconds(2000000);  // 2s delay — give sender time to boot

    digitalWrite(LED_BUILTIN, LOW);   // turn off LED

    Serial.begin(921600);
    Serial.println("Serial communication initialized");

    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);

    // ---- I2S configuration (receiver / master) ----
    // set up cfg object
    auto cfg = i2sStream.defaultConfig(RX_MODE);
    cfg.copyFrom(StreamInfo);
    cfg.i2s_format = I2S_STD_FORMAT;
    cfg.is_master  = true;
    cfg.use_apll   = true;

    // added after by nathan to address shortened wav file
    cfg.buffer_size = 4096;  // nothing beyond this yielded any return
    cfg.buffer_count = 4;    // same

    // cfg.pin_mck    = 3; - not needed
    // pass cfg object to i2sStream begin() call
    i2sStream.begin(cfg);

    printDebugInfo(cfg);

    // CSV output
    csvOutput.begin(StreamInfo);

    Serial.println("Setup complete, waiting for I2S sender...");
    delayMicroseconds(2000000);

    // ---- Wait for I2S sender ----
    // i gotta check this shit fr because even if nothing is connected it hallucinates a connection
    while (true) {
        if (i2sStream.isActive() && i2sStream.available() > 100) {
            Serial.println("I2S sender detected");

            AudioInfo receivedInfo = i2sStream.audioInfo();
            break;
        }
        Serial.println("Waiting for I2S sender...");
        delayMicroseconds(500000);
    }

    // ---- SD card + WAV recording (15-second clip) ----
    bool sdReady = sd_card_init();
    if (sdReady) {
        String wavPath = getWavHeader();
        sdReady = wav_writer_begin(StreamInfo, wavPath.c_str());
    }
    if (sdReady) {
        record_wav_clip(20000);  // 20 seconds - is actually 15 for some fkn reason
        recordingDone = true;
    } else {
        Serial.println("[MAIN] SD unavailable — skipping WAV recording.");
        delayMicroseconds(2000000);  // 2s delay
    }

    // ---- After recording, set up CSV-only output for loop() ----
    multiOutput.add(csvOutput);
    Serial.println("[MAIN] Entering CSV streaming mode.");
}

void loop() {
    copier.copy();
}