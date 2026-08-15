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
  ./help.sh webdav-push        - WebDAV push notification information
  ./help.sh all                - Show all documentation

--------------------------------------------------------------------------------

Available Commands:

  memory       - Show memory strategy documentation
  bootloader   - Show bootloader documentation
  config       - Show hardware configuration documentation
  gui          - Show GUI loading documentation
  huge-gui     - Show huge GUI (500KB) results
  webdav-push  - Show WebDAV push notification documentation
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
  docs/ARCHITECTURE.md          - System architecture
  docs/HARDWARE.md              - Hardware setup
  docs/SOFTWARE.md              - Software components
  docs/NETWORK.md               - Network architecture
  docs/WEBDAV_PUSH.md           - WebDAV push notification system
  src/boot/README.md            - Bootloader documentation
  src/gui/demo_huge_gui_result.txt - Huge GUI results

================================================================================
EOF

echo ""
case "$1" in
    "memory"|"m")
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
        echo ""
        echo "See guikit_hw_config.h for details"
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
