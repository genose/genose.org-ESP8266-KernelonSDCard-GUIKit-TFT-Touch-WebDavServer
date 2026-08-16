# Memory Strategy Configuration

## Overview

The GUIKit memory strategy system provides hierarchical memory management for GUI data:

1. **External RAM (23LC1024 SRAM)** - Primary storage for large GUIs
2. **SD Card Swap** - Secondary storage for very large GUIs using streaming/partial loading
3. **Internal RAM** - Fallback storage for small GUIs

## Strategy Flow

```
GUI Load Request
    ↓
Is External RAM available AND GUI size >= external_ram_min_size?
    ↓ YES
Use External RAM (if fits within external_ram_max_size)
    ↓ NO
    ↓
Is SD Card available AND GUI size >= sd_swap_min_size?
    ↓ YES
Use SD Card Swap
    ↓ NO
    ↓
Is GUI size <= internal_ram_max_size?
    ↓ YES
Use Internal RAM
    ↓ NO
ERROR: All strategies failed
```

## Configuration Struct

The memory strategy configuration is defined in `memory_strategy_config_t` within `guikit_hw_config.h`:

```c
typedef struct {
    // Thresholds for strategy selection
    uint32_t external_ram_min_size;    // Min GUI size to use external RAM (default: 4096)
    uint32_t sd_swap_min_size;        // Min GUI size to use SD swap (default: 16384)
    uint32_t internal_ram_max_size;   // Max GUI size for internal RAM (default: 8192)
    
    // External RAM limits
    uint32_t external_ram_max_size;   // Max size for external RAM storage (default: 131072 = 128KB)
    uint32_t external_ram_base_addr;   // Base address in external RAM (default: 0)
    
    // SD Card swap configuration
    uint16_t sd_swap_block_size;      // SD card sector size (default: 512)
    uint16_t sd_swap_cache_size;      // Cache size for streaming (default: 2048)
    
    // Strategy behavior flags
    bool use_external_ram_by_default; // Try external RAM first if available
    bool use_sd_swap_by_default;     // Try SD swap if external RAM unavailable
    bool check_memory_before_load;   // Validate memory before loading
    bool display_error_on_tft;       // Display errors on TFT screen
    
    // Current/active settings (runtime)
    MemoryStrategyLevel current_strategy;
    bool strategy_initialized;
    
} memory_strategy_config_t;
```

## Default Configuration

The default configuration is defined as:

```c
#define GUIKIT_MEMORY_STRATEGY_DEFAULT \
{
    .external_ram_min_size = 4096,           // 4KB minimum for external RAM
    .sd_swap_min_size = 16384,              // 16KB minimum for SD swap
    .internal_ram_max_size = 8192,          // 8KB maximum for internal RAM
    .external_ram_max_size = 131072,        // 128KB for 23LC1024
    .external_ram_base_addr = 0,             // Start at beginning of SRAM
    .sd_swap_block_size = 512,             // SD card sector size
    .sd_swap_cache_size = 2048,            // 2KB cache for streaming
    .use_external_ram_by_default = true,   // Try external RAM first
    .use_sd_swap_by_default = true,        // Try SD swap if external unavailable
    .check_memory_before_load = true,      // Validate memory before loading
    .display_error_on_tft = true,          // Display errors on TFT
    .current_strategy = MEMORY_STRATEGY_INTERNAL_RAM,
    .strategy_initialized = false
}
```

## Integration with Main Config

The memory strategy configuration is integrated into the main hardware configuration:

```c
typedef struct {
    // Platform info
    bool is_esp8266;
    bool is_esp32;
    
    // RAM configuration
    guikit_ram_config_t ram;
    
    // SPI configuration
    guikit_spi_config_t spi;
    
    // Display configuration
    guikit_display_config_t display;
    
    // Memory strategy configuration
    memory_strategy_config_t memory_strategy;
    
    // System flags
    bool use_sd_card;
    bool use_webdav;
    bool debug_mode;
    
} guikit_hw_config_t;
```

## Usage Examples

### Basic Usage with Defaults

```c
#include "guikit_hw_config.h"
#include "gui_memory_strategy.h"

// Initialize with defaults
guikit_hw_config_t config = GUIKIT_HW_ESP8266_DEFAULT;

// Initialize memory strategy
gui_memory_strategy_init(sram, sdcard, tft, &config.memory_strategy);
```

### Custom Configuration

```c
#include "guikit_hw_config.h"
#include "gui_memory_strategy.h"

// Create custom configuration
guikit_hw_config_t config = GUIKIT_HW_ESP8266_DEFAULT;

// Customize memory strategy
config.memory_strategy.external_ram_min_size = 8192;    // 8KB min for external RAM
config.memory_strategy.sd_swap_min_size = 32768;       // 32KB min for SD swap
config.memory_strategy.internal_ram_max_size = 4096;    // 4KB max for internal RAM
config.memory_strategy.use_external_ram_by_default = true;

// Initialize memory strategy with custom config
gui_memory_strategy_init(sram, sdcard, tft, &config.memory_strategy);
```

### Runtime Configuration Change

```c
// Get current config
const memory_strategy_config_t* current_config = gui_memory_strategy_get_config_ptr();

// Create modified config
memory_strategy_config_t new_config = *current_config;
new_config.external_ram_max_size = 262144;  // 256KB for larger SRAM

// Update configuration
gui_memory_strategy_set_config(&new_config);
```

## Configuration Presets

Several configuration presets are available:

### ESP8266 Default

```c
const guikit_hw_config_t GUIKIT_HW_ESP8266_DEFAULT;
```

### ESP32 Default

```c
const guikit_hw_config_t GUIKIT_HW_ESP32_DEFAULT;
```

### ESP8266 with 1MB SRAM (Huge Demo)

```c
#define GUIKIT_HW_ESP8266_HUGE_DEMO \
{ \
    .is_esp8266 = true, \
    .is_esp32 = false, \
    .ram = { \
        .internal = true, \
        .bank_count = 8, \
        .bank = { \
            {GUIKIT_RAM_SRAM, 131072, 16, true},  // Bank 0 on D0 \
            {GUIKIT_RAM_SRAM, 131072, 0, true},   // Bank 1 \
            // ... 6 more banks \
        } \
    }, \
    // ... other config \
    .memory_strategy = GUIKIT_MEMORY_STRATEGY_DEFAULT,
    // ...
}
```

## Helper Functions

### Configuration Management

```c
// Initialize config with platform defaults
void guikit_hw_config_init(guikit_hw_config_t* cfg);

// Initialize memory strategy config with defaults
void guikit_memory_strategy_config_init(memory_strategy_config_t* cfg);

// Validate configuration
bool guikit_hw_config_validate(const guikit_hw_config_t* cfg);
bool guikit_memory_strategy_config_validate(const memory_strategy_config_t* cfg);
```

### Strategy Selection Helpers

```c
// Check if memory strategy should use external RAM for given GUI size
bool guikit_memory_strategy_should_use_external_ram(const memory_strategy_config_t* cfg, uint32_t gui_size);

// Check if memory strategy should use SD swap for given GUI size
bool guikit_memory_strategy_should_use_sd_swap(const memory_strategy_config_t* cfg, uint32_t gui_size);

// Check if memory strategy should use internal RAM for given GUI size
bool guikit_memory_strategy_should_use_internal_ram(const memory_strategy_config_t* cfg, uint32_t gui_size);

// Select memory strategy based on GUI size and configuration
MemoryStrategyLevel guikit_memory_strategy_select(const memory_strategy_config_t* cfg,
                                                    uint32_t gui_size, 
                                                    bool sram_available, 
                                                    bool sdcard_available);
```

### Utility Functions

```c
// Get total external RAM size
uint32_t guikit_hw_get_external_ram(const guikit_hw_config_t* cfg);

// Get total RAM (internal + external)
uint32_t guikit_hw_get_total_ram(const guikit_hw_config_t* cfg);

// Count number of SPI expander chips
uint8_t guikit_hw_count_expanders(const guikit_hw_config_t* cfg);

// Get total GPIO from expanders
uint16_t guikit_hw_get_expander_gpio(const guikit_hw_config_t* cfg);

// Print configuration summary
void guikit_hw_config_print(const guikit_hw_config_t* cfg);
void guikit_memory_strategy_config_print(const memory_strategy_config_t* cfg);
```

## Memory Strategy in GUI Loading

The memory strategy is integrated into the GUI loading process:

```c
// Load GUI using automatic strategy selection
MemoryStrategyResult result = gui_memory_strategy_load(filepath);

// Check which strategy was used
if (result.success) {
    switch (result.level) {
        case MEMORY_STRATEGY_EXTERNAL_RAM:
            printf("GUI loaded into External RAM\n");
            break;
        case MEMORY_STRATEGY_SD_SWAP:
            printf("GUI loaded using SD Card Swap\n");
            break;
        case MEMORY_STRATEGY_INTERNAL_RAM:
            printf("GUI loaded into Internal RAM\n");
            break;
        default:
            break;
    }
}
```

## Error Handling

The memory strategy system provides error handling through callbacks and TFT display:

```c
// Set error callback
void gui_memory_strategy_set_error_callback(void (*callback)(const char* message));

// Display error on TFT
void gui_memory_strategy_display_error(TFT_ST7789* tft, const MemoryStrategyResult* result);

// Get current configuration as string
const char* gui_memory_strategy_get_config();

// Get current GUI storage location
const char* gui_memory_strategy_get_location();
```

## Migration from Hardcoded Values

Previously, memory strategy thresholds were hardcoded as `#define` macros:

```c
// Old way (deprecated)
#define MEMORY_STRATEGY_EXTERNAL_RAM_MIN_SIZE 4096
#define MEMORY_STRATEGY_SD_SWAP_MIN_SIZE 16384
#define MEMORY_STRATEGY_INTERNAL_RAM_MAX_SIZE (8 * 1024)
#define GUI_EXTERNAL_RAM_MAX_SIZE (128 * 1024)
```

### New Way (Recommended)

```c
// New way (config-based)
memory_strategy_config_t config = GUIKIT_MEMORY_STRATEGY_DEFAULT;
// Customize as needed
config.external_ram_min_size = 4096;
config.sd_swap_min_size = 16384;
config.internal_ram_max_size = 8192;
config.external_ram_max_size = 131072;
```

### Backward Compatibility

For backward compatibility, the old macros are still available but now reference the configuration:

```c
#define MEMORY_STRATEGY_EXTERNAL_RAM_MIN_SIZE (gui_memory_strategy_get_config_ptr() ? \
    gui_memory_strategy_get_config_ptr()->external_ram_min_size : 4096)
```

These macros will be deprecated in future versions.

## Best Practices

1. **Use the config struct** for all new code
2. **Initialize configuration early** in the application startup
3. **Validate configuration** using `guikit_hw_config_validate()`
4. **Use appropriate thresholds** based on your hardware capabilities
5. **Test with different GUI sizes** to ensure strategy selection works correctly
6. **Monitor memory usage** using the stats functions

## Hardware Considerations

### ESP8266

- Internal RAM: ~80KB available for application
- External RAM: 23LC1024 provides 128KB per chip
- Multiple chips can be used for larger storage
- SPI bus speed: typically 20 MHz max

### ESP32

- Internal RAM: 320KB+ available
- External PSRAM: up to 8MB available
- SPI bus speed: up to 40 MHz
- More flexible memory configuration

### Memory Requirements

| GUI Complexity | Approximate Size | Recommended Strategy |
|---------------|-----------------|---------------------|
| Simple UI | < 8KB | Internal RAM |
| Medium UI | 8KB - 16KB | External RAM or SD Swap |
| Complex UI | 16KB - 128KB | External RAM |
| Huge UI | > 128KB | SD Card Swap |

## RAM Length Detection

The memory strategy system relies on accurate RAM size information. The bootloader includes a RAM length detection system that verifies the actual size of external RAM to prevent wiring errors.

### Purpose

The RAM length detection ensures that:
1. The reported RAM size matches the actual physical RAM
2. Wiring errors are detected (e.g., a 64KB chip wired as 256KB)
3. Memory strategy uses correct RAM sizes for decision making
4. Users are warned via "WTM: X wired!" messages when mismatches occur

### Configuration

RAM length detection is configured via `/etc/GUIKIT_autostart.ini`:

```ini
[ram_test]
; Enable RAM length detection at boot
enabled = true

; Number of test passes (1 or 2)
; 1 = Single pattern test (faster)
; 2 = Double pattern test (more reliable, detects wiring errors)
test_passes = 2

; Timeout in milliseconds
timeout_ms = 5000

; Show progress on TFT during test
show_progress = true

; Stop boot on test failure
stop_on_failure = false

; Expected sizes for each RAM bank (0 = auto-detect)
; If detected size doesn't match, a WTM warning is shown
bank_0 = 0
bank_1 = 0
```

### Impact on Memory Strategy

When RAM length detection is enabled:
- **Accurate Sizes**: Memory strategy uses verified RAM sizes instead of assumed values
- **Wiring Error Prevention**: Prevents using incorrect RAM sizes that could cause memory corruption
- **Better Decision Making**: Strategy selection is based on actual available memory

If RAM length detection is disabled or fails, the system falls back to hardware-reported sizes (from SPI device enumeration).

## Troubleshooting

### Common Issues

1. **GUI too large for internal RAM**: Increase `internal_ram_max_size` or use external RAM
2. **External RAM not detected**: Check SPI configuration and hardware connections
3. **SD Card swap not working**: Verify SD card is properly initialized and accessible
4. **Strategy selection not working**: Check configuration thresholds and available hardware

### Debugging

Enable debug mode and print configuration:

```c
config.debug_mode = true;
guikit_hw_config_print(&config);
```

### Error Messages

Common error messages and their meanings:

- `"External RAM not configured"`: No SRAM instance provided
- `"SD card not available"`: SD card not present or not initialized
- `"GUI too large for internal RAM"`: GUI exceeds `internal_ram_max_size`
- `"Cannot allocate buffer for SD swap"`: Not enough memory for SD swap cache
- `"All memory strategies failed"`: No suitable strategy found for the GUI size
