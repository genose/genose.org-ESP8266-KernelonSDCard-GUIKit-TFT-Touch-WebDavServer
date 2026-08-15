/**
 * @file ram_freeze.c
 * @brief RAM Freeze/Thaw implementation for fast boot
 * 
 * Fast RAW save/restore of RAM to/from SD card
 * Simple CRC32 integrity check, no compression
 * Designed for ESP8266/ESP32 with GUIKit
 */

#include "ram_freeze.h"
#include "crc32.h"
#include <string.h>
#include <time.h>

// ============================================================================
// SD CARD WRAPPER (using sd_freeze_wrapper)
// ============================================================================

#include "sd_freeze_wrapper.h"

/// @brief Open a file on SD card
/// @param path File path
/// @param mode "r", "w", "rb", "wb", etc.
/// @return File handle or NULL on error
static void* sd_fopen(const char *path, const char *mode) {
    return sd_freeze_fopen(path, mode);
}

/// @brief Close an SD file
/// @param fp File handle
static void sd_fclose(void *fp) {
    sd_freeze_fclose(fp);
}

/// @brief Read from SD file
/// @param ptr Buffer to read into
/// @param size Item size
/// @param count Item count
/// @param fp File handle
/// @return Number of items read
static size_t sd_fread(void *ptr, size_t size, size_t count, void *fp) {
    return sd_freeze_fread(ptr, size, count, fp);
}

/// @brief Write to SD file
/// @param ptr Buffer to write
/// @param size Item size
/// @param count Item count
/// @param fp File handle
/// @return Number of items written
static size_t sd_fwrite(const void *ptr, size_t size, size_t count, void *fp) {
    return sd_freeze_fwrite(ptr, size, count, fp);
}

/// @brief Check if file exists
/// @param path File path
/// @return true if exists
static bool sd_file_exists(const char *path) {
    return sd_freeze_file_exists(path);
}

/// @brief Get file size
/// @param path File path
/// @return Size in bytes, 0 if doesn't exist
static uint32_t sd_file_size(const char *path) {
    return sd_freeze_file_size(path);
}

/// @brief Create directory
/// @param path Directory path
/// @return true on success
static bool sd_mkdir(const char *path) {
    return sd_freeze_mkdir(path);
}

/// @brief Get free space on SD card
/// @return Free space in bytes
static uint32_t sd_free_space(void) {
    return sd_freeze_get_free_space();
}

/// @brief Check if SD card is initialized
/// @return true if ready
static bool sd_is_ready(void) {
    return sd_freeze_is_ready();
}

// ============================================================================
// HELPER: Calculate header CRC (excluding data_crc field)
// ============================================================================

/// @brief Calculate CRC of header (excluding data_crc)
/// @param header Pointer to header
/// @return CRC32 of header
static uint32_t calculate_header_crc(const RamFreezeHeader *header) {
    // Create a copy without data_crc
    RamFreezeHeader temp = *header;
    temp.data_crc = 0;
    return crc32((const uint8_t*)&temp, offsetof(RamFreezeHeader, data_crc));
}

// ============================================================================
// CORE FUNCTIONS
// ============================================================================

RamFreezeResult ram_freeze_init(void) {
    if (!RAM_FREEZE_IS_ENABLED()) {
        return FREEZE_DISABLED;
    }
    
    if (!sd_is_ready()) {
        return FREEZE_ERROR_SD_NOT_READY;
    }
    
    return FREEZE_OK;
}

// -----------------------------------------------------------------------------
// FREEZE (Save RAM to SD)
// -----------------------------------------------------------------------------

RamFreezeResult ram_freeze_save(void) {
    if (!RAM_FREEZE_IS_ENABLED()) {
        return FREEZE_DISABLED;
    }
    
    if (!sd_is_ready()) {
        return FREEZE_ERROR_SD_NOT_READY;
    }
    
    // Check space
    if (ram_freeze_get_sd_free_space() < RAM_FREEZE_RAM_SIZE + RAM_FREEZE_HEADER_SIZE) {
        return FREEZE_ERROR_NO_SPACE;
    }
    
    // Ensure directory exists
    RamFreezeResult dir_result = ram_freeze_ensure_directory();
    if (!ram_freeze_is_success(dir_result)) {
        return dir_result;
    }
    
    // Calculate CRC of RAM region
    uint32_t data_crc = ram_freeze_calculate_crc(RAM_FREEZE_RAM_START, RAM_FREEZE_RAM_SIZE);
    
    // Create header
    RamFreezeHeader header;
    header.magic = RAM_FREEZE_MAGIC;
    header.version = RAM_FREEZE_VERSION;
    header.timestamp = (uint32_t)time(NULL);
    header.ram_start = RAM_FREEZE_RAM_START;
    header.ram_size = RAM_FREEZE_RAM_SIZE;
    header.data_crc = data_crc;
    header.header_crc = calculate_header_crc(&header);
    
    // Write to SD card
    void *fp = sd_fopen(RAM_FREEZE_PATH, "wb");
    if (!fp) {
        return FREEZE_ERROR_SD_OPEN;
    }
    
    // Write header
    if (sd_fwrite(&header, sizeof(header), 1, fp) != 1) {
        sd_fclose(fp);
        return FREEZE_ERROR_SD_WRITE;
    }
    
    // Write RAM data
    if (sd_fwrite((const void*)RAM_FREEZE_RAM_START, RAM_FREEZE_RAM_SIZE, 1, fp) != 1) {
        sd_fclose(fp);
        return FREEZE_ERROR_SD_WRITE;
    }
    
    sd_fclose(fp);
    
    // Create shutdown flag
    ram_freeze_create_shutdown_flag();
    
    return FREEZE_OK;
}

// -----------------------------------------------------------------------------
// THAW (Restore RAM from SD)
// -----------------------------------------------------------------------------

RamFreezeResult ram_freeze_restore(void) {
    if (!RAM_FREEZE_IS_ENABLED()) {
        return FREEZE_DISABLED;
    }
    
    if (!sd_is_ready()) {
        return FREEZE_ERROR_SD_NOT_READY;
    }
    
    // Check if file exists
    if (!sd_file_exists(RAM_FREEZE_PATH)) {
        return FREEZE_NOT_FOUND;
    }
    
    // Open file
    void *fp = sd_fopen(RAM_FREEZE_PATH, "rb");
    if (!fp) {
        return FREEZE_ERROR_SD_OPEN;
    }
    
    // Read header
    RamFreezeHeader header;
    if (sd_fread(&header, sizeof(header), 1, fp) != 1) {
        sd_fclose(fp);
        return FREEZE_ERROR_SD_READ;
    }
    
    // Validate magic
    if (header.magic != RAM_FREEZE_MAGIC) {
        sd_fclose(fp);
        return FREEZE_ERROR_INVALID_MAGIC;
    }
    
    // Validate version
    if (header.version != RAM_FREEZE_VERSION) {
        sd_fclose(fp);
        return FREEZE_ERROR_INVALID_VERSION;
    }
    
    // Validate header CRC
    uint32_t expected_header_crc = calculate_header_crc(&header);
    if (header.header_crc != expected_header_crc) {
        sd_fclose(fp);
        return FREEZE_ERROR_CRC_MISMATCH;
    }
    
    // Check age (if configured)
    #if RAM_FREEZE_MAX_AGE_SECONDS > 0
    uint32_t current_time = (uint32_t)time(NULL);
    if (current_time > header.timestamp && 
        (current_time - header.timestamp) > RAM_FREEZE_MAX_AGE_SECONDS) {
        sd_fclose(fp);
        return FREEZE_ERROR_TOO_OLD;
    }
    #endif
    
    // Verify RAM region matches
    if (header.ram_start != RAM_FREEZE_RAM_START || 
        header.ram_size != RAM_FREEZE_RAM_SIZE) {
        sd_fclose(fp);
        return FREEZE_ERROR_INVALID_MAGIC; // Configuration mismatch
    }
    
    // Read RAM data
    if (sd_fread((void*)RAM_FREEZE_RAM_START, header.ram_size, 1, fp) != 1) {
        sd_fclose(fp);
        return FREEZE_ERROR_SD_READ;
    }
    
    sd_fclose(fp);
    
    // Verify data CRC
    uint32_t current_crc = ram_freeze_calculate_crc(RAM_FREEZE_RAM_START, RAM_FREEZE_RAM_SIZE);
    if (current_crc != header.data_crc) {
        return FREEZE_ERROR_CRC_MISMATCH;
    }
    
    // Delete shutdown flag (we've successfully resumed)
    ram_freeze_delete_shutdown_flag();
    
    return FREEZE_OK;
}

// -----------------------------------------------------------------------------
// CHECK FREEZE VALIDITY
// -----------------------------------------------------------------------------

RamFreezeResult ram_freeze_check(RamFreezeState *state) {
    if (state) {
        state->freeze_enabled = RAM_FREEZE_IS_ENABLED();
        state->freeze_exists = sd_file_exists(RAM_FREEZE_PATH);
        state->freeze_valid = false;
        state->freeze_timestamp = 0;
        state->freeze_size = 0;
    }
    
    if (!RAM_FREEZE_IS_ENABLED()) {
        return FREEZE_DISABLED;
    }
    
    if (!sd_is_ready()) {
        return FREEZE_ERROR_SD_NOT_READY;
    }
    
    if (!sd_file_exists(RAM_FREEZE_PATH)) {
        return FREEZE_NOT_FOUND;
    }
    
    // Try to validate the file
    void *fp = sd_fopen(RAM_FREEZE_PATH, "rb");
    if (!fp) {
        return FREEZE_ERROR_SD_OPEN;
    }
    
    RamFreezeHeader header;
    if (sd_fread(&header, sizeof(header), 1, fp) != 1) {
        sd_fclose(fp);
        return FREEZE_ERROR_SD_READ;
    }
    
    if (header.magic != RAM_FREEZE_MAGIC || 
        header.version != RAM_FREEZE_VERSION) {
        sd_fclose(fp);
        return FREEZE_ERROR_INVALID_MAGIC;
    }
    
    if (header.header_crc != calculate_header_crc(&header)) {
        sd_fclose(fp);
        return FREEZE_ERROR_CRC_MISMATCH;
    }
    
    #if RAM_FREEZE_MAX_AGE_SECONDS > 0
    uint32_t current_time = (uint32_t)time(NULL);
    if (current_time > header.timestamp && 
        (current_time - header.timestamp) > RAM_FREEZE_MAX_AGE_SECONDS) {
        sd_fclose(fp);
        return FREEZE_ERROR_TOO_OLD;
    }
    #endif
    
    if (state) {
        state->freeze_valid = true;
        state->freeze_timestamp = header.timestamp;
        state->freeze_size = header.ram_size + sizeof(header);
    }
    
    sd_fclose(fp);
    return FREEZE_OK;
}

// -----------------------------------------------------------------------------
// DELETE FREEZE FILE
// -----------------------------------------------------------------------------

RamFreezeResult ram_freeze_delete(void) {
    if (!sd_is_ready()) {
        return FREEZE_ERROR_SD_NOT_READY;
    }
    
    // Delete the freeze file
    if (sd_file_exists(RAM_FREEZE_PATH)) {
        if (!sd_freeze_delete(RAM_FREEZE_PATH)) {
            return FREEZE_ERROR_SD_WRITE;
        }
    }
    
    return FREEZE_OK;
}

// -----------------------------------------------------------------------------
// SHUTDOWN FLAG FUNCTIONS
// -----------------------------------------------------------------------------

RamFreezeResult ram_freeze_create_shutdown_flag(void) {
    if (!sd_is_ready()) {
        return FREEZE_ERROR_SD_NOT_READY;
    }
    
    ram_freeze_ensure_directory();
    
    void *fp = sd_fopen(RAM_FREEZE_SHUTDOWN_FLAG, "wb");
    if (!fp) {
        return FREEZE_ERROR_SD_OPEN;
    }
    
    uint32_t flag = RAM_FREEZE_MAGIC;
    sd_fwrite(&flag, sizeof(flag), 1, fp);
    sd_fclose(fp);
    
    return FREEZE_OK;
}

bool ram_freeze_was_clean_shutdown(void) {
    if (!sd_is_ready()) {
        return false;
    }
    return sd_file_exists(RAM_FREEZE_SHUTDOWN_FLAG);
}

RamFreezeResult ram_freeze_delete_shutdown_flag(void) {
    if (!sd_is_ready()) {
        return FREEZE_ERROR_SD_NOT_READY;
    }
    
    if (sd_file_exists(RAM_FREEZE_SHUTDOWN_FLAG)) {
        if (!sd_freeze_delete(RAM_FREEZE_SHUTDOWN_FLAG)) {
            return FREEZE_ERROR_SD_WRITE;
        }
    }
    
    return FREEZE_OK;
}

// -----------------------------------------------------------------------------
// DIRECTORY MANAGEMENT
// -----------------------------------------------------------------------------

RamFreezeResult ram_freeze_ensure_directory(void) {
    if (!sd_is_ready()) {
        return FREEZE_ERROR_SD_NOT_READY;
    }
    
    if (!sd_file_exists(RAM_FREEZE_DIR)) {
        if (!sd_mkdir(RAM_FREEZE_DIR)) {
            return FREEZE_ERROR_SD_WRITE;
        }
    }
    
    return FREEZE_OK;
}

// -----------------------------------------------------------------------------
// FILE SIZE & SPACE CHECKS
// -----------------------------------------------------------------------------

uint32_t ram_freeze_get_file_size(void) {
    if (!sd_is_ready()) {
        return 0;
    }
    return sd_file_size(RAM_FREEZE_PATH);
}

uint32_t ram_freeze_get_sd_free_space(void) {
    if (!sd_is_ready()) {
        return 0;
    }
    return sd_free_space();
}

// -----------------------------------------------------------------------------
// CRC CALCULATION
// -----------------------------------------------------------------------------

uint32_t ram_freeze_calculate_crc(uint32_t start, uint32_t size) {
    return crc32((const uint8_t*)start, size);
}

// -----------------------------------------------------------------------------
// RESULT STRING CONVERSION
// -----------------------------------------------------------------------------

const char* ram_freeze_result_to_string(RamFreezeResult result) {
    switch (result) {
        case FREEZE_OK: return "OK";
        case FREEZE_ERROR_SD_NOT_READY: return "SD not ready";
        case FREEZE_ERROR_SD_OPEN: return "SD open error";
        case FREEZE_ERROR_SD_WRITE: return "SD write error";
        case FREEZE_ERROR_SD_READ: return "SD read error";
        case FREEZE_ERROR_INVALID_MAGIC: return "Invalid magic";
        case FREEZE_ERROR_INVALID_VERSION: return "Invalid version";
        case FREEZE_ERROR_CRC_MISMATCH: return "CRC mismatch";
        case FREEZE_ERROR_TOO_OLD: return "Freeze too old";
        case FREEZE_ERROR_NO_SPACE: return "No SD space";
        case FREEZE_ERROR_BUTTON_HOLD: return "Button held";
        case FREEZE_DISABLED: return "Disabled";
        case FREEZE_NOT_FOUND: return "Not found";
        default: return "Unknown";
    }
}

// ============================================================================
// BOOT BUTTON IMPLEMENTATION
// ============================================================================

#include "Arduino.h"  // For digitalRead, millis, pinMode

void ram_freeze_button_init(void) {
    pinMode(RAM_FREEZE_BOOT_BUTTON_PIN, INPUT_PULLUP);
}

bool ram_freeze_button_is_held(void) {
    static uint32_t hold_start = 0;
    static bool was_pressed = false;
    
    bool is_pressed = (digitalRead(RAM_FREEZE_BOOT_BUTTON_PIN) == LOW);
    
    if (is_pressed && !was_pressed) {
        hold_start = millis();
        was_pressed = true;
    }
    
    if (!is_pressed && was_pressed) {
        was_pressed = false;
    }
    
    if (was_pressed && (millis() - hold_start) >= RAM_FREEZE_BUTTON_HOLD_MS) {
        return true;
    }
    
    return false;
}

void ram_freeze_button_wait(void) {
    // Small delay to allow button checking during boot
    #if RAM_FREEZE_BUTTON_HOLD_MS > 0
    delay(10);
    #endif
}
