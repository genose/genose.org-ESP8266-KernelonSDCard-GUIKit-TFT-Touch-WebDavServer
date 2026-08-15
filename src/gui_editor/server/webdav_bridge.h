/**
 * GUIKit Web Editor - WebDAV Bridge
 * 
 * Bridge between GUI Editor and WebDAV server for file management
 * on ESP8266.
 */

#ifndef WEBDAV_BRIDGE_H
#define WEBDAV_BRIDGE_H

#include <stdint.h>
#include <stdbool.h>

// Maximum file path length
#define GUIEDITOR_MAX_PATH_LEN 128

// Maximum JSON GUI size (in bytes)
#define GUIEDITOR_MAX_JSON_SIZE 32768

// Error codes
#define GUIEDITOR_OK 0
#define GUIEDITOR_ERROR_NOT_FOUND -1
#define GUIEDITOR_ERROR_INVALID_JSON -2
#define GUIEDITOR_ERROR_FILE_TOO_LARGE -3
#define GUIEDITOR_ERROR_SD_NOT_READY -4
#define GUIEDITOR_ERROR_WEBDAV_NOT_INIT -5

/**
 * Initialize WebDAV bridge for GUI Editor
 * Must be called before any other functions
 * 
 * @param webdav_mount_path Mount path for GUI files on WebDAV
 * @return true on success, false on failure
 */
bool gui_editor_webdav_init(const char* webdav_mount_path);

/**
 * Shutdown WebDAV bridge
 */
void gui_editor_webdav_shutdown(void);

/**
 * Check if WebDAV bridge is initialized
 * 
 * @return true if initialized
 */
bool gui_editor_webdav_is_initialized(void);

/**
 * List GUI files in a directory
 * 
 * @param path Directory path (relative to mount point)
 * @param files Array to store file names (must be pre-allocated)
 * @param max_files Maximum number of files to return
 * @param directories Only list directories
 * @return Number of files found, or negative error code
 */
int gui_editor_webdav_list_files(const char* path, char** files, int max_files, bool directories);

/**
 * Load GUI JSON from file
 * 
 * @param filename File name (relative to mount point)
 * @param buffer Buffer to store JSON content
 * @param buffer_size Size of buffer
 * @return Number of bytes read, or negative error code
 */
int gui_editor_webdav_load_file(const char* filename, char* buffer, int buffer_size);

/**
 * Save GUI JSON to file
 * 
 * @param filename File name (relative to mount point)
 * @param json JSON content to save
 * @param json_len Length of JSON content
 * @return true on success, false on failure
 */
bool gui_editor_webdav_save_file(const char* filename, const char* json, int json_len);

/**
 * Delete GUI file
 * 
 * @param filename File name to delete
 * @return true on success, false on failure
 */
bool gui_editor_webdav_delete_file(const char* filename);

/**
 * Check if GUI file exists
 * 
 * @param filename File name to check
 * @return true if exists
 */
bool gui_editor_webdav_file_exists(const char* filename);

/**
 * Get file size
 * 
 * @param filename File name
 * @return File size in bytes, or -1 if not found
 */
int gui_editor_webdav_get_file_size(const char* filename);

/**
 * Rename GUI file
 * 
 * @param old_name Old file name
 * @param new_name New file name
 * @return true on success, false on failure
 */
bool gui_editor_webdav_rename_file(const char* old_name, const char* new_name);

/**
 * Create directory
 * 
 * @param path Directory path to create
 * @return true on success, false on failure
 */
bool gui_editor_webdav_create_dir(const char* path);

/**
 * Get absolute path from relative GUI filename
 * 
 * @param filename Relative GUI filename
 * @param absolute_path Output buffer for absolute path
 * @param max_len Maximum length of output buffer
 * @return true on success
 */
bool gui_editor_webdav_get_absolute_path(const char* filename, char* absolute_path, int max_len);

/**
 * WebDAV callback for GUI Editor
 * Called when WebDAV server receives GUI-related requests
 * 
 * @param method HTTP method (GET, PUT, DELETE, etc.)
 * @param path Request path
 * @param data Request data (for PUT/POST)
 * @param data_len Data length
 * @param response Response buffer
 * @param response_len Maximum response length
 * @return Number of bytes written to response, or -1 on error
 */
int gui_editor_webdav_handle_request(const char* method, const char* path,
                                      const uint8_t* data, int data_len,
                                      uint8_t* response, int response_len);

#endif // WEBDAV_BRIDGE_H
