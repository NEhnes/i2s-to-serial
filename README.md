# I2S to Serial Converter

> **Project Status**: Active Development | **Last Updated**: March 2024

A modular ESP32 I2S audio data converter system consisting of three nodes for audio transmission, reception, and SD card logging over I2S protocol.

**Table of Contents**
- [Quick Start](#quick-start)
- [Project Overview](#project-overview)
- [System Architecture](#system-architecture)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [Hardware Setup](#hardware-setup)
- [Node Documentation](#node-documentation)
- [Building & Deployment](#building--deployment)
- [Operation](#operation)
- [Troubleshooting](#troubleshooting)
- [Development](#development)
- [License & Credits](#license--credits)

## Quick Start

**First time? Start here in 5 minutes:**

1. **Prerequisites**:
   - Two ESP32 DOIT DevKit V1 boards (minimum 2 for sender+receiver, 3 for logging)
   - USB cables for programming and power
   - I2S connecting wires (3 signal lines: BCLK, WS, DO/DIN)
   - PlatformIO installed

2. **Wire the boards** (see [Hardware Setup](#hardware-setup)):
   - Sender BCLK (GPIO 26) → Receiver BCLK (GPIO 34)
   - Sender WS (GPIO 25) → Receiver WS (GPIO 35)
   - Sender DO (GPIO 14) → Receiver DIN (GPIO 32)
   - Common GND between boards

3. **Build and upload**:
   ```bash
   # Terminal 1: Sender
   cd sender_node && platformio run -t upload
   
   # Terminal 2: Receiver
   cd receiver_node && platformio run -t upload
   ```

4. **Monitor output**:
   ```bash
   platformio device monitor -b 115200
   ```

5. **Verify**: Look for "I2S sender detected!" message on receiver serial output

For detailed setup, see **[GETTING_STARTED.md](./docs/GETTING_STARTED.md)**

---

## Project Overview

This repository implements a complete I2S audio data system with three specialized ESP32 applications:

| Node | Purpose | Mode | Output |
|------|---------|------|--------|
| **sender_node** | I2S audio source | Slave | 44.1 kHz sine wave |
| **receiver_node** | I2S data capture | Master | CSV serial data |
| **sd_logger_node** | I2S with SD logging | Master | WAV file + serial |

### Key Features

- ✅ **I2S Protocol**: Full 32-bit stereo I2S implementation at 44.1 kHz
- ✅ **Multi-Node**: Modular design for flexible audio pipelines
- ✅ **Data Logging**: SD card support for long-term recording
- ✅ **Serial Output**: Real-time data streaming over USB
- ✅ **Arduino Framework**: Cross-platform compatibility
- ✅ **Audio Tools**: Leverages Phil Schatzmann's audio library

---

## System Architecture

```
┌─────────────────────────────────────────────────────────┐
│                   I2S TO SERIAL SYSTEM                   │
└─────────────────────────────────────────────────────────┘

    Sender Node              I2S Bus             Receiver Node
    ───────────              ───────             ─────────────
    
    ┌─────────┐              BCLK ───────────────> ┌─────────┐
    │  Sine   │              WS ─────────────────> │ Audio   │
    │  Wave   │              DO ─────────────────> │ Stream  │──┐
    │  Osc    │              GND ────────────────> │         │  │
    └─────────┘              ↓                     └─────────┘  │
    I2S_NUM_1              Master                                │
    Slave Mode                                    Serial Output  │
    32-bit                  ┌─────────────────────────────────┐ │
    44.1 kHz                │   Alternative: SD Logger Node   │ │
                            │                                 │ │
                            │  ┌──────────────┐  ┌─────────┐ │ │
                            │  │ I2S Receiver │→ │  WAV    │ │ │
                            │  └──────────────┘  │ Writer  │ │ │
                            │                    └─────────┘ │ │
                            │         ↓ (optional)          │ │
                            │    Serial Output              │ │
                            └─────────────────────────────────┘ │
                                       ↑                        │
                                       └────────────────────────┘
                                    CSV/Debug Data to PC
```

For detailed architecture information, see **[ARCHITECTURE.md](./docs/ARCHITECTURE.md)**

---

## Project Structure

```
i2s-to-serial/
├── README.md                           # This file
├── docs/                              # Documentation
│   ├── GETTING_STARTED.md            # Quick start guide
│   ├── ARCHITECTURE.md               # System design
│   ├── HARDWARE_SETUP.md             # Wiring & hardware
│   ├── I2S_PROTOCOL.md              # I2S protocol details
│   ├── BUILD_GUIDE.md               # Build instructions
│   ├── DEBUGGING.md                 # Debugging tips
│   ├── DEVELOPMENT.md               # Dev environment
│   ├── API_REFERENCE.md             # API documentation
│   ├── FAQ.md                       # Frequently asked questions
│   ├── CHANGELOG.md                 # Version history
│   └── CONTRIBUTING.md              # Contribution guidelines
│
├── sender_node/                     # I2S Sine Wave Generator
│   ├── README.md                   # Node-specific docs
│   ├── platformio.ini             # PlatformIO config
│   ├── src/
│   │   └── main.cpp               # Sender implementation
│   ├── include/                   # Header files
│   ├── lib/                       # Local libraries
│   └── test/                      # Test files
│
├── receiver_node/                   # I2S Data Receiver
│   ├── README.md                   # Node-specific docs
│   ├── platformio.ini             # PlatformIO config
│   ├── src/
│   │   └── main.cpp               # Receiver implementation
│   ├── include/
│   ├── lib/
│   └── test/
│
└── sd_logger_node/                  # I2S + SD Card Logger
    ├── README.md                   # Node-specific docs
    ├── platformio.ini             # PlatformIO config
    ├── src/
    │   ├── main.cpp               # Main logger logic
    │   ├── sd_card.cpp            # SD card interface
    │   └── wav_writer.cpp         # WAV file writing
    ├── include/
    ├── lib/
    └── test/
```

---

## Getting Started

### Prerequisites

**Software**:
- [PlatformIO IDE](https://platformio.org/) or PlatformIO CLI
- Python 3.6+ (required by PlatformIO)
- Git (for cloning)

**Hardware**:
- 2–3× ESP32 DOIT DevKit V1 boards
- USB cables (Type B for programming)
- Connecting wires (22-24 AWG recommended)
- Optional: SD card (for sd_logger_node)

### Installation Steps

1. **Clone the repository**:
   ```bash
   git clone https://github.com/your-username/i2s-to-serial.git
   cd i2s-to-serial
   ```

2. **Install PlatformIO** (if not already installed):
   ```bash
   pip install platformio
   ```

3. **Connect your boards** via USB and identify port:
   ```bash
   platformio device list
   ```

4. **Proceed to [GETTING_STARTED.md](./docs/GETTING_STARTED.md)** for detailed first-run walkthrough

---

## Hardware Setup

### Quick Reference: Pin Configuration

#### Sender Node (Slave)
| Signal | GPIO | Standard |
|--------|------|----------|
| BCLK   | 26   | Output   |
| WS     | 25   | Output   |
| DO     | 14   | Output   |
| GND    | GND  | Reference|

#### Receiver Node (Master)
| Signal | GPIO | Standard |
|--------|------|----------|
| BCLK   | 34   | Input    |
| WS     | 35   | Input    |
| DIN    | 32   | Input    |
| GND    | GND  | Reference|

#### SD Logger Node (Master)
| Signal | GPIO | Standard |
|--------|------|----------|
| BCLK   | 34   | Input    |
| WS     | 35   | Input    |
| DIN    | 32   | Input    |
| CS     | 5    | Output   |
| MOSI   | 23   | Output   |
| MISO   | 19   | Input    |
| CLK    | 18   | Output   |
| GND    | GND  | Reference|

### Wiring Instructions

**I2S Connection** (between Sender and Receiver(s)):
```
Sender          →    Receiver / SD Logger
─────────────────────────────────────────
GPIO 26 (BCLK)  →    GPIO 34 (BCLK)
GPIO 25 (WS)    →    GPIO 35 (WS)
GPIO 14 (DO)    →    GPIO 32 (DIN)
GND             →    GND (common)
```

For detailed wiring diagrams and troubleshooting, see **[HARDWARE_SETUP.md](./docs/HARDWARE_SETUP.md)**

---

## Node Documentation

Each node has dedicated documentation:

### [sender_node/README.md](./sender_node/README.md)
- Sine wave generation at 44.1 kHz
- I2S slave mode configuration
- Pin assignments and features
- Building and testing

### [receiver_node/README.md](./receiver_node/README.md)
- Real-time I2S data capture
- CSV serial output format
- Arduino-Audio-Tools integration
- Validation procedures

### [sd_logger_node/README.md](./sd_logger_node/README.md)
- I2S capture with SD storage
- WAV file format writing
- SD card initialization
- Data logging procedures

---

## Building & Deployment

### Prerequisites

All nodes require:
- PlatformIO (IDE or CLI)
- ESP32 board support in PlatformIO
- USB connection to development machine

Additional dependencies:
- **receiver_node** & **sd_logger_node**: `arduino-audio-tools` library (auto-installed)
- **sd_logger_node**: SD card support libraries (auto-installed)

### Build Commands

```bash
# Build specific node
cd sender_node && platformio run

# Upload to board (device must be connected)
cd sender_node && platformio run -t upload

# Monitor serial output (115200 baud)
cd sender_node && platformio device monitor -b 115200

# Build all nodes
for node in sender_node receiver_node sd_logger_node; do
  cd "$node" && platformio run && cd ..
done
```

### Upload Sequence

**For basic sender+receiver setup**:
1. Upload sender_node first (provides I2S clock)
2. Upload receiver_node second
3. Power cycle both boards (sender first)

**For three-node setup**:
1. sender_node (primary clock source)
2. receiver_node (optional if also logging)
3. sd_logger_node (logging node)

### Serial Monitor Settings

- **Baud Rate**: 115200
- **Data Bits**: 8
- **Stop Bits**: 1
- **Parity**: None
- **Flow Control**: None

---

## Operation

### Typical Startup Sequence

1. **Power up sender_node** → waits 2 seconds, initializes I2S, begins transmitting
2. **Power up receiver_node(s)** → listens for I2S clock, detects sender
3. **Serial output starts** → streaming 32-bit stereo samples as CSV

### Expected Output

**Receiver Serial Console**:
```
I2S sender detected!
Starting audio stream processing...
32768, -32768
16384, -16384
8192, -8192
0, 0
-8192, 8192
-16384, 16384
-32768, 32768
...
```

### Data Format

- **Sample Rate**: 44.1 kHz (44100 Hz)
- **Resolution**: 32-bit signed integers
- **Channels**: 2 (stereo: left, right)
- **Output Format**: CSV per line (left, right)
- **Value Range**: -2,147,483,648 to 2,147,483,647

### Stopping Operation

- Press `Ctrl+C` in serial monitor to disconnect
- Power cycle boards to restart
- Data transmission continues until powered off

---

## Troubleshooting

### No Serial Output

**Problem**: Serial monitor shows nothing or garbage data

**Solutions**:
1. ✅ Verify baud rate is **115200**
2. ✅ Confirm USB cable is properly connected
3. ✅ Check device appears in `platformio device list`
4. ✅ Unplug and replug USB cable
5. ✅ Try different USB port
6. ✅ Restart serial monitor application

**Reference**: [DEBUGGING.md](./docs/DEBUGGING.md)

---

### I2S Connection Issues

**Problem**: "I2S sender not detected" on receiver

**Solutions**:
1. ✅ Verify wire connections match pinout table
2. ✅ Check sender is powered on and running first
3. ✅ Confirm GND is connected (common ground)
4. ✅ Test wires with multimeter (should show ~3.3V on data lines)
5. ✅ Try different wires (eliminate bad connections)
6. ✅ Ensure sender uploaded successfully (should show "I2S sender ready")

**Reference**: [HARDWARE_SETUP.md](./docs/HARDWARE_SETUP.md#troubleshooting)

---

### Library Dependencies

**Problem**: Build fails with missing library

**Solution**: Libraries auto-install via platformio.ini
```bash
# Force re-download
rm -rf .pio/libdeps
platformio run -t clean
platformio run
```

**Reference**: [BUILD_GUIDE.md](./docs/BUILD_GUIDE.md#dependency-issues)

---

### SD Card Issues (sd_logger_node)

**Problem**: SD card not detected or write fails

**Solutions**:
1. ✅ Format SD card to FAT32
2. ✅ Ensure CS pin properly connected (GPIO 5)
3. ✅ Check SPI pins (CLK:18, MOSI:23, MISO:19)
4. ✅ Verify card is inserted fully
5. ✅ Try different card (may have compatibility issue)

**Reference**: [sd_logger_node/README.md](./sd_logger_node/README.md#troubleshooting)

---

### Advanced Troubleshooting

For detailed debugging techniques, logic analyzer setup, and performance monitoring:
- See **[DEBUGGING.md](./docs/DEBUGGING.md)**
- See **[FAQ.md](./docs/FAQ.md)**

---

## Development

### For Contributors

See **[CONTRIBUTING.md](./docs/CONTRIBUTING.md)** for:
- Development environment setup
- Code style guidelines
- Pull request process
- Testing requirements

### Development Environment

```bash
# Clone and set up
git clone <repo>
cd i2s-to-serial

# Install dependencies
pip install platformio

# Select environment
export PLATFORMIO_ENVS="esp32"

# Build and test
platformio run -t upload
platformio device monitor
```

See **[DEVELOPMENT.md](./docs/DEVELOPMENT.md)** for detailed setup.

### Building Locally

```bash
# Clean build all nodes
for node in sender_node receiver_node sd_logger_node; do
  cd "$node"
  platformio run -t clean
  platformio run
  cd ..
done
```

See **[BUILD_GUIDE.md](./docs/BUILD_GUIDE.md)** for advanced options.

---

## Further Reading

| Topic | Document |
|-------|----------|
| Quick Start | [GETTING_STARTED.md](./docs/GETTING_STARTED.md) |
| System Design | [ARCHITECTURE.md](./docs/ARCHITECTURE.md) |
| Hardware Wiring | [HARDWARE_SETUP.md](./docs/HARDWARE_SETUP.md) |
| I2S Protocol | [I2S_PROTOCOL.md](./docs/I2S_PROTOCOL.md) |
| Building | [BUILD_GUIDE.md](./docs/BUILD_GUIDE.md) |
| Debugging | [DEBUGGING.md](./docs/DEBUGGING.md) |
| API Reference | [API_REFERENCE.md](./docs/API_REFERENCE.md) |
| Questions? | [FAQ.md](./docs/FAQ.md) |
| Changes | [CHANGELOG.md](./docs/CHANGELOG.md) |

---

## License & Credits

### License

This project is licensed under the **MIT License** - see the [LICENSE](./LICENSE) file for details.

### Credits & Attribution

- **Audio Tools Library**: [Phil Schatzmann - arduino-audio-tools](https://github.com/pschatzmann/arduino-audio-tools)
- **ESP32 I2S Documentation**: [Espressif - ESP32 I2S API Reference](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2s.html)
- **ESP32 Board Support**: [PlatformIO - Espressif32 Platform](https://github.com/platformio/platform-espressif32)

### Author

Created as an open-source project for ESP32-based audio processing systems.

---

**Need help?** Check the [FAQ](./docs/FAQ.md) or [DEBUGGING](./docs/DEBUGGING.md) guide.

**Want to contribute?** See [CONTRIBUTING.md](./docs/CONTRIBUTING.md)

**Found a bug?** Open an issue on GitHub.

*Last updated: March 2024*
