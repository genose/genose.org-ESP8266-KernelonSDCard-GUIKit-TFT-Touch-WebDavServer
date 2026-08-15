/**
 * GUIKit Configuration Loader
 * 
 * Handles /etc/guikitloader.conf configuration file
 * for specifying default GUI and project loading behavior
 */

#ifndef GUIKIT_CONFIG_H
#define GUIKIT_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

// Maximum configuration file size
#define GUIKIT_CONFIG_MAX_SIZE 1024

// Maximum path length
#define GUIKIT_CONFIG_MAX_PATH 256

// Configuration structure
typedef struct {
    char default_gui[GUIKIT_CONFIG_MAX_PATH];  // Default GUI project name (without .GUIKIT suffix)
    char gui_path[GUIKIT_CONFIG_MAX_PATH];     // Path to GUI directory (default: /gui)
    bool use_project_dirs;                     // Use .GUIKIT directories (true) or flat files (false)
    bool auto_load_last;                       // Auto-load last used GUI
    char last_gui[GUIKIT_CONFIG_MAX_PATH];     // Last loaded GUI
} GuikitConfig;

/**
 * Initialize configuration with defaults
 * 
 * @param config Pointer to config structure
 */
void guikit_config_init(GuikitConfig* config);

/**
 * Load configuration from /etc/guikitloader.conf
 * 
 * @param config Pointer to config structure to populate
 * @return true on success, false on failure
 */
bool guikit_config_load(GuikitConfig* config);

/**
 * Save configuration to /etc/guikitloader.conf
 * 
 * @param config Pointer to config structure to save
 * @return true on success, false on failure
 */
bool guikit_config_save(const GuikitConfig* config);

/**
 * Parse configuration from buffer
 * 
 * @param buffer Configuration file content
 * @param config Pointer to config structure to populate
 * @return true on success, false on failure
 */
bool guikit_config_parse(const char* buffer, GuikitConfig* config);

/**
 * Generate configuration file content from structure
 * 
 * @param config Pointer to config structure
 * @param buffer Buffer to write to
 * @param buffer_size Size of buffer
 * @return Number of bytes written, or -1 on error
 */
int guikit_config_generate(const GuikitConfig* config, char* buffer, int buffer_size);

/**
 * Find all .GUIKIT directories on the SD card
 * 
 * @param path Base path to search (e.g., "/gui")
 * @param projects Array to store project names (without .GUIKIT suffix)
 * @param max_projects Maximum number of projects to return
 * @return Number of projects found
 */
int guikit_config_find_projects(const char* path, char** projects, int max_projects);

/**
 * Load GUI from a .GUIKIT project directory
 * 
 * @param project_name Project name (without .GUIKIT suffix)
 * @param gui_filename GUI file to load (e.g., "main_gui.json")
 * @param buffer Buffer to store JSON content
 * @param buffer_size Size of buffer
 * @return Number of bytes read, or -1 on error
 */
int guikit_config_load_project_gui(const char* project_name, const char* gui_filename, char* buffer, int buffer_size);

/**
 * Get the full path to a project's GUI file
 * 
 * @param project_name Project name (without .GUIKIT suffix)
 * @param gui_filename GUI file name (e.g., "main_gui.json")
 * @param full_path Buffer to store full path
 * @param path_size Size of buffer
 */
void guikit_config_get_project_gui_path(const char* project_name, const char* gui_filename, char* full_path, int path_size);

/**
 * Set the default GUI project
 * 
 * @param project_name Project name to set as default
 * @return true on success, false on failure
 */
bool guikit_config_set_default(const char* project_name);

#endif // GUIKIT_CONFIG_H
