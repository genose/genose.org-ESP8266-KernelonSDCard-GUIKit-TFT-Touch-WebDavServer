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

