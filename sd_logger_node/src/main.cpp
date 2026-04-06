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
// 16000 Hz, stereo, 32-bit — must match the sender node.
AudioInfo StreamInfo(16000, 2, 32);

// input to stream copy function
// this represents the signal coming from microphone
I2SStream i2sStream;

// csv output object
CsvOutput<int32_t> csvOutput(Serial);

// flag to track SD card initialization status
bool sdReady = false;

// active session directory name (set during startup)
String sessionDir = "";

// ---------------------------------------------------------------------------
// Create a unique session directory on the SD card at boot.
// Increments until it finds a free slot: AUDIO_LOGGER_SESSION_1, _2, etc.
// Returns true on success.
// ---------------------------------------------------------------------------
bool createSessionDir() {
    if (!sdReady) {
        Serial.println("[SESSION] SD not ready — skipping session dir creation");
        return false;
    }

    for (int i = 1; i <= 999; i++) {
        String dirName = "/AUDIO_LOGGER_SESSION_" + String(i);
        if (!SD.exists(dirName)) {
            if (SD.mkdir(dirName)) {
                sessionDir = dirName;
                Serial.print("[SESSION] Created ");
                Serial.println(sessionDir);
                return true;
            } else {
                Serial.print("[SESSION] Failed to create ");
                Serial.println(dirName);
                return false;
            }
        }
    }

    Serial.println("[SESSION] Session limit (999) exceeded — no dir created");
    return false;
}

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
// Helper function to generate a unique WAV filename for each recording.
// Checks for existing files named recording_1.wav, recording_2.wav, etc. up
// to recording_100.wav, and returns the first available filename.
// If all 100 filenames are taken, returns recording_ERROR.wav and logs an error.
// ---------------------------------------------------------------------------
String getWavPath() {
    // grab prefix for filename based on session directory; if sessionDir is empty, use root
    String prefix = sessionDir.length() > 0 ? sessionDir : "/";
    for (int i = 1; i <= 100; i++) {
        String filename = prefix + "/recording_" + String(i, DEC) + String("_04-05-26.wav");
        if (!SD.exists(filename)) {
            return filename;
        }
    }
    Serial.println("ERROR: Recording limit (100) exceeded");
    return prefix + "/recording_ERROR.wav";
}

// ---------------------------------------------------------------------------
// Write I2S audio to SD card WAV file for specified duration
// ---------------------------------------------------------------------------
void write_sd(unsigned long durationMs) {

    // check if SD card is ready before trying to write
    if (!sdReady) {
        Serial.println("SD card not ready - skipping SD write");
        delayMicroseconds(200000);  // 2s to read serial
        return;
    }

    // check SD card size before trying to write - zero means card is invalid/full
    if (SD.cardSize() == 0) {
        Serial.println("SD card error - skipping SD write");
        delayMicroseconds(200000);  // 2s to read serial
        return;
    }

    // grab file path and header data
    String wavPath = getWavPath();
    if (!wav_writer_begin(StreamInfo, wavPath.c_str())) {
        Serial.println("Failed to open WAV file - skipping SD write");
        delayMicroseconds(200000);
        return;
    }

    StreamCopy wavCopier(wav_writer_stream(), i2sStream);
    unsigned long startTime = millis();

    digitalWrite(LED_BUILTIN, HIGH);

    while ((millis() - startTime) < durationMs) {
        wavCopier.copy();
    }

    digitalWrite(LED_BUILTIN, LOW);
    wav_writer_end();
}

// ---------------------------------------------------------------------------
// Write I2S audio to Serial as CSV for specified duration
// ---------------------------------------------------------------------------
void write_csv(unsigned long durationMs) {

    // set up copier itself
    StreamCopy csvCopier(csvOutput, i2sStream);

    // copy for set amnt of time
    unsigned long startTime = millis();
    while ((millis() - startTime) < durationMs) {
        csvCopier.copy();
    }
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

    // init SD card with retry logic
    for (int i = 0; i < 3; i++) {
        sdReady = sd_card_init();
        if (sdReady) break;
        Serial.printf("[SD] Retry %d/3...\n", i + 1);
        delay(500);
    }

    // create unique session directory
    createSessionDir();

    Serial.println("[MAIN] Entering CSV streaming mode.");
}

void loop() {
    // sd cycle
    Serial.println("Starting SD write cycle");
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);
    write_sd(20000);
    // csv cycle
    Serial.println("Starting CSV write cycle");
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    write_csv(20000);
}