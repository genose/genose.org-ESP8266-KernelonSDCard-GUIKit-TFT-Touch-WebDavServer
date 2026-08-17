#!/bin/bash

# GUIKit Help Script
# Display help information for the GUIKit project

cat << 'EOF'
================================================================================
                    GUIKit - Help Guide
================================================================================

Usage:
  ./help.sh                    - Show this help message
  ./help.sh memory             - Memory strategy information
  ./help.sh bootloader         - Bootloader information
  ./help.sh config             - Configuration information
  ./help.sh gui                - GUI loading information
  ./help.sh multicore          - Multi-core architecture documentation
  ./help.sh ram-test           - RAM length detection information
  ./help.sh webdav-push        - WebDAV push notification information
  ./help.sh mdns               - mDNS service discovery information
  ./help.sh ram                - RAM chip models information
  ./help.sh costs              - Kernel functionality costs and priorities
  ./help.sh all                - Show all documentation

--------------------------------------------------------------------------------

Available Commands:

  memory       - Show memory strategy documentation
  bootloader   - Show bootloader documentation
  config       - Show hardware configuration documentation
  gui          - Show GUI loading documentation
  multicore    - Show multi-core architecture documentation
  ram-test     - Show RAM length detection documentation
  huge-gui     - Show huge GUI (500KB) results
  webdav-push  - Show WebDAV push notification documentation
  mdns         - Show mDNS service discovery documentation
  ram          - Show RAM chip models documentation
  costs        - Show kernel functionality costs and priorities
  all          - Show all documentation

--------------------------------------------------------------------------------

Quick Start:

  1. Build the project:
     $ pio run

  2. Upload bootloader to ESP8266/ESP32

  3. Copy kernel.bin to SD card

  4. Power on the device

--------------------------------------------------------------------------------

Documentation Files:

  README.md                    - Main project documentation
  HELP.md                      - Detailed help guide
  docs/memory_strategy_config.md - Memory strategy configuration
  docs/KERNEL_FUNCTIONALITY_COSTS.md - Kernel functionality RAM costs & priorities
  docs/ARCHITECTURE.md          - System architecture
  docs/HARDWARE.md              - Hardware setup
  docs/SOFTWARE.md              - Software components
  docs/NETWORK.md               - Network architecture
  docs/WEBDAV_PUSH.md           - WebDAV push notification system
  docs/MDNS_SERVICE.md          - mDNS service discovery (Bonjour/Zeroconf)
  about_ram_expansion.md        - RAM chip models & expansion guide
  src/boot/README.md            - Bootloader documentation
  src/gui/demo_huge_gui_result.txt - Huge GUI results

================================================================================
EOF

echo ""
case "$1" in
    "mdns"|"m")
        cat docs/MDNS_SERVICE.md
        ;;
    "memory")
        cat docs/memory_strategy_config.md
        ;;
    "bootloader"|"b")
        cat src/boot/README.md
        ;;
    "config"|"c")
        echo "Hardware Configuration Documentation"
        echo "======================================="
        echo ""
        echo "Configuration is defined in guikit_hw_config.h"
        echo ""
        echo "Main struct: guikit_hw_config_t"
        echo "  - Platform info (is_esp8266, is_esp32)"
        echo "  - RAM configuration (ram internal, bank[])"
        echo "  - SPI configuration (spi expander, bank[])"
        echo "  - Display configuration"
        echo "  - Memory strategy configuration"
        echo ""
        echo "Presets:"
        echo "  - GUIKIT_HW_DEFAULT"
        echo "  - GUIKIT_HW_ESP8266_DEFAULT"
        echo "  - GUIKIT_HW_ESP32_DEFAULT"
        echo "  - GUIKIT_HW_ESP8266_HUGE_DEMO"
        echo "  - GUIKIT_HW_ESP8266_EXPANDER"
        echo "  - GUIKIT_HW_ESP32_PREMIUM"
        echo "  - GUIKIT_HW_ESP8266_LY68L6400 (512KB Lyontek SRAM)"
        echo "  - GUIKIT_HW_ESP8266_CY15V104QSN (512KB Cypress FRAM)"
        echo "  - GUIKIT_HW_ESP32_ISSI_64MB_PSRAM (64MB ISSI PSRAM)"
        echo "  - GUIKIT_HW_ESP32_LY68L6400 (512KB Lyontek SRAM)"
        echo ""
        echo "See guikit_hw_config.h for details"
        ;;
    "multicore"|"multi")
        cat docs/MULTICORE_ARCHITECTURE.md
        ;;
    "ram-test"|"ramtest")
        echo "RAM Length Detection Documentation"
        echo "==================================="
        echo ""
        echo "The RAM Length Detection system tests actual RAM size at boot to detect"
        echo "wiring errors (e.g., a 64K chip wired as 256K)."
        echo ""
        echo "Configuration: [ram_test] section in /etc/GUIKIT_autostart.ini"
        echo ""
        echo "Key Features:"
        echo "  - Binary search detection for efficiency"
        echo "  - 1-2 test passes (single or double pattern verification)"
        echo "  - Progress display on TFT"
        echo "  - Wiring error detection with 'WTM: X wired!' warnings"
        echo "  - Works with all supported RAM chip models"
        echo ""
        echo "See HELP.md for complete documentation"
        ;;
    "ram"|"r")
        echo "================================================================================"
        echo "                    RAM Chip Models - Supported Chips"
        echo "================================================================================"
        echo ""
        echo "All models work with both ESP8266 (via SPI) and ESP32 (via SPI/native):"
        echo ""
        echo "SPI SRAM:"
        echo "  - 23LC512 (64 KB, 20 MHz) - Budget"
        echo "  - 23LC1024 (128 KB, 20 MHz) - RECOMMENDED"
        echo "  - 23LCV1024 (128 KB, 20 MHz) - Low-voltage"
        echo "  - Lyontek LY68L6400 (512 KB, 50 MHz) - Large cache"
        echo ""
        echo "FRAM (Non-Volatile):"
        echo "  - MB85RS256B (32 KB, 20 MHz) - Basic"
        echo "  - CY15V102QN (128 KB, 40 MHz) - Industrial"
        echo "  - CY15V104QSN (512 KB, 40 MHz) - Industrial"
        echo ""
        echo "PSRAM (ESP32 Native):"
        echo "  - APS6404 (1 MB, 40 MHz) - Entry-level"
        echo "  - APS1604 (2 MB, 40 MHz) - Mid-range"
        echo "  - APS3204 (4 MB, 40 MHz) - High-capacity"
        echo "  - W9812G6KH (8 MB, 80 MHz) - Maximum"
        echo "  - ISSI IS66WVS5128ALL (64 MB, 100 MHz) - Industrial"
        echo "  - ISSI IS66WVS5128BLL (64 MB, 100 MHz) - Industrial"
        echo ""
        echo "Configuration Presets in guikit_hw_config.h:"
        echo "  - GUIKIT_HW_ESP8266_LY68L6400"
        echo "  - GUIKIT_HW_ESP8266_CY15V104QSN"
        echo "  - GUIKIT_HW_ESP32_ISSI_64MB_PSRAM"
        echo "  - GUIKIT_HW_ESP32_LY68L6400"
        echo ""
        echo "See about_ram_expansion.md for full specifications"
        ;;
    "costs"|"c")
        echo "==============================================================================="
        echo "              Kernel Functionality Costs & Priority Documentation"
        echo "==============================================================================="
        echo ""
        echo "Complete breakdown of RAM consumption for each kernel functionality:"
        echo ""
        echo "Key Priorities:"
        echo "  - GUIKit Core: External RAM first (performance critical)"
        echo "  - Image Converters: Internal -> External -> Swap (decode speed priority)"
        echo "  - Other components: Follow STOP-at-first-success strategy"
        echo ""
        echo "See docs/KERNEL_FUNCTIONALITY_COSTS.md for complete tables and analysis"
        ;;
    "gui"|"g")
        echo "GUI Loading Documentation"
        echo "=========================="
        echo ""
        echo "Memory Strategy (STOP at first success):"
        echo "  1. Try External RAM -> if (available AND fits) => SELECT & STOP"
        echo "  2. Try SD Card Swap -> if (available AND fits) => SELECT & STOP"
        echo "  3. Try Internal RAM -> if (fits) => SELECT & STOP"
        echo "  4. Else => FAILED"
        echo ""
        echo "Loading Functions:"
        echo "  - gui_memory_strategy_load(filepath)"
        echo "  - gui_memory_strategy_load_json(json)"
        echo "  - gui_memory_strategy_load_webdav(filename)"
        echo "  - gui_memory_strategy_force(filepath, level)"
        echo ""
        ;;
    "huge-gui"|"h")
        cat src/gui/demo_huge_gui_result.txt
        ;;
    "webdav-push"|"webdav"|"w")
        cat docs/WEBDAV_PUSH.md
        ;;
    "all"|"a")
        echo "Displaying all documentation..."
        echo ""
        cat README.md
        echo ""
        echo "================================================================================"
        echo ""
        cat HELP.md
        ;;
    ""|"--help"|"-h")
        # Already shown above
        ;;
    *)
        if [ -n "$1" ]; then
            echo "Unknown command: $1"
            echo ""
        fi
        echo "Run './help.sh --help' for available commands"
        ;;
esac
