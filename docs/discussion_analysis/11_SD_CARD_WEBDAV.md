# SD Card and WebDAV Integration

> Extracted from discussion_guikit.txt - SD card management and WebDAV server implementation

## Overview

This document covers the integration of SD card storage and WebDAV server functionality with GUIKit on ESP8266, enabling:
- Text field content persistence (configuration, notes)
- Loading larger dictionaries from SD card
- Direct file editing on ESP8266 via web browser (WebDAV)
- Custom images and styles storage
- File sharing and download from PC

All implementations respect ESP8266 constraints: 80KB RAM, 4MB Flash, no FPU.

**IMPORTANT**: Uses manual memory management (NOT ARC/reference counting)

---

## Architecture

```
GUIKit/
├── src/
│   ├── sd_card.h/cpp          # SD card management
│   ├── webdav.h/cpp          # WebDAV server (ESPWebDAV)
│   ├── file_manager.h/cpp    # File operations (read/write)
│   ├── textfield.h/cpp       # TextField with save/load support
│   └── ... (existing files)
├── data/                     # Files directory (dictionaries, styles, etc.)
│   ├── dict_fr.txt          # French dictionary (on SD)
│   ├── dict_en.txt          # English dictionary (on SD)
│   └── styles/              # Custom styles
└── platformio.ini
```

---

## PlatformIO Configuration

### Environment Configuration

```ini
[env:esp8266_tft_sd_webdav]
platform = espressif8266
board = nodemcuv2  ; Adjust according to your board (e.g., esp01_1m)
framework = arduino

; Libraries
lib_deps =
    https://github.com/Bodmer/TFT_eSPI.git
    https://github.com/PaulStoffregen/XPT2046_Touchscreen.git
    https://github.com/hoonie/ESPWebDAV.git  ; WebDAV library
    https://github.com/greiman/SdFat.git    ; SD card library (optimized)

; TFT_eSPI Configuration
build_flags =
    -D USER_SETUP_LOADED
    -D ST7789_DRIVER
    -D TFT_WIDTH=240
    -D TFT_HEIGHT=320
    -D TFT_CS=D8
    -D TFT_DC=D3
    -D TFT_RST=D4

; XPT2046 Touchscreen Configuration
build_flags =
    -D TOUCH_CS=D2
    -D XPT2046_IRQ=D1

; SD Card Configuration
build_flags =
    -D SD_CS=D5  ; CS pin for SD card
    -D SD_FAT_TYPE=1  ; 1 = FAT16/FAT32, 2 = exFAT

; ESPWebDAV Configuration
build_flags =
    -D WEBDAV_USERNAME="admin"
    -D WEBDAV_PASSWORD="esp8266"
    -D WEBDAV_PORT=80

; Serial monitor
monitor_speed = 115200
```

### Library Selection Notes

- **SdFat**: Preferred over standard SD.h for better performance and lower memory usage
- **ESPWebDAV**: Lightweight WebDAV server optimized for ESP8266
- **SPI**: Shared between SD card and TFT display (requires careful CS management)

---

## SD Card Management

### Header File (sd_card.h)

```c
#ifndef SD_CARD_H
#define SD_CARD_H

#include <SdFat.h>
#include <stdint.h>
#include <stdbool.h>

// ========== DEFINITIONS ==========
#define SD_CS_PIN D5  // CS pin for SD card
#define MAX_FILE_SIZE 1024  // Max size for read/write at once (RAM limit)

// ========== STRUCTURES ==========
typedef struct {
    char name[32];      // File name (max 32 chars)
    uint32_t size;      // File size in bytes
    bool is_dir;        // Is directory flag
    bool shared;       // Is shared via WebDAV
} FileInfo;

// ========== FUNCTIONS ==========
// Initialize SD card
bool init_sd_card(void);

// Check if SD card is available
bool is_sd_card_available(void);

// Read file from SD card
bool sd_read_file(const char* path, char* buffer, uint16_t buffer_size);

// Write file to SD card
bool sd_write_file(const char* path, const char* data);

// Append data to file
bool sd_append_file(const char* path, const char* data);

// List files in directory
bool sd_list_files(const char* path, FileInfo* files, uint8_t* count, uint8_t max_files);

// Delete file
bool sd_delete_file(const char* path);

// Check if file exists
bool sd_file_exists(const char* path);

// Get file size
uint32_t sd_get_file_size(const char* path);

// Create directory
bool sd_mkdir(const char* path);

// Rename file
bool sd_rename(const char* old_path, const char* new_path);

#endif // SD_CARD_H
```

### Implementation (sd_card.cpp)

```c
#include "sd_card.h"
#include <SdFat.h>
#include <SPI.h>

// ========== GLOBAL VARIABLES ==========
SdFat sd;
SdFile file;

// ========== FUNCTIONS ==========
bool init_sd_card(void) {
    if (!sd.begin(SD_CS_PIN, SPI)) {
        Serial.println("Error: SD card initialization failed!");
        return false;
    }
    Serial.println("SD card initialized successfully.");
    return true;
}

bool is_sd_card_available(void) {
    return sd.card()->isPresent();
}

bool sd_read_file(const char* path, char* buffer, uint16_t buffer_size) {
    if (!is_sd_card_available() || !buffer) return false;

    if (!file.open(path, O_READ)) {
        Serial.print("Error: Cannot open file ");
        Serial.println(path);
        return false;
    }

    uint16_t bytes_read = file.read(buffer, buffer_size - 1);
    buffer[bytes_read] = '\0';  // Null-terminate string
    file.close();
    return true;
}

bool sd_write_file(const char* path, const char* data) {
    if (!is_sd_card_available() || !data) return false;

    if (!file.open(path, O_WRITE | O_CREAT | O_TRUNC)) {
        Serial.print("Error: Cannot create file ");
        Serial.println(path);
        return false;
    }

    uint16_t bytes_written = file.write(data, strlen(data));
    file.close();
    return bytes_written > 0;
}

bool sd_append_file(const char* path, const char* data) {
    if (!is_sd_card_available() || !data) return false;

    if (!file.open(path, O_WRITE | O_APPEND)) {
        Serial.print("Error: Cannot open file for append ");
        Serial.println(path);
        return false;
    }

    uint16_t bytes_written = file.write(data, strlen(data));
    file.close();
    return bytes_written > 0;
}

bool sd_list_files(const char* path, FileInfo* files, uint8_t* count, uint8_t max_files) {
    if (!is_sd_card_available() || !files || !count) return false;

    *count = 0;
    SdFile dir;
    if (!dir.open(path)) {
        Serial.print("Error: Cannot open directory ");
        Serial.println(path);
        return false;
    }

    SdFile entry;
    while (entry.openNext(&dir, O_READ) && *count < max_files) {
        entry.getName(files[*count].name, 32);
        files[*count].size = entry.fileSize();
        files[*count].is_dir = entry.isDir();
        files[*count].shared = false;
        (*count)++;
        entry.close();
    }
    dir.close();
    return true;
}

bool sd_delete_file(const char* path) {
    if (!is_sd_card_available()) return false;

    if (!sd.remove(path)) {
        Serial.print("Error: Cannot delete file ");
        Serial.println(path);
        return false;
    }
    return true;
}

bool sd_file_exists(const char* path) {
    if (!is_sd_card_available()) return false;
    return sd.exists(path);
}

uint32_t sd_get_file_size(const char* path) {
    if (!is_sd_card_available()) return 0;
    
    SdFile temp_file;
    if (!temp_file.open(path, O_READ)) {
        return 0;
    }
    uint32_t size = temp_file.fileSize();
    temp_file.close();
    return size;
}

bool sd_mkdir(const char* path) {
    if (!is_sd_card_available()) return false;
    return sd.mkdir(path);
}

bool sd_rename(const char* old_path, const char* new_path) {
    if (!is_sd_card_available()) return false;
    return sd.rename(old_path, new_path);
}
```

### Memory Management

```c
// FileInfo pool for ESP8266 (avoid dynamic allocation)
#define MAX_FILEINFO_POOL 20
FileInfo fileinfo_pool[MAX_FILEINFO_POOL];

// Use pool-based allocation for file listings
bool sd_list_files_pooled(const char* path, FileInfo** files, uint8_t* count) {
    if (!is_sd_card_available() || !files || !count) return false;
    
    *count = 0;
    SdFile dir;
    if (!dir.open(path)) {
        return false;
    }

    SdFile entry;
    while (entry.openNext(&dir, O_READ) && *count < MAX_FILEINFO_POOL) {
        entry.getName(fileinfo_pool[*count].name, 32);
        fileinfo_pool[*count].size = entry.fileSize();
        fileinfo_pool[*count].is_dir = entry.isDir();
        fileinfo_pool[*count].shared = false;
        (*count)++;
        entry.close();
    }
    dir.close();
    
    *files = fileinfo_pool;
    return true;
}

// Static buffer for file reads
char sd_read_buffer[MAX_FILE_SIZE + 1];

// Read file using static buffer
const char* sd_read_file_static(const char* path) {
    if (!sd_read_file(path, sd_read_buffer, MAX_FILE_SIZE)) {
        sd_read_buffer[0] = '\0';
        return NULL;
    }
    return sd_read_buffer;
}
```

---

## WebDAV Server

### Header File (webdav.h)

```c
#ifndef WEBDAV_H
#define WEBDAV_H

#include <stdint.h>
#include <stdbool.h>
#include "sd_card.h"

// ========== DEFINITIONS ==========
#define WEBDAV_PORT 80
#define WEBDAV_USERNAME "admin"
#define WEBDAV_PASSWORD "esp8266"
#define MAX_SHARED_FILES 10

// ========== FUNCTIONS ==========
// Initialize WebDAV server
bool init_webdav(const char* ssid, const char* password);

// Start WebDAV server
void start_webdav(void);

// Stop WebDAV server
void stop_webdav(void);

// Check if WebDAV server is running
bool is_webdav_running(void);

// Share file via WebDAV
bool webdav_share_file(const char* path, const char* name);

// Unshare file from WebDAV
void webdav_unshare_file(const char* name);

// Process WebDAV client requests (call in loop())
void webdav_handle_client(void);

// Get WebDAV server IP address
const char* webdav_get_ip(void);

#endif // WEBDAV_H
```

### Implementation (webdav.cpp)

```c
#include "webdav.h"
#include "sd_card.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESPWebDAV.h>

// ========== GLOBAL VARIABLES ==========
ESP8266WebServer server(WEBDAV_PORT);
ESPWebDAV webdav_server(&server);

// Shared files tracking
typedef struct {
    char sd_path[64];     // Path on SD card
    char webdav_name[32]; // Name exposed via WebDAV
    bool active;          // Currently shared
} SharedFile;

SharedFile shared_files[MAX_SHARED_FILES];

// ========== FUNCTIONS ==========
bool init_webdav(const char* ssid, const char* password) {
    // Initialize WiFi
    WiFi.begin(ssid, password);
    
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("WiFi connected. IP: ");
    Serial.println(WiFi.localIP());
    
    // Configure WebDAV server
    webdav_server.setAuthentication(WEBDAV_USERNAME, WEBDAV_PASSWORD);
    server.begin();
    
    // Initialize shared files list
    for (uint8_t i = 0; i < MAX_SHARED_FILES; i++) {
        shared_files[i].active = false;
    }
    
    return true;
}

void start_webdav(void) {
    if (!is_webdav_running()) {
        server.begin();
        Serial.println("WebDAV server started.");
    }
}

void stop_webdav(void) {
    server.stop();
    Serial.println("WebDAV server stopped.");
}

bool is_webdav_running(void) {
    return server.hasClient();
}

bool webdav_share_file(const char* path, const char* name) {
    if (!is_sd_card_available() || !path || !name) return false;
    
    // Find empty slot
    uint8_t slot = 0;
    for (; slot < MAX_SHARED_FILES; slot++) {
        if (!shared_files[slot].active) break;
    }
    if (slot >= MAX_SHARED_FILES) {
        Serial.println("Error: Maximum shared files reached");
        return false;
    }
    
    // Read file from SD
    uint32_t file_size = sd_get_file_size(path);
    if (file_size == 0) return false;
    
    char* buffer = (char*)malloc(file_size + 1);
    if (!buffer) return false;
    
    if (!sd_read_file(path, buffer, file_size)) {
        free(buffer);
        return false;
    }
    
    // Add to WebDAV server
    webdav_server.addFile(name, (uint8_t*)buffer, file_size);
    
    // Store in shared files list
    strncpy(shared_files[slot].sd_path, path, 63);
    strncpy(shared_files[slot].webdav_name, name, 31);
    shared_files[slot].active = true;
    
    Serial.print("File shared: ");
    Serial.print(name);
    Serial.print(" (");
    Serial.print(file_size);
    Serial.println(" bytes)");
    
    return true;
}

void webdav_unshare_file(const char* name) {
    if (!name) return;
    
    // Find file in shared list
    for (uint8_t i = 0; i < MAX_SHARED_FILES; i++) {
        if (shared_files[i].active && strcmp(shared_files[i].webdav_name, name) == 0) {
            webdav_server.removeFile(name);
            shared_files[i].active = false;
            Serial.print("File unshared: ");
            Serial.println(name);
            return;
        }
    }
}

void webdav_handle_client(void) {
    server.handleClient();
}

const char* webdav_get_ip(void) {
    return WiFi.localIP().toString().c_str();
}
```

---

## File Manager

### Header File (file_manager.h)

```c
#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "sd_card.h"
#include "webdav.h"

// ========== DEFINITIONS ==========
#define MAX_OPEN_FILES 5
#define FILE_BUFFER_SIZE 512

// ========== STRUCTURES ==========
typedef struct {
    char path[64];        // File path
    char* buffer;        // File content buffer
    uint32_t size;        // File size
    bool modified;       // Modified flag
    bool sd_backed;      // Backed by SD card
    bool webdav_shared;  // Shared via WebDAV
} FileHandle;

// ========== FUNCTIONS ==========
// Initialize file manager
bool init_file_manager(void);

// Open file
FileHandle* file_open(const char* path);

// Close file
bool file_close(FileHandle* handle);

// Read file
bool file_read(FileHandle* handle, char* buffer, uint16_t size);

// Write file
bool file_write(FileHandle* handle, const char* data);

// Save file (to SD)
bool file_save(FileHandle* handle);

// Share file via WebDAV
bool file_share(FileHandle* handle, const char* webdav_name);

// List files
bool file_list(const char* path, FileInfo* files, uint8_t* count, uint8_t max_files);

// Delete file
bool file_delete(const char* path);

#endif // FILE_MANAGER_H
```

### Implementation (file_manager.cpp)

```c
#include "file_manager.h"
#include <string.h>

// ========== GLOBAL VARIABLES ==========
FileHandle file_handles[MAX_OPEN_FILES];
char file_buffers[MAX_OPEN_FILES][FILE_BUFFER_SIZE + 1];

// ========== FUNCTIONS ==========
bool init_file_manager(void) {
    for (uint8_t i = 0; i < MAX_OPEN_FILES; i++) {
        file_handles[i].buffer = file_buffers[i];
        file_handles[i].path[0] = '\0';
        file_handles[i].size = 0;
        file_handles[i].modified = false;
        file_handles[i].sd_backed = false;
        file_handles[i].webdav_shared = false;
    }
    return true;
}

FileHandle* file_open(const char* path) {
    if (!path) return NULL;
    
    // Check if file is already open
    for (uint8_t i = 0; i < MAX_OPEN_FILES; i++) {
        if (file_handles[i].path[0] != '\0' && 
            strcmp(file_handles[i].path, path) == 0) {
            return &file_handles[i];
        }
    }
    
    // Find empty slot
    for (uint8_t i = 0; i < MAX_OPEN_FILES; i++) {
        if (file_handles[i].path[0] == '\0') {
            strncpy(file_handles[i].path, path, 63);
            file_handles[i].size = 0;
            file_handles[i].modified = false;
            file_handles[i].sd_backed = false;
            file_handles[i].webdav_shared = false;
            
            // Load from SD if exists
            if (sd_file_exists(path)) {
                file_handles[i].size = sd_get_file_size(path);
                file_handles[i].sd_backed = true;
                sd_read_file(path, file_handles[i].buffer, FILE_BUFFER_SIZE);
            } else {
                file_handles[i].buffer[0] = '\0';
            }
            
            return &file_handles[i];
        }
    }
    
    return NULL;  // No available slots
}

bool file_close(FileHandle* handle) {
    if (!handle) return false;
    
    // Save if modified
    if (handle->modified && handle->sd_backed) {
        file_save(handle);
    }
    
    handle->path[0] = '\0';
    handle->size = 0;
    handle->modified = false;
    handle->sd_backed = false;
    handle->webdav_shared = false;
    handle->buffer[0] = '\0';
    
    return true;
}

bool file_read(FileHandle* handle, char* buffer, uint16_t size) {
    if (!handle || !buffer) return false;
    
    if (handle->size > 0) {
        strncpy(buffer, handle->buffer, size - 1);
        buffer[size - 1] = '\0';
        return true;
    }
    return false;
}

bool file_write(FileHandle* handle, const char* data) {
    if (!handle || !data) return false;
    
    uint16_t data_len = strlen(data);
    if (data_len >= FILE_BUFFER_SIZE) {
        data_len = FILE_BUFFER_SIZE - 1;
    }
    
    strncpy(handle->buffer, data, data_len);
    handle->buffer[data_len] = '\0';
    handle->size = data_len;
    handle->modified = true;
    
    return true;
}

bool file_save(FileHandle* handle) {
    if (!handle || !handle->sd_backed) return false;
    
    if (sd_write_file(handle->path, handle->buffer)) {
        handle->modified = false;
        return true;
    }
    return false;
}

bool file_share(FileHandle* handle, const char* webdav_name) {
    if (!handle || !webdav_name) return false;
    
    if (webdav_share_file(handle->path, webdav_name)) {
        handle->webdav_shared = true;
        return true;
    }
    return false;
}

bool file_list(const char* path, FileInfo* files, uint8_t* count, uint8_t max_files) {
    return sd_list_files(path, files, count, max_files);
}

bool file_delete(const char* path) {
    return sd_delete_file(path);
}
```

---

## TextField Integration

### Save/Load Functions for TextField

```c
// textfield.h additions
bool textfield_save_to_file(WidgetTextField* textfield, const char* path);
bool textfield_load_from_file(WidgetTextField* textfield, const char* path);

// textfield.cpp additions
bool textfield_save_to_file(WidgetTextField* textfield, const char* path) {
    if (!textfield || !textfield->buffer || !path) return false;
    
    if (!sd_write_file(path, textfield->buffer)) {
        Serial.print("Error: Cannot save textfield to ");
        Serial.println(path);
        return false;
    }
    
    Serial.print("TextField saved to ");
    Serial.println(path);
    return true;
}

bool textfield_load_from_file(WidgetTextField* textfield, const char* path) {
    if (!textfield || !textfield->buffer || !path) return false;
    
    if (!sd_read_file(path, textfield->buffer, textfield->buffer_size)) {
        Serial.print("Error: Cannot load textfield from ");
        Serial.println(path);
        return false;
    }
    
    textfield->cursor_pos = strlen(textfield->buffer);
    textfield->selection.active = false;
    
    Serial.print("TextField loaded from ");
    Serial.println(path);
    return true;
}
```

---

## Usage Examples

### Complete SD + WebDAV Integration

```c
#include "sd_card.h"
#include "webdav.h"
#include "file_manager.h"
#include "textfield.h"

// Configuration
const char* WIFI_SSID = "Your_SSID";
const char* WIFI_PASSWORD = "Your_Password";

void setup() {
    Serial.begin(115200);
    
    // Initialize hardware
    init_renderer();
    init_touch();
    
    // Initialize SD card
    if (!init_sd_card()) {
        Serial.println("Warning: SD card not detected!");
    } else {
        // Initialize file manager
        init_file_manager();
    }
    
    // Initialize WiFi and WebDAV
    init_webdav(WIFI_SSID, WIFI_PASSWORD);
    start_webdav();
    
    // Create UI
    Widget* root = new_widget(WIDGET_TYPE_VIEW);
    
    // Create text field
    WidgetTextField* textfield = new_textfield(512, TEXTFIELD_STYLE_NORMAL);
    textfield->base.rect.position.x = 20;
    textfield->base.rect.position.y = 20;
    textfield->base.rect.size.width = 280;
    textfield->base.rect.size.height = 200;
    
    // Load from SD if exists
    if (sd_file_exists("/notes.txt")) {
        textfield_load_from_file(textfield, "/notes.txt");
    }
    
    widget_add_child(root, &textfield->base);
    
    // Create save button
    WidgetButton* save_btn = new_button();
    strcpy(save_btn->base.text.text, "Save");
    save_btn->base.rect.position.x = 20;
    save_btn->base.rect.position.y = 230;
    save_btn->on_click = []() {
        if (textfield_save_to_file(textfield, "/notes.txt")) {
            Serial.println("File saved successfully!");
        } else {
            Serial.println("Error: Cannot save file.");
        }
    };
    widget_add_child(root, &save_btn->base);
    
    // Create share button
    WidgetButton* share_btn = new_button();
    strcpy(share_btn->base.text.text, "Share");
    share_btn->base.rect.position.x = 120;
    share_btn->base.rect.position.y = 230;
    share_btn->on_click = []() {
        // Share via WebDAV
        if (webdav_share_file("/notes.txt", "notes.txt")) {
            Serial.println("File shared via WebDAV!");
            Serial.print("Access at http://");
            Serial.print(WiFi.localIP());
            Serial.println("/webdav to download.");
        } else {
            Serial.println("Error: Cannot share file.");
        }
    };
    widget_add_child(root, &share_btn->base);
}

void loop() {
    // Handle WebDAV clients
    webdav_handle_client();
    
    // Handle touch
    handle_touch();
    
    // Render
    draw_widget_tree(root);
}
```

---

## ESP8266-Specific Considerations

### Memory Constraints

- **MAX_FILE_SIZE**: 1024 bytes for single read/write operations
- **File pool**: Maximum 5 simultaneously open files
- **Shared files**: Maximum 10 files shared via WebDAV
- **Buffer sizes**: Pre-allocated static buffers to avoid dynamic allocation

### SPI Bus Sharing

The SD card and TFT display share the SPI bus:

```c
// CS pins must be different
#define TFT_CS D8
#define TOUCH_CS D2
#define SD_CS D5

// Ensure only one device is active at a time
void select_tft(void) {
    digitalWrite(TFT_CS, LOW);
    digitalWrite(SD_CS, HIGH);
}

void select_sd(void) {
    digitalWrite(TFT_CS, HIGH);
    digitalWrite(SD_CS, LOW);
}
```

### Power Management

- SD card can be powered down when not in use
- Use `sd.idle()` to reduce power consumption

### Error Handling

Always check if SD card is available before operations:

```c
// BAD - no check
void bad_function(void) {
    sd_read_file("/config.txt", buffer, 256);  // May crash if SD removed
}

// GOOD - with check
void good_function(void) {
    if (is_sd_card_available()) {
        sd_read_file("/config.txt", buffer, 256);
    } else {
        Serial.println("SD card not available");
    }
}
```

---

## Cross-References

- **Widget Implementations**: See `09_WIDGET_IMPLEMENTATIONS.md`
- **Text and Input**: See `10_TEXT_AND_INPUT.md`
- **Memory Management**: See `06_MEMORY_MANAGEMENT.md` and `docs/MEMORY_MANAGEMENT.md`
- **PlatformIO Configuration**: See `15_PROJECT_STRUCTURE.md`

---

## File Locations

Source implementations:
- `src/gui/sd_card.h` - SD card declarations
- `src/gui/sd_card.cpp` - SD card implementation
- `src/gui/webdav.h` - WebDAV server declarations
- `src/gui/webdav.cpp` - WebDAV server implementation
- `src/gui/file_manager.h` - File manager declarations
- `src/gui/file_manager.cpp` - File manager implementation
- `platformio.ini` - PlatformIO configuration
