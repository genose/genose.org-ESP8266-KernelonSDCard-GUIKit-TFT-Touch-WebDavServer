# GUIKit Bootloader

## Overview

The GUIKit bootloader performs hardware detection and memory strategy initialization at system boot. It automatically detects available hardware (External RAM, SPI devices, SD Card, TFT, Touch) and configures the appropriate memory strategy.

## Features

- **Automatic Hardware Detection**: Detects all SPI devices including SRAM, PSRAM, SD Card, TFT, Touch controllers, and GPIO expanders
- **Memory Strategy Configuration**: Dynamically configures memory strategy based on detected hardware
- **TFT Display Support**: Shows boot progress and results on the TFT display
- **Error Handling**: Comprehensive error detection and reporting
- **Platform Support**: Works with both ESP8266 and ESP32

## Hardware Detection Flow

```
Boot Start
    ↓
1. Hardware Detection
   ├── Detect SPI Bus
   ├── Detect SRAM (23LC1024, etc.)
   ├── Detect PSRAM (ESP32)
   ├── Detect SD Card
   ├── Detect TFT Display
   ├── Detect Touch Controller
   └── Detect SPI Expanders (MCP23S17, etc.)
    ↓
2. RAM Initialization
   ├── Initialize Internal RAM
   ├── Initialize External SRAM
   └── Initialize PSRAM
    ↓
3. SD Card Initialization
    ↓
4. TFT Initialization
    ↓
5. Memory Strategy Configuration
   ├── Determine available RAM
   ├── Set thresholds based on hardware
   └── Configure strategy flags
    ↓
6. Memory Strategy Test & Apply
   ├── Test with various GUI sizes
   ├── Initialize memory strategy system
   └── Verify configuration
    ↓
7. Display Results (on TFT if available)
    ↓
Boot Complete
```

## Memory Strategy Configuration Logic

The bootloader automatically configures the memory strategy based on what hardware is detected:

### External RAM Detection

```c
// If SRAM or PSRAM is detected:
if (hw->sram_detected || hw->psram_detected) {
    uint32_t total_external = hw->sram_size + hw->psram_size;
    
    // Use external RAM for GUIs larger than 4KB
    cfg->external_ram_min_size = 4096;
    
    // Set max based on available external RAM
    cfg->external_ram_max_size = total_external;
    
    // If lots of external RAM (> 512KB), use it for larger GUIs
    if (total_external > 512 * 1024) {
        cfg->external_ram_min_size = 16384;  // > 16KB
    }
    
    cfg->use_external_ram_by_default = true;
} else {
    // No external RAM - disable it
    cfg->use_external_ram_by_default = false;
    cfg->external_ram_min_size = UINT32_MAX;
}
```

### SD Card Detection

```c
// If SD Card is detected:
if (hw->sdcard_detected) {
    cfg->use_sd_swap_by_default = true;
    
    // Platform-specific thresholds
    #if defined(ESP8266)
        cfg->sd_swap_min_size = 8192;  // > 8KB on ESP8266
    #elif defined(ESP32)
        cfg->sd_swap_min_size = 32768; // > 32KB on ESP32
    #else
        cfg->sd_swap_min_size = 16384; // > 16KB default
    #endif
} else {
    // No SD Card - disable SD swap
    cfg->use_sd_swap_by_default = false;
    cfg->sd_swap_min_size = UINT32_MAX;
}
```

### Internal RAM Configuration

```c
// Platform-specific internal RAM limits
#if defined(ESP8266)
    cfg->internal_ram_max_size = 65536;  // 64KB max
#elif defined(ESP32)
    cfg->internal_ram_max_size = 262144; // 256KB max
#else
    cfg->internal_ram_max_size = 8192;   // 8KB default
#endif

// SD swap cache size based on platform
#if defined(ESP8266)
    cfg->sd_swap_cache_size = 2048;  // 2KB cache
#elif defined(ESP32)
    cfg->sd_swap_cache_size = 8192;  // 8KB cache
#else
    cfg->sd_swap_cache_size = 4096;  // 4KB default
#endif
```

## Usage

### Basic Usage

```c
#include "guikit_bootloader.h"

int main() {
    // Initialize bootloader with default platform configuration
    BootloaderState state;
    bootloader_init(&state, NULL);
    
    // Run bootloader
    if (guikit_bootloader_run(&state)) {
        // Boot successful
        // state.config contains the complete hardware configuration
        // state.memory_config contains the memory strategy configuration
        // state.selected_strategy is the default strategy for this hardware
        
        // Now you can use the GUIKit with the configured memory strategy
        gui_init(&state.config);
        
    } else {
        // Boot failed
        printf("Boot failed: %s\n", state.error_message);
    }
    
    return 0;
}
```

### Custom Configuration

```c
#include "guikit_bootloader.h"

int main() {
    // Create custom platform configuration
    guikit_hw_config_t platform_config = GUIKIT_HW_ESP8266_DEFAULT;
    
    // Customize SPI configuration
    platform_config.spi.bank_count = 4;
    platform_config.spi.bank[0].type = GUIKIT_SPI_DEVICE;
    platform_config.spi.bank[0].cs_pin = 15;  // TFT at D8
    platform_config.spi.bank[0].enabled = true;
    
    platform_config.spi.bank[1].type = GUIKIT_SPI_DEVICE;
    platform_config.spi.bank[1].cs_pin = 4;   // Touch at D2
    platform_config.spi.bank[1].enabled = true;
    
    platform_config.spi.bank[2].type = GUIKIT_SPI_DEVICE;
    platform_config.spi.bank[2].cs_pin = 5;   // SD at D1
    platform_config.spi.bank[2].enabled = true;
    
    platform_config.spi.bank[3].type = GUIKIT_SPI_DEVICE;
    platform_config.spi.bank[3].cs_pin = 16;  // SRAM at D0
    platform_config.spi.bank[3].enabled = true;
    
    // Initialize bootloader with custom configuration
    BootloaderState state;
    bootloader_init(&state, &platform_config);
    
    // Run bootloader
    guikit_bootloader_run(&state);
    
    // Use the configured system
    // ...
}
```

### Accessing Detected Hardware

```c
BootloaderState state;
bootloader_init(&state, NULL);
guikit_bootloader_run(&state);

// Check what hardware was detected
if (state.hardware.sram_detected) {
    printf("SRAM detected: %s, %lu KB at CS %d\n",
           state.hardware.sram_type,
           state.hardware.sram_size / 1024,
           state.hardware.sram_cs_pin);
}

if (state.hardware.tft_detected) {
    printf("TFT detected: %dx%d at CS %d\n",
           state.hardware.tft_width,
           state.hardware.tft_height,
           state.hardware.tft_cs_pin);
}

// Check memory strategy configuration
printf("Memory Strategy:\n");
printf("  External RAM min: %lu KB\n", 
       state.memory_config.external_ram_min_size / 1024);
printf("  SD Swap min: %lu KB\n", 
       state.memory_config.sd_swap_min_size / 1024);
printf("  Internal RAM max: %lu KB\n", 
       state.memory_config.internal_ram_max_size / 1024);
```

## Example Output

### ESP8266 with External SRAM (128KB)

```
========================================
GUIKit Bootloader Starting
========================================

[BOOT] Step 1/6: Hardware Detection
[SPI] Initializing: SCK=14, MOSI=13, MISO=12, Speed=20 MHz
[SPI] SRAM detected at CS 16: 23LC1024, 128 KB
[SPI] SD Card detected at CS 5
[SPI] TFT detected at CS 15: 320x240
[SPI] Touch detected at CS 4
[BOOT] Hardware detection complete
  SRAM: Yes, PSRAM: No, SD Card: Yes, TFT: Yes, Touch: Yes
  SPI Expanders: 0, GPIO: 0

[BOOT] Step 2/6: RAM Initialization
[RAM] SRAM initialized: 128 KB at CS 16
[BOOT] RAM initialization complete
  Available RAM: 208 KB (Internal: ~80 KB, External: 128 KB)

[BOOT] Step 3/6: SD Card Initialization
[SD] SD Card initialized at CS 5
[BOOT] SD Card initialization complete
  SD Card: Ready

[BOOT] Step 4/6: TFT Initialization
[TFT] TFT initialized: 320x240 at CS 15
[BOOT] TFT initialization complete
  TFT: Ready

[BOOT] Step 5/6: Memory Strategy Configuration
[BOOT] Memory strategy configured
  Strategy thresholds:
    External RAM min: 4 KB
    SD Swap min: 8 KB
    Internal RAM max: 64 KB
  Flags:
    Use External RAM: Yes
    Use SD Swap: Yes
    Check Memory: Yes
    Display Errors: Yes

[BOOT] Step 6/6: Memory Strategy Test and Apply

[MEMORY STRATEGY] Testing with detected hardware:
  External RAM: Yes (128 KB)
  SD Card: Yes
  Internal RAM: ~208 KB

  GUI Size:     1 KB -> Strategy: Internal RAM
  GUI Size:     4 KB -> Strategy: External RAM
  GUI Size:     8 KB -> Strategy: External RAM
  GUI Size:    16 KB -> Strategy: SD Card Swap
  GUI Size:    32 KB -> Strategy: SD Card Swap
  GUI Size:    64 KB -> Strategy: SD Card Swap
  GUI Size:   128 KB -> Strategy: SD Card Swap
  GUI Size:   256 KB -> Strategy: FAILED

[BOOT] Memory strategy initialized
  Current Strategy: Internal RAM

========================================
GUIKit Bootloader Complete
========================================

Boot Summary:
  Platform: ESP8266
  RAM: 208 KB total (80 KB internal, 128 KB external)
  SD Card: Available
  TFT: Available (320x240)
  Touch: Available
  SPI Expanders: 0 (0 GPIO)

Memory Strategy:
  Selected: Internal RAM
  Thresholds:
    External RAM: > 4 KB
    SD Swap: > 8 KB
    Internal RAM: < 64 KB
```

### ESP8266 without External RAM

```
========================================
GUIKit Bootloader Starting
========================================

[BOOT] Step 1/6: Hardware Detection
[SPI] Initializing: SCK=14, MOSI=13, MISO=12, Speed=20 MHz
[SPI] SD Card detected at CS 5
[SPI] TFT detected at CS 15: 320x240
[SPI] Touch detected at CS 4
[BOOT] Hardware detection complete
  SRAM: No, PSRAM: No, SD Card: Yes, TFT: Yes, Touch: Yes
  SPI Expanders: 0, GPIO: 0

[BOOT] Step 2/6: RAM Initialization
[BOOT] RAM initialization complete
  Available RAM: 80 KB (Internal: ~80 KB, External: 0 KB)

[BOOT] Step 3/6: SD Card Initialization
[SD] SD Card initialized at CS 5
[BOOT] SD Card initialization complete
  SD Card: Ready

[BOOT] Step 4/6: TFT Initialization
[TFT] TFT initialized: 320x240 at CS 15
[BOOT] TFT initialization complete
  TFT: Ready

[BOOT] Step 5/6: Memory Strategy Configuration
[BOOT] Memory strategy configured
  Strategy thresholds:
    External RAM min: 4294967295 KB
    SD Swap min: 8 KB
    Internal RAM max: 64 KB
  Flags:
    Use External RAM: No
    Use SD Swap: Yes
    Check Memory: Yes
    Display Errors: Yes

[BOOT] Step 6/6: Memory Strategy Test and Apply

[MEMORY STRATEGY] Testing with detected hardware:
  External RAM: No (0 KB)
  SD Card: Yes
  Internal RAM: ~80 KB

  GUI Size:     1 KB -> Strategy: Internal RAM
  GUI Size:     4 KB -> Strategy: Internal RAM
  GUI Size:     8 KB -> Strategy: SD Card Swap
  GUI Size:    16 KB -> Strategy: SD Card Swap
  GUI Size:    32 KB -> Strategy: SD Card Swap
  GUI Size:    64 KB -> Strategy: SD Card Swap
  GUI Size:   128 KB -> Strategy: FAILED
  GUI Size:   256 KB -> Strategy: FAILED

[BOOT] Memory strategy initialized
  Current Strategy: Internal RAM

========================================
GUIKit Bootloader Complete
========================================
```

### ESP32 with PSRAM (8MB)

```
========================================
GUIKit Bootloader Starting
========================================

[BOOT] Step 1/6: Hardware Detection
[SPI] Initializing: SCK=18, MOSI=23, MISO=19, Speed=40 MHz
[SPI] PSRAM detected at CS 255: 8192 KB
[SPI] SD Card detected at CS 22
[SPI] TFT detected at CS 5: 320x240
[SPI] Touch detected at CS 21
[BOOT] Hardware detection complete
  SRAM: No, PSRAM: Yes, SD Card: Yes, TFT: Yes, Touch: Yes
  SPI Expanders: 0, GPIO: 0

[BOOT] Step 2/6: RAM Initialization
[RAM] PSRAM initialized: 8192 KB
[BOOT] RAM initialization complete
  Available RAM: 8512 KB (Internal: ~320 KB, External: 8192 KB)

[BOOT] Step 3/6: SD Card Initialization
[SD] SD Card initialized at CS 22
[BOOT] SD Card initialization complete
  SD Card: Ready

[BOOT] Step 4/6: TFT Initialization
[TFT] TFT initialized: 320x240 at CS 5
[BOOT] TFT initialization complete
  TFT: Ready

[BOOT] Step 5/6: Memory Strategy Configuration
[BOOT] Memory strategy configured
  Strategy thresholds:
    External RAM min: 16 KB
    SD Swap min: 32 KB
    Internal RAM max: 256 KB
  Flags:
    Use External RAM: Yes
    Use SD Swap: Yes
    Check Memory: Yes
    Display Errors: Yes

[BOOT] Step 6/6: Memory Strategy Test and Apply

[MEMORY STRATEGY] Testing with detected hardware:
  External RAM: Yes (8192 KB)
  SD Card: Yes
  Internal RAM: ~8512 KB

  GUI Size:     1 KB -> Strategy: Internal RAM
  GUI Size:     4 KB -> Strategy: Internal RAM
  GUI Size:     8 KB -> Strategy: Internal RAM
  GUI Size:    16 KB -> Strategy: External RAM
  GUI Size:    32 KB -> Strategy: External RAM
  GUI Size:    64 KB -> Strategy: External RAM
  GUI Size:   128 KB -> Strategy: External RAM
  GUI Size:   256 KB -> Strategy: SD Card Swap

[BOOT] Memory strategy initialized
  Current Strategy: Internal RAM

========================================
GUIKit Bootloader Complete
========================================
```

## TFT Display Output

When a TFT display is available, the bootloader shows a visual summary:

```
+--------------------------------------+
| GUIKit Bootloader                    |
|                                      |
| Hardware Detected:                   |
|   RAM: 80 KB Int, 128 KB Ext         |
|   SD Card: Yes                       |
|   TFT: 320x240                       |
|   Touch: Yes                         |
|                                      |
| Memory Strategy:                     |
|   Strategy: External RAM             |
|   External > 4 KB                    |
|   SD Swap > 8 KB                     |
|   Internal < 64 KB                   |
|                                      |
| Boot Complete!                        |
+--------------------------------------+
```

## Customizing Hardware Detection

Replace the mock detection functions with actual hardware code:

```c
// Example: Real SRAM detection for 23LC1024
bool detect_spi_sram(uint8_t cs_pin, uint32_t* size, const char** type) {
    // Initialize SPI at this CS pin
    SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
    digitalWrite(cs_pin, LOW);
    
    // Send READ command (0x03 for 23LC1024)
    uint8_t cmd = 0x03;
    uint8_t high_addr = 0x00;
    uint8_t low_addr = 0x00;
    SPI.transfer(cmd);
    SPI.transfer(high_addr);
    SPI.transfer(low_addr);
    
    // Try to read a byte
    uint8_t data = SPI.transfer(0x00);
    digitalWrite(cs_pin, HIGH);
    SPI.endTransaction();
    
    // If we got a response, assume SRAM is present
    if (/* valid response */) {
        if (size) *size = 131072;  // 128KB for 23LC1024
        if (type) *type = "23LC1024";
        return true;
    }
    
    return false;
}
```

## Files

- `src/boot/guikit_bootloader.h` - Bootloader header file
- `src/boot/guikit_bootloader.cpp` - Bootloader implementation
- `src/boot/README.md` - This file

## Integration with Main Application

The bootloader can be integrated into your main application as follows:

```c
#include "guikit_bootloader.h"

BootloaderState boot_state;

void setup() {
    // Run bootloader
    bootloader_init(&boot_state, NULL);
    if (!guikit_bootloader_run(&boot_state)) {
        // Handle boot error
        while (1) {
            // Blink error LED or display error on TFT
        }
    }
    
    // Initialize GUIKit with the configured memory strategy
    gui_init(&boot_state.config);
    
    // Load the default GUI
    gui_load("/gui/default.json");
}

void loop() {
    // Main application loop
    gui_update();
}
```

## Platform-Specific Notes

### ESP8266

- Limited internal RAM (~80KB free)
- SPI bus typically on HSPI (D5=SCK, D7=MOSI, D6=MISO)
- Common CS pins: D0-D8
- Recommended to use external SRAM for GUIs > 8KB

### ESP32

- More internal RAM (~320KB free)
- Supports PSRAM (up to 8MB)
- Multiple SPI buses available
- Can handle larger GUIs in internal RAM
- SD swap recommended for GUIs > 32KB

## Troubleshooting

### No Hardware Detected

1. Check SPI bus configuration (SCK, MOSI, MISO pins)
2. Verify CS pin connections
3. Check power connections to peripheral devices
4. Verify device compatibility

### Memory Strategy Not Working

1. Check that hardware detection is successful
2. Verify memory strategy configuration with `guikit_memory_strategy_config_print()`
3. Ensure thresholds are appropriate for your GUI sizes

### TFT Display Not Working

1. Check TFT connections (CS, DC, RESET pins)
2. Verify TFT power supply
3. Check backlight enable pin (if applicable)
4. Test with known-working TFT library

## API Reference

See `guikit_bootloader.h` for complete API documentation.
