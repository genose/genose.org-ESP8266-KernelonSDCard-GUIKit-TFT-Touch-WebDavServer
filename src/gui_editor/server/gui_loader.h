/**
 * GUIKit Web Editor - GUI Loader
 * 
 * Load and save GUI JSON files from/to SD card
 */

#ifndef GUI_LOADER_H
#define GUI_LOADER_H

#include <stdint.h>
#include <stdbool.h>

// Maximum GUI JSON size
#define GUI_LOADER_MAX_SIZE 32768

/**
 * Initialize GUI loader
 * Must be called before any other functions
 * 
 * @return true on success, false on failure
 */
bool gui_loader_init(void);

/**
 * Shutdown GUI loader
 */
void gui_loader_shutdown(void);

/**
 * Load GUI JSON from file on SD card
 * 
 * @param path File path (e.g., "/gui/main_gui.json")
 * @param buffer Buffer to store JSON content
 * @param buffer_size Size of buffer
 * @return Number of bytes read, or negative on error
 */
int gui_loader_load(const char* path, char* buffer, int buffer_size);

/**
 * Save GUI JSON to file on SD card
 * 
 * @param path File path (e.g., "/gui/main_gui.json")
 * @param json JSON content to save
 * @param json_len Length of JSON content
 * @return true on success, false on failure
 */
bool gui_loader_save(const char* path, const char* json, int json_len);

/**
 * Check if GUI file exists on SD card
 * 
 * @param path File path
 * @return true if exists
 */
bool gui_loader_exists(const char* path);

/**
 * Get GUI file size
 * 
 * @param path File path
 * @return File size in bytes, or -1 if not found
 */
int gui_loader_get_size(const char* path);

/**
 * List GUI files in directory
 * 
 * @param path Directory path
 * @param files Array to store file names (must be pre-allocated)
 * @param max_files Maximum number of files to return
 * @return Number of files found
 */
int gui_loader_list_files(const char* path, char** files, int max_files);

/**
 * Load GUI from WebDAV server
 * 
 * @param filename GUI filename
 * @param buffer Buffer to store JSON content
 * @param buffer_size Size of buffer
 * @return Number of bytes read, or negative on error
 */
int gui_loader_load_from_webdav(const char* filename, char* buffer, int buffer_size);

/**
 * Load GUI from SD card
 * 
 * @param filename GUI filename
 * @return Pointer to GUI structure, or NULL on error
 * @note Caller must free the returned pointer
 */
void* gui_loader_load_from_sd(const char* filename);

/**
 * Parse JSON string to GUI structure
 * 
 * @param json JSON string
 * @return Pointer to GUI structure, or NULL on error
 * @note Caller must free the returned pointer
 */
void* gui_loader_parse_json(const char* json);

/**
 * Convert GUI structure to JSON string
 * 
 * @param gui GUI structure pointer
 * @return JSON string, or NULL on error
 * @note Caller must free the returned string
 */
char* gui_loader_to_json(void* gui);

/**
 * Validate GUI JSON
 * 
 * @param json JSON string to validate
 * @return true if valid, false otherwise
 */
bool gui_loader_validate_json(const char* json);

#endif // GUI_LOADER_H
