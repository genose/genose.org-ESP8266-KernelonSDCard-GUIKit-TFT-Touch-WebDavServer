/**
 * @file guikit_freeze_boot.cpp
 * @brief GUIKit Freeze Boot implementation
 * 
 * Fast boot from frozen RAM state for ESP8266/ESP32
 */

#include "guikit_freeze_boot.h"
#include "ram_freeze.h"
#include "crc32.h"
#include "freeze_config.h"
#include <string.h>

// For millis() - use your platform's implementation
#ifdef ARDUINO
#include <Arduino.h>
#else
#include <stdint.h>
static uint32_t millis(void) { return 0; } // Replace with your implementation
static void delay(uint32_t ms) { (void)ms; }
#endif

// ============================================================================
// INITIALIZATION
// ============================================================================

bool freeze_boot_init(FreezeBootStateInfo *state, const FreezeBootConfig *config) {
    if (!state) {
        return false;
    }
    
    // Initialize state
    state->state = FREEZE_BOOT_STATE_INIT;
    state->freeze_result = FREEZE_DISABLED;
    state->boot_start_time = millis();
    state->thaw_time_ms = 0;
    state->resumed_from_freeze = false;
    
    // Set configuration
    if (config) {
        state->config = *config;
    } else {
        state->config = FREEZE_BOOT_CONFIG_DEFAULT;
    }
    
    // Initialize button
    if (state->config.check_button) {
        ram_freeze_button_init();
    }
    
    // Initialize SD wrapper
    sd_freeze_init();
    
    state->state = FREEZE_BOOT_STATE_INIT;
    return true;
}

// ============================================================================
// FREEZE CHECK AND THAW
// ============================================================================

bool freeze_boot_check_and_thaw(FreezeBootStateInfo *state) {
    if (!state) {
        return false;
    }
    
    // Check if thaw is enabled
    if (!state->config.enable_thaw_on_boot) {
        state->state = FREEZE_BOOT_STATE_NORMAL;
        return false;
    }
    
    state->state = FREEZE_BOOT_STATE_CHECKING;
    
    // Check if button is held to bypass freeze
    if (state->config.check_button && ram_freeze_button_is_held()) {
        state->state = FREEZE_BOOT_STATE_BYPASS;
        state->freeze_result = FREEZE_ERROR_BUTTON_HOLD;
        return false; // Normal boot
    }
    
    // Initialize freeze system
    state->freeze_result = ram_freeze_init();
    if (!ram_freeze_is_success(state->freeze_result)) {
        state->state = FREEZE_BOOT_STATE_ERROR;
        return false;
    }
    
    // Check if freeze file exists and is valid
    RamFreezeState freeze_state;
    state->freeze_result = ram_freeze_check(&freeze_state);
    
    if (state->freeze_result == FREEZE_OK && freeze_state.freeze_valid) {
        // Freeze file is valid, attempt thaw
        state->state = FREEZE_BOOT_STATE_THAWING;
        uint32_t thaw_start = millis();
        
        state->freeze_result = ram_freeze_restore();
        
        state->thaw_time_ms = millis() - thaw_start;
        
        if (ram_freeze_is_success(state->freeze_result)) {
            state->state = FREEZE_BOOT_STATE_THAWED;
            state->resumed_from_freeze = true;
            return true; // Successfully thawed
        }
    }
    
    // No valid freeze or thaw failed, continue with normal boot
    state->state = FREEZE_BOOT_STATE_NORMAL;
    state->resumed_from_freeze = false;
    return false;
}

// ============================================================================
// FREEZE SAVE (for shutdown)
// ============================================================================

RamFreezeResult freeze_boot_save(FreezeBootStateInfo *state) {
    if (!state) {
        return FREEZE_DISABLED;
    }
    
    if (!state->config.enable_freeze_on_shutdown) {
        return FREEZE_DISABLED;
    }
    
    // Initialize
    RamFreezeResult result = ram_freeze_init();
    if (!ram_freeze_is_success(result)) {
        return result;
    }
    
    // Ensure SD is ready
    if (!sd_freeze_is_ready()) {
        return FREEZE_ERROR_SD_NOT_READY;
    }
    
    // Save RAM to SD
    result = ram_freeze_save();
    
    if (ram_freeze_is_success(result)) {
        // Create shutdown flag
        ram_freeze_create_shutdown_flag();
    }
    
    return result;
}

// ============================================================================
// DELETE FREEZE
// ============================================================================

RamFreezeResult freeze_boot_delete(FreezeBootStateInfo *state) {
    if (!state) {
        return FREEZE_DISABLED;
    }
    
    // Initialize
    RamFreezeResult result = ram_freeze_init();
    if (!ram_freeze_is_success(result)) {
        return result;
    }
    
    return ram_freeze_delete();
}

// ============================================================================
// STATUS STRING CONVERSION
// ============================================================================

const char* freeze_boot_state_to_string(FreezeBootState state) {
    switch (state) {
        case FREEZE_BOOT_STATE_INIT: return "Init";
        case FREEZE_BOOT_STATE_CHECKING: return "Checking freeze";
        case FREEZE_BOOT_STATE_THAWING: return "Thawing";
        case FREEZE_BOOT_STATE_THAWED: return "Thawed";
        case FREEZE_BOOT_STATE_NORMAL: return "Normal boot";
        case FREEZE_BOOT_STATE_BYPASS: return "Bypass (button)";
        case FREEZE_BOOT_STATE_ERROR: return "Error";
        default: return "Unknown";
    }
}

// ============================================================================
// DISPLAY FUNCTIONS
// ============================================================================

void freeze_boot_display_status(FreezeBootStateInfo *state, void *tft, uint16_t x, uint16_t y) {
    if (!state) return;
    
    // This is a placeholder - implement based on your TFT library
    // Example for TFT_eSPI:
    // TFT_eSPI *tft_inst = (TFT_eSPI*)tft;
    // tft_inst->setCursor(x, y);
    // tft_inst->print("Freeze: ");
    // tft_inst->print(freeze_boot_state_to_string(state->state));
    
    (void)tft; (void)x; (void)y; // Unused in placeholder
}

void freeze_boot_display_serial_status(FreezeBootStateInfo *state) {
    if (!state) return;
    
    // Note: Use printf or your serial output function
    printf("[FREEZE BOOT] State: %s", freeze_boot_state_to_string(state->state));
    
    if (state->resumed_from_freeze) {
        printf(" (Resumed from freeze)");
    }
    printf("\n");
    
    if (state->thaw_time_ms > 0) {
        printf("[FREEZE BOOT] Thaw time: %lu ms\n", state->thaw_time_ms);
    }
    
    if (!ram_freeze_is_success(state->freeze_result) && 
        state->freeze_result != FREEZE_NOT_FOUND) {
        printf("[FREEZE BOOT] Freeze result: %s\n", 
               ram_freeze_result_to_string(state->freeze_result));
    }
}

// ============================================================================
// CONVENIENCE FUNCTIONS
// ============================================================================

/// @brief Quick thaw check without full state management
/// @return true if thawed successfully
bool freeze_quick_thaw(void) {
    FreezeBootStateInfo state;
    FreezeBootConfig config = FREEZE_BOOT_CONFIG_DEFAULT;
    
    if (!freeze_boot_init(&state, &config)) {
        return false;
    }
    
    return freeze_boot_check_and_thaw(&state);
}

/// @brief Quick save without full state management
/// @return FREEZE_OK on success
RamFreezeResult freeze_quick_save(void) {
    FreezeBootStateInfo state;
    FreezeBootConfig config = FREEZE_BOOT_CONFIG_DEFAULT;
    
    if (!freeze_boot_init(&state, &config)) {
        return FREEZE_DISABLED;
    }
    
    return freeze_boot_save(&state);
}
