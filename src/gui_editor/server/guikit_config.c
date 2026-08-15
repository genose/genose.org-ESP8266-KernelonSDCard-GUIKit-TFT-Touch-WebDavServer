/**
 * GUIKit Configuration Loader Implementation
 * 
 * Handles /etc/guikitloader.conf configuration file
 * for specifying default GUI and project loading behavior
 */

#include "guikit_config.h"
#include "gui_loader.h"
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

// Configuration file path
#define GUIKIT_CONFIG_FILE "/etc/guikitloader.conf"

// Configuration file format:
// This is a simple key=value format for ESP8266 compatibility
// Example:
// default_gui=MyProject
// gui_path=/gui
// use_project_dirs=true
// auto_load_last=true
// last_gui=MyProject

// Default configuration values
#define DEFAULT_GUI_PATH "/gui"
#define DEFAULT_USE_PROJECT_DIRS true
#define DEFAULT_AUTO_LOAD_LAST true

// ============================================================================
// Initialization
// ============================================================================

void guikit_config_init(GuikitConfig* config) {
    if (config == NULL) return;
    
    strncpy(config->default_gui, "", sizeof(config->default_gui) - 1);
    config->default_gui[sizeof(config->default_gui) - 1] = '\0';
    
    strncpy(config->gui_path, DEFAULT_GUI_PATH, sizeof(config->gui_path) - 1);
    config->gui_path[sizeof(config->gui_path) - 1] = '\0';
    
    config->use_project_dirs = DEFAULT_USE_PROJECT_DIRS;
    config->auto_load_last = DEFAULT_AUTO_LOAD_LAST;
    
    strncpy(config->last_gui, "", sizeof(config->last_gui) - 1);
    config->last_gui[sizeof(config->last_gui) - 1] = '\0';
}

// ============================================================================
// Configuration Parsing
// ============================================================================

/**
 * Trim whitespace from both ends of a string
 */
static char* trim_whitespace(char* str) {
    if (str == NULL) return NULL;
    
    // Trim leading whitespace
    while (*str == ' ' || *str == '\t' || *str == '\r' || *str == '\n') {
        str++;
    }
    
    // Trim trailing whitespace
    char* end = str + strlen(str) - 1;
    while (end >= str && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) {
        *end = '\0';
        end--;
    }
    
    return str;
}

/**
 * Parse a boolean value from string
 */
static bool parse_bool(const char* value) {
    if (value == NULL) return false;
    
    char lower[32];
    strncpy(lower, value, sizeof(lower) - 1);
    lower[sizeof(lower) - 1] = '\0';
    
    // Convert to lowercase
    for (int i = 0; lower[i]; i++) {
        if (lower[i] >= 'A' && lower[i] <= 'Z') {
            lower[i] = lower[i] - 'A' + 'a';
        }
    }
    
    return (strcmp(lower, "true") == 0 || strcmp(lower, "1") == 0 || strcmp(lower, "yes") == 0);
}

/**
 * Parse a configuration line
 */
static bool parse_config_line(const char* line, GuikitConfig* config) {
    if (line == NULL || config == NULL || line[0] == '#' || line[0] == '\0') {
        return true; // Skip comments and empty lines
    }
    
    // Find the equals sign
    char* equals = strchr(line, '=');
    if (equals == NULL) {
        return true; // Skip malformed lines
    }
    
    // Split key and value
    *equals = '\0';
    char* key = trim_whitespace((char*)line);
    char* value = trim_whitespace(equals + 1);
    
    if (key == NULL || value == NULL) {
        return true;
    }
    
    // Process each key
    if (strcmp(key, "default_gui") == 0) {
        strncpy(config->default_gui, value, sizeof(config->default_gui) - 1);
        config->default_gui[sizeof(config->default_gui) - 1] = '\0';
    } else if (strcmp(key, "gui_path") == 0) {
        strncpy(config->gui_path, value, sizeof(config->gui_path) - 1);
        config->gui_path[sizeof(config->gui_path) - 1] = '\0';
    } else if (strcmp(key, "use_project_dirs") == 0) {
        config->use_project_dirs = parse_bool(value);
    } else if (strcmp(key, "auto_load_last") == 0) {
        config->auto_load_last = parse_bool(value);
    } else if (strcmp(key, "last_gui") == 0) {
        strncpy(config->last_gui, value, sizeof(config->last_gui) - 1);
        config->last_gui[sizeof(config->last_gui) - 1] = '\0';
    }
    
    return true;
}

bool guikit_config_parse(const char* buffer, GuikitConfig* config) {
    if (buffer == NULL || config == NULL) {
        return false;
    }
    
    // Initialize with defaults
    guikit_config_init(config);
    
    // Make a copy we can modify
    char* buffer_copy = strdup(buffer);
    if (buffer_copy == NULL) {
        return false;
    }
    
    // Split into lines
    char* line = strtok(buffer_copy, "\n\r");
    while (line != NULL) {
        parse_config_line(line, config);
        line = strtok(NULL, "\n\r");
    }
    
    free(buffer_copy);
    return true;
}

// ============================================================================
// Configuration Generation
// ============================================================================

int guikit_config_generate(const GuikitConfig* config, char* buffer, int buffer_size) {
    if (config == NULL || buffer == NULL || buffer_size <= 0) {
        return -1;
    }
    
    int written = 0;
    
    // Write configuration
    written += snprintf(buffer + written, buffer_size - written, "# GUIKit Configuration\n");
    written += snprintf(buffer + written, buffer_size - written, "# Generated by GUIKit Web Editor\n\n");
    
    written += snprintf(buffer + written, buffer_size - written, "default_gui=%s\n", 
                       config->default_gui[0] ? config->default_gui : "");
    written += snprintf(buffer + written, buffer_size - written, "gui_path=%s\n", 
                       config->gui_path[0] ? config->gui_path : DEFAULT_GUI_PATH);
    written += snprintf(buffer + written, buffer_size - written, "use_project_dirs=%s\n", 
                       config->use_project_dirs ? "true" : "false");
    written += snprintf(buffer + written, buffer_size - written, "auto_load_last=%s\n", 
                       config->auto_load_last ? "true" : "false");
    written += snprintf(buffer + written, buffer_size - written, "last_gui=%s\n", 
                       config->last_gui[0] ? config->last_gui : "");
    
    return (written < buffer_size) ? written : buffer_size;
}

// ============================================================================
// File I/O (using gui_loader for SD card access)
// ============================================================================

bool guikit_config_load(GuikitConfig* config) {
    if (config == NULL) {
        return false;
    }
    
    // Try to load from SD card
    char buffer[GUIKIT_CONFIG_MAX_SIZE];
    int bytes_read = gui_loader_load(GUIKIT_CONFIG_FILE, buffer, sizeof(buffer));
    
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        return guikit_config_parse(buffer, config);
    }
    
    // File doesn't exist, use defaults
    guikit_config_init(config);
    return true;
}

bool guikit_config_save(const GuikitConfig* config) {
    if (config == NULL) {
        return false;
    }
    
    char buffer[GUIKIT_CONFIG_MAX_SIZE];
    int bytes_written = guikit_config_generate(config, buffer, sizeof(buffer));
    
    if (bytes_written <= 0) {
        return false;
    }
    
    return gui_loader_save(GUIKIT_CONFIG_FILE, buffer, bytes_written);
}

// ============================================================================
// Project Discovery
// ============================================================================

// Simple directory listing parser for ESP8266
// This is a placeholder - actual implementation would use SD library
int guikit_config_find_projects(const char* path, char** projects, int max_projects) {
    if (path == NULL || projects == NULL || max_projects <= 0) {
        return 0;
    }
    
    // On ESP8266, this would list directories and filter for .GUIKIT suffix
    // For now, return 0 as placeholder
    return 0;
}

// ============================================================================
// Project GUI Loading
// ============================================================================

void guikit_config_get_project_gui_path(const char* project_name, const char* gui_filename, char* full_path, int path_size) {
    if (project_name == NULL || gui_filename == NULL || full_path == NULL || path_size <= 0) {
        if (full_path && path_size > 0) {
            full_path[0] = '\0';
        }
        return;
    }
    
    // Build path: /gui/{project_name}.GUIKIT/{gui_filename}
    snprintf(full_path, path_size, "/gui/%s.GUIKIT/%s", project_name, gui_filename);
    full_path[path_size - 1] = '\0';
}

int guikit_config_load_project_gui(const char* project_name, const char* gui_filename, char* buffer, int buffer_size) {
    if (project_name == NULL || gui_filename == NULL || buffer == NULL || buffer_size <= 0) {
        return -1;
    }
    
    char full_path[GUIKIT_CONFIG_MAX_PATH];
    guikit_config_get_project_gui_path(project_name, gui_filename, full_path, sizeof(full_path));
    
    return gui_loader_load(full_path, buffer, buffer_size);
}

// ============================================================================
// Configuration Management
// ============================================================================

bool guikit_config_set_default(const char* project_name) {
    if (project_name == NULL) {
        return false;
    }
    
    GuikitConfig config;
    if (!guikit_config_load(&config)) {
        guikit_config_init(&config);
    }
    
    strncpy(config.default_gui, project_name, sizeof(config.default_gui) - 1);
    config.default_gui[sizeof(config.default_gui) - 1] = '\0';
    
    return guikit_config_save(&config);
}

// ============================================================================
// Default GUI Loading
// ============================================================================

/**
 * Load the default GUI based on configuration
 * 
 * @param buffer Buffer to store JSON content
 * @param buffer_size Size of buffer
 * @return Number of bytes read, or -1 on error
 */
int guikit_config_load_default_gui(char* buffer, int buffer_size) {
    if (buffer == NULL || buffer_size <= 0) {
        return -1;
    }
    
    GuikitConfig config;
    if (!guikit_config_load(&config)) {
        // If config fails, try loading from default location
        return gui_loader_load("/gui/main_gui.json", buffer, buffer_size);
    }
    
    // Try to load the configured default GUI
    if (config.default_gui[0] != '\0') {
        char full_path[GUIKIT_CONFIG_MAX_PATH];
        guikit_config_get_project_gui_path(config.default_gui, "main_gui.json", full_path, sizeof(full_path));
        
        int bytes_read = gui_loader_load(full_path, buffer, buffer_size);
        if (bytes_read > 0) {
            // Update last_gui if auto_load_last is enabled
            if (config.auto_load_last) {
                strncpy(config.last_gui, config.default_gui, sizeof(config.last_gui) - 1);
                config.last_gui[sizeof(config.last_gui) - 1] = '\0';
                guikit_config_save(&config);
            }
            return bytes_read;
        }
    }
    
    // Fallback: try loading from gui_path
    char fallback_path[GUIKIT_CONFIG_MAX_PATH];
    snprintf(fallback_path, sizeof(fallback_path), "%s/main_gui.json", config.gui_path);
    return gui_loader_load(fallback_path, buffer, buffer_size);
}

/**
 * Load the last used GUI
 * 
 * @param buffer Buffer to store JSON content
 * @param buffer_size Size of buffer
 * @return Number of bytes read, or -1 on error
 */
int guikit_config_load_last_gui(char* buffer, int buffer_size) {
    if (buffer == NULL || buffer_size <= 0) {
        return -1;
    }
    
    GuikitConfig config;
    if (!guikit_config_load(&config)) {
        return -1;
    }
    
    if (config.last_gui[0] == '\0') {
        return -1; // No last GUI recorded
    }
    
    char full_path[GUIKIT_CONFIG_MAX_PATH];
    guikit_config_get_project_gui_path(config.last_gui, "main_gui.json", full_path, sizeof(full_path));
    
    return gui_loader_load(full_path, buffer, buffer_size);
}
