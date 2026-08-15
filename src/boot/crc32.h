#ifndef CRC32_H
#define CRC32_H

/**
 * @brief Fast CRC32 implementation for RAM freeze integrity checks
 * 
 * Simple, fast CRC32 for embedded systems (ESP8266/ESP32)
 * No security requirements - just basic integrity verification
 */

#include <stdint.h>
#include <stddef.h>

// ============================================================================
// CRC32 POLYNOMIAL & INITIAL VALUE
// ============================================================================

/// Standard CRC32 polynomial (IEEE 802.3)
#define CRC32_POLYNOMIAL        0xEDB88320

/// Initial CRC value (all bits set)
#define CRC32_INITIAL_VALUE     0xFFFFFFFF

/// Final XOR value
#define CRC32_FINAL_XOR        0xFFFFFFFF

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

/// @brief Calculate CRC32 for a block of data
/// @param data Pointer to data buffer
/// @param length Length of data in bytes
/// @return CRC32 checksum
uint32_t crc32(const uint8_t *data, size_t length);

/// @brief Calculate CRC32 with custom initial value
/// @param data Pointer to data buffer
/// @param length Length of data in bytes
/// @param initial Initial CRC value
/// @return CRC32 checksum
uint32_t crc32_with_initial(const uint8_t *data, size_t length, uint32_t initial);

/// @brief Update CRC32 with additional data (streaming)
/// @param crc Current CRC value
/// @param data Pointer to additional data
/// @param length Length of additional data
/// @return Updated CRC32 value
uint32_t crc32_update(uint32_t crc, const uint8_t *data, size_t length);

// ============================================================================
// HARDWARE ACCELERATION (ESP32)
// ============================================================================

#if defined(ESP32) && defined(RAM_FREEZE_USE_HW_CRC32)
    /// @brief Calculate CRC32 using ESP32 hardware accelerator
    uint32_t crc32_hw(const uint8_t *data, size_t length);
#endif

#endif // CRC32_H
