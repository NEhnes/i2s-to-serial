/**
 * @file streams-i2s-serial.ino
 * @author Phil Schatzmann
 * @brief see https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-stream/streams-i2s-serial/README.md
 * 
 * @author Phil Schatzmann
 * @copyright GPLv3
 */


#include "AudioTools.h"

AudioInfo info(44100, 2, 32);
I2SStream i2sStream; // Access I2S as stream
CsvOutput<int32_t> csvOutput(Serial);
StreamCopy copier(csvOutput, i2sStream); // copy i2sStream to csvOutput

// Arduino Setup
void setup(void) {

    // may need to set the pins to input with pulldown to avoid random noise being detected as data - unsure
    // pinMode(14, INPUT_PULLDOWN); // BCLK
    // pinMode(15, INPUT_PULLDOWN); // LRCLK
    // pinMode(32, INPUT_PULLDOWN); // DIN

    delayMicroseconds(2000000);  // 2 second in microseconds

    Serial.begin(115200);
    Serial.println("Serial communication initialized");

    delayMicroseconds(2000000);  // 2 second in microseconds

    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);
    
    auto cfg = i2sStream.defaultConfig(RX_MODE);
    cfg.copyFrom(info);
    cfg.i2s_format = I2S_STD_FORMAT; // or try with I2S_LSB_FORMAT
    cfg.is_master = true;
    cfg.use_apll = false;  // try with yes
    cfg.pin_mck = 3; 
    i2sStream.begin(cfg);

    // make sure that we have the correct channels set up
    csvOutput.begin(info);

    // TEMPORARY PIN CHECK
    Serial.print("BCLK: "); Serial.println(cfg.pin_bck);
    Serial.print("LRCLK: "); Serial.println(cfg.pin_ws);
    Serial.print("DIN: "); Serial.println(cfg.pin_data);

    Serial.println();
    Serial.println("Setup complete, waiting for I2S sender...");
    delayMicroseconds(2000000);  // 2 second in microseconds

    // Wait for I2S data - added by me to debug
    while (true) {
    if (i2sStream.isActive() && i2sStream.available() > 100) { // require 100+ bytes
        Serial.println("I2S sender detected!");

        AudioInfo receivedInfo = i2sStream.audioInfo();
        Serial.print("Received Audio Info - Sample Rate: "); Serial.print(receivedInfo.sample_rate);
        Serial.print(", Channels: "); Serial.print(receivedInfo.channels);
        Serial.print(", Bits per Sample: "); Serial.println(receivedInfo.bits_per_sample);
        Serial.println("BCLK PIN: " + String(cfg.pin_bck) + ", LRCLK PIN: " + String(cfg.pin_ws) + ", DIN PIN: " + String(cfg.pin_data));

        break;
    }
    Serial.println("Waiting for I2S sender...");
    delayMicroseconds(500000);
}

}

// Arduino loop - copy data
void loop() {
    copier.copy();
    // delayMicroseconds(1000000);  // 1000 ms in microseconds

    // Serial.println("Looping...");
    // delayMicroseconds(1000000);  // 1 second in microseconds
}