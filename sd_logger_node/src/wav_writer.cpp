/**
 * @file wav_writer.cpp
 * @brief WAV file recording implementation
 */

#include "wav_writer.h"

// ---------------------------------------------------------------------------
//  Module-internal state
// ---------------------------------------------------------------------------
static File            wavFile;
static WAVEncoder      wavEncoder;
static EncodedAudioStream encodedStream(&wavFile, &wavEncoder);

// ---------------------------------------------------------------------------
//  Public API
// ---------------------------------------------------------------------------

bool wav_writer_begin(const AudioInfo &info, const char *path) {
    // If a file is already open, close it first
    if (wavFile) {
        wav_writer_end();
    }

    // Open (or create) the file for writing.
    // Using FILE_WRITE truncates an existing file on ESP32 SD lib.
    wavFile = SD.open(path, FILE_WRITE);
    if (!wavFile) {
        Serial.printf("[WAV] ERROR: Could not open %s for writing!\n", path);
        return false;
    }

    // Point the encoder stream at the (now-open) file and start it.
    // begin(info) writes the WAV header automatically.
    encodedStream.begin(info);

    Serial.printf("[WAV] Recording started: %s\n", path);
    Serial.printf("[WAV]   Sample rate : %d Hz\n", info.sample_rate);
    Serial.printf("[WAV]   Channels    : %d\n",    info.channels);
    Serial.printf("[WAV]   Bits/sample : %d\n",    info.bits_per_sample);

    return true;
}

EncodedAudioStream &wav_writer_stream() {
    return encodedStream;
}

void wav_writer_end() {
    if (wavFile) {
        wavFile.flush();
        wavFile.close();
        Serial.println("[WAV] Recording stopped — file closed.");
    }
}
