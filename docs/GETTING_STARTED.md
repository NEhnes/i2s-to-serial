# Getting Started Guide

> Complete first-time setup in 15 minutes

This guide walks you through setting up your I2S to Serial system from scratch. By the end, you'll have working I2S communication between two ESP32 boards.

**Time Required**: ~15 minutes (excluding hardware setup)

**Table of Contents**
- [Prerequisites](#prerequisites)
- [Step 1: Hardware Assembly](#step-1-hardware-assembly)
- [Step 2: Software Installation](#step-2-software-installation)
- [Step 3: Building & Uploading](#step-3-building--uploading)
- [Step 4: Verification](#step-4-verification)
- [Troubleshooting](#troubleshooting)
- [Next Steps](#next-steps)

---

## Prerequisites

### Hardware Checklist

- [ ] 2× ESP32 DOIT DevKit V1 boards (or compatible)
- [ ] 2× USB Type B cables (for programming)
- [ ] 4× Dupont wires (connecting I2S signals)
- [ ] 1× Dupont wire (ground connection)
- [ ] Breadboard (optional, for easier connections)
- [ ] Computer with USB ports

### Software Checklist

- [ ] Python 3.6 or newer installed
- [ ] Git installed (for cloning)
- [ ] Administrator/sudo access (for driver installation)

---

## Step 1: Hardware Assembly

### 1.1 Wire the I2S Connection

**Sender Board → Receiver Board**

Connect these three signals **plus ground**:

```
Sender (Slave)          Receiver (Master)
─────────────────────────────────────────
GPIO 26 (BCLK) ─────→ GPIO 34 (BCLK)
GPIO 25 (WS)   ─────→ GPIO 35 (WS)
GPIO 14 (DO)   ─────→ GPIO 32 (DIN)
GND            ─────→ GND (common ground)
```

### 1.2 Verify Connections

```
Visual Check:
✓ All four wires firmly inserted
✓ Colors don't matter (use 4 different colors if possible for clarity)
✓ No bent pin headers
✓ Common ground is connected
```

### 1.3 Connect USB Cables

- Connect **Sender board** USB to computer (note the port, e.g., `/dev/ttyUSB0`)
- Connect **Receiver board** USB to computer (note the second port)

**On Linux/Mac**:
```bash
ls /dev/ttyUSB* 
# or
ls /dev/cu.usbserial*
```

**On Windows**:
- Check Device Manager → COM Ports

---

## Step 2: Software Installation

### 2.1 Install PlatformIO

```bash
pip install platformio
```

Verify installation:
```bash
platformio --version
```

Expected output: `PlatformIO Core X.XX.X`

### 2.2 Clone the Repository

```bash
git clone https://github.com/your-username/i2s-to-serial.git
cd i2s-to-serial
```

### 2.3 Detect Connected Boards

```bash
platformio device list
```

Expected output (example):
```
/dev/ttyUSB0 - Silicon Labs CP210x UART Bridge
/dev/ttyUSB1 - Silicon Labs CP210x UART Bridge
```

**Note the port names** for the next step.

### 2.4 Configure Port Assignment

PlatformIO should auto-detect boards. If you need to specify, edit `platformio.ini` files:

**sender_node/platformio.ini**:
```ini
[env:esp32]
upload_port = /dev/ttyUSB0  # or COM3 on Windows
```

**receiver_node/platformio.ini**:
```ini
[env:esp32]
upload_port = /dev/ttyUSB1  # or COM4 on Windows
```

---

## Step 3: Building & Uploading

### 3.1 Upload Sender Node

**This must be done FIRST** (provides I2S clock):

```bash
cd sender_node
platformio run -t upload
```

Wait for completion. Expected output ends with:
```
Wrote 0x... bytes to address 0x... in ... seconds
Leaving...
Hard resetting via RTS pin...
```

### 3.2 Upload Receiver Node

**After sender is uploaded**, upload receiver:

```bash
cd ../receiver_node
platformio run -t upload
```

Wait for completion (same ending messages).

### 3.3 Optional: Upload SD Logger Node

If you have a third board with SD card support:

```bash
cd ../sd_logger_node
platformio run -t upload
```

---

## Step 4: Verification

### 4.1 Open Serial Monitor for Receiver

**Terminal 1 - Monitor Receiver**:
```bash
cd receiver_node
platformio device monitor -b 115200
```

You should see:
```
I2S sender detected!
Starting audio stream processing...
```

### 4.2 Check Serial Output

Within a few seconds, you should see CSV output:
```
32768, -32768
16384, -16384
8192, -8192
0, 0
-8192, 8192
-16384, 16384
-32768, 32768
```

**This is a sine wave!** Congratulations! ✅

### 4.3 Verify Data Continuity

The numbers should:
- ✅ Continue flowing continuously
- ✅ Follow a sine wave pattern
- ✅ Show no large jumps or gaps
- ✅ Range from -32768 to 32768 (approximately)

### 4.4 Stop Monitoring

Press `Ctrl+C` to exit the serial monitor.

---

## Troubleshooting

### Issue: "No boards found"

**Cause**: USB drivers not installed or port not recognized

**Solution**:
```bash
# Install USB drivers (if needed)
platformio run -t install-drivers

# Restart and try again
platformio device list
```

---

### Issue: Serial Monitor Shows Garbage

**Cause**: Baud rate mismatch

**Solution**: Ensure baud rate is exactly **115200**:
```bash
platformio device monitor -b 115200
```

---

### Issue: "I2S sender not detected"

**Cause**: Sender board not running or wiring issue

**Solution**:
1. Verify sender is plugged in and powered
2. Check that sender_node was uploaded successfully
3. Review wiring connections (all 4 wires including GND)
4. Try swapping USB ports
5. Unplug both boards, wait 5 seconds, plug in again

---

### Issue: Upload Fails with Permission Error

**Cause**: USB device permissions

**Solution (Linux)**:
```bash
# Add your user to dialout group
sudo usermod -a -G dialout $USER

# Log out and back in, then try again
```

---

### Issue: No Output After 30 Seconds

**Cause**: Receiver still waiting for I2S signal

**Solution**:
1. Power cycle both boards (sender first, then receiver)
2. Check I2S wiring connections with multimeter
3. Ensure common ground is connected
4. Try different USB cable on sender

---

## Next Steps

### ✅ Basic Setup Complete!

Now you can:

1. **Monitor Data in Real-Time**
   ```bash
   cd receiver_node && platformio device monitor -b 115200
   ```

2. **Log Data to File**
   ```bash
   cd receiver_node && platformio device monitor -b 115200 | tee data.csv
   ```

3. **Analyze Data**
   - Save CSV output
   - Import into Python/MATLAB
   - Plot the sine wave
   - Verify frequency content

4. **Try the SD Logger** (if you have a third board)
   - Upload `sd_logger_node` to a board with SD card
   - Data will be saved to WAV file
   - See [sd_logger_node/README.md](../sd_logger_node/README.md)

### 📚 Learn More

- **System Architecture**: See [ARCHITECTURE.md](./ARCHITECTURE.md)
- **Hardware Details**: See [HARDWARE_SETUP.md](./HARDWARE_SETUP.md)
- **I2S Protocol**: See [I2S_PROTOCOL.md](./I2S_PROTOCOL.md)
- **Debugging**: See [DEBUGGING.md](./DEBUGGING.md)
- **Questions**: See [FAQ.md](./FAQ.md)

### 🔧 Configuration

You can customize:
- **Sample rate** (default 44100 Hz)
- **Pin assignments** (see `platformio.ini` in each node)
- **Output format** (currently CSV)
- **Waveform** (currently sine, extensible to square, triangle, etc.)

See [API_REFERENCE.md](./API_REFERENCE.md) for details.

### 🐛 Issues?

1. Check [FAQ.md](./FAQ.md) for common questions
2. Review [DEBUGGING.md](./DEBUGGING.md) for advanced troubleshooting
3. Check serial monitor output for error messages
4. Open an issue on GitHub with:
   - Board type and OS
   - PlatformIO version: `platformio --version`
   - Error messages from build/upload
   - Steps to reproduce

---

## Success Criteria Checklist

You're ready to move forward when:

- [ ] Both boards detected by `platformio device list`
- [ ] Sender node uploads successfully
- [ ] Receiver node uploads successfully
- [ ] Serial monitor shows "I2S sender detected!"
- [ ] CSV data flowing at ~44100 lines/second
- [ ] Data shows sine wave pattern
- [ ] No errors in serial console

---

**Congratulations! You now have a working I2S audio system.** 🎉

Next, explore [ARCHITECTURE.md](./ARCHITECTURE.md) to understand the system design, or dive into [HARDWARE_SETUP.md](./HARDWARE_SETUP.md) for advanced wiring configurations.

*Happy experimenting!*
