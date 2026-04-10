# ⛏️ Duino-Coin Miner for ESP32-C5

<p align="center">
  <img src="https://raw.githubusercontent.com/revoxhere/duino-coin/master/Resources/duco.png" width="120">
  <br>
  <strong>Official Duino-Coin Miner optimized for ESP32-C5 (RISC-V)</strong>
  <br>
  No Display - Maximum Performance
</p>

<p align="center">
  <a href="#-installation"><img src="https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white"></a>
  <a href="#-installation"><img src="https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black"></a>
  <a href="#-installation"><img src="https://img.shields.io/badge/macOS-000000?style=for-the-badge&logo=macos&logoColor=white"></a>
  <img src="https://img.shields.io/badge/ESP32--C5-ESP32-blue?style=for-the-badge&logo=espressif">
  <img src="https://img.shields.io/badge/PlatformIO-FF6600?style=for-the-badge&logo=platformio&logoColor=white">
</p>

## 📋 **Table of Contents**

- [✨ Features](#-features)
- [📦 Requirements](#-requirements)
- [🚀 Installation & Setup](#-installation--setup)
  - [Windows](#windows)
  - [Linux](#linux)
  - [macOS](#macos)
- [⚙️ Configuration](#️-configuration)
- [📤 Upload Firmware](#-upload-firmware)
- [📊 Monitor Activity](#-monitor-activity)
- [🐛 Troubleshooting](#-troubleshooting)
- [📜 License](#-license)

## ✨ **Features**

| Feature | Status |
|---------|--------|
| ⛏️ SHA-1 Mining | ✅ |
| 🔌 Auto WiFi Connection | ✅ |
| 📊 Web Dashboard | ✅ |
| 💡 LED Status Indicator | ✅ |
| 🔄 OTA Update | ✅ |
| 📱 Serial Monitor Logging | ✅ |
| 🖥️ No Display Required | ✅ |

## 📦 **Requirements**

### **Hardware:**
- **ESP32-C5 DevKitC-1** board (or compatible)
- USB-C data cable
- 2.4GHz WiFi network

### **Software:**
- **PlatformIO** (via VS Code or CLI)
- **Python 3.6+** (for CLI usage)
- **Git** (to clone the repository)

## 🚀 **Installation & Setup**

### **Windows**

#### 1. Install Tools
```
# Install Python (if not installed)
winget install Python.Python.3.11

# Install Git (if not installed)
winget install Git.Git

# Install PlatformIO
pip install platformio
```

2. Clone Repository

```
git clone https://github.com/nam348tnh3gp/duco-miner-for-esp32c5.git
cd duco-miner-for-esp32c5
```

3. Install USB Driver (if needed)

· Download driver from Espressif
· Or let Windows Update find it automatically

Linux (Ubuntu/Debian)

1. Install Tools

```
# Install Python and Git
sudo apt update
sudo apt install python3 python3-pip git -y

# Install PlatformIO
pip install platformio

# Add USB permissions
sudo usermod -a -G dialout $USER
# Log out and back in to apply changes
```

2. Clone Repository

```
git clone https://github.com/nam348tnh3gp/duco-miner-for-esp32c5.git
cd duco-miner-for-esp32c5
```

macOS

1. Install Tools

```
# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install Python and Git
brew install python3 git

# Install PlatformIO
pip3 install platformio
```

2. Clone Repository

```
git clone https://github.com/nam348tnh3gp/duco-miner-for-esp32c5.git
cd duco-miner-for-esp32c5
```

⚙️ Configuration

Edit Settings.h

Open Settings.h with any text editor (Notepad, VS Code, Sublime, etc.):

```
// ---------------------- General settings ---------------------- //
// Change to your Duino-Coin username
extern char *DUCO_USER = "your_username_here";        // 👈 CHANGE username

// Change to your mining key (if set in wallet, otherwise "None")
extern char *MINER_KEY = "None";                      // 👈 CHANGE to "None" or your mining key

// Change if you want a custom miner name, use "Auto" to autogenerate
extern char *RIG_IDENTIFIER = "ESP32-C5";             // 👈 Custom rig name (optional)

// WiFi credentials
extern const char SSID[] = "your_wifi_ssid";          // 👈 YOUR WiFi SSID
extern const char PASSWORD[] = "your_wifi_password";  // 👈 YOUR WiFi PASSWORD
```

Advanced Options (Settings.h)

Option Description Default
WEB_DASHBOARD Enable web dashboard ✅ Enabled

LED_BLINKING LED blinks on share found ✅ Enabled

SERIAL_PRINTING Print logs to Serial ✅ Enabled

SERIAL_BAUDRATE Serial baud rate 500000

DISABLE_BROWNOUT Disable brownout detector ✅ Enabled

📤 Upload Firmware

Step 1: Connect ESP32-C5

· Connect ESP32-C5 to your computer via USB-C
· LED on the board should light up

Step 2: Identify COM Port

OS Port Check command
Windows COM3, COM4,... Device Manager → Ports
Linux /dev/ttyUSB0, /dev/ttyACM0 ls /dev/tty*
macOS /dev/cu.usbmodem* ls /dev/cu.usb*

Step 3: Flash Firmware

```
pio run --target upload -e esp32-c5
```

Specify port if needed:

```
pio run --target upload -e esp32-c5 --upload-port COM3        # Windows
pio run --target upload -e esp32-c5 --upload-port /dev/ttyUSB0 # Linux
pio run --target upload -e esp32-c5 --upload-port /dev/cu.usbmodem* # macOS
```

Step 4: Success Output

```
Building .pio/build/esp32-c5/firmware.bin
esptool v5.1.0
Creating ESP32C5 image...
Successfully created ESP32C5 image.
======================== [SUCCESS] Took 107.65 seconds ========================
```

📊 Monitor Activity

Serial Monitor

```
pio device monitor -e esp32-c5 -b 500000
```

Or use any serial terminal (Putty, Arduino IDE, Screen) with baud rate 500000.

Sample Output:

```
Duino-Coin 4.3-C5
ESP32-C5 Miner - No Display Mode
Connecting to WiFi: MyWiFi
WiFi connected
IP: 192.168.1.100
Rig: ESP32-C5
Setup complete - Mining started
Selected node: node.duinocoin.com
New job - Difficulty: 100, Hash: abc123...
Share accepted! Total: 1
Stats - HR: 15.2 kH/s | Shares: 1/1 (100.0%) | Diff: 100 | Ping: 45ms
```

Web Dashboard

Once connected, access the dashboard at:

```
http://<ESP32-C5-IP-ADDRESS>
```

Example: http://192.168.1.100

---

🐛 Troubleshooting

Permission Denied (Linux/macOS)

```
sudo chmod 666 /dev/ttyUSB0
```

Cannot Find Port

· Check USB cable (must support data, not just charging)
· Reinstall USB drivers
· Try a different USB port

Upload Fails - Press BOOT Button

1. Hold the BOOT button on ESP32-C5
2. Press RESET button (or power cycle)
3. Release BOOT button
4. Run upload command again

WiFi Won't Connect

· Ensure SSID and password are correct in Settings.h
· Use 2.4GHz network (ESP32-C5 doesn't support 5GHz for client mode)
· Check router signal strength

Flash Memory Full

Current usage: ~98.2% of 4MB flash. Disable unused features:


// In Settings.h
#define WEB_DASHBOARD  // Comment out to disable web server

📜 License

MIT License - The Duino-Coin Team & Community 2019-2026

<p align="center">
  Made with ⚡ for the Duino-Coin community
  <br>
  <a href="https://duinocoin.com">duinocoin.com</a> | 
  <a href="https://github.com/revoxhere/duino-coin">GitHub</a>
</p>
