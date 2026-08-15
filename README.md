# ESP8266 GUIKit + WebDAV Server

A complete development framework for building GUI applications on ESP8266 with TFT touchscreen display, featuring a WebDAV server for file management and dynamic UI loading from SD card.

---

## 🎯 Overview

This project implements a **separated bootloader-kernel architecture** for ESP8266 that addresses the platform's limited Flash memory by:

- **Minimal bootloader** in Flash (~8-16KB) that initializes hardware and loads the kernel
- **Full kernel binary** (`Kernel.bin`) stored on SD card, containing GUIKit, WebDAV server, and HTTP server
- **Dynamic UI loading** from JSON files on SD card
- **Modular design** with clear separation of concerns

### Key Features

✅ **Flash Space Optimization** – Only bootloader in limited Flash memory  
✅ **Dynamic Updates** – Kernel can be updated by replacing `Kernel.bin.gz` on SD card  
✅ **External UI Design** – UIs generated externally as JSON files  
✅ **Memory Efficiency** – Only loads necessary resources into RAM  
✅ **Complete WebDAV Server** – File management with enhanced features  
✅ **Touch GUI Framework** – Widget-based UI system with CSS-like styling  

---

## 📋 Architecture Documentation

Detailed architecture documentation is available in the [`docs/`](docs/) directory:

| Document | Description |
|----------|-------------|
| [ARCHITECTURE.md](docs/ARCHITECTURE.md) | Complete system architecture overview |
| [HARDWARE.md](docs/HARDWARE.md) | Hardware setup and pin configuration |
| [SOFTWARE.md](docs/SOFTWARE.md) | Software components and modules |
| [NETWORK.md](docs/NETWORK.md) | Network architecture and WebDAV enhancements |
| [DATA_FLOW.md](docs/DATA_FLOW.md) | Data flow diagrams and sequences |
| [MEMORY_MANAGEMENT.md](docs/MEMORY_MANAGEMENT.md) | Objective-C style memory management for ESP8266 |

---

## 🖥️ Hardware Requirements

### Required Components

| Component | Model | Purpose |
|-----------|-------|---------|
| Microcontroller | ESP8266 (NodeMCU v2) | Main processing unit |
| Display | 3.2" TFT LCD | Graphical user interface |
| Display Controller | ST7789 | TFT display driver |
| Touchscreen | XPT2046 | Touch input |
| Storage | MicroSD Card | Kernel storage and file system |

### Pin Configuration

| Component | Pin | Function |
|-----------|-----|----------|
| TFT Display | D8 | Chip Select (CS) |
| TFT Display | D3 | Data/Command (DC) |
| TFT Display | D4 | Reset (RST) |
| Touchscreen | D2 | Chip Select (CS) |
| Touchscreen | D1 | Interrupt (IRQ) |
| SD Card | D5 | Chip Select (CS) |

---

## 📦 Project Structure

```
ESP8266/
├── Bootloader/                    # Minimal bootloader (Flash)
│   ├── bootloader.ino            # Entry point
│   ├── error_screen.h/cpp        # Minimal error display
│   └── storage.h/cpp             # SD card management
│
├── Kernel/                       # Main system (SD Card)
│   ├── src/
│   │   ├── main.cpp              # Kernel entry point
│   │   ├── gui/                  # GUIKit module
│   │   │   ├── widget.h/cpp      # Widget definitions
│   │   │   ├── renderer.h/cpp    # TFT rendering
│   │   │   ├── touch.h/cpp       # Touch handling
│   │   │   └── gui.h/cpp         # GUIKit interface
│   │   │
│   │   ├── web/                  # Web services
│   │   │   ├── webdav_server.h/cpp
│   │   │   ├── web_server.h/cpp
│   │   │   └── remote_access.h/cpp
│   │   │
│   │   └── system/               # System utilities
│   │       ├── file_manager.h/cpp
│   │       ├── network.h/cpp
│   │       └── config.h/cpp
│   │
│   └── data/                      # Static data files
│       └── system/               # System directory (on SD)
│           ├── ui/
│           ├── dict/
│           ├── config/
│           └── logs/
│
├── docs/                         # Documentation
│   ├── ARCHITECTURE.md
│   ├── HARDWARE.md
│   ├── SOFTWARE.md
│   ├── NETWORK.md
│   └── DATA_FLOW.md
│
├── platformio.ini                # Build configuration
└── README.md                     # This file
```

---

## 🏗️ Software Architecture Layers

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

---

## 🚀 Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) installed
- ESP8266 development board (NodeMCU v2 recommended)
- 3.2" TFT display with ST7789 controller
- XPT2046 touchscreen controller
- MicroSD card (FAT32 formatted)
- Required libraries (automatically installed via PlatformIO)

### Quick Start

1. **Clone the repository**
   ```bash
   git clone https://github.com/your-repo/ESP8266-GUIKit-WebDavServer.git
   cd ESP8266-GUIKit-WebDavServer
   ```

2. **Install dependencies**
   ```bash
   pio init
   ```

3. **Configure your hardware**
   - Update pin definitions in `platformio.ini`
   - Adjust TFT and touchscreen settings

4. **Prepare SD card**
   - Format as FAT32
   - Create `/system` directory structure
   - Copy sample UI files to `/system/ui/`

5. **Build and flash bootloader**
   ```bash
   pio run -e bootloader
   pio run -e bootloader -t upload
   ```

6. **Build kernel and copy to SD**
   ```bash
   pio run -e kernel
   # Copy Kernel.bin to SD card root
   ```

7. **Insert SD card and reset ESP8266**

---

## 📖 Widget System

The GUIKit provides a **CSS-like widget system** with the following types:

| Widget Type | Description | Features |
|-------------|-------------|----------|
| `VIEW` | Container widget | Can have child widgets |
| `BUTTON` | Clickable button | Text, callback, pressed state |
| `LABEL` | Text display | Static or dynamic text |
| `SLIDER` | Slider control | Value range, callback |

### Basic Widget Structure

```c
struct t_widget_base {
    uint8_t UUID[16];            // Unique identifier
    WIDGET_TYPE type;           // Widget type
    Background background;      // RGBA565 color, gradient
    Border border;              // Color, width
    Size size;                  // Width, height
    Position position;          // X, Y coordinates
    Bound bound;                // Padding, margin
    Widget** children;          // Child widgets
    uint8_t children_count;
    bool dirty;                 // Needs redrawing
};
```

---

## 🌐 WebDAV Features

The system includes an **enhanced WebDAV server** with the following features:

### Core Features
- ✅ File upload/download/delete
- ✅ Directory listing and creation
- ✅ Basic authentication (username/password)
- ✅ Port 80 operation

### Enhanced Features
- ✅ **Network Mounting** – Map SD paths to virtual mount points
- ✅ **Quota System** – Per-user storage limits
- ✅ **File Locking** – Prevent concurrent write conflicts
- ✅ **History Tracking** – Audit trail of file modifications
- ✅ **Share Links** – Temporary, shareable download links
- ✅ **Remote Access** – Internet access via port forwarding + DNS

---

## 📊 SD Card Structure

```
SD Card Root/
├── Kernel.bin.gz                    # Compressed kernel binary
├── index.html                       # Web interface entry point
└── system/
    ├── ui/                          # UI definitions (JSON)
    │   ├── main_ui.json
    │   ├── settings_ui.json
    │   └── login_ui.json
    │
    ├── dict/                        # Dictionnaires
    │   ├── fr.txt
    │   ├── en.txt
    │   └── custom.txt
    │
    ├── config/                      # Configurations
    │   ├── passwords.txt
    │   ├── styles.json
    │   ├── settings.json
    │   ├── quotas.json
    │   ├── file_locks.json
    │   ├── history.log
    │   └── share_links.json
    │
    └── logs/                        # System logs
        └── system.log
```

---

## 🔧 Configuration

### Build Configuration (platformio.ini)

```ini
; Bootloader Configuration
[env:bootloader]
platform = espressif8266
board = nodemcuv2
framework = arduino
build_flags =
    -D BOOTLOADER_MODE
    -D SD_CS_PIN=D5
monitor_speed = 115200

; Kernel Configuration
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

### Runtime Configuration

Configuration files are stored in `/system/config/` on the SD card:

- `quotas.json` – User storage limits
- `file_locks.json` – File locking state
- `history.log` – Modification history
- `share_links.json` – Shareable links
- `remote_access.json` – Remote access settings

---

## 📚 API Documentation

### WebDAV API

| Endpoint | Method | Description | Authentication |
|----------|--------|-------------|----------------|
| `/webdav/` | GET/POST/PUT/DELETE | WebDAV operations | Required |
| `/webdav/{path}` | GET | Download file | Required |
| `/` | GET | Web interface | None |
| `/ui/{filename}` | GET | Load UI file | None |

### Enhanced API

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/mount` | POST | Create mount point |
| `/quota` | GET | Check user quota |
| `/file_lock` | POST/DELETE | Lock/unlock file |
| `/history` | GET | View modification history |
| `/share` | POST/GET/DELETE | Create/list/delete share links |
| `/remote_access` | POST | Configure remote access |
| `/remote_url` | GET | Get remote access URL |
| `/s/{token}` | GET | Access shared file |

---

## 🛠️ Development

### Adding New Widget Types

1. Add widget type to `WIDGET_TYPE` enum
2. Create widget structure extending `t_widget_base`
3. Add rendering function in `renderer.cpp`
4. Add touch handling in `touch.cpp`
5. Add to widget union in `widget.h`

### Customizing UI

1. Create JSON file in `/system/ui/`
2. Define widget hierarchy
3. Set properties (position, size, style)
4. Reference in `load_and_display_ui()`

---

## 🤝 Contributing

Contributions are welcome! Please follow these guidelines:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgments

- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) - TFT display library
- [XPT2046_Touchscreen](https://github.com/PaulStoffregen/XPT2046_Touchscreen) - Touchscreen library
- [ESPWebDAV](https://github.com/hoonie/ESPWebDAV) - WebDAV server library
- [SdFat](https://github.com/greiman/SdFat) - SD card library

---

## 📞 Support

For questions, issues, or feature requests, please open an issue on GitHub.

---

*Generated from architecture analysis of discussion_guikit.txt*
*Documentation extracted and organized by Mistral Vibe*

