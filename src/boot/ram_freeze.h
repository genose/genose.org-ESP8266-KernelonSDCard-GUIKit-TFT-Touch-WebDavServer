#ifndef RAM_FREEZE_H
#define RAM_FREEZE_H

/**
 * @brief RAM Freeze/Thaw system for fast boot
 * 
 * Implements fast RAW save/restore of RAM to/from SD card
 * Simple CRC32 integrity check, no compression
 * Designed for ESP8266/ESP32 with GUIKit
 */

#include "freeze_config.h"
#include "crc32.h"
#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// FREEZE HEADER STRUCTURE (stored on SD card)
// ============================================================================

/// @brief Header structure for freeze file
/// Stored at the beginning of each freeze file for validation
typedef struct __attribute__((packed)) {
    uint32_t magic;            /// Magic number (RAM_FREEZE_MAGIC)
    uint32_t version;          /// Format version (RAM_FREEZE_VERSION)
    uint32_t timestamp;        /// Unix timestamp when freeze was saved
    uint32_t ram_start;        /// Start address of frozen RAM region
    uint32_t ram_size;         /// Size of frozen RAM region
    uint32_t data_crc;         /// CRC32 of the frozen RAM data
    uint32_t header_crc;       /// CRC32 of this header (excluding data_crc)
} RamFreezeHeader;

/// @brief Size of freeze header in bytes
#define RAM_FREEZE_HEADER_SIZE   sizeof(RamFreezeHeader)

// ============================================================================
// FREEZE RESULT STATUS
// ============================================================================

/// @brief Result codes for freeze/thaw operations
typedef enum {
    FREEZE_OK = 0,              /// Operation succeeded
    FREEZE_ERROR_SD_NOT_READY,  /// SD card not initialized
    FREEZE_ERROR_SD_OPEN,       /// Failed to open SD file
    FREEZE_ERROR_SD_WRITE,      /// Failed to write to SD
    FREEZE_ERROR_SD_READ,       /// Failed to read from SD
    FREEZE_ERROR_INVALID_MAGIC,/// Invalid magic number in header
    FREEZE_ERROR_INVALID_VERSION,/// Invalid version in header
    FREEZE_ERROR_CRC_MISMATCH,   /// CRC checksum mismatch
    FREEZE_ERROR_TOO_OLD,        /// Freeze file is too old (expired)
    FREEZE_ERROR_NO_SPACE,      /// Not enough SD card space
    FREEZE_ERROR_BUTTON_HOLD,    /// Boot button held (bypass freeze)
    FREEZE_DISABLED,            /// Feature is disabled
    FREEZE_NOT_FOUND,           /// No freeze file exists
} RamFreezeResult;

// ============================================================================
// FREEZE STATE STRUCTURE
// ============================================================================

/// @brief Freeze state information
typedef struct {
    bool freeze_enabled;        /// Is freeze feature enabled
    bool freeze_exists;         /// Does freeze file exist on SD
    bool freeze_valid;          /// Is freeze file valid
    uint32_t freeze_timestamp;  /// When freeze was created
    uint32_t freeze_size;        /// Size of frozen data
    RamFreezeResult last_result;/// Last operation result
} RamFreezeState;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

/// @brief Initialize the freeze system
/// @return FREEZE_OK on success
RamFreezeResult ram_freeze_init(void);

/// @brief Freeze current RAM state to SD card
/// @return FREEZE_OK on success
RamFreezeResult ram_freeze_save(void);

/// @brief Thaw RAM state from SD card
/// @return FREEZE_OK on success
RamFreezeResult ram_freeze_restore(void);

/// @brief Check if freeze file exists and is valid
/// @param state Pointer to state struct (can be NULL)
/// @return FREEZE_OK if valid and exists
RamFreezeResult ram_freeze_check(RamFreezeState *state);

/// @brief Delete freeze file from SD card
/// @return FREEZE_OK on success
RamFreezeResult ram_freeze_delete(void);

/// @brief Create shutdown flag (indicates clean shutdown)
/// @return FREEZE_OK on success
RamFreezeResult ram_freeze_create_shutdown_flag(void);

/// @brief Check if shutdown was clean (flag exists)
/// @return true if clean shutdown
bool ram_freeze_was_clean_shutdown(void);

/// @brief Delete shutdown flag
/// @return FREEZE_OK on success
RamFreezeResult ram_freeze_delete_shutdown_flag(void);

/// @brief Ensure freeze directory exists on SD
/// @return FREEZE_OK on success
RamFreezeResult ram_freeze_ensure_directory(void);

/// @brief Get freeze file size on SD
/// @return Size in bytes, or 0 if doesn't exist
uint32_t ram_freeze_get_file_size(void);

/// @brief Check SD card free space
/// @return Free space in bytes
uint32_t ram_freeze_get_sd_free_space(void);

/// @brief Calculate CRC32 of RAM region
/// @param start Start address
/// @param size Size in bytes
/// @return CRC32 checksum
uint32_t ram_freeze_calculate_crc(uint32_t start, uint32_t size);

/// @brief Get human-readable string for result code
/// @param result Result code
/// @return String description
const char* ram_freeze_result_to_string(RamFreezeResult result);

// ============================================================================
// BOOT BUTTON FUNCTIONS
// ============================================================================

/// @brief Initialize boot button for freeze bypass
void ram_freeze_button_init(void);

/// @brief Check if boot button is held (bypass freeze)
/// @return true if button is held for sufficient time
bool ram_freeze_button_is_held(void);

/// @brief Wait for boot button check period
void ram_freeze_button_wait(void);

// ============================================================================
// INLINE HELPER FUNCTIONS
// ============================================================================

/// @brief Check if freeze is available (SD ready, enough space, etc.)
/// @return true if freeze can be performed
static inline bool ram_freeze_is_available(void) {
    return (ram_freeze_get_sd_free_space() > RAM_FREEZE_MIN_FREE_SPACE);
}

/// @brief Check if result indicates success
/// @param result Result code
/// @return true if successful
static inline bool ram_freeze_is_success(RamFreezeResult result) {
    return (result == FREEZE_OK);
}

/// @brief Check if result indicates file not found
/// @param result Result code
/// @return true if not found
static inline bool ram_freeze_is_not_found(RamFreezeResult result) {
    return (result == FREEZE_NOT_FOUND || result == FREEZE_ERROR_SD_OPEN);
}

#endif // RAM_FREEZE_H
