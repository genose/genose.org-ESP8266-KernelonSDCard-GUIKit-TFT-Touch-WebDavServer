/**
 * GUIKit Web Editor - WebDAV Bridge Implementation
 */

#include "webdav_bridge.h"
#include "gui_loader.h"
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

// WebDAV bridge state
static bool webdav_initialized = false;
static char webdav_mount_path[GUIEDITOR_MAX_PATH_LEN] = "/gui";

// Initialize WebDAV bridge
bool gui_editor_webdav_init(const char* mount_path) {
    if (webdav_initialized) {
        return true; // Already initialized
    }

    if (mount_path != NULL) {
        strncpy(webdav_mount_path, mount_path, sizeof(webdav_mount_path) - 1);
        webdav_mount_path[sizeof(webdav_mount_path) - 1] = '\0';
    }

    // Initialize underlying systems if needed
    // This would typically initialize SD card, etc.
    // For now, just mark as initialized
    webdav_initialized = true;

    return true;
}

// Shutdown WebDAV bridge
void gui_editor_webdav_shutdown(void) {
    webdav_initialized = false;
}

// Check if initialized
bool gui_editor_webdav_is_initialized(void) {
    return webdav_initialized;
}

// List files in directory
int gui_editor_webdav_list_files(const char* path, char** files, int max_files, bool directories) {
    if (!webdav_initialized) {
        return GUIEDITOR_ERROR_WEBDAV_NOT_INIT;
    }

    // Implementation would use SD card or filesystem APIs
    // to list files in the specified directory
    // This is a placeholder implementation

    // For now, return empty list
    return 0;
}

// Load GUI JSON from file
int gui_editor_webdav_load_file(const char* filename, char* buffer, int buffer_size) {
    if (!webdav_initialized) {
        return GUIEDITOR_ERROR_WEBDAV_NOT_INIT;
    }

    if (filename == NULL || buffer == NULL || buffer_size <= 0) {
        return GUIEDITOR_ERROR_NOT_FOUND;
    }

    // Build absolute path
    char absolute_path[GUIEDITOR_MAX_PATH_LEN];
    snprintf(absolute_path, sizeof(absolute_path), "%s/%s", webdav_mount_path, filename);

    // Load from GUI loader (which uses SD card)
    return gui_loader_load(absolute_path, buffer, buffer_size);
}

// Save GUI JSON to file
bool gui_editor_webdav_save_file(const char* filename, const char* json, int json_len) {
    if (!webdav_initialized) {
        return false;
    }

    if (filename == NULL || json == NULL || json_len <= 0) {
        return false;
    }

    // Build absolute path
    char absolute_path[GUIEDITOR_MAX_PATH_LEN];
    snprintf(absolute_path, sizeof(absolute_path), "%s/%s", webdav_mount_path, filename);

    // Save via GUI loader
    return gui_loader_save(absolute_path, json, json_len);
}

// Delete GUI file
bool gui_editor_webdav_delete_file(const char* filename) {
    if (!webdav_initialized) {
        return false;
    }

    if (filename == NULL) {
        return false;
    }

    // Build absolute path
    char absolute_path[GUIEDITOR_MAX_PATH_LEN];
    snprintf(absolute_path, sizeof(absolute_path), "%s/%s", webdav_mount_path, filename);

    // Implementation would use filesystem APIs to delete the file
    // This is a placeholder
    return false;
}

// Check if file exists
bool gui_editor_webdav_file_exists(const char* filename) {
    if (!webdav_initialized) {
        return false;
    }

    if (filename == NULL) {
        return false;
    }

    // Build absolute path
    char absolute_path[GUIEDITOR_MAX_PATH_LEN];
    snprintf(absolute_path, sizeof(absolute_path), "%s/%s", webdav_mount_path, filename);

    // Check via GUI loader
    return gui_loader_exists(absolute_path);
}

// Get file size
int gui_editor_webdav_get_file_size(const char* filename) {
    if (!webdav_initialized) {
        return -1;
    }

    if (filename == NULL) {
        return -1;
    }

    // Build absolute path
    char absolute_path[GUIEDITOR_MAX_PATH_LEN];
    snprintf(absolute_path, sizeof(absolute_path), "%s/%s", webdav_mount_path, filename);

    // Get size via GUI loader
    return gui_loader_get_size(absolute_path);
}

// Rename file
bool gui_editor_webdav_rename_file(const char* old_name, const char* new_name) {
    if (!webdav_initialized) {
        return false;
    }

    if (old_name == NULL || new_name == NULL) {
        return false;
    }

    // Build absolute paths
    char old_path[GUIEDITOR_MAX_PATH_LEN];
    char new_path[GUIEDITOR_MAX_PATH_LEN];
    snprintf(old_path, sizeof(old_path), "%s/%s", webdav_mount_path, old_name);
    snprintf(new_path, sizeof(new_path), "%s/%s", webdav_mount_path, new_name);

    // Implementation would use filesystem APIs to rename the file
    // This is a placeholder
    return false;
}

// Create directory
bool gui_editor_webdav_create_dir(const char* path) {
    if (!webdav_initialized) {
        return false;
    }

    if (path == NULL) {
        return false;
    }

    // Implementation would use filesystem APIs to create directory
    // This is a placeholder
    return false;
}

// Get absolute path
bool gui_editor_webdav_get_absolute_path(const char* filename, char* absolute_path, int max_len) {
    if (!webdav_initialized || filename == NULL || absolute_path == NULL || max_len <= 0) {
        return false;
    }

    snprintf(absolute_path, max_len, "%s/%s", webdav_mount_path, filename);
    return true;
}

// WebDAV request handler
int gui_editor_webdav_handle_request(const char* method, const char* path,
                                      const uint8_t* data, int data_len,
                                      uint8_t* response, int response_len) {
    if (!webdav_initialized) {
        return -1;
    }

    // Check if this is a GUI Editor request
    if (strncmp(path, webdav_mount_path, strlen(webdav_mount_path)) != 0) {
        return -1; // Not a GUI Editor request
    }

    const char* relative_path = path + strlen(webdav_mount_path);

    // Skip leading slash
    if (*relative_path == '/') {
        relative_path++;
    }

    // Handle different HTTP methods
    if (strcmp(method, "GET") == 0) {
        // Load file
        char buffer[GUIEDITOR_MAX_JSON_SIZE];
        int bytes_read = gui_editor_webdav_load_file(relative_path, buffer, sizeof(buffer));
        
        if (bytes_read > 0) {
            if (bytes_read >= response_len) {
                bytes_read = response_len - 1;
            }
            memcpy(response, buffer, bytes_read);
            response[bytes_read] = '\0';
            return bytes_read;
        }
        return -1;

    } else if (strcmp(method, "PUT") == 0) {
        // Save file
        if (data != NULL && data_len > 0) {
            // Add null terminator
            char json_buffer[GUIEDITOR_MAX_JSON_SIZE];
            if (data_len >= sizeof(json_buffer)) {
                return -1; // Too large
            }
            memcpy(json_buffer, data, data_len);
            json_buffer[data_len] = '\0';

            if (gui_editor_webdav_save_file(relative_path, json_buffer, data_len)) {
                return snprintf((char*)response, response_len, "OK");
            }
        }
        return -1;

    } else if (strcmp(method, "DELETE") == 0) {
        // Delete file
        if (gui_editor_webdav_delete_file(relative_path)) {
            return snprintf((char*)response, response_len, "OK");
        }
        return -1;
    }

    return -1; // Unsupported method
}
