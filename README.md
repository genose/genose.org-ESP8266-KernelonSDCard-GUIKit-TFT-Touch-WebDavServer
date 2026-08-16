# ESP8266 GUIKit + WebDAV Server

A complete development framework for building GUI applications on ESP8266 with TFT touchscreen display, featuring a WebDAV server for file management and dynamic UI loading from SD card.

---

## 🎯 Overview

This project implements a **separated bootloader-kernel architecture** for ESP8266/ESP32 that addresses the platform's limited Flash memory by:

- **Minimal bootloader** in Flash (~8-16KB) that initializes hardware and loads the kernel
- **Full kernel binary** (`Kernel.bin`) stored on SD card, containing GUIKit, WebDAV server, HTTP server, Push Notification system, and mDNS service discovery
- **Dynamic UI loading** from JSON files on SD card
- **Modular design** with clear separation of concerns
- **Real-time notifications** via WebDAV Push system with secure authentication
- **Zero-configuration discovery** via mDNS/Bonjour for device access at `[hostname].local`

### Key Features

✅ **Flash Space Optimization** – Only bootloader in limited Flash memory  
✅ **Dynamic Updates** – Kernel can be updated by replacing `Kernel.bin.gz` on SD card  
✅ **External UI Design** – UIs generated externally as JSON files  
✅ **Memory Efficiency** – Only loads necessary resources into RAM  
✅ **Complete WebDAV Server** – File management with enhanced features  
✅ **Touch GUI Framework** – Widget-based UI system with CSS-like styling
✅ **Push Notifications** – Real-time WebDAV notifications with authentication
✅ **mDNS Discovery** – Device auto-discovery via [hostname].local (Bonjour/Zeroconf)  

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
| [memory_strategy_config.md](docs/memory_strategy_config.md) | Memory strategy configuration with config struct |
| [KERNEL_FUNCTIONALITY_COSTS.md](docs/KERNEL_FUNCTIONALITY_COSTS.md) | Kernel functionality RAM costs and priorities |
| [about_ram_expansion.md](about_ram_expansion.md) | External RAM expansion analysis |
| [about_port_expander.md](about_port_expander.md) | SPI port expander analysis |
| [about_huge_demo_ram_requirements.md](about_huge_demo_ram_requirements.md) | Huge demo RAM requirements |
| [WEBDAV_PUSH.md](docs/WEBDAV_PUSH.md) | WebDAV push notification system with authentication |
| [MDNS_SERVICE.md](docs/MDNS_SERVICE.md) | mDNS service discovery (Bonjour/Zeroconf) |
| [demo_huge_gui_result.txt](src/gui/demo_huge_gui_result.txt) | Huge GUI memory strategy results |

---

## 🎯 New Features

### mDNS Service Discovery

The system includes **mDNS (Bonjour/Zeroconf)** service discovery, allowing devices to be accessed via `[hostname].local` without requiring DNS configuration or `/etc/hosts` entries.

**Key Benefits:**
- Zero-configuration network discovery
- Works on Linux (avahi), macOS (Bonjour), Windows (Bonjour service)
- Automatic IP address resolution
- Service advertisement for HTTP, WebDAV, and GUIKit

**Quick Start:**
```c
#include "mdns_service.h"

// After WiFi connection
mdns_init(NULL);  // Device now discoverable as esp8266.local

// Clients can connect via:
// http://esp8266.local/
// http://esp8266.local/webdav
// http://esp8266.local/gui
```

**Client Discovery:**
```bash
# Linux
avahi-browse -a -r _http._tcp

# macOS
dns-sd -B _http._tcp

# Python
from zeroconf import Zeroconf, ServiceBrowser
```

### Memory Strategy System

The GUIKit now features a **hierarchical memory strategy** with explicit STOP-at-first-success behavior:

```
1. Try External RAM -> if (available AND GUI fits) => SELECT & STOP
2. Try SD Card Swap -> if (available AND GUI fits) => SELECT & STOP
3. Try Internal RAM -> if (GUI fits) => SELECT & STOP
4. Else => FAILED
```

**Component Loading Priorities:**
- **GUIKit Core, Rendering Engine, Widget Definitions, Touch Handling** → External RAM first (performance critical)
- **Image Converters (PNG, JPEG, TIFF)** → **Internal RAM → External RAM → SD Swap** (decode speed priority)
- **Other components** → Follow default STOP-at-first-success strategy

**Key Components:**
- `memory_strategy_config_t` in `guikit_hw_config.h` - Configurable thresholds and behavior flags
- `gui_memory_strategy.h/cpp` - Strategy selection and loading implementation
- `demo_huge_gui_result.txt` - Results for 500KB GUI with different hardware configurations
- `docs/KERNEL_FUNCTIONALITY_COSTS.md` - Complete RAM cost breakdown per functionality

**Configuration Options:**
- `external_ram_min_size` - Minimum GUI size to use external RAM (default: 4KB)
- `sd_swap_min_size` - Minimum GUI size for SD swap (default: 16KB)
- `internal_ram_max_size` - Maximum GUI size for internal RAM (default: 8KB)
- `external_ram_max_size` - Maximum external RAM size (default: 128KB)

### Bootloader

A **hardware detection bootloader** that automatically:
1. Detects all SPI devices (SRAM, PSRAM, SD Card, TFT, Touch, Expanders)
2. **RAM Length Detection** - Tests actual RAM size with 1-2 passes to detect wiring errors (e.g., 64K chip wired as 256K)
3. Initializes RAM (internal and external) with verified sizes
4. Configures memory strategy based on detected hardware
5. Tests strategy with various GUI sizes
6. Displays RAM test progress and results on TFT (if available)

**Files:**
- `src/boot/guikit_bootloader.h` - Bootloader header
- `src/boot/guikit_bootloader.cpp` - Full implementation
- `src/boot/README.md` - Complete bootloader documentation

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

### Optional External RAM Models

**SPI SRAM (ESP8266/ESP32):**
| Model | Size | Speed | Voltage | Package | Price | Notes |
|-------|------|-------|---------|---------|-------|-------|
| 23LC512 | 64 KB | 20 MHz | 2.5-5.5V | SOIC-8 | ~$1.50 | Budget |
| **23LC1024** | **128 KB** | **20 MHz** | **2.5-5.5V** | **SOIC-8** | **~$3.00** | **Recommended** |
| 23LCV1024 | 128 KB | 20 MHz | 1.7-5.5V | SOIC-8 | ~$3.50 | Low-voltage |
| **Lyontek LY68L6400** | **512 KB** | **50 MHz** | **2.7-3.6V** | **SOIC-8** | **~$4.00** | **Large cache** |

**FRAM - Non-Volatile (ESP8266/ESP32):**
| Model | Size | Speed | Interface | Price | Notes |
|-------|------|-------|-----------|-------|-------|
| MB85RS256B | 32 KB | 20 MHz | SPI | ~$5 | Basic |
| **CY15V102QN** | **128 KB** | **40 MHz** | **SPI Quad** | **~$12** | **Industrial** |
| **CY15V104QSN** | **512 KB** | **40 MHz** | **SPI Quad** | **~$20** | **Industrial** |

**PSRAM (ESP32 Native):**
| Model | Size | Speed | Interface | Price | Notes |
|-------|------|-------|-----------|-------|-------|
| APS6404 | 1 MB | 40 MHz | Quad SPI | ~$3 | Entry-level |
| APS1604 | 2 MB | 40 MHz | Quad SPI | ~$5 | Mid-range |
| APS3204 | 4 MB | 40 MHz | Quad SPI | ~$8 | High-capacity |
| W9812G6KH | 8 MB | 80 MHz | Quad SPI | ~$10 | Maximum |
| **ISSI IS66WVS5128ALL** | **64 MB** | **100 MHz** | **Octal SPI** | **~$15** | **Industrial** |
| **ISSI IS66WVS5128BLL** | **64 MB** | **100 MHz** | **Octal SPI** | **~$15** | **Industrial** |

**Configuration Presets:**
- `GUIKIT_HW_ESP8266_LY68L6400` - ESP8266 + 512KB Lyontek SRAM
- `GUIKIT_HW_ESP8266_CY15V104QSN` - ESP8266 + 512KB Cypress FRAM
- `GUIKIT_HW_ESP32_ISSI_64MB_PSRAM` - ESP32 + 64MB ISSI PSRAM
- `GUIKIT_HW_ESP32_LY68L6400` - ESP32 + 512KB Lyontek SRAM

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

## 📡 WebDAV Push Notifications

The system now includes a **real-time push notification system** that enables Linux/macOS WebDAV clients to receive immediate notifications when files change on the ESP8266/ESP32 device.

### Features

**Transport Mechanisms:**
- ✅ **Server-Sent Events (SSE)** - HTTP-based streaming for modern clients
- ✅ **WebSocket** - Persistent bidirectional connections
- ✅ **Long Polling** - Fallback for legacy clients

**Authentication System:**
- ✅ **Multiple Auth Methods** - Basic (username/password), Token, Digest, Anonymous
- ✅ **Permission System** - READ, WRITE, ADMIN flags with granular control
- ✅ **Session Management** - 16 max sessions, 5-minute timeout, lockout after 5 failed attempts
- ✅ **Token Management** - 32 max tokens, 24-hour expiry, revocation support
- ✅ **Rate Limiting** - 60 notifications/minute per client to prevent abuse
- ✅ **Default Admin** - `admin`/`admin` with full permissions

**Event Types:**
- File created, modified, deleted, moved
- Folder created, deleted
- Project updates
- GUI updates
- System events
- Custom events

**Key Benefits:**
- Real-time updates for Linux (davfs2, nautilus) and macOS (Finder) clients
- Secure authentication prevents unauthorized access
- Low memory footprint (~5KB max for all clients and sessions)
- Multiple transport options for compatibility
- JSON-based notifications (~300 bytes each)

### Quick Example

```c
#include "webdav_push.h"
#include "webdav_push_auth.h"

// Initialize
webdav_push_init(NULL);
webdav_push_auth_init(NULL);

// Register client
uint32_t client_id = webdav_push_client_register(WEBDAV_PUSH_CLIENT_SSE, "/");

// Authenticate
webdav_push_auth_login(client_id, "admin", "admin");

// Watch a path
webdav_push_watch_add("/projects", true, WEBDAV_PUSH_FILE_ALL);

// Check for changes (call periodically)
webdav_push_check_changes();
```

### HTTP Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/webdav/push/sse` | SSE event stream |
| GET | `/webdav/push/ws` | WebSocket connection |
| GET | `/webdav/push/poll` | Long polling |
| POST | `/webdav/push/subscribe` | Subscribe to path |
| POST | `/webdav/push/unsubscribe` | Unsubscribe |

### Default Watched Paths

The system automatically watches:
- `/gui` - GUI project changes
- `/projects` - User project changes
- `/tmp/task_comm` - Task communication files

### Files

| File | Description |
|------|-------------|
| `src/gui_editor/server/webdav_push.h` | Push notification API |
| `src/gui_editor/server/webdav_push.c` | Push notification implementation |
| `src/gui_editor/server/webdav_push_auth.h` | Authentication API |
| `src/gui_editor/server/webdav_push_auth.c` | Authentication implementation |

### Client Integration

**Linux (davfs2 with inotify):**
```bash
# Mount WebDAV
mount.davfs http://esp8266.local/webdav /mnt/webdav

# Monitor for changes
inotifywait -m -r -e create -e modify -e delete /mnt/webdav | \
  while read path action file; do
    echo "Change: $action $file"
  done
```

**macOS (Finder):**
1. Connect to WebDAV: `Go > Connect to Server > http://esp8266.local/webdav`
2. Finder automatically refreshes on push notifications

**Python SSE Client:**
```python
import requests
import sseclient

url = 'http://esp8266.local:8080/webdav/push/sse'
for event in sseclient.SSEClient(url):
    import json
    data = json.loads(event.data)
    print(f"Event: {data['type']} at {data['path']}")
```

---

## 📖 Widget System

The GUIKit provides a **CSS-like widget system** with the following types:

| Widget Type | Description | Features |
|-------------|-------------|----------|
| `VIEW` | Container widget | Can have child widgets |
| `BUTTON` | Clickable button | Text, callback, pressed state |
| `LABEL` | Text display | Static or dynamic text |
| `SLIDER` | Slider control | Value range, callback |
| `IMAGE` | Bitmap image | Path, scaling, transparency |
| `SPRITE` | Sprite/Offscreen | Cached rendering, animation |

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

### IMAGE Widget

Displays bitmap images from SD card or embedded resources.

**JSON Definition:**
```json
{
  "id": "my_image",
  "type": "image",
  "x": 10,
  "y": 10,
  "width": 100,
  "height": 100,
  "source": "/assets/images/logo.bmp",
  "format": "bmp",
  "transparent_color": "#FF00FF",
  "scale_mode": "aspect_fit",
  "cache": false
}
```

**Properties:**

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `source` | string | required | Path to image file on SD card |
| `format` | string | `"bmp"` | Image format: bmp, raw |
| `transparent_color` | string | none | Color to treat as transparent (hex) |
| `scale_mode` | string | `"none"` | stretch, aspect_fit, aspect_fill, tile, center, none |
| `cache` | boolean | `false` | Keep image in RAM after loading |
| `rotation` | integer | `0` | Rotation in degrees (0, 90, 180, 270) |
| `flip_h` | boolean | `false` | Flip horizontally |
| `flip_v` | boolean | `false` | Flip vertically |

**C Structure:**
```c
struct t_widget_image {
    t_widget_base base;
    char source_path[256];       // Path on SD card
    uint16_t* pixel_buffer;      // Cached pixels (RGBA565)
    uint16_t transparent;        // Transparent color value
    IMAGE_SCALE_MODE scale_mode; // Scaling behavior
    bool cached;                // Already loaded
    bool flip_h;                // Horizontal flip
    bool flip_v;                // Vertical flip
    uint8_t rotation;           // 0, 1, 2, 3 = 0°, 90°, 180°, 270°
};
```

**Scale Modes:**

| Mode | Behavior |
|------|----------|
| `stretch` | Fill entire widget bounds, ignore aspect ratio |
| `aspect_fit` | Fit within bounds, maintain aspect, letterbox |
| `aspect_fill` | Fill bounds, maintain aspect, crop edges |
| `tile` | Repeat image to fill area |
| `center` | Center image at native size |
| `none` | Draw at top-left at native size |

**Supported Formats:**

| Format | Support | Notes |
|--------|---------|-------|
| **BMP** | ✅ Full | 24-bit, 16-bit (565), 8-bit with palette |
| **RAW** | ✅ Full | Direct RGBA565 pixel data |
| **JPG** | ⚠️ Partial | Requires additional library |
| **PNG** | ❌ None | Too complex for ESP8266 |

**Memory Considerations:**

| Image Size | Memory Required |
|------------|-----------------|
| 100x100 | 20,000 bytes (20KB) |
| 200x200 | 80,000 bytes (80KB) |
| 320x240 | 153,600 bytes (150KB) |

**ESP8266 RAM:** ~80KB available for widgets

**Recommendations:**
- ✅ Cache images < 100x100
- ⚠️ Load images 100-200px on demand
- ❌ Avoid caching full-screen images
- ✅ Use sprites for complex compositing

**BMP Requirements:**
- Bit depth: 24-bit, 16-bit (565), or 8-bit with palette
- Color order: BGR (standard BMP)
- Compression: None (uncompressed only)
- Origin: Bottom-left (can be configured)

**Example with Image:**
```json
{
  "version": "1.0",
  "name": "ImageDemo",
  "size": { "width": 320, "height": 240 },
  "background": "#000000",
  "assets": ["assets/images/logo.bmp", "assets/images/bg.bmp"],
  "widgets": [
    {
      "id": "background",
      "type": "image",
      "x": 0, "y": 0,
      "width": 320, "height": 240,
      "source": "/assets/images/bg.bmp",
      "scale_mode": "stretch"
    },
    {
      "id": "logo",
      "type": "image",
      "x": 100, "y": 50,
      "width": 120, "height": 120,
      "source": "/assets/images/logo.bmp",
      "transparent_color": "#FF00FF"
    }
  ]
}
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
- ✅ **Push Notifications** – Real-time file change notifications with authentication
- ✅ **mDNS Discovery** – Zero-config device discovery via [hostname].local

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

### RAM Test Configuration

The `/etc/GUIKIT_autostart.ini` file includes a `[ram_test]` section for configuring RAM length detection:

```ini
[ram_test]
; Enable RAM length detection test at boot
enabled = true

; Number of test passes (1 or 2)
; 1 = Single pattern test
; 2 = Double pattern test (more reliable, detects wiring errors)
test_passes = 2

; Timeout in milliseconds
timeout_ms = 5000

; Show progress on TFT during test
show_progress = true

; Stop boot on test failure
stop_on_failure = false

; Expected sizes for each RAM bank (0 = auto-detect)
bank_0 = 0
bank_1 = 0
```

The RAM test detects wiring errors (e.g., 64K chip wired as 256K) and displays "WTM: X wired!" warnings on TFT.

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

### Push Notification API

| Endpoint | Method | Description | Authentication |
|----------|--------|-------------|----------------|
| `/webdav/push/sse` | GET | Server-Sent Events stream | Required |
| `/webdav/push/ws` | GET | WebSocket connection | Required |
| `/webdav/push/poll` | GET | Long polling endpoint | Required |
| `/webdav/push/subscribe` | POST | Subscribe to path | Required |
| `/webdav/push/unsubscribe` | POST | Unsubscribe | Required |

### mDNS Discovery

The device advertises itself via mDNS/Bonjour at `[hostname].local`:

| Service | Type | Port | Path | Description |
|---------|------|------|------|-------------|
| HTTP | `_http._tcp` | 80 | `/` | Web server |
| WebDAV | `_webdav._tcp` | 80 | `/webdav` | File server |
| GUIKit | `_guikit._tcp` | 8080 | `/gui` | GUI management |

Connect using:
```bash
# HTTP
curl http://esp8266.local/

# WebDAV
curl http://esp8266.local/webdav/

# GUIKit
# Open in browser: http://esp8266.local/gui/
```

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

## 🎨 GUIKit Web Editor

The **GUIKit Web Editor** is a full-featured web-based editor for creating and managing GUIKit projects directly from your browser.

### Features

- **Project Management**
  - Create new projects from templates (empty, basic_ui)
  - Open existing `.GUIKIT` projects from user or system directories
  - Browse project file tree with recursive directory listing
  - Recent projects tracking (last 10)

- **File Editing**
  - Tabbed editor supporting multiple open files
  - Syntax-aware editing for JSON, JavaScript, CSS, Lua
  - Live cursor position (Line:X, Col:Y)
  - Modified file indicators (*)
  
- **Contextual Menu**
  - Long press (~2 seconds) on text fields opens context menu
  - Copy, Cut, Paste, Select All operations
  - Clipboard management across files

- **WebDAV Integration**
  - Seamless connection to ESP8266 WebDAV server
  - User home directory (`/home/(username)/`) as WebDAV root
  - Automatic project loading from user's `projects/` directory
  - System project access for admin users

- **Temp Buffer Support**
  - Working copies saved to `/tmp/{filename}_edit.txt`
  - Auto-save to temp buffer on every change
  - Recovery support if editor closes unexpectedly

### Project Templates

| Template | Description | Files Created |
|----------|-------------|---------------|
| `empty` | Blank project | `main_gui.json`, `project.meta.json` |
| `basic_ui` | Basic UI with button | `main_gui.json`, `scripts/main.js`, `project.meta.json` |

### Usage

1. Open WebDAV connection in editor
2. Click "New" to create a project or "Open" to browse existing ones
3. Navigate project tree in left panel
4. Click files to open in tabbed editor
5. Edit and save - changes go directly to SD card

---

## 👥 User Management System

### Home Directory Structure

Each user gets their own home directory created from the skeleton template:

```
/etc/user.skel/                    # Template for new users
├── README.md                     # User documentation
└── projects/                     # Default projects directory

/home/(username)/                # User home directory
├── README.md                     # Copied from skeleton
├── projects/                     # User's GUIKit projects
│   ├── MyProject.GUIKIT/
│   │   ├── main_gui.json
│   │   ├── project.meta.json
│   │   ├── scripts/
│   │   └── styles/
│   └── AnotherProject.GUIKIT/
│       └── ...
```

### User Creation Workflow

1. Admin creates user via `users.GUIKIT`
2. System creates `/home/(username)/`
3. Recursively copies contents from `/etc/user.skel/`
4. Sets user home path in user profile
5. WebDAV authentication uses this home as root

### System GUIs

| GUI | Purpose | Location |
|-----|---------|----------|
| `chooser.GUIKIT` | Project chooser/launcher | `/gui/` |
| `webdav.GUIKIT` | WebDAV server management | `/gui/` |
| `users.GUIKIT` | User account management | `/gui/` |
| `editor.GUIKIT` | Web-based GUI editor | `/gui/` |

---

## 📦 Project File Structure

All GUIKit projects follow the `.GUIKIT` directory convention:

```
{project_name}.GUIKIT/
├── main_gui.json          # REQUIRED: Root GUI definition
├── project.meta.json     # OPTIONAL: Project metadata
├── assets/               # OPTIONAL: Static resources
│   ├── images/           # Image files
│   ├── fonts/            # Font files
│   └── sounds/           # Audio files
├── gui/                  # OPTIONAL: Additional GUI files
│   └── sub_gui.json
├── scripts/              # OPTIONAL: Script files
│   ├── main.js           # JavaScript functions
│   └── helpers.lua       # Lua scripts
└── styles/               # OPTIONAL: CSS/theme files
    └── theme.css
```

### Project Metadata (project.meta.json)

```json
{
  "name": "MyProject",
  "description": "Sample GUIKit project",
  "author": "Developer Name",
  "version": "1.0.0",
  "created": "2026-08-15T00:00:00Z",
  "modified": "2026-08-15T00:00:00Z",
  "gui_files": ["main_gui.json", "settings.json"],
  "scripts": ["scripts/main.js"],
  "styles": ["styles/theme.css"],
  "dependencies": ["webdav.GUIKIT"],
  "category": "user",
  "type": "application"
}
```

### GUI File Format (main_gui.json)

```json
{
  "version": "1.0",
  "name": "MyGUI",
  "size": { "width": 320, "height": 240 },
  "background": "#1E1E1E",
  "theme": "dark",
  "widgets": [
    {
      "id": "my_button",
      "type": "button",
      "x": 100, "y": 50,
      "width": 120, "height": 40,
      "text": "Click Me",
      "background": "#1177BB",
      "action": "my_function"
    }
  ]
}
```

---

## 💾 Updated SD Card Structure

```
SD Card Root/
├── Kernel.bin.gz                    # Compressed kernel binary
├── index.html                       # Web interface entry point
├── /etc/                           # System configuration
│   ├── user.skel/                  # User home template
│   │   ├── README.md
│   │   └── projects/
│   └── guikitloader.conf            # GUIKit loader configuration
│
├── /gui/                           # System GUIs (shared)
│   ├── chooser.GUIKIT/             # Project chooser
│   │   ├── main_gui.json
│   │   └── project.meta.json
│   ├── webdav.GUIKIT/              # WebDAV management
│   │   ├── main_gui.json
│   │   ├── project.meta.json
│   │   └── scripts/webdav.js
│   ├── users.GUIKIT/               # User management
│   │   ├── main_gui.json
│   │   ├── project.meta.json
│   │   └── scripts/users.js
│   └── editor.GUIKIT/              # Web editor
│       ├── main_gui.json
│       ├── project.meta.json
│       └── scripts/editor.js
│
├── /home/                          # User home directories
│   └── (username)/                 # Per-user directory
│       ├── README.md
│       └── projects/               # User's projects
│           └── MyProject.GUIKIT/
│
├── /system/                        # System data
│   ├── ui/                          # Legacy UI definitions
│   ├── dict/                        # Dictionaries
│   ├── config/                      # Configurations
│   └── logs/                        # System logs
│
└── /tmp/                           # Temporary files
    └── {filename}_edit.txt          # Editor temp buffers
```

---

## 🚀 Quick Start with New Features

### 1. First Boot

1. Flash bootloader and copy Kernel.bin to SD card
2. Insert SD card and power on ESP8266
3. The `chooser.GUIKIT` loads automatically
4. Scan and display available `.GUIKIT` projects

### 2. Create Your First User

1. Open `users.GUIKIT` from chooser
2. Click "New User" button
3. Enter username, password, permissions
4. System creates `/home/(username)/` from `/etc/user.skel/`

### 3. WebDAV Access

1. Open `webdav.GUIKIT`
2. Enter server URL, username, password
3. Connect - WebDAV root = `/home/(username)/`
4. Browse and manage files

### 4. Create a Project

1. Open `editor.GUIKIT`
2. Click "New" button
3. Enter project name (e.g., "MyApp")
4. Select template (basic_ui recommended)
5. Project created at `/home/(username)/projects/MyApp.GUIKIT/`
6. `main_gui.json` opens automatically

### 5. Edit and Save

1. Modify widgets in the JSON editor
2. Use contextual menu (long press) for text operations
3. Click "Save" or "Save All"
4. Changes written directly to SD card

---

## 🛠️ Script Reference

### GUIKitEditor API (editor.js)

```javascript
// Initialization
GUIKitEditor.init();

// Project Management
GUIKitEditor.newProject();      // Create new project
GUIKitEditor.openProject();     // Open existing project
GUIKitEditor.getCurrentProject(); // Get current project name

// File Operations
GUIKitEditor.openFile(path);    // Open file by path
GUIKitEditor.saveFile(path, content); // Save file
GUIKitEditor.getCurrentFile();  // Get current file info
GUIKitEditor.getFileContent(); // Get current file content
GUIKitEditor.isModified();      // Check if modified

// Editor Operations
GUIKitEditor.copy();            // Copy selection
GUIKitEditor.cut();             // Cut selection  
GUIKitEditor.paste();           // Paste from clipboard
GUIKitEditor.selectAll();       // Select all text

// WebDAV
GUIKitEditor.connectWebDAV();   // Connect to WebDAV
GUIKitEditor.isConnected();    // Check connection status
GUIKitEditor.getUserHome();     // Get user home directory
```

### UserManager API (users.js)

```javascript
UserManager.init();
UserManager.create(username, password, permissions);
UserManager.getByUsername(username);
UserManager.getAll();
UserManager.update(username, updates);
UserManager.remove(username);
```

### WebDAVManager API (webdav.js)

```javascript
WebDAVManager.connect(url, username, password);
WebDAVManager.disconnect();
WebDAVManager.isConnected();
WebDAVManager.getUserHome();
WebDAVManager.setUser(username, password, home);
WebDAVManager.browse();
```

---

## 🎮 Multi-Platform Support

### ESP32 Support Added

The project now supports **both ESP8266 and ESP32** with a unified codebase using preprocessor directives for platform-specific code.

#### Platform-Specific Features

| Feature | ESP8266 | ESP32 |
|---------|---------|-------|
| Internal RAM | ~80KB | ~320KB |
| PSRAM Support | ❌ No | ✅ Yes (8MB+) |
| SPI Speed | 20MHz max | 40MHz+ |
| Dual Core | ❌ No | ✅ Yes |
| Bluetooth | ❌ No | ✅ Yes |
| WiFi | ✅ Yes | ✅ Yes (faster) |

#### Build Commands

```bash
# ESP8266 (default)
make all
pio run -e esp8266_default

# ESP32
make all PLATFORM=esp32
pio run -e esp32_default

# ESP32 with PSRAM
pio run -e esp32_psram
```

#### Platform Detection

The bootloader detects the MCU platform at **compile time** using preprocessor definitions:

```c
#ifdef ESP8266
    // ESP8266-specific code
    #define GUIKIT_PLATFORM_ESP8266 1
#elif ESP32
    // ESP32-specific code
    #define GUIKIT_PLATFORM_ESP32 1
#endif
```

The same source code compiles for both platforms with appropriate configurations.

#### Flash Layout Differences

**ESP8266 (4MB Flash):**
- 0x00000-0x01000: Bootloader (4KB)
- 0x01000-0x10000: User Code (60KB)
- 0x10000-0x11000: Config Partition (4KB)
- 0x11000+: SPIFFS (~3.8MB)

**ESP32 (8MB Flash):**
- 0x00000-0x01000: Bootloader (4KB)
- 0x01000-0x08000: Partition Table + NVS (48KB)
- 0x08000-0x09000: Phy_init (4KB)
- 0x09000-0x10000: Reserved (28KB)
- 0x10000-0x11000: Config Partition (4KB)
- 0x11000-0x20000: Kernel Partition (64KB)
- 0x20000+: Storage (~7.8MB)

### Config Partition

A dedicated 4KB partition at address `0x10000` stores:
- Hardware detection results
- Memory strategy configuration
- Boot information (boot count, last error)
- User preferences

This allows the system to persist configuration across reboots.

### TFT Error Display

If the bootloader detects an unsupported MCU or configuration error, it attempts to display an error message on the TFT for debugging.

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
*GUIKit Web Editor and User Management System added*

