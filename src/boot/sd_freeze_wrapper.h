#ifndef SD_FREEZE_WRAPPER_H
#define SD_FREEZE_WRAPPER_H

/**
 * @brief SD Card wrapper for freeze system
 * 
 * Provides a simple file I/O interface for the freeze system
 * Adapts to your existing SD library (SdFat, SPIFFS, etc.)
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// TYPE DEFINITIONS
// ============================================================================

/// File handle type (opaque pointer to your SD library's file type)
typedef void* sd_file_t;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

/// @brief Initialize SD card for freeze operations
/// @return true on success
bool sd_freeze_init(void);

/// @brief Check if SD card is ready
/// @return true if initialized and ready
bool sd_freeze_is_ready(void);

/// @brief Open a file on SD card
/// @param path File path
/// @param mode "r", "w", "rb", "wb"
/// @return File handle or NULL on error
sd_file_t sd_freeze_fopen(const char *path, const char *mode);

/// @brief Close an SD file
/// @param fp File handle
void sd_freeze_fclose(sd_file_t fp);

/// @brief Read from SD file
/// @param ptr Buffer to read into
/// @param size Item size in bytes
/// @param count Number of items
/// @param fp File handle
/// @return Number of items read
size_t sd_freeze_fread(void *ptr, size_t size, size_t count, sd_file_t fp);

/// @brief Write to SD file
/// @param ptr Buffer to write
/// @param size Item size in bytes
/// @param count Number of items
/// @param fp File handle
/// @return Number of items written
size_t sd_freeze_fwrite(const void *ptr, size_t size, size_t count, sd_file_t fp);

/// @brief Check if file exists
/// @param path File path
/// @return true if exists
bool sd_freeze_file_exists(const char *path);

/// @brief Get file size
/// @param path File path
/// @return Size in bytes, 0 if doesn't exist
uint32_t sd_freeze_file_size(const char *path);

/// @brief Create directory
/// @param path Directory path
/// @return true on success
bool sd_freeze_mkdir(const char *path);

/// @brief Delete a file
/// @param path File path
/// @return true on success
bool sd_freeze_delete(const char *path);

/// @brief Get free space on SD card
/// @return Free space in bytes
uint32_t sd_freeze_get_free_space(void);

/// @brief Flush any caches
void sd_freeze_flush(void);

// ============================================================================
// ERROR CODES
// ============================================================================

typedef enum {
    SD_FREEZE_OK = 0,
    SD_FREEZE_ERROR_NOT_INITIALIZED,
    SD_FREEZE_ERROR_FILE_NOT_FOUND,
    SD_FREEZE_ERROR_OPEN_FAILED,
    SD_FREEZE_ERROR_READ_FAILED,
    SD_FREEZE_ERROR_WRITE_FAILED,
    SD_FREEZE_ERROR_NOT_ENOUGH_SPACE,
} SdFreezeError;

/// @brief Get last error
/// @return Last error code
SdFreezeError sd_freeze_get_last_error(void);

/// @brief Get error string
/// @param error Error code
/// @return String description
const char* sd_freeze_error_to_string(SdFreezeError error);

#endif // SD_FREEZE_WRAPPER_H
