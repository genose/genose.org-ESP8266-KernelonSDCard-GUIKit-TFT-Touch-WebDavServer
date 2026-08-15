#ifndef GUIKIT_FREEZE_BOOT_H
#define GUIKIT_FREEZE_BOOT_H

/**
 * @brief GUIKit Freeze Boot - Fast boot from frozen RAM state
 * 
 * This module provides freeze/thaw functionality for the GUIKit bootloader
 * It allows resuming from a saved RAM state for fast boot
 * 
 * Integration:
 * 1. Call freeze_boot_init() early in bootloader
 * 2. Call freeze_boot_check_and_thaw() to attempt thaw
 * 3. If thaw fails, continue with normal boot
 * 4. Call freeze_boot_save() before shutdown for freeze
 */

#include "freeze_config.h"
#include "ram_freeze.h"
#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// FREEZE BOOT STATES
// ============================================================================

/// @brief Freeze boot states
typedef enum {
    FREEZE_BOOT_STATE_INIT = 0,       /// Initialization
    FREEZE_BOOT_STATE_CHECKING,      /// Checking for freeze file
    FREEZE_BOOT_STATE_THAWING,       /// Restoring RAM from freeze
    FREEZE_BOOT_STATE_THAWED,        /// Successfully thawed
    FREEZE_BOOT_STATE_NORMAL,        /// No freeze, normal boot
    FREEZE_BOOT_STATE_BYPASS,        /// Button held, bypass freeze
    FREEZE_BOOT_STATE_ERROR,         /// Error occurred
} FreezeBootState;

// ============================================================================
// FREEZE BOOT CONFIGURATION
// ============================================================================

/// @brief Freeze boot configuration
typedef struct {
    bool enable_thaw_on_boot;        /// Enable thaw on boot
    bool enable_freeze_on_shutdown;   /// Enable freeze on shutdown
    bool check_button;               /// Check boot button for bypass
    uint8_t button_pin;               /// Boot button GPIO pin
    uint32_t button_hold_ms;          /// Button hold time for bypass
    uint32_t max_freeze_age_seconds;  /// Maximum age of freeze file
} FreezeBootConfig;

/// @brief Default freeze boot configuration
#define FREEZE_BOOT_CONFIG_DEFAULT \
    { \
        .enable_thaw_on_boot = true, \
        .enable_freeze_on_shutdown = true, \
        .check_button = true, \
        .button_pin = RAM_FREEZE_BOOT_BUTTON_PIN, \
        .button_hold_ms = RAM_FREEZE_BUTTON_HOLD_MS, \
        .max_freeze_age_seconds = RAM_FREEZE_MAX_AGE_SECONDS \
    }

// ============================================================================
// FREEZE BOOT STATE
// ============================================================================

/// @brief Freeze boot state information
typedef struct {
    FreezeBootState state;            /// Current state
    FreezeBootConfig config;          /// Configuration
    RamFreezeResult freeze_result;    /// Result of freeze/thaw operations
    uint32_t boot_start_time;         /// When boot started
    uint32_t thaw_time_ms;            /// Time taken for thaw (ms)
    bool resumed_from_freeze;        /// True if resumed from freeze
} FreezeBootStateInfo;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

/// @brief Initialize freeze boot system
/// @param state Pointer to state struct
/// @param config Configuration (or NULL for defaults)
/// @return true on success
bool freeze_boot_init(FreezeBootStateInfo *state, const FreezeBootConfig *config);

/// @brief Check for freeze file and attempt thaw
/// @param state Pointer to state struct
/// @return true if thawed successfully, false for normal boot
bool freeze_boot_check_and_thaw(FreezeBootStateInfo *state);

/// @brief Save current RAM state to SD (for shutdown)
/// @param state Pointer to state struct
/// @return FREEZE_OK on success
RamFreezeResult freeze_boot_save(FreezeBootStateInfo *state);

/// @brief Delete freeze file
/// @param state Pointer to state struct
/// @return FREEZE_OK on success
RamFreezeResult freeze_boot_delete(FreezeBootStateInfo *state);

/// @brief Get state string
/// @param state Freeze boot state
/// @return String description
const char* freeze_boot_state_to_string(FreezeBootState state);

/// @brief Display freeze boot status on TFT
/// @param state Pointer to state struct
/// @param tft TFT instance pointer
/// @param x X position
/// @param y Y position
void freeze_boot_display_status(FreezeBootStateInfo *state, void *tft, uint16_t x, uint16_t y);

/// @brief Display freeze boot status on serial
/// @param state Pointer to state struct
void freeze_boot_display_serial_status(FreezeBootStateInfo *state);

// ============================================================================
// INLINE HELPERS
// ============================================================================

/// @brief Check if resumed from freeze
/// @param state Pointer to state struct
/// @return true if resumed from freeze
static inline bool freeze_boot_resumed_from_freeze(FreezeBootStateInfo *state) {
    return state && state->resumed_from_freeze;
}

/// @brief Check if thaw should be attempted
/// @param state Pointer to state struct
/// @return true if should attempt thaw
static inline bool freeze_boot_should_thaw(FreezeBootStateInfo *state) {
    return state && state->config.enable_thaw_on_boot && state->state != FREEZE_BOOT_STATE_ERROR;
}

#endif // GUIKIT_FREEZE_BOOT_H
