# System Architecture

> Understanding the I2S to Serial system design

This document describes the overall system architecture, data flow, and component interactions of the I2S to Serial converter.

**Table of Contents**
- [System Overview](#system-overview)
- [Component Architecture](#component-architecture)
- [Data Flow](#data-flow)
- [I2S Protocol Basics](#i2s-protocol-basics)
- [Module Responsibilities](#module-responsibilities)
- [Communication Sequences](#communication-sequences)
- [Buffer Management](#buffer-management)
- [Timing Considerations](#timing-considerations)

---

## System Overview

The I2S to Serial system consists of three specialized ESP32 nodes that work together to capture, transmit, and optionally store audio data.

### High-Level Architecture

```
┌──────────────────────────────────────────────────────────────┐
│                      I2S SYSTEM LAYERS                        │
└──────────────────────────────────────────────────────────────┘

Application Layer:
├── Sender (Sine Wave Generator)
├── Receiver (Data Processor)
└── Logger (Storage Handler)

        │
        ▼

Hardware Abstraction Layer (Arduino-Audio-Tools):
├── I2S Input Stream
├── I2S Output Stream
└── SD Card Interface

        │
        ▼

ESP32 Hardware:
├── I2S Peripheral (Serial Audio Interface)
├── DMA Controller (Direct Memory Access)
├── SPI Controller (for SD Card)
└── UART (Serial Communication)

        │
        ▼

Physical I2S Bus:
├── BCLK (Bit Clock)
├── WS (Word Select)
└── DO/DIN (Data)
```

---

## Component Architecture

### Sender Node (Audio Source)

```
┌─────────────────────────────────┐
│    Sender Node (ESP32 #1)       │
│  I2S Master Generator/Slave Out │
└─────────────────────────────────┘

┌─────────────────────────────────┐
│   Sine Wave Lookup Table        │
│   256 samples pre-calculated    │
│   Range: -32767 to 32767        │
└─────────────────────────────────┘
          │ reads
          ▼
┌─────────────────────────────────┐
│   Oscillator (main loop)        │
│   Updates index at 44.1 kHz     │
└─────────────────────────────────┘
          │ generates samples
          ▼
┌─────────────────────────────────┐
│   I2S Output Buffer             │
│   32-bit stereo samples         │
│   2 channels × 32-bit           │
└─────────────────────────────────┘
          │ I2S_NUM_1
          ▼
    ┌─────────────┐
    │   BCLK      │ ───→
    │   WS        │ ───→
    │   DO        │ ───→
    └─────────────┘
```

**Key Characteristics**:
- Generates 256-point sine lookup table at startup
- Runs in slave mode (clock provided by receiver)
- Outputs 32-bit stereo samples
- Uses I2S_NUM_1 peripheral

---

### Receiver Node (Data Processor)

```
┌─────────────────────────────────┐
│   Receiver Node (ESP32 #2)      │
│   I2S Master Receiver           │
└─────────────────────────────────┘

    ┌─────────────┐
    │   BCLK      │ ←───
    │   WS        │ ←───
    │   DIN       │ ←───
    └─────────────┘
          │
          ▼
┌─────────────────────────────────┐
│   I2S Input Stream              │
│   Arduino-Audio-Tools           │
│   Auto-format detection         │
└─────────────────────────────────┘
          │ 32-bit stereo at 44.1 kHz
          ▼
┌─────────────────────────────────┐
│   Stream Processing             │
│   Reads samples continuously    │
└─────────────────────────────────┘
          │
          ▼
┌─────────────────────────────────┐
│   Format Conversion             │
│   32-bit int → CSV string       │
└─────────────────────────────────┘
          │
          ▼
┌─────────────────────────────────┐
│   Serial Output (UART)          │
│   Baud: 115200                  │
│   Format: "left, right\n"       │
└─────────────────────────────────┘
```

**Key Characteristics**:
- Runs in master mode (provides I2S clock)
- Detects sender automatically
- Processes stereo 32-bit samples
- Streams data to serial at 115200 baud

---

### SD Logger Node (Storage Handler)

```
┌─────────────────────────────────┐
│   SD Logger Node (ESP32 #3)     │
│   I2S Master + SD Writer        │
└─────────────────────────────────┘

    ┌─────────────┐
    │   I2S Bus   │ ←───────────┐
    └─────────────┘             │
          │                 (same as receiver)
          │
    ┌─────────────┐
    │   SD Card   │ ←───────────┐
    │   via SPI   │         (GPIO 18/23/19/5)
    └─────────────┘
          │
          ▼
    ┌──────────────────────────────────┐
    │  Dual Stream Processing:         │
    │  ├─ I2S Input Capture            │
    │  ├─ Serial Output (debug)        │
    │  └─ SD Write (storage)           │
    └──────────────────────────────────┘
          │
          ├─→ UART (Serial)
          └─→ SD Card (WAV file)
```

**Key Characteristics**:
- Master mode I2S receiver
- Simultaneous SD card writing
- Optional serial debug output
- WAV format file storage

---

## Data Flow

### Complete Data Path (Full System)

```
┌─────────────────────────────────────────────────────────────┐
│ DATA FLOW: Sender → I2S Bus → Receiver → Serial/SD          │
└─────────────────────────────────────────────────────────────┘

1. GENERATION PHASE
   ├─ Sine LUT initialized (256 samples, 16-bit)
   ├─ Main loop runs at 44.1 kHz
   └─ Index incremented for each sample
       
2. OUTPUT PHASE
   ├─ 32-bit value created (left=sine, right=sine)
   ├─ Formatted as I2S_STD_FORMAT
   └─ Pushed to I2S buffer (DMA-managed)

3. TRANSMISSION PHASE
   ├─ BCLK rising edges clock out bits
   ├─ WS indicates L/R channel (44.1 kHz rate)
   ├─ 32 bits per sample transmitted
   └─ Entire stereo frame: 64 bits @ ~1.41 MHz

4. RECEPTION PHASE
   ├─ Receiver I2S listening (master mode)
   ├─ Auto-detects format (I2S_STD_FORMAT)
   ├─ DMA fills internal buffer
   └─ Stereo samples continuously available

5. PROCESSING PHASE
   ├─ Stream object reads 32-bit values
   ├─ Converts to signed integers
   └─ Formats as CSV string

6. OUTPUT PHASE
   ├─ Serial UART transmits CSV
   ├─ Optional: SD writes WAV
   └─ Data available to host PC
```

---

## I2S Protocol Basics

### I2S Signal Characteristics

**Bit Clock (BCLK)**
- Frequency: 44.1 kHz × 32 bits × 2 channels = 2.8224 MHz
- Source: Receiver (master mode)
- Used by: Sender (reads BCLK)
- Purpose: Synchronizes data transmission

**Word Select (WS) / LRCLK**
- Frequency: 44.1 kHz
- Source: Receiver (master mode)
- Timing: 
  - LOW (0) = Left channel data
  - HIGH (1) = Right channel data
- Duration: 32 BCLK cycles per side

**Data (DO/DIN)**
- Sender: GPIO 14 (DO - Data Out)
- Receiver: GPIO 32 (DIN - Data In)
- Timing: MSB-first on BCLK rising edges
- Format: 32-bit signed PCM

### Sample Frame Structure

```
Frame Duration: 1 / 44100 Hz = 22.68 microseconds

Each Frame Contains:
┌──────────────────────────────────────────┐
│ LEFT CHANNEL (32 bits)                   │
│ WS=0 for 32 BCLK cycles                 │
│ Transmitted MSB first                    │
└──────────────────────────────────────────┘
    ↓ (WS transitions from 0→1)
┌──────────────────────────────────────────┐
│ RIGHT CHANNEL (32 bits)                  │
│ WS=1 for 32 BCLK cycles                 │
│ Transmitted MSB first                    │
└──────────────────────────────────────────┘
    ↓ (WS transitions from 1→0)
    [Next frame begins]

Total Bits per Frame: 64 (32L + 32R)
Frame Rate: 44100 frames/second
Bit Rate: 44100 × 64 = 2,822,400 bits/second
```

---

## Module Responsibilities

### Sender Node Responsibilities

| Component | Responsibility |
|-----------|-----------------|
| **Sine Oscillator** | Generate frequency-stable sine wave at 44.1 kHz |
| **LUT Manager** | Maintain 256-sample lookup table |
| **I2S Interface** | Output stereo 32-bit samples over I2S_NUM_1 |
| **Initialization** | Setup GPIO, I2S peripheral, wait for receiver |
| **Main Loop** | Read LUT index, update I2S buffer at sample rate |

### Receiver Node Responsibilities

| Component | Responsibility |
|-----------|-----------------|
| **I2S Listener** | Monitor BCLK/WS for sender presence |
| **Stream Manager** | Create AudioI2SStream for continuous data |
| **Format Detection** | Auto-identify I2S_STD_FORMAT from sender |
| **Sample Processing** | Read 32-bit stereo samples as they arrive |
| **CSV Formatter** | Convert each sample pair to CSV string |
| **Serial Output** | Write formatted data to UART at 115200 baud |

### SD Logger Responsibilities

| Component | Responsibility |
|-----------|-----------------|
| **SD Interface** | Initialize SD card via SPI |
| **WAV Writer** | Create and write WAV format file |
| **I2S Receiver** | Capture I2S data (same as receiver node) |
| **Dual Output** | Simultaneously write to serial and SD |
| **File Management** | Create timestamped WAV files |
| **Buffer Control** | Prevent SD write delays from affecting I2S timing |

---

## Communication Sequences

### Startup Sequence

```
PHASE 1: Sender Initialization (Power On)
└─ T=0ms: Sender GPIO configured
└─ T=10ms: I2S_NUM_1 initialized (slave mode)
└─ T=20ms: Sine LUT populated
└─ T=2000ms: Ready (waits for BCLK from receiver)

PHASE 2: Receiver Initialization (Power On, delayed)
└─ T=0ms: Receiver GPIO configured
└─ T=10ms: I2S_NUM_0 initialized (master mode)
└─ T=15ms: BCLK starts (2.8224 MHz)
└─ T=20ms: WS starts (44.1 kHz)
└─ T=25ms: Receiver detects BCLK edge
└─ T=30ms: Stream parser auto-detects format
└─ T=50ms: First samples received in buffer
└─ T=100ms: CSV output begins to serial

PHASE 3: Normal Operation
└─ Sender receives BCLK, starts outputting data
└─ Receiver captures continuously
└─ Serial output streams to host
```

### Runtime Data Transfer

```
Each 22.68 microsecond cycle:

T=0.0µs    ┌─ Left Channel starts
           │  WS: 0
           │  32 BCLK edges transmit 32-bit value
T=11.34µs  ├─ Right Channel starts
           │  WS: 1
           │  32 BCLK edges transmit 32-bit value
T=22.68µs  └─ Next frame begins (sample rate = 44100 Hz)

Meanwhile (every 22.68µs):
├─ Receiver DMA fills internal buffer
├─ Stream object reads when buffer has data
└─ CSV formatted and queued to UART (~1.15 Mbps)
```

---

## Buffer Management

### I2S DMA Buffers

ESP32 I2S uses DMA (Direct Memory Access) for efficiency:

```
┌─────────────────────────────────┐
│ I2S DMA Ring Buffer             │
│ (Managed by ESP-IDF)            │
└─────────────────────────────────┘

Each buffer typically:
- Size: 256-4096 bytes
- Multiple buffers (typical: 2-4)
- Updated continuously by DMA
- Application reads newest data
- Underrun if app too slow


Receiver Buffer Flow:
────────────────────

I2S Peripheral
    ↓ (DMA writes)
[Buffer A] ← Being filled by DMA
[Buffer B] ← Previously filled, ready to read
[Buffer C] ← Application reading
    ↓
Stream Handler
    ↓
CSV Formatter
    ↓
UART TX Buffer
    ↓
Serial Output
```

### Serial Output Buffer

```
UART TX Buffer (115200 baud):
├─ Buffer Size: Typically 256-512 bytes
├─ Data Rate: 115200 bits/sec ≈ 14400 bytes/sec
├─ Sample Rate: 44100 Hz × 2 channels × 4 bytes = 352800 bytes/sec
├─ Ratio: 352800 / 14400 ≈ 24.5x faster than serial can output
└─ Result: CSV output is DOWNSAMPLED by serial buffer limits
```

**Important**: Serial output cannot keep up with real-time I2S data.
- Full I2S rate: 352 KB/sec (44.1kHz × 2ch × 32bit)
- Serial rate: ~14 KB/sec (115200 baud)
- Only ~4% of data reaches serial in real-time

---

## Timing Considerations

### Sample Rate Precision

The sender generates samples at exactly 44.1 kHz using:
- Counter-based timing
- Receiver I2S clock (master)
- Typical accuracy: ±0.1% (sufficient for audio)

### Latency Analysis

```
Total System Latency from Sender Output to Serial:

Sender I2S Output: 0 µs (baseline)
    ↓
I2S Bus Propagation: ~5 µs (wire delays)
    ↓
Receiver DMA Write: ~22.68 µs (one sample frame)
    ↓
Stream Read: ~22.68 µs (sample-dependent)
    ↓
CSV Conversion: ~2-5 µs
    ↓
UART Transmission: ~90 µs (9 bytes @ 115200)
    ├─ "32768, 32768\n" = 13 bytes
    └─ @ 115200: 113 µs per line

Total Latency: ~150-300 µs (1.5-3 sample periods)
```

### Clock Distribution

```
Master Clock (Receiver provides):
├─ BCLK: 2.8224 MHz (44.1 kHz × 32 × 2)
│  └─ Precision: Crystal oscillator on ESP32
├─ WS: 44.1 kHz (sample rate)
│  └─ Derived from BCLK counter
└─ Timing Drift: <0.1% over extended periods

Sender Synchronization:
├─ Locks to received BCLK
├─ Timing: Reads BCLK edges to determine sample boundary
└─ Result: Phase-locked operation
```

### Synchronization Between Nodes

```
Multiple Receiver Nodes:
┌─────────────────────┐
│  One Sender         │ (clock source)
└─────────────────────┘
        │
    I2S Bus (BCLK, WS)
        │
    ┌───┴────┬────────┐
    │        │        │
    ▼        ▼        ▼
[Rcv1]   [Rcv2]   [SD Log]

All receivers synchronized to same clock:
✓ Sender BCLK drives all nodes
✓ WS common to all nodes
✓ DO/DIN separate for each receiver
✓ Natural synchronization: all see same BCLK transitions
```

---

## Performance Characteristics

### Throughput

| Parameter | Value |
|-----------|-------|
| Sample Rate | 44,100 samples/sec |
| Channels | 2 (stereo) |
| Bit Depth | 32-bit |
| Raw I2S Rate | 44,100 × 2 × 32 = 2,822,400 bits/sec |
| Raw I2S Rate | = 352,800 bytes/sec |
| Serial Output Rate | 115,200 bits/sec = 14,400 bytes/sec |
| Compression Ratio | 24.5 : 1 (serial much slower) |

### Resource Usage

| Resource | Sender | Receiver | Logger |
|----------|--------|----------|--------|
| I2S Peripheral | 1 | 1 | 1 |
| DMA Channels | 1 | 1 | 2 |
| GPIO Pins | 3 | 3 | 3+4 |
| Memory (code+data) | ~150 KB | ~200 KB | ~250 KB |
| Real-time Priority | Medium | High | High |

---

**Next Steps**: 
- See [I2S_PROTOCOL.md](./I2S_PROTOCOL.md) for protocol details
- See [HARDWARE_SETUP.md](./HARDWARE_SETUP.md) for physical implementation
- See [DEBUGGING.md](./DEBUGGING.md) for timing verification techniques
