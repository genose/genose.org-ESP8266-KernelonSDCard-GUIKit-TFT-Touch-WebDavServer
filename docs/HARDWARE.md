# Hardware Architecture

This document describes the hardware components, connections, and configuration for the ESP8266 GUIKit + WebDAV Server system.

---

## Table of Contents

1. [Hardware Components](#hardware-components)
2. [Pin Configuration](#pin-configuration)
3. [SD Card Structure](#sd-card-structure)
4. [Hardware Dependencies](#hardware-dependencies)

---

## Hardware Components

### Required Components

| Component | Model | Purpose | Library |
|-----------|-------|---------|---------|
| Microcontroller | ESP8266 (NodeMCU v2) | Main processing unit | Arduino Core |
| Display | 3.2" TFT LCD | Graphical user interface | TFT_eSPI |
| Display Controller | ST7789 | TFT display driver | TFT_eSPI |
| Touchscreen | XPT2046 | Touch input | XPT2046_Touchscreen |
| Storage | MicroSD Card | Kernel storage and file system | SdFat |

### Hardware Block Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                         ESP8266 (NodeMCU v2)                    │
│                                                                  │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────────┐  │
│  │   TFT 3.2"   │    │  Touchscreen  │    │    SD Card        │  │
│  │  ST7789      │    │  XPT2046      │    │    (SPI)         │  │
│  │  240×320     │    │  (SPI)        │    │                  │  │
│  └──────┬───────┘    └──────┬───────┘    └─────────┬────────┘  │
│         │                   │                    │            │
│         └───────────────────┼────────────────────┘            │
│                             │                              │
│                    ┌────────▼────────┐                     │
│                    │   GPIO Pins      │                     │
│                    │   D1-D8, CS, DC  │                     │
│                    └──────────────────┘                     │
└─────────────────────────────────────────────────────────────┘
      │
      ▼
┌─────────────────────────────────────────────────────────────┐
│                    SD Card File System                        │
│                                                                  │
│  /Kernel.bin.gz                                             │
│  /system/                                                   │
│    ├── ui/                 # UI definitions                   │
│    ├── dict/               # Dictionaries                      │
│    ├── config/             # Configurations                   │
│    └── logs/               # System logs                       │
│                                                                  │
└─────────────────────────────────────────────────────────────┘
```

---

## Pin Configuration

### Pin Assignment Table

| Component | Pin | Function | Notes |
|-----------|-----|----------|-------|
| TFT Display | D8 | Chip Select (CS) | Active low |
| TFT Display | D3 | Data/Command (DC) | High = data, Low = command |
| TFT Display | D4 | Reset (RST) | Active low |
| Touchscreen | D2 | Chip Select (CS) | Active low |
| Touchscreen | D1 | Interrupt (IRQ) | Touch detect interrupt |
| SD Card | D5 | Chip Select (CS) | Active low |

### SPI Bus Configuration

The system uses the ESP8266's hardware SPI interface:

| SPI Signal | ESP8266 Pin | Purpose |
|------------|------------|---------|
| MOSI | GPIO13 (D7) | Master Out Slave In |
| MISO | GPIO12 (D6) | Master In Slave Out |
| SCK | GPIO14 (D5) | Serial Clock |

Note: D5 is used for both SCK (SPI clock) and SD Card CS. The actual pin assignments may vary based on board layout. See PlatformIO configuration for exact mapping.

---

## SD Card Structure

### Directory Hierarchy

```
SD Card Root/
├── Kernel.bin.gz                    # Compressed kernel binary
├── index.html                       # Web interface entry point
└── system/                           # System directory
    │
    ├── ui/                           # UI definitions (JSON format)
    │   ├── main_ui.json              # Main user interface
    │   ├── settings_ui.json          # Settings interface
    │   └── login_ui.json             # Login interface
    │
    ├── dict/                         # Dictionary files
    │   ├── fr.txt                    # French dictionary
    │   ├── en.txt                    # English dictionary
    │   └── custom.txt                # Custom/user dictionary
    │
    ├── config/                       # Configuration files
    │   ├── passwords.txt             # User passwords (hashed/encrypted)
    │   ├── styles.json               # Custom widget styles
    │   ├── settings.json             # General system settings
    │   ├── quotas.json               # User storage quotas
    │   ├── file_locks.json           # Current file locks
    │   ├── history.log               # File modification history
    │   └── share_links.json          # Active share links
    │
    └── logs/                         # System log files
        └── system.log                # Main system log
```

### File Descriptions

| File/Directory | Purpose | Format | Notes |
|---------------|---------|--------|-------|
| `Kernel.bin.gz` | Compressed kernel binary | gzip binary | Loaded by bootloader |
| `index.html` | Web interface | HTML | Served by HTTP server |
| `/system/ui/` | UI definitions | JSON | Dynamically loaded |
| `/system/dict/` | Dictionaries | Text | For spell checking, autocomplete |
| `/system/config/` | Configurations | JSON/Text | Various settings |
| `/system/logs/` | Log files | Text | System diagnostics |

---

## Hardware Dependencies

### Libraries

| Library | Version | Repository | Purpose |
|---------|---------|------------|---------|
| TFT_eSPI | Latest | [Bodmer/TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) | TFT display driver |
| XPT2046_Touchscreen | Latest | [PaulStoffregen/XPT2046](https://github.com/PaulStoffregen/XPT2046_Touchscreen) | Touchscreen driver |
| SdFat | Latest | [greiman/SdFat](https://github.com/greiman/SdFat) | SD card access |
| ESP8266WiFi | Built-in | Arduino ESP8266 Core | WiFi connectivity |
| ESP8266WebServer | Built-in | Arduino ESP8266 Core | HTTP server |
| ESPWebDAV | Latest | [hoonie/ESPWebDAV](https://github.com/hoonie/ESPWebDAV) | WebDAV server |

### Library Configuration

The libraries are configured via PlatformIO build flags in `platformio.ini`:

```ini
; TFT Configuration
build_flags =
    -D USER_SETUP_LOADED
    -D ST7789_DRIVER
    -D TFT_WIDTH=240
    -D TFT_HEIGHT=320
    -D TFT_CS=D8
    -D TFT_DC=D3
    -D TFT_RST=D4

; Touchscreen Configuration
build_flags =
    -D TOUCH_CS=D2
    -D XPT2046_IRQ=D1

; SD Card Configuration
build_flags =
    -D SD_CS=D5
    -D SD_FAT_TYPE=1
```

---

## Hardware Setup Instructions

### 1. Connect TFT Display

| TFT Pin | ESP8266 Pin | Notes |
|---------|-------------|-------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| CS | D8 | Chip select |
| DC | D3 | Data/Command |
| RST | D4 | Reset |
| SCL | D5 (SCK) | SPI Clock |
| SDA | D7 (MOSI) | SPI Data |

### 2. Connect Touchscreen

| Touch Pin | ESP8266 Pin | Notes |
|-----------|-------------|-------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| CS | D2 | Chip select |
| IRQ | D1 | Interrupt |
| SCL | D5 (SCK) | SPI Clock (shared) |
| SDA | D7 (MOSI) | SPI Data (shared) |
| DO | D6 (MISO) | SPI Data Out |

### 3. Connect SD Card

| SD Pin | ESP8266 Pin | Notes |
|--------|-------------|-------|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| CS | D5 | Chip select |
| DI | D7 (MOSI) | SPI Data In |
| DO | D6 (MISO) | SPI Data Out |
| CLK | D5 (SCK) | SPI Clock |

---

## Hardware Considerations

### Memory Constraints

| Resource | ESP8266 Limit | Usage |
|----------|---------------|-------|
| Flash | 512KB-1MB | Bootloader: ~8-16KB, Kernel: on SD |
| RAM | ~80KB | Careful allocation required |
| EEPROM | 4KB | Limited storage |

### Power Considerations

- All components use 3.3V logic
- Total current draw: ~200-500mA depending on display
- Ensure power supply can provide sufficient current
- Use external power for SD card if needed

### Timing Considerations

- SPI bus shared between TFT, Touchscreen, and SD Card
- CS lines must be properly managed to avoid bus conflicts
- SD Card operations may be slower due to shared SPI
- Consider using software SPI for one device if performance is critical

---

*See [ARCHITECTURE.md](ARCHITECTURE.md) for overall system architecture*  
*See [SOFTWARE.md](SOFTWARE.md) for software components*  

---

*Generated from architecture analysis of discussion_guikit.txt*
*Documentation extracted and organized by Mistral Vibe*
