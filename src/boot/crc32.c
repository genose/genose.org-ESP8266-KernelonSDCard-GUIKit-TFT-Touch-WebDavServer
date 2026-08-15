/**
 * @file crc32.c
 * @brief Fast CRC32 implementation for RAM freeze integrity checks
 * 
 * Optimized for embedded systems (ESP8266/ESP32)
 * Simple lookup table approach for good speed on 80MHz+ MCUs
 */

#include "crc32.h"
#include <string.h>

// ============================================================================
// LOOKUP TABLE (generated at compile time for speed)
// ============================================================================

/// @brief CRC32 lookup table (one entry per byte value)
/// Generated using standard CRC32 polynomial
static uint32_t crc32_table[256];

/// @brief Flag to track if table is initialized
static bool crc32_table_initialized = false;

/// @brief Initialize CRC32 lookup table
static void crc32_init_table(void) {
    if (crc32_table_initialized) return;
    
    uint32_t polynomial = CRC32_POLYNOMIAL;
    
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++) {
            if (c & 1) {
                c = polynomial ^ (c >> 1);
            } else {
                c >>= 1;
            }
        }
        crc32_table[i] = c;
    }
    
    crc32_table_initialized = true;
}

// ============================================================================
// MAIN CRC32 FUNCTIONS
// ============================================================================

uint32_t crc32(const uint8_t *data, size_t length) {
    return crc32_with_initial(data, length, CRC32_INITIAL_VALUE);
}

uint32_t crc32_with_initial(const uint8_t *data, size_t length, uint32_t initial) {
    crc32_init_table();
    
    uint32_t crc = initial ^ CRC32_INITIAL_VALUE;
    
    for (size_t i = 0; i < length; i++) {
        crc = crc32_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }
    
    return crc ^ CRC32_FINAL_XOR;
}

uint32_t crc32_update(uint32_t crc, const uint8_t *data, size_t length) {
    crc32_init_table();
    
    for (size_t i = 0; i < length; i++) {
        crc = crc32_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }
    
    return crc;
}

// ============================================================================
// HARDWARE ACCELERATION (ESP32)
// ============================================================================

#if defined(ESP32) && defined(RAM_FREEZE_USE_HW_CRC32)

#include "esp_crc.h"

uint32_t crc32_hw(const uint8_t *data, size_t length) {
    // ESP32 has hardware CRC32 support
    esp_crc32_init();
    return esp_crc32_le(CRC32_INITIAL_VALUE, data, length) ^ CRC32_FINAL_XOR;
}

#endif // ESP32 && RAM_FREEZE_USE_HW_CRC32

// ============================================================================
// SLOW BIT-BY-BIT VERSION (fallback, no table)
// ============================================================================

/// @brief Slow CRC32 implementation (no table, for very small data)
/// Use only when memory is extremely constrained
uint32_t crc32_slow(const uint8_t *data, size_t length) {
    uint32_t crc = CRC32_INITIAL_VALUE;
    uint32_t polynomial = CRC32_POLYNOMIAL;
    
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ polynomial;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc ^ CRC32_FINAL_XOR;
}
