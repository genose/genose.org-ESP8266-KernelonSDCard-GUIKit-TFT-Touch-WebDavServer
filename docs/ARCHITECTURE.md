# System Architecture Overview

This document provides a comprehensive overview of the ESP8266 GUIKit + WebDAV Server architecture.

---

## Table of Contents

1. [System Overview](#system-overview)
2. [Layered Architecture](#layered-architecture)
3. [Bootloader Architecture](#bootloader-architecture)
4. [Kernel Architecture](#kernel-architecture)
5. [Project Structure](#project-structure)

---

## System Overview

### Architecture Philosophy

The system implements a **separated bootloader-kernel architecture** that addresses ESP8266's limited Flash memory (512KB-1MB) by:

- **Minimal bootloader** in Flash (~8-16KB)
- **Full kernel binary** (`Kernel.bin`) stored on SD card
- **Dynamic UI loading** from JSON files on SD
- **Modular design** with clear separation of concerns

### Key Benefits

| Benefit | Implementation |
|---------|----------------|
| Flash Space Optimization | Only bootloader in limited Flash |
| Dynamic Updates | Kernel can be updated by replacing `Kernel.bin.gz` on SD card |
| External UI Design | UIs generated externally as JSON files |
| Memory Efficiency | Only loads necessary resources into RAM |
| Modularity | Each component (GUI, WebDAV, HTTP) is independent |

---

## Layered Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        APPLICATION LAYER                        │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────────┐  ┌─────────────────────┐             │
│  │      GUIKit          │  │     Web Services     │             │
│  │  - Widget System     │  │  - WebDAV Server     │             │
│  │  - Rendering Engine   │  │  - HTTP Server       │             │
│  │  - Touch Handling     │  │  - Remote Access     │             │
│  │  - UI Loader          │  │  - Share Links        │             │
│  └─────────────────────┘  └─────────────────────┘             │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                       SYSTEM SERVICES LAYER                    │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐           │
│  │  File Manager │  │  Network      │  │  SD Card      │           │
│  │  - UI Loading │  │  - WiFi       │  │  - SdFat      │           │
│  │  - Config     │  │  - TCP/IP     │  │  - File I/O   │           │
│  │  - Quotas     │  │  - DNS        │  │  - Storage    │           │
│  └──────────────┘  └──────────────┘  └──────────────┘           │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                        HARDWARE LAYER                           │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐           │
│  │   TFT_eSPI    │  │  XPT2046      │  │  SdFat        │           │
│  │   - Display   │  │  - Touch      │  │  - SD Access  │           │
│  │   - Graphics  │  │  - Calibration│  │  - FAT32       │           │
│  └──────────────┘  └──────────────┘  └──────────────┘           │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                        BOOT LAYER                               │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────────────────────────────────────────────┐  │
│  │                 BOOTLOADER (in Flash)                       │  │
│  │  - SD Card Initialization                                 │  │
│  │  - Kernel.bin.gz Verification                              │  │
│  │  - Minimal Error Display (TFT)                            │  │
│  │  - Kernel Loading & Decompression                         │  │
│  └─────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

### Layer Responsibilities

| Layer | Responsibility | Components |
|-------|----------------|------------|
| Application | User-facing features | GUIKit, WebDAV, HTTP |
| System Services | Core services | File Manager, Network, SD Card |
| Hardware | Device drivers | TFT_eSPI, XPT2046, SdFat |
| Boot | System initialization | Bootloader |

---

## Bootloader Architecture

### Bootloader Workflow

```
Power On
  │
  ▼
Initialize Serial @115200
  │
  ▼
Initialize SD Card
  │
  ▼
SD Card OK?
  │
  ├── NO → Error Screen ("SD Card Not Detected") → Halt
  │
  └── YES
      │
      ▼
  Check Kernel.bin.gz exists
      │
      ├── NO → Error Screen ("Kernel.bin Not Found") → Halt
      │
      └── YES
          │
          ▼
      Load & Decompress Kernel
          │
          ▼
      Jump to Kernel Execution
```

### Bootloader Components

```
Bootloader/ (in Flash)
├── bootloader.ino          # Main entry point
│   ├── setup()             # Initialization
│   └── loop()              # Empty (should not reach)
│
├── error_screen.h/cpp      # Error display functions
│   ├── init_error_screen()
│   ├── display_error_message()
│   └── clear_error_screen()
│
└── storage.h/cpp           # SD card management
    └── SD initialization & file checks
```

### Bootloader Configuration

```ini
[env:bootloader]
platform = espressif8266
board = nodemcuv2
framework = arduino
build_flags =
    -D BOOTLOADER_MODE
    -D SD_CS_PIN=D5
monitor_speed = 115200
```

---

## Kernel Architecture

### Kernel Composition

The kernel binary (`Kernel.bin`) on SD card contains:

✅ **GUIKit Complete** – Widgets, rendering, touch handling, UI loading  
✅ **WebDAV Server** – ESPWebDAV library integration with port 80  
✅ **HTTP Server** – Lightweight server with port forwarding  
✅ **Dynamic UI Loading** – Load UI definitions from `/system/ui/`  
✅ **File Management** – SD card operations with enhanced features  

### Kernel Initialization Sequence

```
kernel_main()
  │
  ▼
setup_kernel()
  │
  ├── Serial.begin(115200)
  ├── SD card initialization
  ├── WiFi connection (SSID, password)
  ├── WebDAV server initialization
  ├── HTTP server initialization
  ├── GUIKit initialization
  └── Load main UI: /system/ui/main_ui.json
      │
      ▼
  loop_kernel()
      │
      ├── http_server.handleClient()
      └── update_gui()
          │
          └── (continuous loop)
```

### Kernel Project Structure

```
Kernel/
├── src/
│   ├── main.cpp                   # Kernel entry point
│   │
│   ├── gui/                       # GUIKit module
│   │   ├── widget.h/cpp           # Widget definitions
│   │   ├── renderer.h/cpp         # TFT rendering
│   │   ├── touch.h/cpp            # Touch handling
│   │   ├── ui_loader.h/cpp        # JSON UI loading
│   │   └── gui.h/cpp              # GUIKit interface
│   │
│   ├── web/                       # Web services
│   │   ├── webdav_server.h/cpp    # WebDAV server
│   │   ├── web_server.h/cpp       # HTTP server
│   │   └── remote_access.h/cpp   # Remote access
│   │
│   └── system/                    # System utilities
│       ├── file_manager.h/cpp    # File system management
│       ├── network.h/cpp          # Network utilities
│       └── config.h/cpp           # Configuration management
│
└── Kernel.bin                     # Compiled output
```

### Kernel Configuration

```ini
[env:kernel]
platform = espressif8266
board = nodemcuv2
framework = arduino

lib_deps =
    https://github.com/Bodmer/TFT_eSPI.git
    https://github.com/PaulStoffregen/XPT2046_Touchscreen.git
    https://github.com/hoonie/ESPWebDAV.git
    https://github.com/greiman/SdFat.git

build_flags =
    -D USER_SETUP_LOADED
    -D ST7789_DRIVER
    -D TFT_WIDTH=240
    -D TFT_HEIGHT=320
    -D TFT_CS=D8
    -D TFT_DC=D3
    -D TFT_RST=D4
    -D TOUCH_CS=D2
    -D XPT2046_IRQ=D1
    -D SD_CS=D5
    -D WEBDAV_USERNAME="admin"
    -D WEBDAV_PASSWORD="esp8266"
    -D WEBDAV_PORT=80
```

---

## Project Structure

### Complete Repository Structure

```
ESP8266/
├── Bootloader/                    # Minimal bootloader (Flash)
│   ├── bootloader.ino
│   ├── error_screen.h/cpp
│   └── storage.h/cpp
│
├── Kernel/                       # Main system (SD Card)
│   ├── src/
│   │   ├── main.cpp
│   │   ├── gui/
│   │   │   ├── widget.h/cpp
│   │   │   ├── renderer.h/cpp
│   │   │   ├── touch.h/cpp
│   │   │   ├── ui_loader.h/cpp
│   │   │   └── gui.h/cpp
│   │   ├── web/
│   │   │   ├── webdav_server.h/cpp
│   │   │   ├── web_server.h/cpp
│   │   │   └── remote_access.h/cpp
│   │   └── system/
│   │       ├── file_manager.h/cpp
│   │       ├── network.h/cpp
│   │       └── config.h/cpp
│   └── Kernel.bin
│
├── docs/                         # Documentation
│   ├── ARCHITECTURE.md
│   ├── HARDWARE.md
│   ├── SOFTWARE.md
│   ├── NETWORK.md
│   └── DATA_FLOW.md
│
├── platformio.ini
└── README.md
```

---

*See [HARDWARE.md](HARDWARE.md) for hardware architecture details*  
*See [SOFTWARE.md](SOFTWARE.md) for software components and widget system*  
*See [NETWORK.md](NETWORK.md) for network and WebDAV architecture*  
*See [DATA_FLOW.md](DATA_FLOW.md) for data flow diagrams*  

---

*Generated from architecture analysis of discussion_guikit.txt*
*Documentation extracted and organized by Mistral Vibe*
