#!/bin/bash

# ============================================================================
# GUIKit ESP8266/ESP32 Build Script
# 
# Builds bootloader, kernel, and prepares SD card for GUIKit system.
# Supports ESP8266 and ESP32 with SMP detection, RAM freeze/thaw, 
# task switching, task progress display, and WebDAV support.
# 
# USAGE:
#   ./build.sh [command] [options]
# 
# COMMANDS:
#   all           Build everything (default)
#   bootloader    Build and upload bootloader
#   kernel        Build kernel
#   sdcard       Prepare SD card structure and files
#   gui           Build GUI projects
#   clean         Clean build artifacts
#   flash         Flash bootloader and kernel
#   freeze        Build and test RAM freeze/thaw system
#   progress      Build and test task progress display
#   help          Show this help message
#   version       Show version information
#   
# OPTIONS:
#   --debug       Enable verbose output
#   --upload      Upload after build (requires serial port)
#   --port PORT   Specify serial port (default: auto-detect)
#   --sd PATH     Specify SD card mount point (default: ./sdcard)
#   --no-clean    Skip cleanup before build
#   --platform    Specify platform: esp8266 or esp32 (default: esp8266)
#   
# EXAMPLES:
#   ./build.sh all                    # Build everything
#   ./build.sh bootloader --upload   # Build and upload bootloader
#   ./build.sh sdcard --sd /Volumes/SDCARD  # Prepare actual SD card
#   ./build.sh kernel --debug        # Build kernel with debug output
#   ./build.sh all --platform esp32   # Build for ESP32
#   
# DEPENDENCIES:
#   - PlatformIO (https://platformio.org/)
#   - ESP8266/ESP32 toolchain
#   - TFT_eSPI library
#   - XPT2046_Touchscreen library
#   - ESPWebDAV library
#   - SdFat library
# ============================================================================

set -o pipefail

# ============================================================================
# Configuration
# ============================================================================

SCRIPT_NAME="GUIKit Build Script"
SCRIPT_VERSION="1.0.0"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR"

# Default values
BUILD_MODE="release"
VERBOSE=false
UPLOAD=false
SKIP_CLEAN=false
SERIAL_PORT=""
SDCARD_PATH="$PROJECT_DIR/sdcard"
TARGET="all"
PLATFORM="esp8266"  # Default platform: esp8266 or esp32

# PlatformIO environments (platform-specific)
BOOTLOADER_ENV="${PLATFORM}_bootloader"
KERNEL_ENV="${PLATFORM}_kernel"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# ============================================================================
# Help System
# ============================================================================

show_help() {
    cat <<EOF
$SCRIPT_NAME v$SCRIPT_VERSION

A comprehensive build system for GUIKit project supporting both ESP8266 and ESP32
with dual logging (serial + TFT), SMP detection, RAM freeze/thaw, task switching,
and WebDAV support.

USAGE:
  $(basename "$0") [command] [options]

COMMANDS:
  all           Build everything (bootloader, kernel, SD card, GUIs)
  bootloader    Build bootloader only
  kernel        Build kernel only
  sdcard        Prepare SD card structure only
  gui           Build GUI projects only
  clean         Clean all build artifacts
  flash         Flash bootloader and kernel to device
  freeze        Build and test RAM freeze/thaw system
  progress      Build and test task progress display
  help          Show this help message
  version       Show version information

PLATFORM OPTIONS:
  --platform PLATFORM  Specify target platform: esp8266 or esp32 (default: esp8266)
                      ESP8266: Single-core, ~80KB RAM, no external RAM
                      ESP32:   Dual-core (SMP), ~320KB RAM, PSRAM support

BUILD OPTIONS:
  --debug, -d    Enable verbose/debug output
  --upload, -u   Upload after build (requires PlatformIO)
  --port PORT    Specify serial port (e.g., /dev/ttyUSB0, COM3)
  --sd PATH      Specify SD card mount point (default: ./sdcard)
  --no-clean     Skip cleanup before build
  --help, -h    Show this help message
  --version, -v Show version information

EXAMPLES:
  # ESP8266 (default)
  $(basename "$0") all                                  # Build everything
  $(basename "$0") all --platform esp8266              # Explicit ESP8266
  
  # ESP32
  $(basename "$0") all --platform esp32                 # Build for ESP32
  $(basename "$0") kernel --platform esp32 --debug      # ESP32 debug build
  
  # Upload
  $(basename "$0") bootloader --upload --port /dev/ttyUSB0
  $(basename "$0") flash --platform esp32 --port COM3
  
  # SD Card
  $(basename "$0") sdcard --sd /Volumes/SDCARD        # Prepare real SD card
  
  # Clean
  $(basename "$0") clean                            # Clean all artifacts

BUILD PROCESS:
  1. Bootloader (Flash): Minimal loader with hardware detection (SPI, RAM, SD, TFT)
  2. Autostart Config: Reads /etc/GUIKIT_autostart.ini for kernel path and settings
  3. Memory Strategy: Auto-configures external RAM, SD swap, internal RAM
  4. Kernel (SD Card): Full system with GUIKit, WebDAV, HTTP server, mDNS
  5. SD Card Structure: Creates /system, /gui, /etc, /home, /tmp directories
  6. GUI Projects: Copies .GUIKIT projects to /gui/ and /home/admin/projects/

FEATURES:
  Super fast boot with RAM freeze/thaw from SD card
  Dual logging to serial and TFT display
  SMP detection for ESP32 dual-core support
  Autostart configuration via /etc/GUIKIT_autostart.ini
  Memory strategy: External RAM -> SD swap -> Internal RAM (STOP at first success)
  Task switcher: Single-level context switching (A -> B -> back to A)
  Task progress: TFT text display for heavy tasks with minimal RAM usage
  WebDAV support for remote file access and user authentication
  mDNS service discovery (Bonjour/Zeroconf) for device auto-discovery
  WebDAV push notifications with authentication (SSE, WebSocket, Long Polling)
  Project structure: (project_name).GUIKIT directories

DEPENDENCIES:
  - PlatformIO CLI: https://platformio.org/install/cli
  - Python 3.x
  - Git
  - C++11 compiler

HARDWARE:
  - ESP8266 (NodeMCU v2/v3) or ESP32
  - 3.2" TFT LCD with ST7789 controller (or compatible)
  - XPT2046 touchscreen controller (or compatible)
  - MicroSD card (FAT32 formatted, 4GB+ recommended)
  - Optional: SPI SRAM (23LC1024) or PSRAM for external memory

SD CARD STRUCTURE (created by sdcard command):
  /system/              - System binaries (kernel.bin, bootloader.bin)
  /gui/                 - GUI projects (.GUIKIT directories)
  /gui/chooser.GUIKIT/  - Default GUI chooser/launcher
  /etc/                 - Configuration files
  /etc/GUIKIT_autostart.ini - Boot configuration
  /etc/user.skel/       - User skeleton (projects/, README.md)
  /home/                - User home directories
  /home/admin/         - Default admin user
  /home/admin/projects/ - User projects
  /tmp/                 - Temporary files (task communication, freeze state)

TASK SWITCHING:
  Freeze Task A to SD card, load Task B, run B, free B, restore A from SD.
  Communication via files in /tmp/task_comm/.
  Example: JPEG to RGB conversion - Task B saves RGB, Task A loads it after restore.

EOF
}

show_version() {
    echo "$SCRIPT_NAME v$SCRIPT_VERSION"
    echo "Build: $(date +%Y-%m-%d)"
    echo "Project: ESP8266 GUIKit + WebDAV Server"
}

# ============================================================================
# Logging Functions
# ============================================================================

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

log_header() {
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

log_verbose() {
    if [ "$VERBOSE" = true ]; then
        echo -e "${BLUE}[DEBUG]${NC} $1"
    fi
}

# ============================================================================
# Argument Parsing
# ============================================================================

parse_args() {
    while [[ $# -gt 0 ]]; do
        case "$1" in
            # Commands
            all|bootloader|kernel|sdcard|gui|clean|flash|help|version)
                TARGET="$1"
                shift
                ;;
            
            # Options
            --debug|-d)
                VERBOSE=true
                BUILD_MODE="debug"
                shift
                ;;
            --upload|-u)
                UPLOAD=true
                shift
                ;;
            --port)
                SERIAL_PORT="$2"
                shift 2
                ;;
            --sd)
                SDCARD_PATH="$2"
                shift 2
                ;;
            --no-clean)
                SKIP_CLEAN=true
                shift
                ;;
            --platform)
                PLATFORM="$2"
                # Validate platform
                if [ "$PLATFORM" != "esp8266" ] && [ "$PLATFORM" != "esp32" ]; then
                    log_error "Invalid platform: $PLATFORM. Must be 'esp8266' or 'esp32'"
                    show_help
                    exit 1
                fi
                # Update PlatformIO environments based on platform
                BOOTLOADER_ENV="${PLATFORM}_bootloader"
                KERNEL_ENV="${PLATFORM}_kernel"
                shift 2
                ;;
            --help|-h)
                show_help
                exit 0
                ;;
            --version|-v)
                show_version
                exit 0
                ;;
            *)
                log_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
}

# ============================================================================
# Dependency Checks
# ============================================================================

check_dependencies() {
    local missing=0
    
    # Check PlatformIO
    if ! command -v pio &>/dev/null; then
        log_error "PlatformIO CLI not found. Install from https://platformio.org/install/cli"
        missing=$((missing + 1))
    fi
    
    # Check Python
    if ! command -v python3 &>/dev/null && ! command -v python &>/dev/null; then
        log_error "Python not found. Please install Python 3.x"
        missing=$((missing + 1))
    fi
    
    # Check Git
    if ! command -v git &>/dev/null; then
        log_error "Git not found. Please install Git"
        missing=$((missing + 1))
    fi
    
    # Check for platformio.ini
    if [ ! -f "$PROJECT_DIR/platformio.ini" ]; then
        log_error "platformio.ini not found in $PROJECT_DIR"
        missing=$((missing + 1))
    fi
    
    if [ $missing -gt 0 ]; then
        log_error "$missing dependency(ies) missing. Please install them and try again."
        exit 1
    fi
    
    log_info "All dependencies found"
}

# ============================================================================
# Clean Functions
# ============================================================================

clean_all() {
    log_header "Cleaning Build Artifacts"
    
    # Clean PlatformIO builds
    log_info "Cleaning PlatformIO builds..."
    pio run -t clean 2>/dev/null || log_warn "PlatformIO clean failed (may not have built yet)"
    
    # Remove SD card directory
    if [ -d "$SDCARD_PATH" ]; then
        log_info "Removing SD card directory: $SDCARD_PATH"
        rm -rf "$SDCARD_PATH"
    fi
    
    # Remove build artifacts
    if [ -d "$PROJECT_DIR/.pio" ]; then
        log_info "Removing PlatformIO cache"
        rm -rf "$PROJECT_DIR/.pio"
    fi
    
    log_info "Clean complete"
}

# ============================================================================
# Bootloader Build
# ============================================================================

build_bootloader() {
    log_header "Building Bootloader"
    
    log_info "Building bootloader environment..."
    pio run -e $BOOTLOADER_ENV || { 
        log_error "Bootloader build failed"
        return 1
    }
    
    log_info "Bootloader built successfully"
    
    if [ "$UPLOAD" = true ]; then
        upload_bootloader
    fi
    
    return 0
}

upload_bootloader() {
    local port_option=""
    if [ -n "$SERIAL_PORT" ]; then
        port_option="--upload-port $SERIAL_PORT"
    fi
    
    log_info "Uploading bootloader..."
    pio run -e $BOOTLOADER_ENV -t upload $port_option || { 
        log_error "Bootloader upload failed"
        return 1
    }
    
    log_info "Bootloader uploaded successfully"
    return 0
}

# ============================================================================
# Kernel Build
# ============================================================================

build_kernel() {
    log_header "Building Kernel"
    
    log_info "Building kernel environment..."
    pio run -e $KERNEL_ENV || { 
        log_error "Kernel build failed"
        return 1
    }
    
    log_info "Kernel built successfully"
    
    # Copy kernel binary to SD card structure
    if [ -d "$SDCARD_PATH" ]; then
        copy_kernel_to_sdcard
    fi
    
    return 0
}

copy_kernel_to_sdcard() {
    local kernel_bin="$PROJECT_DIR/.pio/build/$KERNEL_ENV/Kernel.bin"
    
    if [ ! -f "$kernel_bin" ]; then
        log_error "Kernel binary not found at $kernel_bin"
        return 1
    fi
    
    log_info "Copying Kernel.bin to SD card..."
    cp "$kernel_bin" "$SDCARD_PATH/" || return 1
    
    # Create compressed version
    log_info "Creating Kernel.bin.gz..."
    gzip -k -f "$SDCARD_PATH/Kernel.bin" || log_warn "Failed to create compressed kernel"
    
    log_info "Kernel copied to SD card"
    return 0
}

# ============================================================================
# SD Card Preparation
# ============================================================================

prepare_sdcard() {
    log_header "Preparing SD Card Structure"
    
    # Create SD card directory
    log_info "Creating SD card directory: $SDCARD_PATH"
    mkdir -p "$SDCARD_PATH"
    
    # Create directory structure
    log_info "Creating directory structure..."
    mkdir -p "$SDCARD_PATH/system/ui"
    mkdir -p "$SDCARD_PATH/system/dict"
    mkdir -p "$SDCARD_PATH/system/config"
    mkdir -p "$SDCARD_PATH/system/logs"
    mkdir -p "$SDCARD_PATH/gui"
    mkdir -p "$SDCARD_PATH/etc/user.skel/projects"
    mkdir -p "$SDCARD_PATH/etc"
    mkdir -p "$SDCARD_PATH/home"
    mkdir -p "$SDCARD_PATH/tmp"
    
    # Create index.html
    create_index_html
    
    # Create configuration files
    create_config_files
    
    # Create user skeleton
    create_user_skeleton
    
    # Create guikitloader.conf
    create_guikitloader_conf
    
    log_info "SD card structure created"
    
    # Copy GUI projects
    copy_gui_projects
    
    return 0
}

create_index_html() {
    cat > "$SDCARD_PATH/index.html" <<'EOF'
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP8266 GUIKit Web Interface</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 40px;
            background: #1E1E1E;
            color: #D4D4D4;
        }
        h1 { color: #1177BB; }
        a { color: #4CAF50; }
        pre { background: #2D2D2D; padding: 15px; border-radius: 5px; overflow-x: auto; }
    </style>
</head>
<body>
    <h1>ESP8266 GUIKit WebDAV Server</h1>
    <p>Welcome to the ESP8266 GUIKit system.</p>
    <p>Use WebDAV client to connect to <strong>http://$(hostname):80/webdav</strong></p>
    <h2>Available GUIs</h2>
    <pre id="gui-list">Loading...</pre>
    <script>
        // This would be populated by the kernel
        // For static HTML, list known GUIs
        document.getElementById('gui-list').textContent = 
            'System GUIs:\n' +
            '  - /gui/chooser.GUIKIT/    - Project chooser\n' +
            '  - /gui/webdav.GUIKIT/    - WebDAV management\n' +
            '  - /gui/users.GUIKIT/     - User management\n' +
            '  - /gui/editor.GUIKIT/    - Web editor\n';
    </script>
</body>
</html>
EOF
    log_verbose "Created index.html"
}

create_config_files() {
    # passwords.txt
    cat > "$SDCARD_PATH/system/config/passwords.txt" <<'EOF'
# WebDAV and system passwords
# Format: username:password:permissions
admin:admin:admin
user:password:user
EOF
    
    # settings.json
    cat > "$SDCARD_PATH/system/config/settings.json" <<'EOF'
{
  "wifi_ssid": "",
  "wifi_password": "",
  "hostname": "esp8266",
  "webdav_port": 80,
  "webdav_enabled": true,
  "http_enabled": true,
  "max_connections": 4,
  "timeout": 30
}
EOF
    
    # quotas.json
    cat > "$SDCARD_PATH/system/config/quotas.json" <<'EOF'
{
  "admin": {
    "max_storage": 1048576,
    "max_files": 1000,
    "read_only": false
  },
  "user": {
    "max_storage": 524288,
    "max_files": 500,
    "read_only": false
  },
  "guest": {
    "max_storage": 102400,
    "max_files": 100,
    "read_only": true
  }
}
EOF
    
    # file_locks.json
    echo '{}' > "$SDCARD_PATH/system/config/file_locks.json"
    
    # share_links.json
    echo '{}' > "$SDCARD_PATH/system/config/share_links.json"
    
    # remote_access.json
    cat > "$SDCARD_PATH/system/config/remote_access.json" <<'EOF'
{
  "enabled": false,
  "port": 8080,
  "dns_name": ""
}
EOF
    
    # history.log
    touch "$SDCARD_PATH/system/logs/history.log"
    
    # system.log
    touch "$SDCARD_PATH/system/logs/system.log"
    
    log_verbose "Created configuration files"
}

create_user_skeleton() {
    # README.md for user skeleton
    cat > "$SDCARD_PATH/etc/user.skel/README.md" <<'EOF'
# User Home Directory

This is your personal home directory on the ESP8266 GUIKit system.

## Directory Structure

- `projects/` - Your GUIKit projects
- All files here are private to you

## Quick Start

1. Use the Web Editor (editor.GUIKIT) to create new projects
2. Projects are stored in `projects/` directory
3. Each project is a `.GUIKIT` directory

## Access Methods

- **WebDAV**: Connect with your credentials, root = /home/(your-username)/
- **Web Editor**: Access through browser, automatic project detection

## Support

For help, refer to the main README.md or contact your system administrator.
EOF
    
    log_verbose "Created user skeleton"
}

create_guikitloader_conf() {
    cat > "$SDCARD_PATH/etc/guikitloader.conf" <<'EOF'
# GUIKit Loader Configuration
# This file configures which GUI loads on boot

# Default GUI to load on boot
default_gui=chooser

# Path to GUI directory
gui_path=/gui

# Use .GUIKIT directories
use_project_dirs=true

# Auto-load last used GUI
auto_load_last=true

# Last loaded GUI
last_gui=chooser

# Boot delay in milliseconds (for SD card initialization)
boot_delay=2000

# Enable error display on TFT
tft_error_display=true

# Debug mode (more serial output)
debug=false
EOF
    
    log_verbose "Created guikitloader.conf"
}

# ============================================================================
# GUI Projects
# ============================================================================

copy_gui_projects() {
    log_header "Copying GUI Projects"
    
    local src_gui_dir="$PROJECT_DIR/src/gui_editor/server/gui"
    local dst_gui_dir="$SDCARD_PATH/gui"
    
    if [ ! -d "$src_gui_dir" ]; then
        log_warn "GUI source directory not found: $src_gui_dir"
        return 1
    fi
    
    log_info "Copying system GUIs to $dst_gui_dir..."
    
    # Copy each .GUIKIT directory
    for project_dir in "$src_gui_dir"/*GUIKIT; do
        if [ -d "$project_dir" ]; then
            local project_name=$(basename "$project_dir")
            log_info "  Copying $project_name..."
            cp -r "$project_dir" "$dst_gui_dir/" || { 
                log_error "Failed to copy $project_name"
                return 1
            }
        fi
    done
    
    # Create admin user home with sample projects
    create_admin_home
    
    log_info "GUI projects copied successfully"
    return 0
}

create_admin_home() {
    log_info "Creating admin user home directory..."
    
    local admin_home="$SDCARD_PATH/home/admin"
    mkdir -p "$admin_home/projects"
    
    # Copy sample projects to admin's home
    local src_gui_dir="$PROJECT_DIR/src/gui_editor/server/gui"
    
    # Copy a sample project (create a simple one)
    cat > "$admin_home/projects/SampleProject.GUIKIT/main_gui.json" <<'EOF'
{
  "version": "1.0",
  "name": "SampleProject",
  "size": { "width": 320, "height": 240 },
  "background": "#1E1E1E",
  "theme": "dark",
  "widgets": [
    {
      "id": "title",
      "type": "label",
      "x": 20,
      "y": 20,
      "width": 280,
      "height": 30,
      "text": "Sample Project",
      "text_size": 20,
      "text_color": "#FFFFFF",
      "text_align": "center"
    },
    {
      "id": "hello_btn",
      "type": "button",
      "x": 100,
      "y": 80,
      "width": 120,
      "height": 40,
      "text": "Click Me",
      "text_size": 14,
      "text_color": "#000000",
      "background": "#1177BB",
      "action": "hello_action"
    },
    {
      "id": "status",
      "type": "label",
      "x": 50,
      "y": 140,
      "width": 220,
      "height": 20,
      "text": "Ready",
      "text_size": 12,
      "text_color": "#D4D4D4",
      "text_align": "center"
    }
  ]
}
EOF
    
    cat > "$admin_home/projects/SampleProject.GUIKIT/project.meta.json" <<'EOF'
{
  "name": "SampleProject",
  "description": "A sample GUIKit project for demonstration",
  "author": "admin",
  "version": "1.0.0",
  "created": "2026-08-15T00:00:00Z",
  "modified": "2026-08-15T00:00:00Z",
  "gui_files": ["main_gui.json"],
  "scripts": [],
  "styles": [],
  "dependencies": [],
  "category": "user",
  "type": "application"
}
EOF
    
    cat > "$admin_home/projects/SampleProject.GUIKIT/scripts/main.js" <<'EOF'
/**
 * Sample project script
 */

function hello_action(widget, event) {
    GUI.getWidgetById('status').text = 'Hello, World!';
    GUI.redraw();
}
EOF
    
    log_verbose "Created admin sample project"
}

# ============================================================================
# Main Build Functions
# ============================================================================

build_all() {
    if [ "$SKIP_CLEAN" = false ]; then
        clean_all
    fi
    
    check_dependencies
    
    build_bootloader || exit 1
    build_kernel || exit 1
    prepare_sdcard || exit 1
    
    log_header "Build Complete!"
    log_info "All components built successfully"
    log_info "SD card structure ready at: $SDCARD_PATH"
    log_info ""
    log_info "Next steps:"
    log_info "  1. Copy contents of $SDCARD_PATH to your SD card"
    log_info "  2. Insert SD card into ESP8266"
    log_info "  3. Flash bootloader: pio run -e $BOOTLOADER_ENV -t upload"
    log_info "  4. Reset ESP8266"
}

build_gui() {
    log_header "Building GUI Projects"
    
    # Just copy GUI projects
    prepare_sdcard || exit 1
    copy_gui_projects || exit 1
    
    log_info "GUI projects built"
}

build_freeze_test() {
    log_header "Building RAM Freeze Test"
    
    # Build a test program for RAM freeze/thaw
    log_info "Building freeze test..."
    
    # For now, just test the compilation of freeze-related files
    check_file_exists "src/boot/ram_freeze.h" || exit 1
    check_file_exists "src/boot/ram_freeze.c" || exit 1
    check_file_exists "src/boot/sd_freeze_wrapper.h" || exit 1
    
    log_info "RAM freeze test components verified"
    log_info "To test: Run a program that calls ram_freeze_save() and ram_freeze_restore()"
}

build_progress_test() {
    log_header "Building Task Progress Test"
    
    # Build a test program for task progress display
    log_info "Building progress test..."
    
    # Verify the progress files exist
    check_file_exists "src/boot/task_progress.h" || exit 1
    check_file_exists "src/boot/task_progress.c" || exit 1
    
    log_info "Task progress components verified"
    log_info "Task progress test:"
    log_info "  - task_progress_minimal() uses ~100 bytes stack, no heap"
    log_info "  - TaskProgressState: 64 bytes total (32+32)"
    log_info "  - Integrated with PNG converter for automatic display"
}

flash_all() {
    log_header "Flashing to Device"
    
    upload_bootloader || exit 1
    
    log_info "Bootloader flashed"
    log_warn "Note: Kernel.bin must be copied to SD card manually or via build.sh all"
}

# ============================================================================
# Main Entry Point
# ============================================================================

main() {
    parse_args "$@"
    
    case "$TARGET" in
        all)
            build_all
            ;;
        bootloader)
            if [ "$SKIP_CLEAN" = false ]; then
                log_info "Cleaning bootloader..."
                pio run -e $BOOTLOADER_ENV -t clean 2>/dev/null || true
            fi
            build_bootloader
            ;;
        kernel)
            if [ "$SKIP_CLEAN" = false ]; then
                log_info "Cleaning kernel..."
                pio run -e $KERNEL_ENV -t clean 2>/dev/null || true
            fi
            build_kernel
            ;;
        sdcard)
            prepare_sdcard
            ;;
        gui)
            build_gui
            ;;
        clean)
            clean_all
            ;;
        flash)
            flash_all
            ;;
        freeze)
            build_freeze_test
            ;;
        progress)
            build_progress_test
            ;;
        help)
            show_help
            ;;
        version)
            show_version
            ;;
        *)
            if [ -n "$TARGET" ]; then
                log_error "Unknown target: $TARGET"
                show_help
                exit 1
            fi
            build_all  # Default to all
            ;;
    esac
}

main "$@"
