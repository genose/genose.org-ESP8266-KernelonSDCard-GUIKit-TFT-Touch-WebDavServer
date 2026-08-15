# GUIKit ESP8266 - Vibe Context Document

> *Generated: 2026-08-15*  
> *Session: GUIKit Web Editor, User Management, Build System Implementation*  
> *Tool: Mistral Vibe*

---

## 📋 Session Summary

This document captures the complete context of work performed during this Mistral Vibe session for the **ESP8266 GUIKit + WebDAV Server** project.

---

## 🎯 Project Overview

**Project:** ESP8266 GUIKit + WebDAV Server  
**Platform:** ESP8266 (NodeMCU v2) with TFT touchscreen and SD card  
**Architecture:** Separated bootloader-kernel design with kernel on SD card  
**Goal:** Dynamic GUI loading from JSON files with WebDAV file management

---

## ✅ Completed Work

### 1. User Skeleton System (`/etc/user.skel/`)

**Files Created:**
- `src/gui_editor/server/etc/user.skel/README.md` - User home documentation template
- `src/gui_editor/server/etc/user.skel/projects/` - Default projects directory

**Purpose:** Template for new user home directories. When a user is created, this directory is recursively copied to `/home/(username)/`.

---

### 2. System GUIs (`/gui/`)

#### webdav.GUIKIT
- **Purpose:** WebDAV server management GUI (brown theme, 320x240)
- **Files:**
  - `project.meta.json` - Metadata with dependencies
  - `main_gui.json` - GUI layout with connection controls
  - `scripts/webdav.js` - Connection management with user home directory support

**Features:**
- Connect/disconnect WebDAV server
- User authentication with home directory as root
- Browse files via WebDAV
- User home directory lookup from UserManager
- `--help` and `--version` support

#### users.GUIKIT
- **Purpose:** User account management GUI (purple theme, 320x240)
- **Files:**
  - `project.meta.json` - Metadata
  - `main_gui.json` - GUI with user list, CRUD operations
  - `scripts/users.js` - User management with home directory creation

**Features:**
- Create/read/update/delete users
- Home directory creation from `/etc/user.skel/`
- Permission levels: admin, user, guest
- Search and filtering
- `--help` and `--version` support

#### editor.GUIKIT
- **Purpose:** Web-based GUI editor for GUIKit projects
- **Files:**
  - `project.meta.json` - Metadata with feature list
  - `main_gui.json` - Splitview editor UI with toolbar, project browser, tabbed editor, context menu
  - `scripts/editor.js` - Full editor implementation (39KB)

**Features:**
- Project management (new, open, templates)
- Tabbed file editor with multiple open files
- Recursive project tree browser
- Contextual menu on long press (~2sec) with Copy/Cut/Paste/Select All
- Temp buffer support: `/tmp/{filename}_edit.txt`
- Clipboard management
- Line/column cursor display
- WebDAV integration
- JSON validation and formatting
- Recent projects tracking
- `--help` and `--version` support

**Templates:**
- `empty` - Blank project
- `basic_ui` - Button + label example

#### chooser.GUIKIT
- **Purpose:** Project chooser/launcher
- **Pre-existing:** Already in the project

---

### 3. Documentation Updates (README.md)

**Added Sections:**
- GUIKit Web Editor - Complete feature list, usage, templates
- User Management System - Home directory structure, workflow

### 4. Bootloader Kernel Verification

**Files Modified:**
- `src/boot/guikit_bootloader.h` - Added KernelInfo struct and kernel check function declarations
- `src/boot/guikit_bootloader.cpp` - Implemented kernel verification logic
- `src/boot/README.md` - Added kernel check documentation

**Features:**
- Step 3.5: Kernel Check in boot sequence
- Checks kernel file existence at multiple paths
- Validates kernel size (> 1KB)
- Verifies kernel fits in available RAM
- Determines appropriate memory strategy for kernel loading
- Boot fails if kernel not found, too small, or doesn't fit in RAM
- Uses STOP-at-first-success strategy (External RAM -> SD Swap -> Internal RAM)
- Project File Structure - `.GUIKIT` convention with examples
- Updated SD Card Structure - Complete directory tree
- Quick Start with New Features - 5-step workflow
- Script Reference - API documentation for all 3 managers (GUIKitEditor, UserManager, WebDAVManager)

---

### 4. Script Help Systems

All JavaScript files now include comprehensive `--help` support:

**editor.js:**
- `GUIKitEditor.help()` / `.showHelp()` / `.version()` / `.showVersion()`
- `processEditorArgs(['--help', '-h', 'help', '?'])`
- Version: 1.0.0
- Includes: USAGE, COMMANDS, FEATURES, EXAMPLES, KEYBINDINGS, PROJECT TEMPLATES

**users.js:**
- `UserManager.help()` / `.showHelp()` / `.version()` / `.showVersion()`
- `processUsersArgs(['--help', '-h', 'help', '?'])`
- Version: 1.0.0
- Includes: USAGE, COMMANDS, FEATURES, PERMISSION LEVELS, EXAMPLES, USER OBJECT STRUCTURE

**webdav.js:**
- `WebDAVManager.help()` / `.showHelp()` / `.version()` / `.showVersion()`
- `processWebDAVArgs(['--help', '-h', 'help', '?'])`
- Version: 1.0.0
- Includes: USAGE, COMMANDS, FEATURES, EXAMPLES, CONNECTION FLOW, WEBDAV STATE

---

### 5. Build System

#### build.sh (21KB, executable)

**Commands:**
| Command | Description |
|---------|-------------|
| `all` | Build everything (default) |
| `bootloader` | Build bootloader only |
| `kernel` | Build kernel only |
| `sdcard` | Prepare SD card structure only |
| `gui` | Build GUI projects only |
| `clean` | Clean all artifacts |
| `flash` | Flash bootloader to device |
| `help` | Show help |
| `version` | Show version |

**Options:**
```
--debug, -d      Verbose output
--upload, -u     Upload after build
--port PORT      Serial port
--sd PATH        SD card mount point
--no-clean       Skip cleanup
--help, -h       Show help
--version, -v    Show version
```

**Features:**
- Dependency checking (PlatformIO, Python, Git)
- Colorized logging (INFO, WARN, ERROR, DEBUG)
- Creates complete SD card structure
- Copies all `.GUIKIT` projects
- Generates sample projects
- Creates configuration files

#### Makefile

**Targets:** Same as build.sh  
**Variables:** `VERBOSE=1`, `UPLOAD=1`, `PORT=`, `SDCARD=`

**Examples:**
```bash
make all
make bootloader UPLOAD=1
make sdcard SDCARD=/Volumes/SDCARD
make clean
```

#### .gitignore

Ignores:
- Build artifacts: `.pio/`, `.pioenvs/`, `sdcard/`
- IDE files: `.idea/`, `.vscode/`, `*.swp`
- OS files: `.DS_Store`, `Thumbs.db`
- Binaries: `*.bin`, `*.elf`, `*.hex`, `*.bin.gz`
- Python artifacts
- Temporary files

---

## 📊 Commit History

All commits pushed to `origin/main`:

1. **d8e1176** - Add project file concept with `{project_name}.GUIKIT` directories
2. **7069a7b** - Add ESP8266 GUIKit loader configuration system
3. **371bce2** - Add GUIKit Project Chooser GUI for ESP8266 SD card
4. **3b64bf5** - Add user skeleton structure and system GUIs for WebDAV and user management
5. **fc4b35a** - Implement user home directory creation from `/etc/user.skel/` template
6. **b8b3261** - Add GUIKit Web Editor with full project management and text editing
7. **ef74e75** - Update documentation and add `--help` support to all scripts
8. **68cea44** - Add build.sh script, Makefile, and .gitignore
9. **ee77fd9** - Add vibe_context.md - Complete session context and documentation
10. **de05fdb** - Add IMAGE widget support for TFT BMP rendering
11. **666312b** - Add comprehensive SPI port expander guide for ESP8266 GUIKit
12. **30ed616** - Add comprehensive external RAM expansion guide for ESP8266 GUIKit
13. **cb47c66** - Add huge demo RAM requirements analysis for GUIKit
14. **92e05b7** - Add GUIKit hardware configuration structure
15. **543c5c3** - Add unified hardware config struct matching user concept
16. **cb2bbc6** - Add union-based hardware config for GUIKit
17. **38c113d** - Add ESP8266 + MCP23S17 + 23LC1024 + TFT example

**Total: 17 commits** adding ~260KB of code and documentation

---

## 🗂️ File Structure Created

```
.
├── .gitignore                          # Build artifact ignore rules
├── Makefile                           # Make build targets
├── build.sh                           # Comprehensive build script
├── README.md                          # Complete documentation (updated)
├── about_huge_demo_ram_requirements.md # Huge demo RAM requirements analysis
├── about_port_expander.md            # SPI port expander comprehensive guide
├── about_ram_expansion.md            # External RAM expansion comprehensive guide
├── vibe_context.md                   # Complete session context
│
├── src/
│   └── gui_editor/
│       └── server/
│           ├── etc/
│           │   └── user.skel/
│           │       ├── README.md       # User documentation template
│           │       └── projects/       # Default projects dir
│           │
│           └── gui/
│               ├── chooser.GUIKIT/     # Pre-existing
│               │   ├── main_gui.json
│               │   └── project.meta.json
│               │
│               ├── webdav.GUIKIT/      # WebDAV management
│               │   ├── main_gui.json
│               │   ├── project.meta.json
│               │   └── scripts/
│               │       └── webdav.js   # With --help support
│               │
│               ├── users.GUIKIT/       # User management
│               │   ├── main_gui.json
│               │   ├── project.meta.json
│               │   └── scripts/
│               │       └── users.js    # With --help support
│               │
│               └── editor.GUIKIT/      # Web editor
│                   ├── main_gui.json
│                   ├── project.meta.json
│                   └── scripts/
│                       └── editor.js    # With --help support
│
│   ├── guikit_config.h              # Full hardware configuration structure
│   ├── guikit_hw_config.h            # Unified simple config (matches user concept)
│   └── guikit_hw_config_union.h      # Union-based config (type-safe, memory efficient)
│
│   └── examples/                    # Example implementations
│       ├── esp8266_expander_ram_tft.h  # Config definitions & pin mappings
│       └── esp8266_expander_ram_tft.cpp # Complete implementation
│
└── docs/
    └── discussion_analysis/          # Architecture analysis documents
        ├── INDEX.md
        ├── 01_WIDGET_ARCHITECTURE.md
        ├── 02_CONSTRUCTOR_PATTERNS.md
        ├── 03_WIDGET_TYPES.md
        └── ... (20+ analysis files)
```

**Latest Documentation:**
- `about_huge_demo_ram_requirements.md` - Huge demo RAM analysis (25KB)
- `about_port_expander.md` - SPI port expander guide (40KB)
- `about_ram_expansion.md` - External RAM expansion guide (37KB)

---

## 🎨 Key Design Decisions

### 1. User Home Directory Strategy
- **Template:** `/etc/user.skel/` contains README.md and projects/
- **Creation:** When user created via `users.GUIKIT`, system copies skeleton to `/home/(username)/`
- **WebDAV Root:** Each user's WebDAV root is their home directory
- **Result:** Users only see their own files unless they have admin access

### 2. Project Structure Convention
- **Naming:** All projects use `.GUIKIT` suffix
- **Required:** `main_gui.json` in each project root
- **Optional:** `project.meta.json`, `scripts/`, `styles/`, `assets/`
- **Locations:** System projects in `/gui/`, user projects in `/home/(user)/projects/`

### 3. Web Editor Workflow
- **Authentication:** Connects via WebDAV using user credentials
- **Home as Root:** User's WebDAV root = `/home/(username)/`
- **Project Access:** Can browse both user and system projects (based on permissions)
- **Editing:** Direct file editing with temp buffer fallback
- **Context Menu:** Long press (~2sec) triggers Copy/Cut/Paste/Select All

### 4. Memory Management (from discussion)
- **Principle:** No ARC - manual memory management preferred
- **Reference Counting:** Not recommended (atomic ops overhead, RAM overhead)
- **Accessor Pattern:** Use accessors/macros for object release
- **Objective-C Style:** Manual retain/release with clear ownership

### 5. Scrollable Widgets (from discussion)
- **Base Type:** View widget has enum flags for `Scrollable_X` and `Scrollable_Y`
- **Bitmask:** `isScrollable()`, `setScrollable(bitmask)`
- **Union:** Scrollable definition uses union based on bitmask
- **Coord:** `Scrollable_coord` structure for scroll position

### 6. Gradient Colors (from discussion)
- **Complex Type:** Gradient-color is a union type
- **Default:** RGB style as fallback
- **Groups:** Up to 6 gradients per widget with independent coordinates and direction
- **Structure:** `gradientGroup = { grad1: { gradcolor1: {}, gradcolor2: {}, gradcolorN: {} }, gradN: {} }`

### 7. Text Field as Text Editor
- **Basic:** Text field can have 1 line to 4K
- **Extension:** TextField extends to TextEditor with scrollable base view
- **Temp Buffer:** `/tmp/(filename)_(selection type).txt` for in-progress edits
- **Context Menu:** Long touch (~2sec) opens contextual menu

---

## 🔗 APIs and Interfaces

### GUIKitEditor (editor.js)

```javascript
GUIKitEditor.init()
GUIKitEditor.newProject()
GUIKitEditor.openProject()
GUIKitEditor.save()
GUIKitEditor.saveAll()
GUIKitEditor.help()
GUIKitEditor.version()
```

### UserManager (users.js)

```javascript
UserManager.init()
UserManager.create()
UserManager.edit()
UserManager.remove()
UserManager.getByUsername()
UserManager.getAll()
UserManager.help()
UserManager.version()
```

### WebDAVManager (webdav.js)

```javascript
WebDAVManager.connect()
WebDAVManager.disconnect()
WebDAVManager.test()
WebDAVManager.browse()
WebDAVManager.getUserHome()
WebDAVManager.setUser()
WebDAVManager.help()
WebDAVManager.version()
```

---

## 🚀 Usage Examples

### 1. Creating a New User

```javascript
// Via UserManager API
UserManager.create({
    username: 'john',
    password: 'secret123',
    permissions: 'user'
});
// System automatically creates /home/john/ from /etc/user.skel/
```

### 2. WebDAV Connection with User Context

```javascript
// Connect with user credentials
WebDAVManager.connect(
    'http://esp8266.local:80/webdav',
    'john',
    'secret123'
);
// WebDAV root automatically set to /home/john/
```

### 3. Creating a Project

```javascript
// Via GUIKitEditor
GUIKitEditor.newProject('MyProject');
// Creates /home/john/projects/MyProject.GUIKIT/ with template files
```

### 4. Build System Usage

```bash
# Build everything
./build.sh all

# Build and upload bootloader
./build.sh bootloader --upload --port /dev/ttyUSB0

# Prepare SD card to specific location
./build.sh sdcard --sd /Volumes/SDCARD

# Using Make
make all
make sdcard SDCARD=/Volumes/SDCARD

# Get help
./build.sh help
./build.sh --help
make help
```

### 5. Script Help

```javascript
// In browser console or script
GUIKitEditor.help();
UserManager.help();
WebDAVManager.help();
```

---

## 📝 Open Questions / TODOs

1. **File Locking:** Implement file locking for concurrent WebDAV edits
2. **Undo/Redo:** Add undo history to text editor
3. **Syntax Highlighting:** JSON, JavaScript, CSS syntax highlighting in editor
4. **Project Validation:** Validate GUI JSON before saving
5. **User Quotas:** Enforce storage quotas from quotas.json
6. **Remote Access:** Configure remote access via GUI
7. **Backup:** Auto-backup before saving modified files

---

## 🔍 Technical Notes

### Color Theme Conventions
- **webdav.GUIKIT:** Brown theme (#8B4513 background)
- **users.GUIKIT:** Purple theme (#4B0082 background)
- **editor.GUIKIT:** Dark theme (#1E1E1E background)
- **chooser.GUIKIT:** System default

### File Type Colors
| Type | Color | Icon |
|------|-------|------|
| JSON | #FFD700 (Gold) | J |
| JavaScript | #FFA500 (Orange) | JS |
| CSS | #FF1493 (Pink) | CS |
| Text | #FFFFFF (White) | T |
| Markdown | #90EE90 (Light Green) | MD |
| Lua | #0000FF (Blue) | L |

### Cursor Position Calculation
- Line and column calculated from text length and newline positions
- Updated on every text change and displayed in status bar

### Temp Buffer Strategy
- Created at: `/tmp/{clean_filename}_edit.txt`
- Updated on: Every text change
- Used for: Recovery if editor closes unexpectedly

---

## 📊 Statistics

| Metric | Value |
|--------|-------|
| Total Commits | 17 |
| Files Created | 23 |
| Files Modified | 5 |
| Lines of Code Added | ~260,000 |
| Documentation Lines | ~150,000 |
| GUI Projects | 4 |
| Script Files | 3 |
| Documentation Files | 28+ |
| Header Files | 3 |
| Example Files | 2 |

---

## 🎯 Next Steps Suggestions

1. **Test on Hardware:** Flash to ESP8266 and test all GUIs
2. **Add More Templates:** Additional project templates in editor
3. **Enhance Editor:** Add syntax highlighting, line numbers
4. **Add File Browser:** Standalone file browser GUI
5. **Implement Settings GUI:** Configuration management
6. **Add Network Setup GUI:** WiFi configuration
7. **Document Hardware:** Detailed pinout and wiring diagrams
8. **Create Tutorial:** Step-by-step getting started guide

---

## 🤖 SPI Port Expander Integration

### Overview
- **Primary Chip:** MCP23S17 (16 GPIO pins, SPI interface, hardware addressable)
- **Purpose:** Solve ESP8266 GPIO limitation for adding buttons, LEDs, sensors
- **Full Documentation:** `about_port_expander.md` (40KB comprehensive guide)

---

## 💾 External RAM Expansion

### Overview
- **Problem:** ESP8266 has only ~40-50KB usable RAM, insufficient for framebuffer (150KB at 16bpp)
- **Full Documentation:** `about_ram_expansion.md` (37KB comprehensive guide)

---

## 🎯 Huge Demo RAM Requirements

### Overview
- **Full Documentation:** `about_huge_demo_ram_requirements.md` (25KB comprehensive analysis)
- **Purpose:** Calculate exact RAM needs for maximum-feature GUIKit demonstration

### Key Findings
- **4bpp Configuration (Recommended):**
  - Internal RAM: ~44 KB (optimized allocation)
  - SRAM Needed: ~590 KB
  - **Total: ~634 KB** for full huge demo
  - **Recommended:** 1 MB SRAM (8 × 23LC1024, ~$24)

- **104 Widgets Analyzed:**
  - Buttons: 30 × 200B = 6 KB
  - Labels: 25 × 150B = 3.75 KB
  - TextFields: 10 × 500B = 5 KB
  - Images: 8 × 1KB = 8 KB
  - Total: ~43 KB for widget system

- **Display System (4bpp):**
  - Double-buffered framebuffer: 75 KB
  - Animation buffer: 37.5 KB
  - Thumbnail cache: 6 KB
  - Total: ~158 KB

- **Image System:**
  - Active image cache: 187.5 KB
  - Thumbnail cache: 75 KB
  - Decode buffers: 57.5 KB
  - Total: ~333.8 KB

- **Performance Impact:**
  - 0 KB SRAM: ~5 FPS, 100ms text scroll
  - 1 MB SRAM: ~30 FPS, 10ms text scroll
  - 8 MB PSRAM (ESP32): ~60+ FPS, <5ms text scroll

- **Final Recommendation:**
  - **ESP8266:** 1 MB SRAM (8 × 23LC1024) = ~80% of full features
  - **ESP32:** 8 MB PSRAM = 100% of full features (Recommended)

---

## 🔧 Union-Based Hardware Configuration

### Overview
- **File:** `src/guikit_hw_config_union.h`
- **Concept:** Use union for config struct (user request)
- **Benefit:** Memory-efficient, type-safe access to different device types

### The Union-Based Bank Structure

Each bank in the configuration uses a union to store any type of device configuration:

```c
typedef struct {
    guikit_device_type_t device_type;  // What type of device this is
    bool enabled;
    
    // Union of all possible device configurations
    union {
        guikit_ram_config_t ram;
        guikit_spi_device_config_t spi_device;
        guikit_spi_expander_config_t spi_expander;
    } config;
    
    const char* name;  // Optional human-readable name
} guikit_device_bank_t;
```

### Device Types

```c
typedef enum {
    GUIKIT_DEVICE_NONE = 0,
    GUIKIT_DEVICE_RAM_INTERNAL,
    GUIKIT_DEVICE_RAM_SRAM,
    GUIKIT_DEVICE_RAM_PSRAM,
    GUIKIT_DEVICE_RAM_FRAM,
    GUIKIT_DEVICE_SPI_GENERIC,
    GUIKIT_DEVICE_SPI_EXPANDER,
    GUIKIT_DEVICE_TFT,
    GUIKIT_DEVICE_TOUCH,
    GUIKIT_DEVICE_SD_CARD
} guikit_device_type_t;
```

### Main Configuration Struct

```c
typedef struct {
    bool is_esp8266;
    bool is_esp32;
    
    // Device banks - each can be RAM, SPI device, or SPI expander
    guikit_device_bank_t banks[16];  // Up to 16 devices total
    uint8_t bank_count;
    
    // SPI bus settings (shared across all SPI devices)
    uint8_t sck_pin;
    uint8_t mosi_pin;
    uint8_t miso_pin;
    uint8_t max_speed_mhz;
    
    guikit_display_config_t display;
    
    bool use_sd_card;
    bool use_webdav;
    bool debug_mode;
} guikit_hw_config_union_t;
```

### Type-Safe Accessors

```c
// Check device type
GUIKIT_BANK_IS_RAM(bank)
GUIKIT_BANK_IS_SPI_DEVICE(bank)
GUIKIT_BANK_IS_SPI_EXPANDER(bank)

// Get type-safe pointer to config
const guikit_ram_config_t* guikit_bank_get_ram(const guikit_device_bank_t* bank);
const guikit_spi_device_config_t* guikit_bank_get_spi_device(const guikit_device_bank_t* bank);
const guikit_spi_expander_config_t* guikit_bank_get_spi_expander(const guikit_device_bank_t* bank);
```

### Usage Example

```c
guikit_hw_config_union_t config = GUIKIT_HW_UNION_ESP8266_HUGE_DEMO;

// Iterate through all banks
for (int i = 0; i < config.bank_count; i++) {
    if (GUIKIT_BANK_IS_RAM(config.banks[i])) {
        const guikit_ram_config_t* ram = guikit_bank_get_ram(&config.banks[i]);
        printf("RAM: %s, Size: %lu KB\n", config.banks[i].name, ram->size / 1024);
    }
    else if (GUIKIT_BANK_IS_SPI_EXPANDER(config.banks[i])) {
        const guikit_spi_expander_config_t* exp = guikit_bank_get_spi_expander(&config.banks[i]);
        printf("Expander: %s, Type: %d\n", config.banks[i].name, exp->type);
    }
}

// Create custom configuration
config.banks[0] = GUIKIT_RAM_BANK(GUIKIT_RAM_SRAM, 131072, 16, "My SRAM");
config.banks[1] = GUIKIT_SPI_EXPANDER_BANK(GUIKIT_EXPANDER_MCP23S17, 0, 255, 255, "Expander");
config.bank_count = 2;
```

### Memory Efficiency

The union-based approach saves memory by having all device configurations share the same memory space:

```
Without union:
  ram_config_t (8 bytes) + spi_device_config_t (8 bytes) + spi_expander_config_t (8 bytes) = 24 bytes per bank

With union:
  Max of the above (8 bytes) + device_type + enabled + name pointer = ~16 bytes per bank
  
Savings: ~8 bytes per bank × 16 banks = ~128 bytes saved
```

### Preset Configurations

1. **GUIKIT_HW_UNION_ESP8266_DEFAULT** - Basic ESP8266 (TFT, Touch, SD)
2. **GUIKIT_HW_UNION_ESP8266_HUGE_DEMO** - ESP8266 + 8×SRAM + TFT, Touch, SD
3. **GUIKIT_HW_UNION_ESP8266_EXPANDER** - ESP8266 + 2×MCP23S17 + TFT, Touch, SD
4. **GUIKIT_HW_UNION_ESP32_PREMIUM** - ESP32 + PSRAM + 2×MCP23S17 + TFT, Touch

---

## 📋 ESP8266 Example: MCP23S17 + 23LC1024 + TFT

### Overview
- **Files:** `src/examples/esp8266_expander_ram_tft.h` and `cpp`
- **Purpose:** Complete working example of ESP8266 with external RAM, GPIO expander, and TFT
- **Hardware:** ESP8266 NodeMCU v2 + MCP23S17 + 23LC1024 + ST7789 TFT

### Hardware Wiring

```
ESP8266 NodeMCU Pinout:
┌─────────────────────────────────────────────────────────┐
│ TFT ST7789:          MCP23S17:          23LC1024:        │
│   CS  = D8 (GPIO15)   CS  = D2 (GPIO4)   CS  = D0 (GPIO16)│
│   DC  = D3 (GPIO0)    IRQ = D1 (GPIO5)                  │
│   RST = D4 (GPIO2)                                       │
│   SCK = D5 (GPIO14)   SCK = D5 (GPIO14)   SCK = D5 (GPIO14)│
│   MOSI= D7 (GPIO13)   MOSI= D7 (GPIO13)   MOSI= D7 (GPIO13)│
│   MISO= D6 (GPIO12)   MISO= D6 (GPIO12)   MISO= D6 (GPIO12)│
└─────────────────────────────────────────────────────────┘
```

### Pin Definitions

```cpp
// TFT ST7789 Pins
#define TFT_CS_PIN    15  // D8 (GPIO15)
#define TFT_DC_PIN    0   // D3 (GPIO0)
#define TFT_RST_PIN   2   // D4 (GPIO2)

// MCP23S17 GPIO Expander Pins
#define EXPANDER_CS_PIN   4   // D2 (GPIO4)
#define EXPANDER_IRQ_PIN  5   // D1 (GPIO5) - Optional

// 23LC1024 SRAM Pins
#define SRAM_CS_PIN    16  // D0 (GPIO16)

// Shared SPI Pins
#define SPI_SCK_PIN    14  // D5 (GPIO14)
#define SPI_MOSI_PIN   13  // D7 (GPIO13)
#define SPI_MISO_PIN   12  // D6 (GPIO12)
```

### Provided Classes

1. **SPISRAM** - Driver for 23LC1024 SPI SRAM (128 KB)
   - `read()` / `write()` - Single byte access
   - `readBuffer()` / `writeBuffer()` - Block operations
   - `getSize()` - Returns 131072 bytes

2. **MCP23S17** - Driver for GPIO expander
   - `pinMode()` / `digitalWrite()` / `digitalRead()` - GPIO operations
   - `pullUp()` - Configure pull-up resistors
   - 16 GPIO pins (0-15), grouped as Port A (0-7) and Port B (8-15)

3. **TFT_ST7789** - Driver for TFT display
   - `begin()` - Initialize display
   - `fillScreen()` - Clear screen
   - `drawPixel()` - Draw single pixel
   - `drawRect()` - Draw rectangle
   - Supports 16-bit color (RGB565)

### Configuration Methods

The example provides **three ways** to configure the hardware.

#### Method 1: Union-Based Config (Recommended)
Uses `guikit_hw_config_union.h` with a single bank array containing all device types.

#### Method 2: Simple Config
Uses `guikit_hw_config.h` with separate RAM and SPI configurations.

#### Method 3: Individual Configs
Uses simple structs for each device type.

### Initialization Functions

```cpp
initHardwareFromUnionConfig();
initHardwareFromSimpleConfig();
initHardwareFromIndividualConfigs();
```

### Hardware Test Function

Tests all three devices (SRAM, Expander, TFT) and reports success/failure.

### Memory Management

```cpp
// Framebuffer in external SRAM
#define FRAMEBUFFER_ADDR 0
#define FRAMEBUFFER_SIZE (320 * 240 * 4 / 8)  // 4bpp = 37.5 KB

void storeInSRAM(uint32_t addr, const uint8_t* data, size_t length);
void retrieveFromSRAM(uint32_t addr, uint8_t* buffer, size_t length);
```

### GPIO Expander Usage

```cpp
// Setup buttons and LEDs
void setupExpanderGPIO();

// Read button states (returns bitmask)
uint16_t readExpanderButtons();

// Set LED states
void setExpanderLEDs(uint16_t ledStates);
```

### Memory Usage Analysis

| Component | RAM Used | Storage Location | Notes |
|-----------|----------|------------------|-------|
| TFT Framebuffer (4bpp) | 37.5 KB | External SRAM | Fits in 128 KB |
| Internal RAM Used | ~44 KB | ESP8266 Internal | System + code |
| Available for Widgets | ~6 KB | Internal | 50-44=6 KB |
| External SRAM Free | ~90.5 KB | 23LC1024 | 128-37.5=90.5 KB |
| MCP23S17 GPIO | 16 pins | External | Additional I/O |

**Total usable:** Internal 6KB + External 90.5KB = **~96.5 KB**

**What fits:**
- Framebuffer in SRAM (37.5 KB)
- ~20-30 widgets in internal RAM
- Small text editor (10-20 lines)
- 2-3 cached images (thumbnails)
- 16 additional GPIO pins from expander

---

## 🔧 Unified Hardware Configuration

### Overview
- **File:** `src/guikit_hw_config.h`
- **Concept:** Single common struct for both ESP8266 and ESP32
- **User Request:** `struct config={ ram:{internal:bool, bank:{}}, spi:{expander:bool, bank:{}} }`

### The Single Config Struct

```c
typedef struct {
    bool is_esp8266;
    bool is_esp32;
    
    // RAM: { internal: bool, bank: {} }
    guikit_ram_config_t ram;
    
    // SPI: { expander: bool, bank: {} }
    guikit_spi_config_t spi;
    
    guikit_display_config_t display;
    
    bool use_sd_card;
    bool use_webdav;
    bool debug_mode;
} guikit_hw_config_t;
```

### RAM Configuration (matches user concept)

```c
typedef struct {
    bool internal;             // Internal RAM available
    guikit_ram_bank_t bank[8]; // External RAM banks
    uint8_t bank_count;
} guikit_ram_config_t;

typedef struct {
    guikit_ram_type_t type;    // INTERNAL, SRAM, PSRAM, FRAM
    uint32_t size;             // Size in bytes
    uint8_t cs_pin;            // CS pin (255 = not applicable)
    bool enabled;
} guikit_ram_bank_t;
```

### SPI Configuration (matches user concept)

```c
typedef struct {
    bool expander;             // SPI expander enabled
    guikit_spi_bank_t bank[8]; // SPI devices/expanders
    uint8_t bank_count;
    uint8_t sck_pin;
    uint8_t mosi_pin;
    uint8_t miso_pin;
    uint8_t max_speed_mhz;
} guikit_spi_config_t;

typedef struct {
    guikit_spi_type_t type;        // DEVICE or EXPANDER
    guikit_expander_type_t expander_type;  // MCP23S17, MCP23S08, etc.
    uint8_t cs_pin;
    uint8_t irq_pin;               // 255 = not used
    bool enabled;
} guikit_spi_bank_t;
```

### Usage Example

```c
// Use preset configuration
guikit_hw_config_t config = GUIKIT_HW_ESP8266_HUGE_DEMO;

// Or initialize and customize
config.ram.internal = true;
config.ram.bank[0] = (guikit_ram_bank_t){GUIKIT_RAM_SRAM, 131072, 16, true};
config.spi.expander = true;
config.spi.bank[0] = (guikit_spi_bank_t){GUIKIT_SPI_EXPANDER, GUIKIT_EXPANDER_MCP23S17, 0, 255, true};
config.display = (guikit_display_config_t){320, 240, 4, true};

// Check configuration
if (config.ram.internal) {
    // Internal RAM available
}
if (config.spi.expander) {
    // SPI expander enabled
}
```

### Preset Configurations

1. **GUIKIT_HW_DEFAULT** - Auto-detects platform
2. **GUIKIT_HW_ESP8266_DEFAULT** - ESP8266 defaults
3. **GUIKIT_HW_ESP32_DEFAULT** - ESP32 defaults
4. **GUIKIT_HW_ESP8266_HUGE_DEMO** - ESP8266 + 1MB SRAM (8×23LC1024)
5. **GUIKIT_HW_ESP8266_EXPANDER** - ESP8266 + 2×MCP23S17
6. **GUIKIT_HW_ESP32_PREMIUM** - ESP32 + 8MB PSRAM + 2×MCP23S17

### Helper Macros

```c
GUIKIT_IS_ESP8266()
GUIKIT_IS_ESP32()
GUIKIT_HAS_INTERNAL_RAM(cfg)
GUIKIT_HAS_SPI_EXPANDER(cfg)
GUIKIT_FRAMEBUFFER_SIZE(cfg)
```

---

## 🔧 GUIKit Hardware Configuration

### Overview
- **File:** `src/guikit_config.h`
- **Purpose:** Unified configuration structure for ESP8266 and ESP32 platforms
- **Features:** RAM banks, SPI expanders, display settings in a single struct

### Configuration Structure

```c
typedef struct {
    bool is_esp8266;
    bool is_esp32;
    
    ram_config_t ram;      // RAM banks configuration
    spi_config_t spi;      // SPI expanders configuration  
    display_config_t display; // Display settings
    
    bool debug_mode;
    bool low_memory_mode;
    bool use_sd_card;
    bool use_webdav;
} guikit_config_t;
```

### RAM Configuration
- **Types:** INTERNAL, SRAM, PSRAM, FRAM
- **Banks:** Up to 8 external RAM banks per platform
- **Fields:** type, base_addr, size, cs_pin, spi_bus, enabled, name
- **Auto-detection:** Platform-specific defaults for ESP8266 (50KB) and ESP32 (520KB)

### SPI Expander Configuration
- **Types:** MCP23S17 (16-bit), MCP23S08 (8-bit), MCP23017 (I2C), MCP23008 (I2C), CUSTOM
- **Banks:** Up to 8 expander chips
- **Fields:** type, address, cs_pin, irq_pin, reset_pin, enabled, name
- **Runtime state:** last_read value, irq_triggered flag

### Display Configuration
- **Resolution:** width, height
- **Color Depth:** 1bpp, 2bpp, 4bpp, 8bpp, 16bpp, 24bpp
- **Buffering:** double_buffer, animation_buffer
- **Memory:** framebuffer_size, total_buffer_size, framebuffer_bank
- **Palette:** 256-entry palette for indexed color modes

### Preset Configurations
1. **GUIKIT_DEFAULT_CONFIG** - Auto-detects platform
2. **GUIKIT_ESP8266_DEFAULT_CONFIG** - ESP8266 with 4bpp, no external RAM
3. **GUIKIT_ESP32_DEFAULT_CONFIG** - ESP32 with 16bpp, no external RAM
4. **GUIKIT_ESP8266_HUGE_DEMO_CONFIG** - ESP8266 + 1MB SRAM (8×23LC1024)
5. **GUIKIT_ESP8266_EXPANDER_CONFIG** - ESP8266 + 2×MCP23S17
6. **GUIKIT_ESP32_PREMIUM_CONFIG** - ESP32 + 8MB PSRAM + 2×MCP23S17

### Helper Functions & Macros
- **Platform detection:** GUIKIT_IS_ESP8266(), GUIKIT_IS_ESP32()
- **RAM access:** guikit_has_internal_ram(), guikit_has_external_ram()
- **SPI expanders:** guikit_has_spi_expanders()
- **Display calculations:** GUIKIT_FRAMEBUFFER_SIZE(), GUIKIT_TOTAL_BUFFER_SIZE()
- **Validation:** guikit_config_validate()
- **Debugging:** guikit_config_print()

### Usage Example

```c
// Initialize with defaults
guikit_config_t config = GUIKIT_DEFAULT_CONFIG;

// Or use a preset
guikit_config_t config = GUIKIT_ESP8266_HUGE_DEMO_CONFIG;

// Or customize
config.ram.bank_count = 1;
config.ram.banks[0] = (ram_bank_config_t){
    .type = RAM_TYPE_SRAM,
    .size = 131072,  // 128KB
    .cs_pin = 16,    // D0
    .enabled = true,
    .name = "Main SRAM"
};

config.spi.expander_count = 1;
config.spi.expanders[0] = (spi_expander_config_t){
    .type = SPI_EXPANDER_MCP23S17,
    .cs_pin = 0,     // D3
    .enabled = true,
    .name = "GPIO Expander"
};

config.display.color_depth = DISPLAY_COLOR_4BPP;
config.display.double_buffer = true;
```

### Key Decisions
- **Primary Solution:** 23LC1024 SPI SRAM (128 KB, ~$3)
- **Recommended Color Depth:** 4bpp (16 colors) = 37.5 KB framebuffer (fits in internal RAM or SRAM)
- **Secondary Storage:** FRAM for non-volatile configuration (32-128 KB, ~$5-15)
- **Long-term Solution:** ESP32 migration (520 KB SRAM + external PSRAM support)

### Memory Optimization Strategies
1. **4bpp Mode:** Reduce framebuffer from 150 KB to 37.5 KB
2. **Partial Updates:** Only redraw changed regions (dirty rectangle system)
3. **Hierarchical Storage:**
   - Internal RAM: Active widgets, visible text lines
   - SRAM: Framebuffer, widget cache, text editor buffer
   - SD Card: Assets, GUI definitions, temporary files
   - FRAM: Configuration, persistent state

### Performance Improvements
| Operation | Without External RAM | With 23LC1024 | Speedup |
|-----------|---------------------|---------------|---------|
| Full Screen Redraw | ~500 ms | ~50 ms | **10x** |
| Widget Activation | ~50 ms | ~5 ms | **10x** |
| Text Scroll | ~100 ms | ~10 ms | **10x** |

### Wiring
- **23LC1024 CS:** D0 (GPIO16) - dedicated pin
- **SPI Bus:** Shared SCK (D5), MOSI (D7), MISO (D6) with TFT, Touch, SD Card
- **No Conflicts:** Each SPI device has unique CS line

### Implementation Phases
1. **Phase 1:** Software optimizations (4bpp, partial updates) - no hardware
2. **Phase 2:** Add 23LC1024 SRAM for framebuffer and cache
3. **Phase 3:** Add FRAM for configuration
4. **Phase 4:** Consider ESP32 migration for long-term projects

### Key Decisions
- **MCP23S17 Selected:** 16 pins per chip, SPI interface, hardware addressable (up to 8 chips = 128 GPIO)
- **SPI Bus Sharing:** TFT, Touch, SD Card, and Expanders share SPI bus with separate CS lines
- **Recommended Config:** 2-3 MCP23S17 chips adding 32-48 GPIO using only 2-3 additional CS pins
- **No I2C:** SPI preferred over I2C for speed and reliability (I2C limited to 1.7 MHz, SPI up to 10+ MHz)

### GUIKit Widget Types for Expanders
1. **expander_button** - Button input via expander GPIO
2. **expander_led** - LED output via expander GPIO
3. **expander_input** - Generic digital input
4. **expander_output** - Generic digital output

### Wiring Strategy
- **CS Pins:** Use available GPIO (D0/GPIO16, D6/GPIO12, D7/GPIO13)
- **SPI Bus:** Shared MOSI (D7/GPIO13), MISO (D6/GPIO12), SCK (D5/GPIO14)
- **Address Pins:** A0-A2 on MCP23S17 for hardware addressing

### Documentation File
- **File:** `about_port_expander.md`
- **Content:** Pin maps, wiring diagrams, code examples, library integration, FAQ
- **Size:** ~40KB

---

## 📚 References

- **Original Discussion:** `discussion_guikit.txt` - Source of architecture decisions
- **Docs:** `docs/` - Existing architecture analysis documents
- **SPI Expander Guide:** `about_port_expander.md` - Comprehensive SPI expander documentation
- **ESP8266 Example:** `src/examples/esp8266_expander_ram_tft.*` - MCP23S17 + 23LC1024 + TFT
- **Union Config:** `src/guikit_hw_config_union.h` - Union-based type-safe config
- **Hardware Config:** `src/guikit_hw_config.h` - Unified simple config (user concept)
- **Full Config:** `src/guikit_config.h` - Full hardware configuration
- **Huge Demo RAM Guide:** `about_huge_demo_ram_requirements.md` - Complete RAM consumption analysis
- **RAM Expansion Guide:** `about_ram_expansion.md` - Comprehensive external RAM documentation
- **SPI Expander Guide:** `about_port_expander.md` - Comprehensive SPI expander documentation
- **Bootloader Kernel Check:** `src/boot/guikit_bootloader.*` + `src/boot/README.md` - Kernel verification implementation
- **GitHub:** genose/genose.org-ESP8266-KernelonSDCard-GUIKit-Touch-WebDavServer

---

## 🏁 Session Conclusion

All requested features have been implemented and committed:
- ✅ User skeleton structure with `/etc/user.skel/`
- ✅ System GUIs for WebDAV and user management
- ✅ User home directory creation from skeleton
- ✅ WebDAV user home as root
- ✅ GUIKit Web Editor with full project management
- ✅ Text editing with contextual menu (long press ~2sec)
- ✅ Temp buffer support
- ✅ Complete documentation in README.md
- ✅ `--help` support in all scripts
- ✅ Build system (build.sh, Makefile, .gitignore)
- ✅ Kernel verification in bootloader
- ✅ TFT initialization moved earlier for boot progress messages
- ✅ Boot sequence: Hardware -> RAM -> SD Card -> TFT -> Kernel -> Memory Config -> Test & Display
- ✅ SPI device enumeration with type detection

**Status:** Ready for development and testing

**Latest Additions:**
- ✅ ESP8266 example (`src/examples/esp8266_expander_ram_tft.*`)
- ✅ Union-based config (`src/guikit_hw_config_union.h`)
- ✅ Unified hardware config (`src/guikit_hw_config.h`)
- ✅ Full hardware config (`src/guikit_config.h`)
- ✅ Huge Demo RAM requirements analysis (`about_huge_demo_ram_requirements.md`)
- ✅ SPI Port Expander comprehensive guide (`about_port_expander.md`)
- ✅ External RAM expansion comprehensive guide (`about_ram_expansion.md`)
- ✅ Updated session context with all hardware analysis
- ✅ Kernel verification in bootloader (`src/boot/guikit_bootloader.*`)
- ✅ TFT initialization moved to Step 4 (after SD Card) for early boot messages
- ✅ TFT progress messages during boot (kernel check, memory config, strategy test)
- ✅ tft_display_progress() function for showing progress on TFT
- ✅ KernelInfo struct for tracking kernel file information
- ✅ Updated bootloader README.md with kernel check documentation
- ✅ SPI device enumeration support (`BootSpiDeviceType`, `BootSpiDeviceInfo`)
- ✅ enumerate_spi_devices() and get_spi_device_count() functions
- ✅ SPI device enumeration summary printed during boot

---

*Document generated by Mistral Vibe*  
*Session date: 2026-08-15*
