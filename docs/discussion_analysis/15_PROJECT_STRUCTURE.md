# Project Structure

> Extracted from discussion_guikit.txt - Complete file organization, PlatformIO configuration, and build settings

## Overview

This document covers the complete project structure for GUIKit, including:
- File organization and directory structure
- PlatformIO configuration for ESP8266
- Library dependencies
- Build configurations
- Memory settings
- Hardware pin assignments

All implementations follow Objective-C style memory management (NOT ARC/reference counting) for ESP8266 compatibility.

---

## Complete Project Structure

```
GUIKit/
├── docs/                              # Documentation
│   ├── ARCHITECTURE.md               # Architecture overview
│   ├── HARDWARE.md                   # Hardware configuration
│   ├── SOFTWARE.md                   # Software components
│   ├── NETWORK.md                    # Network configuration
│   ├── DATA_FLOW.md                  # Data flow diagrams
│   ├── MEMORY_MANAGEMENT.md          # Memory management concepts
│   └── discussion_analysis/          # Extracted concepts from discussion
│       ├── INDEX.md                  # Index of all extracted concepts
│       ├── 01_WIDGET_ARCHITECTURE.md  # Base widget structures
│       ├── 02_CONSTRUCTOR_PATTERNS.md # Constructor implementations
│       ├── 03_WIDGET_TYPES.md         # Widget type definitions
│       ├── 04_DRAW_STYLES.md          # Draw style system
│       ├── 05_STYLE_SYSTEM.md         # Complete style system
│       ├── 06_MEMORY_MANAGEMENT.md    # Memory management
│       ├── 07_RENDERING_SYSTEM.md     # TFT_eSPI rendering
│       ├── 08_TOUCH_HANDLING.md       # XPT2046 touch handling
│       ├── 09_WIDGET_IMPLEMENTATIONS.md # Widget implementations
│       ├── 10_TEXT_AND_INPUT.md       # Text field and keyboard
│       ├── 11_SD_CARD_WEBDAV.md      # SD card and WebDAV
│       ├── 12_OPTIMIZATIONS.md        # ESP8266 optimizations
│       ├── 13_UI_PARSER.md            # JSON UI parser
│       ├── 14_ADVANCED_FEATURES.md    # Clipboard, gestures, history
│       └── 15_PROJECT_STRUCTURE.md    # This file
│
├── src/
│   └── gui/                           # GUI source code
│       ├── widget.h                   # Widget definitions and types
│       ├── widget.cpp                 # Widget implementations
│       ├── widget_macros.h            # Constructor and utility macros
│       ├── widget_text.h              # Text buffer management
│       ├── widget_pool.h              # Object pool declarations
│       ├── widget_pool.c              # Object pool implementation
│       ├── style.h                    # Draw style definitions
│       ├── style.cpp                  # Style utility functions
│       ├── renderer.h                 # Renderer declarations
│       ├── renderer.cpp               # Renderer implementations
│       ├── renderer_optimized.h       # Optimized renderer with clipping
│       ├── renderer_optimized.cpp     # Optimized renderer implementation
│       ├── touch.h                    # Touch handling declarations
│       ├── touch.cpp                  # Touch handling implementations
│       ├── textfield.h                # Text field widget
│       ├── textfield.cpp              # Text field implementation
│       ├── keyboard.h                 # Virtual keyboard
│       ├── keyboard.cpp               # Virtual keyboard implementation
│       ├── corrector.h                # Advanced text corrector
│       ├── corrector.cpp              # Text corrector implementation
│       ├── clipboard.h                # Clipboard system
│       ├── clipboard.cpp              # Clipboard implementation
│       ├── gestures.h                 # Gesture recognition
│       ├── gestures.cpp               # Gesture implementation
│       ├── history.h                  # Undo/redo history
│       ├── history.cpp                # History implementation
│       ├── command.h                  # Command pattern
│       ├── command.cpp                # Command implementation
│       ├── scope_guard.h              # RAII-style scope guards
│       ├── sd_card.h                  # SD card management
│       ├── sd_card.cpp                # SD card implementation
│       ├── webdav.h                   # WebDAV server
│       ├── webdav.cpp                 # WebDAV implementation
│       ├── file_manager.h             # File management
│       ├── file_manager.cpp           # File manager implementation
│       ├── ui_parser.h                # UI parser
│       ├── ui_parser.cpp              # UI parser implementation
│       └── main.cpp                   # Main application
│
├── lib/                              # External libraries
│   └── TFT_eSPI/                      # TFT display library
│       ├── TFT_eSPI.h                # Main header
│       ├── TFT_eSPI.cpp              # Implementation
│       └── User_Setup.h              # User-specific configuration
│
├── data/                             # Data files (on SD card)
│   ├── system/                       # System files
│   │   ├── ui/                       # UI definitions (JSON)
│   │   │   ├── main_ui.json          # Main interface
│   │   │   ├── settings_ui.json      # Settings interface
│   │   │   └── login_ui.json         # Login interface
│   │   ├── dict/                     # Dictionaries
│   │   │   ├── fr.txt               # French dictionary
│   │   │   ├── en.txt               # English dictionary
│   │   │   └── custom.txt           # Custom dictionary
│   │   ├── config/                  # Configurations
│   │   │   ├── passwords.txt        # Passwords (hashed)
│   │   │   ├── styles.json          # Custom styles
│   │   │   └── settings.json        # General settings
│   │   └── logs/                    # Logs
│   │       └── system.log           # System log
│   └── user/                        # User files
│       ├── notes.txt                # User notes
│       └── ...
│
├── include/                          # Additional include files
│   └── User_Config.h                 # User configuration
│
├── platformio.ini                    # PlatformIO configuration
├── README.md                         # Project README
└── .gitignore                        # Git ignore file
```

---

## PlatformIO Configuration

### Complete platformio.ini

```ini
; PlatformIO Project Configuration File
; For ESP8266 with TFT display, XPT2046 touchscreen, SD card, and WebDAV

[env:esp8266_tft_sd_webdav]
platform = espressif8266
board = nodemcuv2  ; Adjust according to your board
framework = arduino

; ========== LIBRARY DEPENDENCIES ==========
lib_deps =
    ; TFT Display Library
    https://github.com/Bodmer/TFT_eSPI.git
    
    ; Touchscreen Library
    https://github.com/PaulStoffregen/XPT2046_Touchscreen.git
    
    ; SD Card Library (optimized for ESP8266)
    https://github.com/greiman/SdFat.git
    
    ; WebDAV Server Library
    https://github.com/hoonie/ESPWebDAV.git
    
    ; JSON Library (for UI parser)
    https://github.com/bblanchon/ArduinoJson.git@6.x
    
    ; SHA1 Library (for password hashing)
    https://github.com/Chris--A/SHA1.git

; ========== BUILD FLAGS ==========
build_flags =
    ; TFT_eSPI Configuration
    -D USER_SETUP_LOADED
    -D ST7789_DRIVER        ; Adjust for your TFT controller
    -D TFT_WIDTH=240        ; Adjust for your display
    -D TFT_HEIGHT=320       ; Adjust for your display
    -D TFT_CS=D8            ; Chip Select pin
    -D TFT_DC=D3            ; Data/Command pin
    -D TFT_RST=D4           ; Reset pin
    -D TFT_BL=D0            ; Backlight pin (optional)
    -D TFT_BACKLIGHT_ON=HIGH
    
    ; XPT2046 Touchscreen Configuration
    -D TOUCH_CS=D2          ; Touchscreen Chip Select
    -D XPT2046_IRQ=D1      ; Touchscreen interrupt pin
    
    ; SD Card Configuration
    -D SD_CS=D5            ; SD card Chip Select
    -D SD_FAT_TYPE=1       ; 1 = FAT16/FAT32, 2 = exFAT
    
    ; WebDAV Configuration
    -D WEBDAV_USERNAME="admin"
    -D WEBDAV_PASSWORD="esp8266"
    -D WEBDAV_PORT=80
    
    ; Memory Optimization Flags
    -D F_CPU=80000000L     ; CPU frequency (80 MHz)
    -D ESP8266             ; ESP8266 platform
    -D ARDUINO_ARCH_ESP8266
    -D ARDUINO_ESP8266_NODEMCU
    
    ; Debug Flags
    ; -D DEBUG             ; Uncomment for debug mode
    ; -D SERIAL_DEBUG      ; Uncomment for serial debug

; ========== MONITOR CONFIGURATION ==========
monitor_speed = 115200
monitor_port = /dev/cu.usbserial-XXXX  ; Adjust for your system
monitor_dtr = 0
monitor_rts = 0

; ========== UPLOAD CONFIGURATION ==========
upload_speed = 115200
upload_port = /dev/cu.usbserial-XXXX  ; Adjust for your system
upload_reset_method = nodemcu

; ========== DEBUG CONFIGURATION ==========
debug_tool = serial, esp8266-exception-decoder
debug_port = /dev/cu.usbserial-XXXX

; ========== ADVANCED CONFIGURATION ==========
; Enable OTA updates
; upload_protocol = espota
; upload_host = 192.168.1.100
; upload_port = 8266

; Enable, RAM/Flash usage reporting
; build_src_flags = -Wl,-Map=output.map
```

### Board-Specific Configurations

#### NodeMCU v2

```ini
[env:nodemcuv2_tft]
board = nodemcuv2
build_flags =
    -D ST7789_DRIVER
    -D TFT_WIDTH=240
    -D TFT_HEIGHT=320
    -D TFT_CS=D8
    -D TFT_DC=D3
    -D TFT_RST=D4
    -D TOUCH_CS=D2
    -D XPT2046_IRQ=D1
    -D SD_CS=D5
```

#### ESP-01S

```ini
[env:esp01_1m_tft]
board = esp01_1m
build_flags =
    -D ST7789_DRIVER
    -D TFT_WIDTH=240
    -D TFT_HEIGHT=320
    -D TFT_CS=0  ; GPIO0
    -D TFT_DC=2  ; GPIO2
    -D TFT_RST=16 ; GPIO16
    -D TOUCH_CS=4 ; GPIO4
    -D XPT2046_IRQ=5 ; GPIO5
    -D SD_CS=13 ; GPIO13 (D7)
```

#### Wemos D1 Mini

```ini
[env:wemos_d1_mini_tft]
board = d1_mini
build_flags =
    -D ST7789_DRIVER
    -D TFT_WIDTH=240
    -D TFT_HEIGHT=320
    -D TFT_CS=D8
    -D TFT_DC=D3
    -D TFT_RST=D4
    -D TOUCH_CS=D2
    -D XPT2046_IRQ=D1
    -D SD_CS=D5
```

---

## TFT_eSPI User Setup

### User_Setup.h Configuration

```cpp
// User_Setup.h - Configuration for TFT_eSPI

// Select the driver (uncomment one)
#define ST7789_DRIVER    // For ST7789 display controller
// #define ILI9341_DRIVER  // For ILI9341 display controller
// #define S6D02A1_DRIVER  // For S6D02A1 display controller

// Display dimensions
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// Display rotation (0-3)
#define TFT_ROTATION 1  // Landscape mode

// Pin definitions (adjust according to your wiring)
#define TFT_CS   D8     // Chip Select
#define TFT_DC   D3     // Data/Command
#define TFT_RST  D4     // Reset
#define TFT_BL   D0     // Backlight (optional)

// Backlight control
#define TFT_BACKLIGHT_ON HIGH

// SPI configuration
#define TFT_SPI_PORT SPI
#define SPI_FREQUENCY  40000000  // 40 MHz (max for ESP8266)

// Color depth
#define COLOR_DEPTH 16  // 16-bit color (RGB565)

// Enable/disable features
#define LOAD_GLCD  1  // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  1  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, provides custom characters
#define LOAD_FONT4  1  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH
#define LOAD_FONT6  1  // Font 6. Large 48 pixel font, needs ~17728 bytes in FLASH
#define LOAD_FONT7  1  // Font 7. 7 segment 48 pixel font, needs ~1628 bytes in FLASH
#define LOAD_FONT8  1  // Font 8. Large 75 pixel font Needs ~4280 bytes in FLASH
#define LOAD_GFXFF  1  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT 1  // Smooth all fonts (adds ~200 bytes)

// Touchscreen calibration (adjust after calibration)
#define TOUCH_CALIBRATE 1
#define CALIBRATION_FILE "/calibration.dat"
```

---

## XPT2046 Touchscreen Configuration

### Touchscreen Setup

```cpp
// Touchscreen configuration
#define TOUCH_CS D2        // Chip Select pin
#define XPT2046_IRQ D1     // Interrupt pin (optional)

// Touchscreen orientation and calibration
#define TOUCH_ORIENTATION PORTRAIT

// Calibration values (set after running calibration)
#define TS_MIN_X 100
#define TS_MIN_Y 100
#define TS_MAX_X 3800
#define TS_MAX_Y 3800

// Pressure threshold (to detect touch)
#define TOUCH_PRESSURE_THRESHOLD 100

// SPI settings for touchscreen
#define TOUCH_SPI_PORT SPI
#define TOUCH_SPI_FREQUENCY 2000000  // 2 MHz (slower for touchscreen)
```

---

## Hardware Pin Assignments

### Recommended Pin Assignments

| Function | Pin | Notes |
|----------|-----|-------|
| TFT_CS | D8 | TFT Chip Select |
| TFT_DC | D3 | TFT Data/Command |
| TFT_RST | D4 | TFT Reset |
| TFT_BL | D0 | TFT Backlight (optional) |
| TOUCH_CS | D2 | Touchscreen Chip Select |
| TOUCH_IRQ | D1 | Touchscreen Interrupt (optional) |
| SD_CS | D5 | SD Card Chip Select |
| SPI_MOSI | D7 | SPI MOSI (shared) |
| SPI_MISO | D6 | SPI MISO (shared) |
| SPI_SCK | D5 | SPI SCK (shared) |

### Pin Conflicts to Avoid

1. **SPI Bus**: TFT, Touchscreen, and SD card all use SPI bus
   - Ensure only one device is active at a time (CS pins must be different)
   - Use high-speed SPI for TFT (40 MHz)
   - Use lower-speed SPI for SD card (20 MHz)
   - Use lowest-speed SPI for touchscreen (2 MHz)

2. **I2C Pins**: D1 (SDA) and D2 (SCL) are used by default for I2C
   - If using I2C devices, avoid using these pins for other purposes
   - Touchscreen interrupt can use D1 if I2C is not needed

3. **Serial Pins**: D0 (RX) and D1 (TX) are used for serial communication
   - Avoid using these pins for other purposes during development
   - Can be used for GPIO after development is complete

4. **Boot Mode Pins**: D3, D4, D8 must be high at boot for normal operation
   - Don't pull these pins low at startup

### SPI Bus Management

```c
// CS pins
#define TFT_CS D8
#define TOUCH_CS D2
#define SD_CS D5

// Track current SPI device
typedef enum {
    SPI_DEVICE_NONE,
    SPI_DEVICE_TFT,
    SPI_DEVICE_TOUCH,
    SPI_DEVICE_SD
} SPIDevice;

SPIDevice current_spi_device = SPI_DEVICE_NONE;

// Select SPI device
void select_spi_device(SPIDevice device) {
    if (current_spi_device == device) return;
    
    // Deselect current device
    switch (current_spi_device) {
        case SPI_DEVICE_TFT:
            digitalWrite(TFT_CS, HIGH);
            break;
        case SPI_DEVICE_TOUCH:
            digitalWrite(TOUCH_CS, HIGH);
            break;
        case SPI_DEVICE_SD:
            digitalWrite(SD_CS, HIGH);
            break;
    }
    
    // Select new device
    switch (device) {
        case SPI_DEVICE_TFT:
            digitalWrite(TFT_CS, LOW);
            SPI.setFrequency(40000000);  // 40 MHz for TFT
            break;
        case SPI_DEVICE_TOUCH:
            digitalWrite(TOUCH_CS, LOW);
            SPI.setFrequency(2000000);   // 2 MHz for touchscreen
            break;
        case SPI_DEVICE_SD:
            digitalWrite(SD_CS, LOW);
            SPI.setFrequency(20000000);  // 20 MHz for SD card
            break;
    }
    
    current_spi_device = device;
}

// Macro for convenience
#define SELECT_TFT() select_spi_device(SPI_DEVICE_TFT)
#define SELECT_TOUCH() select_spi_device(SPI_DEVICE_TOUCH)
#define SELECT_SD() select_spi_device(SPI_DEVICE_SD)
#define DESELECT_ALL() select_spi_device(SPI_DEVICE_NONE)
```

---

## Build Configuration

### Memory Settings

```ini
; Memory configuration for ESP8266
[env]
build_flags =
    ; Stack size (default: 4096 bytes)
    -D CONT_STACK_SIZE=8192
    
    ; Heap size adjustments
    ; Total RAM: 80KB (approximately)
    ; Stack: 4KB-8KB (configurable)
    ; Heap: Remaining RAM
    
    ; Optimize for size
    -Os  ; Optimize for size (recommended for ESP8266)
    ; -O2  ; Optimize for speed (use if you have RAM to spare)
    
    ; Linker flags
    -Wl,-Tesp8266.flash.4m1m.ld  ; 4MB Flash, 1MB SPIFFS
    ; -Wl,-Tesp8266.flash.4m.ld   ; 4MB Flash, no SPIFFS
    ; -Wl,-Tesp8266.flash.2m.ld   ; 2MB Flash
```

### Flash Layout Options

| Option | Description | Flash for Code | Flash for SPIFFS |
|--------|-------------|----------------|-------------------|
| 4m1m | 4MB Flash, 1MB SPIFFS | 3MB | 1MB |
| 4m | 4MB Flash, no SPIFFS | 4MB | 0MB |
| 2m | 2MB Flash | 2MB | 0MB |
| 1m256 | 1MB Flash, 256KB SPIFFS | 640KB | 256KB |

### Recommended Settings for GUIKit

```ini
; For 4MB ESP8266 (NodeMCU, Wemos D1 Mini)
build_flags =
    -D CONT_STACK_SIZE=8192
    -Os
    -Wl,-Tesp8266.flash.4m1m.ld

; For 1MB ESP8266 (ESP-01)
build_flags =
    -D CONT_STACK_SIZE=4096
    -Os
    -Wl,-Tesp8266.flash.1m256.ld
```

---

## Main Application (main.cpp)

### Complete Application Structure

```cpp
#include <Arduino.h>
#include <SPI.h>

// GUIKit headers
#include "gui/widget.h"
#include "gui/widget_macros.h"
#include "gui/style.h"
#include "gui/renderer.h"
#include "gui/touch.h"
#include "gui/textfield.h"
#include "gui/keyboard.h"
#include "gui/clipboard.h"
#include "gui/gestures.h"
#include "gui/history.h"
#include "gui/sd_card.h"
#include "gui/webdav.h"
#include "gui/file_manager.h"
#include "gui/ui_parser.h"

// WiFi credentials
const char* WIFI_SSID = "Your_SSID";
const char* WIFI_PASSWORD = "Your_Password";

// Global variables
Widget* root_view;
WidgetTextField* textfield;
WidgetKeyboard* keyboard;

// UI State
Widget* current_ui = NULL;

// ========== CALLBACKS ==========
void on_textfield_change(const char* text) {
    Serial.print("TextField: ");
    Serial.println(text);
    history_save_state(textfield);
}

void on_textfield_enter(const char* text) {
    Serial.print("Text entered: ");
    Serial.println(text);
    keyboard_hide(keyboard);
}

void on_save_click() {
    if (!textfield) return;
    if (textfield_save_to_file(textfield, "/notes.txt")) {
        Serial.println("File saved successfully!");
    } else {
        Serial.println("Error: Cannot save file.");
    }
}

void on_load_click() {
    if (!textfield) return;
    if (textfield_load_from_file(textfield, "/notes.txt")) {
        Serial.println("File loaded successfully!");
        draw_widget((Widget*)textfield);
    } else {
        Serial.println("Error: Cannot load file.");
    }
}

void on_share_click() {
    if (webdav_share_file("/notes.txt", "notes.txt")) {
        Serial.println("File shared via WebDAV!");
        Serial.print("Access at http://");
        Serial.print(WiFi.localIP());
        Serial.println("/webdav to download.");
    } else {
        Serial.println("Error: Cannot share file.");
    }
}

void on_undo_click() {
    textfield_undo(textfield);
    draw_widget((Widget*)textfield);
}

void on_redo_click() {
    textfield_redo(textfield);
    draw_widget((Widget*)textfield);
}

// ========== GESTURE HANDLING ==========
void handle_gestures(GestureType gesture, uint16_t x, uint16_t y) {
    switch (gesture) {
        case GESTURE_TAP:
            // Handle tap
            break;
            
        case GESTURE_DOUBLE_TAP:
            if (textfield) {
                textfield_select_all(textfield);
                draw_widget((Widget*)textfield);
            }
            break;
            
        case GESTURE_LONG_PRESS:
            if (textfield && textfield->selection.active) {
                textfield_copy(textfield);
            }
            break;
            
        case GESTURE_SWIPE_LEFT:
        case GESTURE_SWIPE_RIGHT:
            // Switch UIs
            break;
    }
}

// ========== SETUP ==========
void setup() {
    Serial.begin(115200);
    delay(100);
    
    Serial.println("\n=== GUIKit Initialization ===");
    
    // Initialize hardware
    Serial.println("Initializing TFT...");
    init_renderer();
    
    Serial.println("Initializing Touch...");
    init_touch();
    
    Serial.println("Initializing SD Card...");
    if (!init_sd_card()) {
        Serial.println("Warning: SD card not detected!");
    } else {
        Serial.println("SD card initialized.");
        
        // Initialize file manager
        init_file_manager();
        
        // Create system folders
        create_system_folders();
    }
    
    // Initialize WiFi and WebDAV
    Serial.println("Initializing WiFi...");
    init_webdav(WIFI_SSID, WIFI_PASSWORD);
    start_webdav();
    
    // Initialize systems
    Serial.println("Initializing Clipboard...");
    init_clipboard();
    
    Serial.println("Initializing Gestures...");
    init_gestures();
    set_gesture_callback(handle_gestures);
    
    // Create root view
    Serial.println("Creating UI...");
    root_view = new_widget(WIDGET_TYPE_VIEW);
    root_view->rect.position.x = 0;
    root_view->rect.position.y = 0;
    root_view->rect.size.width = tft.width();
    root_view->rect.size.height = tft.height();
    root_view->style.draw_style = WIDGET_DRAW_STYLE_SOLID_FILL;
    root_view->style.colors.primary = 0xFFFF; // White background
    
    // Create text field
    textfield = new_textfield(512, TEXTFIELD_STYLE_NORMAL);
    textfield->base.rect.position.x = 20;
    textfield->base.rect.position.y = 20;
    textfield->base.rect.size.width = 280;
    textfield->base.rect.size.height = 100;
    textfield->on_change = on_textfield_change;
    textfield->on_enter = on_textfield_enter;
    
    // Initialize history for text field
    init_history(textfield);
    
    // Load from SD if exists
    if (sd_file_exists("/notes.txt")) {
        textfield_load_from_file(textfield, "/notes.txt");
    } else {
        strncpy(textfield->buffer, "Hello!\nType your text here...", textfield->buffer_size);
        textfield->cursor_pos = strlen(textfield->buffer);
    }
    
    widget_add_child(root_view, &textfield->base);
    
    // Create keyboard
    keyboard = new_keyboard(textfield);
    keyboard->visible = false;
    widget_add_child(root_view, &keyboard->base);
    
    // Create buttons
    WidgetButton* save_btn = new_button();
    save_btn->base.rect.position.x = 20;
    save_btn->base.rect.position.y = 130;
    save_btn->base.rect.size.width = 60;
    save_btn->base.rect.size.height = 30;
    strncpy(save_btn->base.text.text, "Save", MAX_TEXT_LENGTH - 1);
    save_btn->on_click = on_save_click;
    widget_add_child(root_view, &save_btn->base);
    
    WidgetButton* load_btn = new_button();
    load_btn->base.rect.position.x = 90;
    load_btn->base.rect.position.y = 130;
    load_btn->base.rect.size.width = 60;
    load_btn->base.rect.size.height = 30;
    strncpy(load_btn->base.text.text, "Load", MAX_TEXT_LENGTH - 1);
    load_btn->on_click = on_load_click;
    widget_add_child(root_view, &load_btn->base);
    
    WidgetButton* share_btn = new_button();
    share_btn->base.rect.position.x = 160;
    share_btn->base.rect.position.y = 130;
    share_btn->base.rect.size.width = 60;
    share_btn->base.rect.size.height = 30;
    strncpy(share_btn->base.text.text, "Share", MAX_TEXT_LENGTH - 1);
    share_btn->on_click = on_share_click;
    widget_add_child(root_view, &share_btn->base);
    
    WidgetButton* undo_btn = new_button();
    undo_btn->base.rect.position.x = 230;
    undo_btn->base.rect.position.y = 130;
    undo_btn->base.rect.size.width = 60;
    undo_btn->base.rect.size.height = 30;
    strncpy(undo_btn->base.text.text, "Undo", MAX_TEXT_LENGTH - 1);
    undo_btn->on_click = on_undo_click;
    widget_add_child(root_view, &undo_btn->base);
    
    WidgetButton* redo_btn = new_button();
    redo_btn->base.rect.position.x = 20;
    redo_btn->base.rect.position.y = 170;
    redo_btn->base.rect.size.width = 60;
    redo_btn->base.rect.size.height = 30;
    strncpy(redo_btn->base.text.text, "Redo", MAX_TEXT_LENGTH - 1);
    redo_btn->on_click = on_redo_click;
    widget_add_child(root_view, &redo_btn->base);
    
    // Save initial state
    history_save_state(textfield);
    
    Serial.println("\n=== Setup Complete ===");
    Serial.print("WiFi IP: ");
    Serial.println(WiFi.localIP());
    Serial.println("Access WebDAV at: http://" + WiFi.localIP().toString() + "/webdav");
    
    // Draw initial UI
    draw_widget_tree(root_view);
}

// ========== LOOP ==========
void loop() {
    // Handle WebDAV clients
    webdav_handle_client();
    
    // Handle touch
    TouchPoint touch = get_touch_point();
    if (touch.pressed) {
        handle_gesture_touch(touch.x, touch.y, touch.pressed);
        handle_touch(root_view, touch.x, touch.y);
    }
    
    // Handle keyboard
    handle_keyboard(keyboard);
    
    // Render if needed
    static uint32_t last_render = 0;
    if (millis() - last_render > 16) {  // ~60 FPS
        draw_widget_tree_optimized(root_view);
        last_render = millis();
    }
    
    // Small delay to prevent watchdog reset
    delay(1);
}
```

---

## Memory Usage Analysis

### RAM Usage Breakdown

| Component | Estimated Size | Notes |
|-----------|---------------|-------|
| Widget Pool (50 widgets) | ~10 KB | Depends on widget size |
| Text Field Pool (10) | ~5 KB | With 256-byte buffers |
| File Info Pool (20) | ~1 KB | File listings |
| UI Buffer | ~2 KB | JSON parsing |
| Clipboard | ~64 bytes | Static buffer |
| Gesture State | ~20 bytes | Single state |
| History (10 states) | ~320 bytes | Text states |
| SD Card Library | ~1 KB | SdFat library |
| WebDAV Server | ~2 KB | ESPWebDAV library |
| **Total** | **~22 KB** | **Well within 80 KB limit** |

### Flash Usage Breakdown

| Component | Estimated Size | Notes |
|-----------|---------------|-------|
| TFT_eSPI Library | ~20 KB | With font support |
| XPT2046 Library | ~5 KB | Touchscreen driver |
| SdFat Library | ~15 KB | SD card support |
| ESPWebDAV Library | ~15 KB | WebDAV server |
| ArduinoJson Library | ~10 KB | JSON parsing |
| GUIKit Source Code | ~30 KB | All widget implementations |
| **Total** | **~95 KB** | **Well within 4MB limit** |

### Optimization Tips

1. **Disable unused fonts**: In TFT_eSPI User_Setup.h, disable fonts you don't need
2. **Use PROGMEM**: Store large constant data in Flash instead of RAM
3. **Limit pool sizes**: Reduce widget/text field pools if you don't need many
4. **Disable features**: If not using SD card or WebDAV, disable those libraries
5. **Use smaller buffers**: Reduce buffer sizes if you don't need large text

---

## Build and Upload

### Building the Project

```bash
# Clone the project
cd GUIKit

# Install PlatformIO
pio install

# Build for ESP8266
pio run -e esp8266_tft_sd_webdav

# Upload to device
pio run -e esp8266_tft_sd_webdav -t upload

# Monitor serial output
pio run -e esp8266_tft_sd_webdav -t monitor
```

### Troubleshooting

#### Common Issues

1. **Build fails with out of memory**
   - Reduce library dependencies
   - Disable unused features
   - Use `-Os` optimization flag

2. **Upload fails**
   - Check serial port connection
   - Verify board selection in platformio.ini
   - Try manual reset (press reset button during upload)

3. **White screen on startup**
   - Check TFT wiring (CS, DC, RST)
   - Verify TFT driver selection in User_Setup.h
   - Check backlight is enabled

4. **Touch not working**
   - Check TOUCH_CS pin assignment
   - Verify touchscreen wiring
   - Run calibration (implement calibration routine)

5. **SD card not detected**
   - Check SD_CS pin assignment
   - Verify SD card is formatted as FAT32
   - Ensure SD card is properly inserted

#### Debugging

```cpp
// Enable debug output
#define DEBUG 1
#define SERIAL_DEBUG 1

// Debug macros
#if DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

// Memory debugging
void print_free_memory() {
    DEBUG_PRINT("Free Heap: ");
    DEBUG_PRINT(ESP.getFreeHeap());
    DEBUG_PRINT(" bytes, Max Block: ");
    DEBUG_PRINT(ESP.getMaxFreeBlockSize());
    DEBUG_PRINTLN(" bytes");
}

// Call periodically to monitor memory
void loop() {
    static uint32_t last_debug = 0;
    if (millis() - last_debug > 5000) {
        print_free_memory();
        last_debug = millis();
    }
    // ... rest of loop
}
```

---

## Cross-References

- **Widget Architecture**: See `01_WIDGET_ARCHITECTURE.md`
- **Constructor Patterns**: See `02_CONSTRUCTOR_PATTERNS.md`
- **Widget Types**: See `03_WIDGET_TYPES.md`
- **Draw Styles**: See `04_DRAW_STYLES.md`
- **Style System**: See `05_STYLE_SYSTEM.md`
- **Memory Management**: See `06_MEMORY_MANAGEMENT.md` and `docs/MEMORY_MANAGEMENT.md`
- **Rendering System**: See `07_RENDERING_SYSTEM.md`
- **Touch Handling**: See `08_TOUCH_HANDLING.md`
- **Widget Implementations**: See `09_WIDGET_IMPLEMENTATIONS.md`
- **Text and Input**: See `10_TEXT_AND_INPUT.md`
- **SD Card/WebDAV**: See `11_SD_CARD_WEBDAV.md`
- **Optimizations**: See `12_OPTIMIZATIONS.md`
- **UI Parser**: See `13_UI_PARSER.md`
- **Advanced Features**: See `14_ADVANCED_FEATURES.md`

---

## File Locations

- `platformio.ini` - PlatformIO configuration
- `src/gui/main.cpp` - Main application
- `src/gui/*.h` - Header files
- `src/gui/*.cpp` - Implementation files
- `lib/TFT_eSPI/` - TFT display library
- `data/system/` - System files on SD card
- `docs/` - Documentation
