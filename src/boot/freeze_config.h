#ifndef FREEZE_CONFIG_H
#define FREEZE_CONFIG_H

/**
 * @brief Configuration for RAM freeze/thaw feature
 * 
 * Fast RAW save/restore for ESP8266/ESP32 boot optimization
 * No compression, simple CRC32 integrity check
 */

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// FEATURE TOGGLES
// ============================================================================

/// Enable RAM freeze/thaw feature (1 = enabled, 0 = disabled)
#define RAM_FREEZE_ENABLED           1

/// Enable freeze on controlled shutdown (power-off, deep sleep)
#define RAM_FREEZE_ON_SHUTDOWN       1

/// Enable thaw on boot (if freeze file exists and is valid)
#define RAM_FREEZE_ON_BOOT           1

// ============================================================================
// MAGIC NUMBERS & VERSIONING
// ============================================================================

/// Magic number for freeze file validation
#define RAM_FREEZE_MAGIC            0xDEADBEEF

/// Current freeze file format version
#define RAM_FREEZE_VERSION          1

/// Invalid magic (for detection)
#define RAM_FREEZE_MAGIC_INVALID    0x00000000

// ============================================================================
// FREEZE REGION CONFIGURATION
// ============================================================================

// For ESP8266:
//   Internal RAM: 0x20000000 - 0x2001FFFF (80KB total)
//   Reserve 8KB for system stack/heap: 0x20000000 - 0x20001FFF
//   GUIKit RAM region: 0x20002000 - 0x2001FFFF (76KB)

// For ESP32:
//   Internal RAM: 0x3FFE0000 - 0x3FFFBFFF (320KB total)
//   GUIKit RAM region: 0x3FFE0000 - 0x3FFF8000 (128KB)

#ifdef ESP8266
    #define RAM_FREEZE_RAM_START     0x20002000  /// Start of freeze region
    #define RAM_FREEZE_RAM_SIZE      0x1E000     /// 120KB (adjustable)
#elif defined(ESP32)
    #define RAM_FREEZE_RAM_START     0x3FFE0000  /// Start of freeze region
    #define RAM_FREEZE_RAM_SIZE      0x10000     /// 64KB (adjustable)
#else
    #define RAM_FREEZE_RAM_START     0x20002000  /// Default ESP8266
    #define RAM_FREEZE_RAM_SIZE      0x1E000     /// 120KB
#endif

// ============================================================================
// FILE PATH CONFIGURATION
// ============================================================================

/// Path to freeze file on SD card
#define RAM_FREEZE_PATH             "/system/freeze/ram_freeze.bin"

/// Path to shutdown flag (indicates clean shutdown)
#define RAM_FREEZE_SHUTDOWN_FLAG    "/system/freeze/shutdown_flag.bin"

/// Freeze directory
#define RAM_FREEZE_DIR              "/system/freeze"

// ============================================================================
// TIMEOUTS & RETRIES
// ============================================================================

/// Maximum time to wait for SD card during freeze (ms)
#define RAM_FREEZE_SD_TIMEOUT_MS    500

/// Maximum retries for SD operations
#define RAM_FREEZE_SD_RETRIES       3

// ============================================================================
// CRC32 CONFIGURATION
// ============================================================================

/// Use hardware CRC32 if available (ESP32 has hardware CRC)
#define RAM_FREEZE_USE_HW_CRC32    1

// ============================================================================
// BOOT BUTTON CONFIGURATION (for freeze bypass)
// ============================================================================

/// GPIO pin for boot button (hold during boot to skip freeze)
#define RAM_FREEZE_BOOT_BUTTON_PIN  0  /// Typically D3 on NodeMCU

/// Button must be held for this many ms to bypass freeze
#define RAM_FREEZE_BUTTON_HOLD_MS   2000

// ============================================================================
// THAW VALIDATION
// ============================================================================

/// Maximum age of freeze file in seconds (0 = no expiration)
#define RAM_FREEZE_MAX_AGE_SECONDS  86400  /// 24 hours

/// Minimum SD card free space required for freeze (bytes)
#define RAM_FREEZE_MIN_FREE_SPACE   (RAM_FREEZE_RAM_SIZE * 2)

// ============================================================================
// INLINE CONFIGURATION CHECKS
// ============================================================================

/// Check if freeze feature is fully enabled
#define RAM_FREEZE_IS_ENABLED() \
    (RAM_FREEZE_ENABLED && RAM_FREEZE_ON_SHUTDOWN && RAM_FREEZE_ON_BOOT)

/// Check if freeze region is valid
#define RAM_FREEZE_REGION_VALID() \
    (RAM_FREEZE_RAM_START != 0 && RAM_FREEZE_RAM_SIZE > 0)

#endif // FREEZE_CONFIG_H
