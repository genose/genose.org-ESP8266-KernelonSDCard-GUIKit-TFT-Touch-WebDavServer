# GUIKit Help Guide

## Table of Contents
1. [Quick Start](#quick-start)
2. [Memory Strategy](#memory-strategy)
3. [Bootloader](#bootloader)
4. [Autostart Configuration](#autostart-configuration)
5. [Hardware Configuration](#hardware-configuration)
6. [GUI Loading](#gui-loading)
7. [Troubleshooting](#troubleshooting)
8. [Command Reference](#command-reference)

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
- `src/boot/README.md` - Bootloader documentation
- `etc/GUIKIT_autostart.ini` - Example autostart configuration file

### Source Files

#### Bootloader
- `src/boot/guikit_bootloader.h` - Bootloader header
- `src/boot/guikit_bootloader.cpp` - Bootloader implementation
- `src/boot/guikit_autostart_config.h` - Autostart configuration header
- `src/boot/guikit_autostart_config.cpp` - Autostart configuration implementation
- `src/boot/ini_parser.h` - INI parser header
- `src/boot/ini_parser.c` - INI parser implementation

#### Memory Strategy
- `src/gui/gui_memory_strategy.h` - Memory strategy header
- `src/gui/gui_memory_strategy.cpp` - Memory strategy implementation
- `src/guikit_hw_config.h` - Hardware configuration header
- `src/guikit_hw_config.cpp` - Hardware configuration implementation
- `src/guikit_hw_config_union.h` - Union-based configuration

#### GUI
- `src/gui/*.h/cpp` - GUI framework files
- `src/gui/demo_huge_gui_result.txt` - Huge GUI results demonstration

---

## Version History

- **Latest**: Autostart configuration from /etc/GUIKIT_autostart.ini
- **Latest**: INI parser for embedded systems
- **Latest**: Memory strategy with STOP-at-first-success behavior
- **Latest**: Bootloader with automatic hardware detection
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
*Date: 2026-08-15*
