/**
 * @file sd_freeze_wrapper.cpp
 * @brief SD Card wrapper implementation using SdFat library
 * 
 * Adapts SdFat library to the freeze system interface
 * Can be replaced with different SD library implementations
 */

#include "sd_freeze_wrapper.h"
#include "SdFat.h"

// ============================================================================
// SD LIBRARY CONFIGURATION
// ============================================================================

// Use existing SD card instance from your project
// This assumes you have a global SdFat instance

// If you're using a different pin configuration, adjust these
extern SdFat SD;

// ============================================================================
// GLOBAL STATE
// ============================================================================

/// SD initialization state
static bool sd_initialized = false;

/// Last error code
static SdFreezeError last_error = SD_FREEZE_OK;

/// SdFat file type (for type safety)
typedef File SdFile;

// ============================================================================
// INITIALIZATION
// ============================================================================

bool sd_freeze_init(void) {
    if (sd_initialized) {
        return true;
    }
    
    // SD should already be initialized by your existing code
    // If not, you may need to initialize it here
    if (!SD.begin()) {
        last_error = SD_FREEZE_ERROR_NOT_INITIALIZED;
        return false;
    }
    
    sd_initialized = true;
    last_error = SD_FREEZE_OK;
    return true;
}

bool sd_freeze_is_ready(void) {
    return sd_initialized && SD.card()->isInitialized();
}

// ============================================================================
// FILE OPERATIONS
// ============================================================================

sd_file_t sd_freeze_fopen(const char *path, const char *mode) {
    if (!sd_freeze_is_ready()) {
        last_error = SD_FREEZE_ERROR_NOT_INITIALIZED;
        return NULL;
    }
    
    // SdFat doesn't use mode strings like fopen
    // We'll interpret the mode
    bool read_mode = (mode[0] == 'r');
    bool write_mode = (mode[0] == 'w' || mode[0] == 'a');
    bool binary_mode = (mode[1] == 'b');
    
    SdFile *file = new SdFile();
    
    if (read_mode && write_mode) {
        // "r+" or "w+" - not supported by SdFat, use read or write
        delete file;
        last_error = SD_FREEZE_ERROR_OPEN_FAILED;
        return NULL;
    }
    
    if (read_mode) {
        if (!file->open(path, O_READ)) {
            delete file;
            last_error = SD_FREEZE_ERROR_OPEN_FAILED;
            return NULL;
        }
    } else if (write_mode) {
        // Create or truncate
        if (!file->open(path, O_WRITE | O_CREAT | O_TRUNC)) {
            delete file;
            last_error = SD_FREEZE_ERROR_OPEN_FAILED;
            return NULL;
        }
    } else {
        // Append mode
        if (!file->open(path, O_WRITE | O_CREAT | O_APPEND)) {
            delete file;
            last_error = SD_FREEZE_ERROR_OPEN_FAILED;
            return NULL;
        }
    }
    
    last_error = SD_FREEZE_OK;
    return (sd_file_t)file;
}

void sd_freeze_fclose(sd_file_t fp) {
    if (fp) {
        SdFile *file = (SdFile*)fp;
        file->close();
        delete file;
    }
}

size_t sd_freeze_fread(void *ptr, size_t size, size_t count, sd_file_t fp) {
    if (!fp) {
        last_error = SD_FREEZE_ERROR_FILE_NOT_FOUND;
        return 0;
    }
    
    SdFile *file = (SdFile*)fp;
    size_t bytes_to_read = size * count;
    
    size_t result = file->read(ptr, bytes_to_read);
    
    if (result != bytes_to_read) {
        // Check if we hit EOF
        if (file->getWriteError()) {
            last_error = SD_FREEZE_ERROR_READ_FAILED;
        }
    }
    
    return result / size;
}

size_t sd_freeze_fwrite(const void *ptr, size_t size, size_t count, sd_file_t fp) {
    if (!fp) {
        last_error = SD_FREEZE_ERROR_FILE_NOT_FOUND;
        return 0;
    }
    
    SdFile *file = (SdFile*)fp;
    size_t bytes_to_write = size * count;
    
    size_t result = file->write(ptr, bytes_to_write);
    
    if (result != bytes_to_write) {
        if (file->getWriteError()) {
            last_error = SD_FREEZE_ERROR_WRITE_FAILED;
        }
    }
    
    return result / size;
}

// ============================================================================
// FILE SYSTEM OPERATIONS
// ============================================================================

bool sd_freeze_file_exists(const char *path) {
    if (!sd_freeze_is_ready()) {
        return false;
    }
    
    SdFile file;
    return file.exists(path);
}

uint32_t sd_freeze_file_size(const char *path) {
    if (!sd_freeze_is_ready()) {
        return 0;
    }
    
    SdFile file;
    if (!file.open(path, O_READ)) {
        return 0;
    }
    
    uint32_t size = file.fileSize();
    file.close();
    return size;
}

bool sd_freeze_mkdir(const char *path) {
    if (!sd_freeze_is_ready()) {
        return false;
    }
    
    // SdFat mkdir creates parent directories automatically
    return SD.mkdir(path);
}

bool sd_freeze_delete(const char *path) {
    if (!sd_freeze_is_ready()) {
        return false;
    }
    
    return SD.remove(path);
}

uint32_t sd_freeze_get_free_space(void) {
    if (!sd_freeze_is_ready()) {
        return 0;
    }
    
    // Get free space in clusters and convert to bytes
    uint32_t free_clusters = SD.vol()->freeClusterCount();
    uint32_t bytes_per_cluster = SD.vol()->bytesPerCluster();
    return free_clusters * bytes_per_cluster;
}

void sd_freeze_flush(void) {
    if (sd_initialized) {
        // Flush any caches
        // SdFat doesn't have a global flush, but we can sync
    }
}

// ============================================================================
// ERROR HANDLING
// ============================================================================

SdFreezeError sd_freeze_get_last_error(void) {
    return last_error;
}

const char* sd_freeze_error_to_string(SdFreezeError error) {
    switch (error) {
        case SD_FREEZE_OK: return "OK";
        case SD_FREEZE_ERROR_NOT_INITIALIZED: return "SD not initialized";
        case SD_FREEZE_ERROR_FILE_NOT_FOUND: return "File not found";
        case SD_FREEZE_ERROR_OPEN_FAILED: return "Open failed";
        case SD_FREEZE_ERROR_READ_FAILED: return "Read failed";
        case SD_FREEZE_ERROR_WRITE_FAILED: return "Write failed";
        case SD_FREEZE_ERROR_NOT_ENOUGH_SPACE: return "Not enough space";
        default: return "Unknown error";
    }
}
