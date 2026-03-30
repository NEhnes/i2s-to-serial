/**
 * @file wav_writer.h
 * @brief WAV file recording to SD card using arduino-audio-tools
 *
 * Provides a simple interface to open a .wav file on the SD card,
 * expose it as a stream destination, and close it cleanly.
 *
 * ============================================================================
 *  Bit Depth Toggle
 * ============================================================================
 *
 *  By default, the WAV file matches whatever AudioInfo you pass in
 *  (e.g., 32-bit if your I2S is 32-bit). To record at 16-bit instead:
 *
 *    1. Change the AudioInfo you pass to wav_writer_begin():
 *         AudioInfo wavInfo(44100, 2, 16);        // <-- 16-bit
 *         wav_writer_begin(wavInfo, "/rec.wav");
 *
 *    2. If your I2S source is 32-bit, you'll also need a
 *       FormatConverterStream to downsample from 32→16 bit before
 *       the WAV encoder. Example:
 *
 *         NumberFormatConverterStream converter(i2sStream);
 *         auto convCfg = converter.defaultConfig();
 *         convCfg.from_bits_per_sample = 32;
 *         convCfg.to_bits_per_sample   = 16;
 *         convCfg.copyFrom(wavInfo);
 *         converter.begin(convCfg);
 *
 *       Then use `converter` as the source in your StreamCopy instead
 *       of the raw i2sStream.
 *
 * ============================================================================
 */

#pragma once

#include <SD.h>
#include "AudioTools.h"

/**
 * @brief Open a WAV file on the SD card and prepare the encoder pipeline.
 *
 * Must be called AFTER sd_card_init() succeeds.
 *
 * @param info  Audio format (sample rate, channels, bits per sample).
 *              The WAV file will use this exact format.
 * @param path  File path on the SD card, e.g. "/recording.wav".
 * @return true  File opened and encoder ready.
 * @return false Failed to open file.
 */
bool wav_writer_begin(const AudioInfo &info, const char *path);

/**
 * @brief Get a reference to the encoded output stream.
 *
 * Use this as a destination for StreamCopy or MultiOutput.
 * Only valid after wav_writer_begin() returns true.
 */
EncodedAudioStream &wav_writer_stream();

/**
 * @brief Finalize and close the WAV file.
 *
 * Flushes remaining data and closes the file. The WAV header is
 * updated so the file is playable. Call this when you want to stop
 * recording (e.g., after a timed period, on button press, etc.).
 */
void wav_writer_end();
