# I2S to Serial Converter

This repository contains two ESP32-based applications that convert I2S audio data to serial output:

- **sender_node**: An I2S transmitter that generates a sine wave and outputs it via I2S protocol
- **receiver_node**: An I2S receiver that captures the I2S data and outputs it as CSV-formatted serial data

Both applications are designed to work together for audio data transmission between two ESP32 devices.

## Hardware Requirements

- Two ESP32 development boards (ESP32DOIT DevKit V1 recommended)
- I2S connection between boards:
  - BCLK (Bit Clock) - Connect sender pin 26 to receiver pin 34 (default)
  - WS (Word Select/LRCLK) - Connect sender pin 25 to receiver pin 35 (default)
  - DO (Data Out) - Connect sender pin 14 to receiver pin 32 (default)

## Project Structure

```
i2s-to-serial/
├── README.md
├── sender_node/
│   ├── platformio.ini
│   └── src/
│       └── main.cpp
└── receiver_node/
    ├── platformio.ini
    └── src/
        └── main.cpp
```

## Sender Node (I2S Transmitter)

The sender_node generates a 32-bit sine wave at 44.1 kHz and transmits it over I2S in slave mode.

### Features:
- Generates a 256-point sine wave lookup table
- Configures I2S in slave mode with 32-bit samples
- Outputs stereo data (right and left channels)
- Uses I2S_NUM_1 for transmission

### Pin Configuration:
- BCLK (Bit Clock): GPIO 26
- WS (Word Select): GPIO 25
- DO (Data Out): GPIO 14

## Receiver Node (I2S Receiver)

The receiver_node captures I2S data and outputs it as CSV-formatted serial data for analysis.

### Features:
- Uses the Arduino-Audio-Tools library to handle I2S streaming
- Configures I2S in master mode to receive data
- Converts received 32-bit integers to CSV format via Serial output
- Auto-detects I2S sender and displays audio configuration

### Pin Configuration:
- BCLK (Bit Clock): GPIO 34
- WS (Word Select): GPIO 35
- DIN (Data In): GPIO 32

## Compilation & Deployment

### Prerequisites:
- PlatformIO IDE installed
- ESP32 board support installed

### Build and Upload:
1. Open the project in PlatformIO
2. Select the appropriate environment (env:esp32)
3. Build and upload sender_node first
4. Then build and upload receiver_node

### Serial Monitor Settings:
- Baud rate: 115200

## Operation Sequence

1. Power on the sender_node device
2. Wait for 2 seconds (sender initializes)
3. Power on the receiver_node device
4. Receiver will wait for I2S data and display confirmation once detected
5. Serial output will show CSV-formatted 32-bit integer values

## Notes

- The sender generates a sine wave at 44.1 kHz with 32-bit resolution
- The receiver expects the I2S data in standard format (I2S_STD_FORMAT)
- Both devices use the same audio configuration (44.1 kHz sample rate, 32-bit samples)
- The receiver has a 2-second delay at startup to allow sender to initialize fully
- The receiver uses the Arduino-Audio-Tools library for audio stream processing
- Pin assignments for the receiver match the example from pschatzmann/arduino-audio-tools

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Author

Created by [Your Name]

## Credits

- Audio Tools Library by Phil Schatzmann: https://github.com/pschatzmann/arduino-audio-tools
- ESP32 I2S documentation: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2s.html

## Troubleshooting

1. **No data received**: Ensure I2S line connections are correct and all devices are powered
2. **Garbage data**: Check that both devices use the same I2S format (I2S_STD_FORMAT)
3. **Timing issues**: Adjust delays in receiver_node if necessary
4. **Pin conflicts**: Verify GPIO pins aren't used by other peripherals
5. **Missing library**: Ensure `arduino-audio-tools` is installed in receiver_node

## Debugging Tips

- Use the Serial Monitor to check for "I2S sender detected!" message on receiver
- Verify sender outputs "I2S sender ready" message
- Test connections with a multimeter on I2S lines
- Try toggling `use_apll` setting in receiver configuration
- Monitor receiver's serial output for consistent 32-bit integer values

## Future Enhancements

- Add volume control in sender_node
- Implement different waveforms (square, triangle)
- Add buffering to handle data rate differences
- Configure for different sample rates
- Add data logging to SD card

---

**NOTE**: This documentation accompanies the code in this repository. No logic changes were made to the source code files as requested. Only comments were added for clarity.
