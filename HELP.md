# GUIKit Help Guide

## Table of Contents
1. [Quick Start](#quick-start)
2. [Memory Strategy](#memory-strategy)
3. [Bootloader](#bootloader)
4. [Autostart Configuration](#autostart-configuration)
5. [Task Switcher](#task-switcher)
6. [Task Progress](#task-progress)
7. [Hardware Configuration](#hardware-configuration)
8. [GUI Loading](#gui-loading)
9. [WebDAV Push Notifications](#webdav-push-notifications)
10. [mDNS Service Discovery](#mdns-service-discovery)
11. [Troubleshooting](#troubleshooting)
12. [Command Reference](#command-reference)

---

## Quick Start

### Building

```bash
# Clone the repository
git clone https://github.com/genose/genose.org-ESP8266-KernelonSDCard-GUIKit-TFT-Touch-WebDavServer.git
cd genose.org-ESP8266-KernelonSDCard-GUIKit-TFT-Touch-WebDavServer

# Build the project (PlatformIO recommended)
pio run

# Or use Arduino IDE
# Open the .ino file and compile
```

### Running

```bash
# Upload bootloader
# Upload kernel.bin to SD card
# Power on the device
```

---

## Memory Strategy

### Overview

The GUIKit uses a **hierarchical memory strategy** with STOP-at-first-success behavior:

```
1. Try External RAM -> if (available AND GUI fits) => SELECT & STOP
2. Try SD Card Swap -> if (available AND GUI fits) => SELECT & STOP
3. Try Internal RAM -> if (GUI fits) => SELECT & STOP
4. Else => FAILED
```

### Configuration

Memory strategy is configured through `memory_strategy_config_t`:

```c
memory_strategy_config_t config = GUIKIT_MEMORY_STRATEGY_DEFAULT;

// Customize thresholds
config.external_ram_min_size = 4096;    // 4KB minimum for external RAM
config.sd_swap_min_size = 16384;       // 16KB minimum for SD swap
config.internal_ram_max_size = 8192;   // 8KB maximum for internal RAM
config.external_ram_max_size = 131072;  // 128KB maximum external RAM

// Customize behavior
config.use_external_ram_by_default = true;
config.use_sd_swap_by_default = true;
config.check_memory_before_load = true;
config.display_error_on_tft = true;

// Initialize memory strategy
gui_memory_strategy_init(sram, sdcard, tft, &config);
```

### Huge GUI Results

For a **500KB GUI**, the results are:

| Hardware Configuration | Result | Reason |
|------------------------|--------|--------|
| ESP8266 + 128KB SRAM + SD Card | **SD_CARD_SWAP** [STOP] | 500KB > 128KB external, but SD Card available |
| ESP8266 + No SRAM + SD Card | **SD_CARD_SWAP** [STOP] | No external RAM, but SD Card available |
| ESP32 + 8MB PSRAM + SD Card | **EXTERNAL_RAM** [STOP] | 500KB fits in 8MB external RAM |
| ESP32 + No PSRAM + SD Card | **SD_CARD_SWAP** [STOP] | No external RAM, but SD Card available |
| ESP8266 + 128KB SRAM + NO SD Card | **FAILED** | 500KB > 128KB external, no SD Card |
| Minimal: No SRAM + No SD Card | **FAILED** | 500KB > internal RAM limit |

See `src/gui/demo_huge_gui_result.txt` for detailed flow diagrams.

### Supported RAM Chip Models

**All models work with both ESP8266 (via SPI) and ESP32 (via SPI or native interface):**

**SPI SRAM:**
| Model | Size | Speed | Price | Use Case |
|-------|------|-------|-------|----------|
| 23LC512 | 64 KB | 20 MHz | ~$1.50 | Small cache |
| **23LC1024** | **128 KB** | **20 MHz** | **~$3.00** | **Recommended** |
| 23LCV1024 | 128 KB | 20 MHz | ~$3.50 | Low-voltage |
| **Lyontek LY68L6400** | **512 KB** | **50 MHz** | **~$4.00** | **Large cache** |

**FRAM (Non-Volatile):**
| Model | Size | Speed | Price | Use Case |
|-------|------|-------|-------|----------|
| MB85RS256B | 32 KB | 20 MHz | ~$5 | Config storage |
| **CY15V102QN** | **128 KB** | **40 MHz** | **~$12** | **Industrial** |
| **CY15V104QSN** | **512 KB** | **40 MHz** | **~$20** | **Industrial** |

**PSRAM (ESP32 Native):**
| Model | Size | Speed | Price | Use Case |
|-------|------|-------|-------|----------|
| APS6404 | 1 MB | 40 MHz | ~$3 | Entry-level |
| APS1604 | 2 MB | 40 MHz | ~$5 | Mid-range |
| APS3204 | 4 MB | 40 MHz | ~$8 | High-capacity |
| W9812G6KH | 8 MB | 80 MHz | ~$10 | Maximum |
| **ISSI IS66WVS5128ALL** | **64 MB** | **100 MHz** | **~$15** | **Industrial** |
| **ISSI IS66WVS5128BLL** | **64 MB** | **100 MHz** | **~$15** | **Industrial** |

**Configuration Presets:**
```c
// ESP8266 with Lyontek LY68L6400 (512KB SRAM)
GUIKIT_HW_ESP8266_LY68L6400

// ESP8266 with Cypress CY15V104QSN (512KB FRAM)
GUIKIT_HW_ESP8266_CY15V104QSN

// ESP32 with ISSI IS66WVS5128ALL (64MB PSRAM)
GUIKIT_HW_ESP32_ISSI_64MB_PSRAM

// ESP32 with Lyontek LY68L6400 (512KB SRAM)
GUIKIT_HW_ESP32_LY68L6400

// Union configs also available:
GUIKIT_HW_UNION_ESP8266_LY68L6400
GUIKIT_HW_UNION_ESP8266_CY15V104QSN
GUIKIT_HW_UNION_ESP32_ISSI_64MB_PSRAM
GUIKIT_HW_UNION_ESP32_LY68L6400
GUIKIT_HW_UNION_ESP32_CY15V104QSN
```

---

## Bootloader

### Usage

```c
#include "guikit_bootloader.h"

BootloaderState state;
bootloader_init(&state, NULL);  // NULL = use platform defaults

if (guikit_bootloader_run(&state)) {
    // Boot successful
    // state.config contains hardware configuration
    // state.memory_config contains memory strategy
    // state.selected_strategy is the default strategy
    
    // Initialize GUIKit
    gui_init(&state.config);
} else {
    // Boot failed
    printf("Boot failed: %s\n", state.error_message);
}
```

### Custom Configuration

```c
#include "guikit_bootloader.h"

// Create custom platform configuration
guikit_hw_config_t platform_config = GUIKIT_HW_ESP8266_DEFAULT;

// Add external SRAM
platform_config.ram.bank_count = 1;
platform_config.ram.bank[0].type = GUIKIT_RAM_SRAM;
platform_config.ram.bank[0].size = 131072;  // 128KB
platform_config.ram.bank[0].cs_pin = 16;   // D0
platform_config.ram.bank[0].enabled = true;

// Configure SPI
platform_config.spi.bank_count = 4;
platform_config.spi.bank[0].type = GUIKIT_SPI_DEVICE;
platform_config.spi.bank[0].cs_pin = 15;    // TFT at D8
// ... configure other SPI devices

// Run bootloader
BootloaderState state;
bootloader_init(&state, &platform_config);
guikit_bootloader_run(&state);
```

### Boot Sequence

1. **MCU Platform Detection** - Detects ESP8266 or ESP32
2. **SMP Detection** - Detects multi-core capability
3. **Hardware Detection** - Detects all SPI devices
4. **RAM Initialization** - Initializes internal and external RAM
5. **SD Card Initialization** - Initializes SD card if present
6. **Autostart Configuration Loading** - Loads /etc/GUIKIT_autostart.ini
7. **TFT Initialization** - Initializes display if present
8. **Kernel Check** - Checks kernel file (using autostart config path)
9. **Memory Strategy Configuration** - Configures based on hardware and autostart settings
10. **Memory Strategy Test** - Tests with various GUI sizes
11. **Display Results** - Shows boot summary on TFT

---

## Autostart Configuration

### Overview

The bootloader reads `/etc/GUIKIT_autostart.ini` on the SD card to determine:
- Which kernel.bin to load
- Memory strategy configuration
- Which GUI to start automatically
- Memory bank allocation rules

This provides a little OS-like boot configuration mechanism.

### Configuration File Format

The file uses INI format with sections:

```ini
[kernel]
path = /kernel.bin
compress = false
expected_size = 0
max_size = 0
verify = true

[memory]
strategy = auto
stop_at_first_success = true
internal.enabled = true
external.enabled = true
sd_swap.enabled = true

[gui]
path = /gui/chooser.GUIKIT
auto_start = true
theme = default
width = 0
height = 0

[allocations]
name = gui_widgets
bank = external
fallback = sd_swap
size = 65536
replace = false

[flags]
debug = false
tft_boot_messages = true
serial_boot_messages = true
```

### Memory Strategy Options

The `strategy` field in the `[memory]` section supports:

| Strategy | Description | Behavior |
|----------|-------------|----------|
| `auto` | Automatic selection | Try external RAM, then SD swap, then internal RAM (STOP at first success) |
| `external_first` | External RAM first | Try external RAM first, then SD swap, then internal RAM |
| `sd_swap_first` | SD swap first | Try SD swap first, then external RAM, then internal RAM |
| `internal_only` | Internal RAM only | Only use internal RAM, disable external and SD swap |
| `custom` | Custom allocation | Use allocation rules from `[allocations]` section |

### Memory Bank Configuration

Each memory bank can be individually enabled/disabled:

```ini
[memory]
internal.enabled = true    ; Internal MCU RAM
external.enabled = true    ; External SRAM/PSRAM
sd_swap.enabled = true    ; SD card swap space
```

### Custom Allocation Rules

When `strategy = custom`, you can define specific allocation rules:

```ini
[allocations]
; Format: name, bank, fallback, size, replace

name = gui_widgets
bank = external
fallback = sd_swap
size = 65536
replace = false

name = image_cache
bank = sd_swap
fallback = external
size = 131072
replace = true
```

### Programmatic Usage

You can also access the autostart configuration programmatically:

```c
#include "guikit_autostart_config.h"

// Initialize with defaults
guikit_autostart_init_default(&config);

// Parse from file
if (guikit_autostart_parse_ini(&config, "/etc/GUIKIT_autostart.ini")) {
    // Config loaded successfully
}

// Access configuration
printf("Kernel path: %s\n", config.kernel.path);
printf("Strategy: %s\n", guikit_autostart_strategy_name(config.strategy));
printf("GUI path: %s\n", config.gui.gui_path);

// Apply memory strategy
MemBankType bank = guikit_autostart_apply_strategy(&config, gui_size);
```

### Boot Sequence Integration

The autostart configuration is loaded in the boot sequence after SD Card initialization:

1. Hardware Detection
2. RAM Initialization
3. SPI Detection
4. SD Card Initialization
5. **Autostart Configuration Loading** (new)
6. TFT Initialization
7. Kernel Check (using autostart kernel path)
8. Memory Strategy Configuration (using autostart settings)
9. Memory Strategy Test and Apply
10. Display Results

---

## Task Switcher

### Overview

The **Task Switcher** implements single-level context switching with RAM freeze/thaw, allowing you to temporarily switch from one task to another and then restore the original task exactly as it was.

This is useful for:
- Heavy operations (JPEG decoding, complex calculations)
- Specialized utilities (file browsers, settings menus)
- Modal dialogs that need full RAM
- Any operation requiring dedicated RAM resources

### How It Works

**Single-Level Switching Pattern:**

```
Task A (GUI with state) 
  → task_switch_to("/kernel/converter.bin")
  → Freeze A RAM to SD card
  → Load Task B into RAM
  → Run Task B
  → User clicks "Back" OR B calls task_restore()
  → FREE Task B (not saved)
  → RESTORE Task A from SD card
  → Task A resumes with FULL original state
```

**Key Points:**
- Only ONE frozen task at a time (A → B → back to A)
- Task B is **freed** (discarded) when returning to A
- Task A is **defrosted** (fully restored) from SD card
- Tasks A and B are responsible for their own standard methods
- Communication between tasks via SD card files

### Memory Requirements

- **SD card required** for storing frozen state
- **80%+ free RAM** required to switch (configurable)
- Task B runs in the same RAM space that Task A occupied

### API Reference

```c
#include "task_switcher.h"

// Initialize (in setup/main)
task_switcher_init();

// Switch from current task to new task
bool success = task_switch_to("/kernel/jpeg_converter.bin");
// Does NOT return if successful - new task starts running

// Restore parent task (in Task B or "Back" button handler)
bool success = task_restore();
// Does NOT return if successful - parent task resumes

// Check if we have a frozen parent
if (task_has_frozen_parent()) {
    // We are Task B (child task)
}

// Get error information
TaskSwitcherError err = task_switcher_get_error();
const char* msg = task_switcher_error_to_string(err);
```

### Task Communication

Tasks communicate via files on SD card in `/tmp/task_comm/`:

```c
// Task B: Save data for parent
uint8_t results[1024] = { ... };
task_save_for_parent("results.bin", results, sizeof(results));

// Task A (after restore): Load data from child
uint8_t buffer[1024];
uint32_t loaded = task_load_from_child("results.bin", buffer, sizeof(buffer));

// Check if child saved a file
if (task_child_has_file("results.bin")) {
    // File exists
}
```

### Example: JPEG to RGB Conversion

**Task A (GUI Image Viewer):**

```c
#include "task_switcher.h"
#include "task_switcher_example.h"

void on_convert_button_click(const char* jpeg_path) {
    if (gui_trigger_jpeg_conversion(jpeg_path)) {
        // After restore, execution continues here
        // Task A state is exactly as it was before switch
        
        RgbImageHeader header;
        uint8_t rgb_buffer[1024 * 768 * 3];
        uint32_t loaded = gui_load_conversion_result(&header, rgb_buffer, sizeof(rgb_buffer));
        if (loaded > 0) {
            display_rgb_image(&header, rgb_buffer);
        }
    }
}
```

**Task B (JPEG Converter):**

```c
#include "task_switcher.h"

void main(void) {
    task_switcher_init();
    
    // Perform JPEG to RGB conversion
    RgbImageHeader header;
    uint8_t* rgb_data = convert_jpeg_to_rgb("/images/input.jpg", &header);
    
    // Save results for parent
    task_save_for_parent("converted.hdr", &header, sizeof(header));
    task_save_for_parent("converted.rgb", rgb_data, header.data_size);
    
    free(rgb_data);
    
    // Wait for user to click "Back"
    while (1) {
        if (back_button_pressed()) {
            task_restore();  // Returns to Task A
        }
    }
}
```

### Configuration

Customize task switcher behavior:

```c
TaskSwitcherConfig config = {
    .frozen_task_file = "/tmp/frozen.bin",  // SD card path for frozen state
    .min_free_ram = 80,                      // Require 80% free RAM
    .auto_cleanup = true,                   // Delete frozen file after restore
    .debug = false                          // Enable debug output
};
task_switcher_init_with_config(&config);
```

### Error Handling

```c
TaskSwitcherError err = task_switcher_get_error();

switch (err) {
    case TASK_ERROR_NONE:
        break;
    case TASK_ERROR_NO_SDCARD:
        show_tft_error("SD Card Required for Task Switching!");
        break;
    case TASK_ERROR_FREEZE_FAILED:
        show_tft_error("Failed to Freeze RAM!");
        break;
    case TASK_ERROR_LOAD_FAILED:
        show_tft_error("Failed to Load New Task!");
        break;
    case TASK_ERROR_RESTORE_FAILED:
        show_tft_error("Failed to Restore Parent!");
        break;
    case TASK_ERROR_NO_FROZEN_TASK:
        show_tft_error("No Parent Task to Restore!");
        break;
    case TASK_ERROR_INSUFFICIENT_RAM:
        show_tft_error("Need 80%+ Free RAM to Switch!");
        break;
}
```

### Files

| File | Purpose |
|------|---------|
| `src/boot/task_switcher.h` | Main header with API |
| `src/boot/task_switcher.c` | Core implementation |
| `src/boot/task_switcher_example.h` | JPEG conversion example header |
| `src/boot/task_switcher_example.c` | JPEG conversion example implementation |
| `docs/TASK_SWITCHER.md` | Detailed documentation |

---

## Task Progress

### Overview

The **Task Progress** system provides lightweight progress display on TFT for heavy tasks, designed to work when only 10KB of RAM is available.

### Key Features

- **Minimal RAM usage**: TaskProgressState uses only 64 bytes total
- **Stack-based**: Functions use only stack, no heap allocation
- **TFT text display**: Shows "Message: XX%" on TFT at configurable positions
- **Progress bar support**: Optional progress bar with configurable colors
- **Multiple modes**: Full state tracking or minimal one-call display

### RAM Usage

| Component | Size | Type |
|-----------|------|------|
| TaskProgressState | 32 bytes | Static global |
| Message buffer | 32 bytes | In state |
| Stack usage (per call) | ~100 bytes | Stack |
| **Total** | **~164 bytes** | No heap |

### API Reference

```c
#include "task_progress.h"

// Initialize with text only (minimum RAM)
task_progress_init(false, true);

// Initialize with custom position
task_progress_init_custom(10, 200, 10, 220, true, true);

// Start progress
task_progress_start("Converting PNG", 100);

// Update progress
task_progress_update(45);

// Minimal version (no state, no heap)
task_progress_minimal("Processing", 75);

// Clean up
task_progress_minimal_clear();
task_progress_done();

// TFT instance management
task_progress_set_tft(my_tft_pointer);
void* tft = task_progress_get_tft();
```

### Configuration Constants

```c
// Positions
#define PROGRESS_X 10
#define PROGRESS_Y 200
#define PROGRESS_BAR_X 10
#define PROGRESS_BAR_Y 220
#define PROGRESS_BAR_WIDTH 300
#define PROGRESS_BAR_HEIGHT 20

// Colors (16-bit RGB565)
#define PROGRESS_BG_COLOR 0x0000    // Black
#define PROGRESS_TEXT_COLOR 0xFFFF  // White
#define PROGRESS_BAR_COLOR 0x07E0   // Green
```

### Integration with Heavy Tasks

The PNG converter automatically uses task_progress_minimal() for progress display:

```c
// In png_converter_run():
// 1. Validating PNG: 0%, 50%, 100%
// 2. Allocating memory: 0%, 100%
// 3. Decoding PNG: 0-100% (updates every ~1%)
// 4. Saving results: 0%, 100%
```

### Files

| File | Purpose |
|------|---------|
| `src/boot/task_progress.h` | Header with API and state |
| `src/boot/task_progress.c` | Implementation with TFT functions |
| `docs/PNG_CONVERTER.md` | PNG converter with progress integration |

---

## Hardware Configuration

### Platform Detection

The system automatically detects the platform:

```c
#if defined(ESP8266)
    // ESP8266 specific configuration
    platform_config = GUIKIT_HW_ESP8266_DEFAULT;
#elif defined(ESP32)
    // ESP32 specific configuration
    platform_config = GUIKIT_HW_ESP32_DEFAULT;
#else
    // Generic configuration
    platform_config = GUIKIT_HW_DEFAULT;
#endif
```

### SPI Configuration

```c
guikit_spi_config_t spi_config = {
    .expander = false,           // SPI expander enabled
    .bank_count = 4,            // Number of SPI devices
    .bank = {
        // TFT
        {GUIKIT_SPI_DEVICE, GUIKIT_EXPANDER_NONE, 15, 255, true},  // D8
        // Touch
        {GUIKIT_SPI_DEVICE, GUIKIT_EXPANDER_NONE, 4, 255, true},   // D2
        // SD Card
        {GUIKIT_SPI_DEVICE, GUIKIT_EXPANDER_NONE, 5, 255, true},   // D1
        // External SRAM
        {GUIKIT_SPI_DEVICE, GUIKIT_EXPANDER_NONE, 16, 255, true}  // D0
    },
    .sck_pin = 14,              // D5
    .mosi_pin = 13,             // D7
    .miso_pin = 12,             // D6
    .max_speed_mhz = 20
};
```

### RAM Configuration

```c
guikit_ram_config_t ram_config = {
    .internal = true,            // Internal RAM available
    .bank_count = 1,            // Number of external RAM banks
    .bank = {
        // External SRAM
        {GUIKIT_RAM_SRAM, 131072, 16, true}  // 128KB at CS D0
    }
};
```

---

## GUI Loading

### Loading from File

```c
// Load GUI from SD card
MemoryStrategyResult result = gui_memory_strategy_load("/gui/main.json");

if (result.success) {
    printf("GUI loaded using: %s\n", 
           result.level == MEMORY_STRATEGY_EXTERNAL_RAM ? "External RAM" :
           result.level == MEMORY_STRATEGY_SD_SWAP ? "SD Card Swap" :
           "Internal RAM");
    printf("Memory used: %lu bytes\n", result.memory_used);
    printf("Storage location: %s\n", result.storage_location);
} else {
    printf("Failed to load GUI: %s\n", result.error_message);
}
```

### Loading from JSON String

```c
const char* json = "{\"type\":\"view\",\"widgets\":[]}";
MemoryStrategyResult result = gui_memory_strategy_load_json(json);
```

### Loading from WebDAV

```c
MemoryStrategyResult result = gui_memory_strategy_load_webdav("main.json");
```

### Forcing a Specific Strategy

```c
// Force External RAM
MemoryStrategyResult result = gui_memory_strategy_force(
    "/gui/main.json", 
    MEMORY_STRATEGY_EXTERNAL_RAM
);

// Force SD Card Swap
MemoryStrategyResult result = gui_memory_strategy_force(
    "/gui/main.json", 
    MEMORY_STRATEGY_SD_SWAP
);

// Force Internal RAM
MemoryStrategyResult result = gui_memory_strategy_force(
    "/gui/main.json", 
    MEMORY_STRATEGY_INTERNAL_RAM
);
```

---

## WebDAV Push Notifications

The WebDAV Push Notification System enables real-time notifications from the ESP8266/ESP32 device to Linux/macOS WebDAV clients. It supports multiple transport mechanisms and includes a comprehensive authentication system.

### Features

**Transport Mechanisms:**
- Server-Sent Events (SSE) - HTTP-based streaming
- WebSocket - Persistent bidirectional connections
- Long Polling - Legacy client compatibility

**Authentication:**
- Multiple auth methods: Basic (username/password), Token, Digest, Anonymous
- Permission system: READ, WRITE, ADMIN flags
- Session management: 16 max sessions, 5-minute timeout
- Token management: 32 max tokens, 24-hour default expiry
- Rate limiting: 60 notifications/minute per client
- Lockout after 5 failed attempts (1-minute duration)
- Default admin user: `admin`/`admin` with full permissions

**Event Types:**
- File created, modified, deleted, moved
- Folder created, deleted
- Project updates
- GUI updates
- System events
- Custom events

### Quick Start

```c
#include "webdav_push.h"
#include "webdav_push_auth.h"

// Initialize push system
webdav_push_init(NULL);

// Initialize authentication
webdav_push_auth_init(NULL);

// Register a client
uint32_t client_id = webdav_push_client_register(
    WEBDAV_PUSH_CLIENT_SSE,
    "/projects/my_project"
);

// Authenticate client
webdav_push_auth_login(client_id, "admin", "admin");

// Send notification
WebDAVPushNotification notif = {
    .type = WEBDAV_PUSH_FILE_CREATED,
    .timestamp = millis(),
    .path = "/projects/file.txt",
    .size = 1024,
};
webdav_push_notify(&notif);
```

### Authentication API

**User Management:**
```c
// Add user
webdav_push_user_add("username", "password", WEBDAV_PUSH_PERM_READ);

// Update permissions
webdav_push_user_update_permissions("username", WEBDAV_PUSH_PERM_ADMIN);

// Get user
WebDAVPushUser* user = webdav_push_user_get("username");
```

**Token Management:**
```c
// Generate token
const char* token = webdav_push_token_generate("username", WEBDAV_PUSH_PERM_READ, 86400000);

// Authenticate with token
webdav_push_auth_token(client_id, token);

// Validate token
bool valid = webdav_push_token_validate(token);

// Revoke token
webdav_push_token_revoke(token);
```

**Session Management:**
```c
// Create session
const char* session_id = webdav_push_session_create("username", WEBDAV_PUSH_PERM_READ);

// Check authentication
bool authenticated = webdav_push_auth_is_authenticated(client_id);

// Check permissions
bool can_read = webdav_push_auth_has_permission(client_id, WEBDAV_PUSH_PERM_READ);
```

**Rate Limiting:**
```c
// Check rate limit
if (webdav_push_rate_limit_check(client_id)) {
    // Rate limited
}

// Record notification
webdav_push_rate_limit_record(client_id);

// Get remaining
uint32_t remaining = webdav_push_rate_limit_remaining(client_id);
```

### Configuration

```c
// Push configuration
WebDAVPushConfig push_config = {
    .enabled = true,
    .port = 8080,
    .use_sse = true,
    .use_websocket = true,
    .use_long_poll = true,
};

// Auth configuration
WebDAVPushAuthConfig auth_config = {
    .require_auth = true,
    .default_method = WEBDAV_PUSH_AUTH_BASIC,
    .allow_anonymous = false,
    .use_session_tokens = true,
    .token_expiry = 86400000,  // 24 hours
};
```

### HTTP Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/webdav/push/sse` | SSE event stream |
| GET | `/webdav/push/ws` | WebSocket connection |
| GET | `/webdav/push/poll` | Long polling |
| POST | `/webdav/push/subscribe` | Subscribe to path |
| POST | `/webdav/push/unsubscribe` | Unsubscribe |

### Memory Usage

| Component | Size | Notes |
|-----------|------|-------|
| Push client state | ~300 bytes | Per client (8 max) |
| Auth state | ~100 bytes | Per client |
| Notification struct | ~600 bytes | Temporary |
| **Total (max)** | **~5KB** | All clients + sessions |

### Files

| File | Description |
|------|-------------|
| `src/gui_editor/server/webdav_push.h` | Push notification API |
| `src/gui_editor/server/webdav_push.c` | Push notification implementation |
| `src/gui_editor/server/webdav_push_auth.h` | Authentication API |
| `src/gui_editor/server/webdav_push_auth.c` | Authentication implementation |
| `docs/WEBDAV_PUSH.md` | Full documentation |

### Default Watched Paths

| Path | Recursive | Events |
|------|-----------|--------|
| `/gui` | Yes | File create/modify/delete |
| `/projects` | Yes | File create/modify/delete |
| `/tmp/task_comm` | No | File create |

---

## mDNS Service Discovery

The **mDNS (Multicast DNS / Bonjour / Zeroconf)** service enables ESP8266/ESP32 devices to be discovered on the local network using simple hostnames like `esp8266.local` without requiring DNS configuration or `/etc/hosts` entries.

### Overview

mDNS allows devices to advertise themselves and their services on the local network. Clients can then discover and connect to these services using human-readable names instead of IP addresses.

**Key Benefits:**
- Zero-configuration: No DNS server needed
- Automatic: Devices advertise themselves automatically
- Cross-platform: Works on Linux (avahi), macOS (Bonjour), Windows (Bonjour service)
- Service discovery: Find all available services and their metadata

### How It Works

1. ESP8266 connects to WiFi network
2. Device calls `mdns_init()` to start mDNS service
3. Device advertises itself as `[hostname].local` (default: `esp8266.local`)
4. Device advertises services: HTTP, WebDAV, GUIKit
5. Clients can query the network to discover the device and its IP address
6. Clients connect using `http://esp8266.local/`, `http://esp8266.local/webdav`, etc.

### Service Types

| Service Type | Port | Path | Description |
|--------------|------|------|-------------|
| `_http._tcp` | 80 | `/` | HTTP/Web server |
| `_webdav._tcp` | 80 | `/webdav` | WebDAV file server |
| `_guikit._tcp` | 8080 | `/gui` | GUIKit management |

Each service includes TXT records with metadata:
- `model` - Device model (e.g., "ESP8266 NodeMCU")
- `manufacturer` - Manufacturer (e.g., "Genose.org")
- `serial` - Serial number
- `version` - Firmware version
- `path` - Service path

### Quick Start

```c
#include "mdns_service.h"

void setup() {
    // 1. Connect to WiFi first
    WiFi.begin("SSID", "password");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    // 2. Initialize mDNS with defaults
    mdns_init(NULL);
    
    // Device is now discoverable as esp8266.local
    Serial.print("Device ready at: http://");
    Serial.println(mdns_get_fqdn());
}

void loop() {
    // Process mDNS events
    mdns_process();
}
```

### Configuration

```c
mDNSConfig config = {
    .hostname = "my-esp8266",      // -> my-esp8266.local
    .enable_http = true,           // Advertise HTTP
    .enable_webdav = true,        // Advertise WebDAV
    .enable_guikit = true,         // Advertise GUIKit
    .http_port = 80,
    .webdav_port = 80,
    .guikit_port = 8080,
    .model = "ESP8266 NodeMCU",
    .manufacturer = "Genose.org",
    .serial_number = "DEV-001",
    .version = "1.0.0"
};

mdns_init(&config);
```

### Hostname Management

```c
// Set custom hostname
mdns_set_hostname("my-device");

// Get current hostname
const char* name = mdns_get_hostname();

// Get Fully Qualified Domain Name
const char* fqdn = mdns_get_fqdn();  // e.g., "my-device.local"

// Validate hostname
if (mdns_is_valid_hostname("my-device")) {
    mdns_set_hostname("my-device");
}

// Generate unique hostname from MAC
char unique_name[64];
mdns_generate_unique_hostname("esp", unique_name);
```

### Custom Services

```c
// Add a custom service
mdns_add_service(
    "ESP8266-API",      // Instance name
    "_api._tcp",        // Service type
    8080,              // Port
    "/api",            // Path
    "api_version",     // TXT key
    "1.0"              // TXT value
);

// Add service with multiple TXT records
const char* txt_records[] = {
    "api_version", "1.0",
    "description", "REST API",
    NULL
};
mdns_add_service_with_txt("ESP8266-API", "_api._tcp", 8080, "/api", txt_records);

// Remove a service
mdns_remove_service("ESP8266-API", "_api._tcp");

// Remove all custom services
mdns_remove_all_services();
```

### Kernel Integration

```c
// In kernel setup
void kernel_setup() {
    // ... other initialization ...
    
    // After WiFi connection
    kernel_mdns_init("GUIKit-Device");
    
    // Get URLs
    char webdav_url[128];
    mdns_get_webdav_url(webdav_url, sizeof(webdav_url));
    // -> "http://GUIKit-Device.local/webdav"
}
```

### Client Discovery

**Linux (avahi-browser):**
```bash
# List all services
avahi-browse -a -r

# List HTTP services
avahi-browse -a -r _http._tcp

# List WebDAV services
avahi-browse -a -r _webdav._tcp

# List GUIKit services
avahi-browse -a -r _guikit._tcp
```

**macOS (dns-sd):**
```bash
# List all services
dns-sd -B _services._dns-sd._udp

# List HTTP services
dns-sd -B _http._tcp

# Resolve specific service
dns-sd -L "ESP8266" _http._tcp
```

**Windows:**
- Use Bonjour Browser utility
- Or install Bonjour SDK and use `dns-sd`

**Python:**
```python
from zeroconf import Zeroconf, ServiceBrowser

class MyListener:
    def add_service(self, zeroconf, type, name):
        print(f"Found: {name} ({type})")

zeroconf = Zeroconf()
listener = MyListener()
browser = ServiceBrowser(zeroconf, "_http._tcp", listener)

try:
    input("Press Enter to exit...")
finally:
    zeroconf.close()
```

### Memory Usage

| Component | Size | Notes |
|-----------|------|-------|
| mDNS state | ~500 bytes | Hostname, FQDN, config |
| Custom services | ~64 bytes each | Max 8 services |
| **Total (max)** | **~1.1KB** | All services |

### Files

| File | Description |
|------|-------------|
| `src/system/mdns_service.h` | Header with API |
| `src/system/mdns_service.c` | Implementation |
| `docs/MDNS_SERVICE.md` | Full documentation |

### Integration with GUIKit

mDNS is automatically initialized as part of Kernel.bin:

```c
// kernel_main.cpp
void setup() {
    // ... hardware init ...
    // ... WiFi connection ...
    
    // Initialize mDNS
    kernel_mdns_init("GUIKit-Device");
    
    // Start servers
    start_web_server();
    start_webdav_server();
    
    // Initialize push notifications
    webdav_push_init(NULL);
    webdav_push_auth_init(NULL);
}
```

### Troubleshooting

**Service not discoverable:**
1. Verify WiFi is connected before `mdns_init()`
2. Check firewall allows UDP port 5353 multicast
3. Verify mDNS is running: `mdns_is_running()`
4. Test with `avahi-browse -a -r` on Linux

**Name conflict:**
- Use unique hostnames for each device
- Include MAC address in hostname

**mDNS not starting:**
1. Check WiFi connection
2. Verify hostname is valid
3. Check for memory constraints

---

## Troubleshooting

### Common Issues

#### No Hardware Detected

**Symptoms:** All SPI devices show as "NOT Available"

**Solutions:**
1. Check SPI bus configuration (SCK, MOSI, MISO pins)
2. Verify CS pin connections for each device
3. Check power connections to peripheral devices
4. Verify device compatibility with voltage levels

#### Memory Strategy Not Working

**Symptoms:** GUI fails to load or uses wrong strategy

**Solutions:**
1. Check hardware detection results
2. Verify memory strategy configuration
3. Ensure thresholds are appropriate for your GUI sizes
4. Use `guikit_memory_strategy_config_print()` to debug

#### External RAM Not Detected

**Symptoms:** External SRAM/PSRAM shows as NOT Available

**Solutions:**
1. Check CS pin connection
2. Verify SRAM chip type and size
3. Check SPI bus speed (23LC1024 typically needs <= 20MHz)
4. Test with known-working SRAM chip

#### SD Card Swap Not Working

**Symptoms:** SD Card shows as available but swap fails

**Solutions:**
1. Verify SD card is properly formatted (FAT32)
2. Check SD card speed class (Class 10 recommended)
3. Ensure SD card has enough free space
4. Test with different SD card

#### TFT Display Not Working

**Symptoms:** TFT shows as detected but no display

**Solutions:**
1. Check TFT connections (CS, DC, RESET pins)
2. Verify TFT power supply (3.3V or 5V as required)
3. Check backlight enable pin
4. Test with known-working TFT library

### Debug Output

Enable debug output to see what's happening:

```c
// Enable debug mode in configuration
config.debug_mode = true;

// Print configuration
guikit_hw_config_print(&config);
guikit_memory_strategy_config_print(&config.memory_strategy);

// Print current strategy stats
uint32_t total_ram, used_ram, free_ram;
MemoryStrategyLevel strategy;
gui_memory_strategy_get_stats(&total_ram, &used_ram, &free_ram, &strategy);
```

---

## Command Reference

### Bootloader Commands

| Function | Description |
|----------|-------------|
| `bootloader_init(state, config)` | Initialize bootloader state |
| `guikit_bootloader_run(state)` | Run complete boot sequence |
| `configure_memory_strategy(state)` | Configure strategy based on hardware |
| `detect_spi_devices(state)` | Detect all SPI devices |
| `init_ram(state)` | Initialize RAM |
| `init_sd_card(state)` | Initialize SD card |
| `init_tft(state)` | Initialize TFT display |
| `test_memory_strategy(state)` | Test strategy with various GUI sizes |

### Memory Strategy Commands

| Function | Description |
|----------|-------------|
| `gui_memory_strategy_init(sram, sd, tft, config)` | Initialize memory strategy system |
| `gui_memory_strategy_load(filepath)` | Load GUI from file |
| `gui_memory_strategy_load_json(json)` | Load GUI from JSON string |
| `gui_memory_strategy_load_webdav(filename)` | Load GUI from WebDAV |
| `gui_memory_strategy_force(filepath, level)` | Force specific strategy |
| `gui_memory_strategy_select(size, sram_avail, sd_avail)` | Select strategy |
| `gui_memory_strategy_get_config_ptr()` | Get current config |
| `gui_memory_strategy_set_config(config)` | Set memory strategy config |
| `gui_memory_strategy_get_stats(...)` | Get memory usage stats |
| `gui_memory_strategy_get_location()` | Get current storage location |

### Configuration Commands

| Function | Description |
|----------|-------------|
| `guikit_hw_config_init(config)` | Initialize config with defaults |
| `guikit_hw_config_validate(config)` | Validate configuration |
| `guikit_hw_config_print(config)` | Print configuration |
| `guikit_hw_get_external_ram(config)` | Get total external RAM |
| `guikit_hw_get_total_ram(config)` | Get total RAM |
| `guikit_hw_count_expanders(config)` | Count SPI expanders |
| `guikit_hw_get_expander_gpio(config)` | Get total expander GPIO |

### Autostart Configuration Commands

| Function | Description |
|----------|-------------|
| `guikit_autostart_init_default(config)` | Initialize with default autostart config |
| `guikit_autostart_parse_ini(config, filepath)` | Parse INI file into config |
| `guikit_autostart_save_ini(config, filepath)` | Save config to INI file |
| `guikit_autostart_validate(config)` | Validate autostart configuration |
| `guikit_autostart_detect_banks(config)` | Detect available memory banks |
| `guikit_autostart_load_kernel(config, buffer, size)` | Load kernel using autostart config |
| `guikit_autostart_boot(config)` | Run complete autostart boot process |
| `guikit_autostart_alloc(config, size, name)` | Allocate memory using autostart strategy |
| `guikit_autostart_free(config, ptr, name)` | Free memory |
| `guikit_autostart_apply_strategy(config, size)` | Apply memory strategy |
| `guikit_autostart_bank_name(bank)` | Get bank name string |
| `guikit_autostart_strategy_name(strategy)` | Get strategy name string |
| `guikit_autostart_print_config(config)` | Print autostart config |

### Task Switcher Commands

| Function | Description |
|----------|-------------|
| `task_switcher_init()` | Initialize task switcher with defaults |
| `task_switcher_init_with_config(config)` | Initialize with custom configuration |
| `task_switch_to(path)` | Freeze current task, load and run new task |
| `task_restore()` | Free current task, restore parent from SD card |
| `task_has_frozen_parent()` | Check if a parent task is frozen |
| `task_get_frozen_parent(info)` | Get info about frozen parent task |
| `task_get_current(info)` | Get info about current task |
| `task_switcher_get_state()` | Get current task switcher state |
| `task_switcher_get_error()` | Get last error code |
| `task_switcher_error_to_string(err)` | Get error message string |
| `task_switcher_has_enough_ram(percent)` | Check if enough RAM available |
| `task_switcher_get_free_ram_percent()` | Get current free RAM percentage |
| `task_switcher_cleanup_frozen()` | Clean up frozen task file |
| `task_save_for_parent(filename, data, size)` | Save data for parent task to read |
| `task_load_from_child(filename, buffer, size)` | Load data saved by child task |
| `task_child_has_file(filename)` | Check if child saved a file |

### Task Progress Commands

| Function | Description |
|----------|-------------|
| `task_progress_get_state()` | Get global progress state pointer |
| `task_progress_init(show_bar, show_text)` | Initialize progress display |
| `task_progress_init_custom(x, y, bar_x, bar_y, show_bar, show_text)` | Initialize with custom position |
| `task_progress_done()` | Shutdown and clear progress display |
| `task_progress_start(message, max)` | Start progress with message and max value |
| `task_progress_update(current)` | Update progress to current value |
| `task_progress_update_msg(current, message)` | Update progress with new message |
| `task_progress_inc()` | Increment progress by 1 |
| `task_progress_complete()` | Complete progress (show 100%) |
| `task_progress_text(message, percent)` | Show simple text progress |
| `task_progress_text_clear()` | Clear text progress |
| `task_progress_minimal(message, percent)` | Minimal progress (no state, stack only) |
| `task_progress_minimal_clear()` | Clear minimal progress |
| `task_progress_set_tft(tft)` | Set TFT instance for progress display |
| `task_progress_get_tft()` | Get current TFT instance |

### WebDAV Push Commands

| Function | Description |
|----------|-------------|
| `webdav_push_init(config)` | Initialize push notification system |
| `webdav_push_shutdown()` | Shutdown push notification system |
| `webdav_push_notify(notif)` | Send notification to all clients |
| `webdav_push_notify_client(client_id, notif)` | Send to specific client |
| `webdav_push_client_register(type, path)` | Register a client |
| `webdav_push_client_unregister(client_id)` | Unregister a client |
| `webdav_push_watch_add(path, recursive, events)` | Watch a path |
| `webdav_push_watch_remove(path)` | Stop watching a path |
| `webdav_push_check_changes()` | Check for changes |

### WebDAV Push Authentication Commands

| Function | Description |
|----------|-------------|
| `webdav_push_auth_init(config)` | Initialize authentication system |
| `webdav_push_auth_shutdown()` | Shutdown authentication system |
| `webdav_push_auth_required()` | Check if auth is required |
| `webdav_push_auth_login(client_id, user, pass)` | Login with username/password |
| `webdav_push_auth_token(client_id, token)` | Login with token |
| `webdav_push_auth_logout(client_id)` | Logout a client |
| `webdav_push_auth_is_authenticated(client_id)` | Check if authenticated |
| `webdav_push_auth_has_permission(client_id, perm)` | Check permissions |
| `webdav_push_user_add(user, pass, perms)` | Add a user |
| `webdav_push_user_remove(user)` | Remove a user |
| `webdav_push_user_get(user)` | Get user by username |
| `webdav_push_user_update_password(user, pass)` | Update password |
| `webdav_push_user_update_permissions(user, perms)` | Update permissions |
| `webdav_push_session_create(user, perms)` | Create a session |
| `webdav_push_session_destroy(session_id)` | Destroy a session |
| `webdav_push_session_cleanup()` | Cleanup expired sessions |
| `webdav_push_token_generate(user, perms, expiry)` | Generate a token |
| `webdav_push_token_validate(token)` | Validate a token |
| `webdav_push_token_revoke(token)` | Revoke a token |
| `webdav_push_token_revoke_all(user)` | Revoke all user tokens |
| `webdav_push_rate_limit_check(client_id)` | Check rate limit |
| `webdav_push_rate_limit_record(client_id)` | Record notification |
| `webdav_push_rate_limit_remaining(client_id)` | Get remaining count |

### mDNS Service Commands

| Function | Description |
|----------|-------------|
| `mdns_init(config)` | Initialize mDNS service (NULL for defaults) |
| `mdns_shutdown()` | Shutdown mDNS service |
| `mdns_is_running()` | Check if mDNS is active |
| `mdns_reinit(config)` | Restart mDNS with new configuration |
| `mdns_set_hostname(name)` | Set device hostname |
| `mdns_get_hostname()` | Get current hostname |
| `mdns_get_fqdn()` | Get FQDN (hostname.local) |
| `mdns_is_valid_hostname(name)` | Validate hostname format |
| `mdns_generate_unique_hostname(base, buf)` | Generate unique hostname |
| `mdns_add_service(instance, type, port, path, key, val)` | Add custom service |
| `mdns_add_service_with_txt(...)` | Add service with TXT records |
| `mdns_remove_service(instance, type)` | Remove a service |
| `mdns_remove_all_services()` | Remove all custom services |
| `mdns_get_service_info(index, info)` | Get service information |
| `mdns_process()` | Process mDNS events (call in loop) |
| `mdns_get_error()` | Get last error code |
| `mdns_error_to_string(err)` | Convert error code to string |
| `kernel_mdns_init(name)` | Initialize mDNS for kernel |
| `mdns_get_webdav_url(buf, size)` | Get WebDAV discovery URL |
| `mdns_get_guikit_url(buf, size)` | Get GUIKit discovery URL |

---

## File Reference

### Documentation

- `README.md` - Main project readme
- `HELP.md` - This help guide
- `docs/memory_strategy_config.md` - Memory strategy configuration
- `docs/ARCHITECTURE.md` - System architecture
- `docs/HARDWARE.md` - Hardware setup
- `docs/SOFTWARE.md` - Software components
- `docs/NETWORK.md` - Network architecture
- `docs/WEBDAV_PUSH.md` - WebDAV push notification system
- `docs/MDNS_SERVICE.md` - mDNS service discovery (Bonjour/Zeroconf)
- `src/boot/README.md` - Bootloader documentation
- `etc/GUIKIT_autostart.ini` - Example autostart configuration file
- `docs/TASK_SWITCHER.md` - Task switcher documentation
- `docs/PNG_CONVERTER.md` - PNG converter documentation

### Source Files

#### Bootloader
- `src/boot/guikit_bootloader.h` - Bootloader header
- `src/boot/guikit_bootloader.cpp` - Bootloader implementation
- `src/boot/guikit_autostart_config.h` - Autostart configuration header
- `src/boot/guikit_autostart_config.cpp` - Autostart configuration implementation
- `src/boot/ini_parser.h` - INI parser header
- `src/boot/ini_parser.c` - INI parser implementation
- `src/boot/task_switcher.h` - Task switcher header
- `src/boot/task_switcher.c` - Task switcher implementation
- `src/boot/task_switcher_example.h` - Task switcher example header
- `src/boot/task_switcher_example.c` - Task switcher example implementation
- `src/boot/task_progress.h` - Task progress display header
- `src/boot/task_progress.c` - Task progress display implementation

#### Memory Strategy
- `src/gui/gui_memory_strategy.h` - Memory strategy header
- `src/gui/gui_memory_strategy.cpp` - Memory strategy implementation
- `src/guikit_hw_config.h` - Hardware configuration header
- `src/guikit_hw_config.cpp` - Hardware configuration implementation
- `src/guikit_hw_config_union.h` - Union-based configuration

#### GUI
- `src/gui/*.h/cpp` - GUI framework files
- `src/gui/demo_huge_gui_result.txt` - Huge GUI results demonstration

#### WebDAV Push
- `src/gui_editor/server/webdav_push.h` - Push notification header
- `src/gui_editor/server/webdav_push.c` - Push notification implementation
- `src/gui_editor/server/webdav_push_auth.h` - Authentication header
- `src/gui_editor/server/webdav_push_auth.c` - Authentication implementation

#### mDNS Service
- `src/system/mdns_service.h` - mDNS header with API
- `src/system/mdns_service.c` - mDNS implementation

---

## Version History

- **Latest**: mDNS service discovery (Bonjour/Zeroconf) for device auto-discovery
- **Latest**: WebDAV push notification authentication system (Basic, Token, Digest, Anonymous)
- **Latest**: Task switcher for single-level context switching (A -> B -> back to A)
- **Latest**: JPEG to RGB conversion example with task switching
- **Latest**: Autostart configuration from /etc/GUIKIT_autostart.ini
- **Latest**: INI parser for embedded systems
- **Latest**: Memory strategy with STOP-at-first-success behavior
- **Latest**: Bootloader with automatic hardware detection
- **Latest**: RAM freeze/thaw system for fast boot
- **Latest**: Config struct for memory strategy configuration

---

## Support

For issues, questions, or contributions:

1. Check this help guide
2. Check the documentation in `docs/`
3. Review the example configurations
4. Enable debug output for troubleshooting
5. Check the test files for usage examples

---

*Generated by Mistral Vibe*
*Date: 2026-08-16*
