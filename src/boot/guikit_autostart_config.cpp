/**
 * @file guikit_autostart_config.cpp
 * @brief GUIKit Autostart Configuration Implementation
 * 
 * Reads /etc/GUIKIT_autostart.ini and manages:
 * - Kernel loading configuration
 * - Memory strategy and bank allocation
 * - GUI start configuration
 * - Memory allocation rules
 */

#include "guikit_autostart_config.h"
#include "ini_parser.h"
#include "sd_freeze_wrapper.h"
#include "crc32.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// STATIC CALLBACKS FOR INI PARSING
// ============================================================================

typedef struct {
    GUIKitAutostartConfig *config;
    bool in_kernel_section;
    bool in_memory_section;
    bool in_gui_section;
    bool in_allocations_section;
    bool in_flags_section;
    uint8_t allocation_index;
} ParseContext;

static void section_callback(void *user_data, const char *section) {
    ParseContext *ctx = (ParseContext*)user_data;
    if (!ctx || !section) return;
    
    ctx->in_kernel_section = (strcmp(section, INI_SECTION_KERNEL) == 0);
    ctx->in_memory_section = (strcmp(section, INI_SECTION_MEMORY) == 0);
    ctx->in_gui_section = (strcmp(section, INI_SECTION_GUI) == 0);
    ctx->in_allocations_section = (strcmp(section, INI_SECTION_ALLOCATIONS) == 0);
    ctx->in_flags_section = (strcmp(section, INI_SECTION_FLAGS) == 0);
}

static void key_callback(void *user_data, const char *section, const char *key, const char *value) {
    ParseContext *ctx = (ParseContext*)user_data;
    if (!ctx || !ctx->config) return;
    
    // Kernel section
    if (ctx->in_kernel_section) {
        if (strcmp(key, "path") == 0) {
            strncpy(ctx->config->kernel.path, value, sizeof(ctx->config->kernel.path) - 1);
        } else if (strcmp(key, "compress") == 0) {
            ctx->config->kernel.compress = ini_get_bool(NULL, NULL, key, value, false);
        } else if (strcmp(key, "expected_size") == 0) {
            ctx->config->kernel.expected_size = ini_get_uint(NULL, NULL, key, value, 0);
        } else if (strcmp(key, "max_size") == 0) {
            ctx->config->kernel.max_size = ini_get_uint(NULL, NULL, key, value, 0);
        } else if (strcmp(key, "verify") == 0) {
            ctx->config->kernel.verify = ini_get_bool(NULL, NULL, key, value, true);
        }
        return;
    }
    
    // Memory section
    if (ctx->in_memory_section) {
        if (strcmp(key, "strategy") == 0) {
            if (strcasecmp(value, "auto") == 0) {
                ctx->config->strategy = MEM_STRATEGY_AUTO;
            } else if (strcasecmp(value, "external_first") == 0) {
                ctx->config->strategy = MEM_STRATEGY_EXTERNAL_FIRST;
            } else if (strcasecmp(value, "sd_swap_first") == 0) {
                ctx->config->strategy = MEM_STRATEGY_SD_SWAP_FIRST;
            } else if (strcasecmp(value, "internal_only") == 0) {
                ctx->config->strategy = MEM_STRATEGY_INTERNAL_ONLY;
            } else if (strcasecmp(value, "custom") == 0) {
                ctx->config->strategy = MEM_STRATEGY_CUSTOM;
            }
        } else if (strcmp(key, "stop_at_first_success") == 0) {
            ctx->config->strategy_stop_at_first_success = ini_get_bool(NULL, NULL, key, value, true);
        }
        return;
    }
    
    // GUI section
    if (ctx->in_gui_section) {
        if (strcmp(key, "path") == 0) {
            strncpy(ctx->config->gui.gui_path, value, sizeof(ctx->config->gui.gui_path) - 1);
        } else if (strcmp(key, "auto_start") == 0) {
            ctx->config->gui.auto_start = ini_get_bool(NULL, NULL, key, value, true);
        } else if (strcmp(key, "theme") == 0) {
            strncpy(ctx->config->gui.theme, value, sizeof(ctx->config->gui.theme) - 1);
        } else if (strcmp(key, "width") == 0) {
            ctx->config->gui.width = (uint16_t)ini_get_uint(NULL, NULL, key, value, 0);
        } else if (strcmp(key, "height") == 0) {
            ctx->config->gui.height = (uint16_t)ini_get_uint(NULL, NULL, key, value, 0);
        }
        return;
    }
    
    // Allocations section
    if (ctx->in_allocations_section && ctx->allocation_index < 16) {
        MemAllocationRule *rule = &ctx->config->allocations[ctx->allocation_index];
        
        if (strcmp(key, "name") == 0) {
            // Will set name when we have all fields
        } else if (strcmp(key, "bank") == 0) {
            if (strcasecmp(value, "internal") == 0) {
                rule->bank = MEM_BANK_INTERNAL;
            } else if (strcasecmp(value, "external") == 0) {
                rule->bank = MEM_BANK_EXTERNAL;
            } else if (strcasecmp(value, "sd_swap") == 0) {
                rule->bank = MEM_BANK_SD_SWAP;
            }
        } else if (strcmp(key, "fallback") == 0) {
            if (strcasecmp(value, "internal") == 0) {
                rule->fallback = MEM_BANK_INTERNAL;
            } else if (strcasecmp(value, "external") == 0) {
                rule->fallback = MEM_BANK_EXTERNAL;
            } else if (strcasecmp(value, "sd_swap") == 0) {
                rule->fallback = MEM_BANK_SD_SWAP;
            }
        } else if (strcmp(key, "size") == 0) {
            rule->size = ini_get_uint(NULL, NULL, key, value, 0);
        } else if (strcmp(key, "replace") == 0) {
            rule->replace = ini_get_bool(NULL, NULL, key, value, false);
        }
        
        // If we have all fields, increment
        if (rule->bank != MEM_BANK_COUNT && rule->size > 0) {
            ctx->allocation_index++;
        }
        ctx->config->allocation_count = ctx->allocation_index;
        return;
    }
    
    // Flags section
    if (ctx->in_flags_section) {
        if (strcmp(key, "debug") == 0) {
            ctx->config->debug = ini_get_bool(NULL, NULL, key, value, false);
        } else if (strcmp(key, "tft_boot_messages") == 0) {
            ctx->config->tft_boot_messages = ini_get_bool(NULL, NULL, key, value, true);
        } else if (strcmp(key, "serial_boot_messages") == 0) {
            ctx->config->serial_boot_messages = ini_get_bool(NULL, NULL, key, value, true);
        }
        return;
    }
}

// ============================================================================
// INITIALIZATION AND DEFAULT CONFIG
// ============================================================================

void guikit_autostart_init_default(GUIKitAutostartConfig *config) {
    if (!config) return;
    
    *config = GUIKIT_AUTOSTART_DEFAULT_CONFIG;
    
    // Initialize bank names
    config->banks[MEM_BANK_INTERNAL].name = "internal";
    config->banks[MEM_BANK_EXTERNAL].name = "external";
    config->banks[MEM_BANK_SD_SWAP].name = "sd_swap";
}

// ============================================================================
// INI PARSING
// ============================================================================

bool guikit_autostart_parse_ini(GUIKitAutostartConfig *config, const char *filepath) {
    if (!config || !filepath) return false;
    
    // Initialize with defaults
    guikit_autostart_init_default(config);
    
    // Create parse context
    ParseContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.config = config;
    ctx.allocation_index = 0;
    
    // Parse the file
    bool result = ini_parse_file(filepath, &ctx, section_callback, key_callback);
    
    if (result) {
        strncpy(config->config_file, filepath, sizeof(config->config_file) - 1);
    }
    
    return result;
}

// ============================================================================
// CONFIG VALIDATION
// ============================================================================

bool guikit_autostart_validate(const GUIKitAutostartConfig *config) {
    if (!config) return false;
    
    // Check kernel path
    if (config->kernel.path[0] == '\0') {
        return false;
    }
    
    // Check at least one bank is enabled
    bool bank_enabled = false;
    for (uint8_t i = 0; i < MEM_BANK_COUNT; i++) {
        if (config->banks[i].enabled) {
            bank_enabled = true;
            break;
        }
    }
    if (!bank_enabled) return false;
    
    // Check GUI path if auto_start
    if (config->gui.auto_start && config->gui.gui_path[0] == '\0') {
        return false;
    }
    
    return true;
}

// ============================================================================
// BANK DETECTION
// ============================================================================

void guikit_autostart_detect_banks(GUIKitAutostartConfig *config) {
    if (!config) return;
    
    // This is a placeholder - implement with your hardware detection
    // For now, we'll just mark all banks as potentially available
    
    // ESP8266 defaults
    #ifdef ESP8266
    config->banks[MEM_BANK_INTERNAL].start = 0x20000000;
    config->banks[MEM_BANK_INTERNAL].size = 80 * 1024;  // 80KB
    config->banks[MEM_BANK_INTERNAL].available = true;
    
    config->banks[MEM_BANK_EXTERNAL].available = false;  // Not typically on ESP8266
    config->banks[MEM_BANK_SD_SWAP].available = true;     // If SD card present
    
    // ESP32 defaults
    #elif defined(ESP32)
    config->banks[MEM_BANK_INTERNAL].start = 0x3FFE0000;
    config->banks[MEM_BANK_INTERNAL].size = 320 * 1024;  // 320KB
    config->banks[MEM_BANK_INTERNAL].available = true;
    
    config->banks[MEM_BANK_EXTERNAL].available = false;  // Check PSRAM
    config->banks[MEM_BANK_SD_SWAP].available = true;     // If SD card present
    
    // Generic
    #else
    config->banks[MEM_BANK_INTERNAL].available = true;
    config->banks[MEM_BANK_EXTERNAL].available = false;
    config->banks[MEM_BANK_SD_SWAP].available = true;
    #endif
    
    // SD card is typically always available if present
    // This should be detected by your SD initialization
}

// ============================================================================
// KERNEL LOADING
// ============================================================================

bool guikit_autostart_find_kernel(const GUIKitAutostartConfig *config,
                                    char *found_path, uint32_t found_path_size) {
    if (!config || !found_path || found_path_size == 0) return false;
    
    // Try to find kernel in common locations
    const char *search_paths[] = {
        config->kernel.path,
        "/kernel.bin",
        "/Kernel.bin",
        "/kernel.gz",
        "/Kernel.gz",
        "/system/kernel.bin",
        "/system/Kernel.bin",
        "/system/kernel.gz",
        "/system/Kernel.gz",
        "/gui/kernel.bin",
        NULL
    };
    
    for (int i = 0; search_paths[i]; i++) {
        if (sd_freeze_file_exists(search_paths[i])) {
            strncpy(found_path, search_paths[i], found_path_size - 1);
            found_path[found_path_size - 1] = '\0';
            return true;
        }
    }
    
    return false;
}

bool guikit_autostart_load_kernel(const GUIKitAutostartConfig *config,
                                   uint8_t *buffer, uint32_t buffer_size) {
    if (!config) return false;
    
    // Find kernel
    char kernel_path[64];
    if (!guikit_autostart_find_kernel(config, kernel_path, sizeof(kernel_path))) {
        return false;
    }
    
    // Get file size
    uint32_t file_size = sd_freeze_file_size(kernel_path);
    if (file_size == 0) return false;
    
    // Check max size if configured
    if (config->kernel.max_size > 0 && file_size > config->kernel.max_size) {
        return false;
    }
    
    // Load file
    sd_file_t fp = sd_freeze_fopen(kernel_path, "rb");
    if (!fp) return false;
    
    size_t bytes_to_read = (buffer && buffer_size > 0) ? min(buffer_size, file_size) : file_size;
    size_t read = sd_freeze_fread(buffer, 1, bytes_to_read, fp);
    sd_freeze_fclose(fp);
    
    if (read != bytes_to_read) return false;
    
    // Verify checksum if enabled
    if (config->kernel.verify) {
        // For now, we don't have a stored checksum, so we'll skip
        // In a real implementation, you might store the checksum in the config
        // or in a separate file
    }
    
    return true;
}

bool guikit_autostart_verify_kernel(const uint8_t *buffer, uint32_t size, uint32_t expected_crc) {
    if (!buffer || size == 0) return false;
    if (expected_crc == 0) return true; // Skip verification
    
    uint32_t actual_crc = crc32(buffer, size);
    return (actual_crc == expected_crc);
}

// ============================================================================
// MEMORY MANAGEMENT
// ============================================================================

bool guikit_autostart_bank_can_fit(GUIKitAutostartConfig *config, MemBankType bank, uint32_t size) {
    if (!config || bank >= MEM_BANK_COUNT) return false;
    
    MemBankConfig *bank_config = &config->banks[bank];
    if (!bank_config->available || !bank_config->enabled) return false;
    
    // For SD swap, we can always fit (it uses SD card space)
    if (bank == MEM_BANK_SD_SWAP) {
        return (size <= sd_freeze_get_free_space());
    }
    
    // For internal/external RAM, check if size fits
    return (size <= bank_config->size);
}

MemBankType guikit_autostart_apply_strategy(GUIKitAutostartConfig *config, uint32_t size) {
    if (!config) return MEM_BANK_COUNT;
    
    // Try banks in order based on strategy
    switch (config->strategy) {
        case MEM_STRATEGY_AUTO:
        case MEM_STRATEGY_EXTERNAL_FIRST:
            // Try external first
            if (guikit_autostart_bank_can_fit(config, MEM_BANK_EXTERNAL, size)) {
                return MEM_BANK_EXTERNAL;
            }
            // Fall through to SD swap
            if (guikit_autostart_bank_can_fit(config, MEM_BANK_SD_SWAP, size)) {
                return MEM_BANK_SD_SWAP;
            }
            // Fall through to internal
            if (guikit_autostart_bank_can_fit(config, MEM_BANK_INTERNAL, size)) {
                return MEM_BANK_INTERNAL;
            }
            break;
            
        case MEM_STRATEGY_SD_SWAP_FIRST:
            if (guikit_autostart_bank_can_fit(config, MEM_BANK_SD_SWAP, size)) {
                return MEM_BANK_SD_SWAP;
            }
            if (guikit_autostart_bank_can_fit(config, MEM_BANK_EXTERNAL, size)) {
                return MEM_BANK_EXTERNAL;
            }
            if (guikit_autostart_bank_can_fit(config, MEM_BANK_INTERNAL, size)) {
                return MEM_BANK_INTERNAL;
            }
            break;
            
        case MEM_STRATEGY_INTERNAL_ONLY:
            if (guikit_autostart_bank_can_fit(config, MEM_BANK_INTERNAL, size)) {
                return MEM_BANK_INTERNAL;
            }
            break;
            
        case MEM_STRATEGY_CUSTOM:
            // For custom strategy, use allocation rules
            // This would need more complex logic
            if (guikit_autostart_bank_can_fit(config, MEM_BANK_EXTERNAL, size)) {
                return MEM_BANK_EXTERNAL;
            }
            if (guikit_autostart_bank_can_fit(config, MEM_BANK_SD_SWAP, size)) {
                return MEM_BANK_SD_SWAP;
            }
            if (guikit_autostart_bank_can_fit(config, MEM_BANK_INTERNAL, size)) {
                return MEM_BANK_INTERNAL;
            }
            break;
    }
    
    return MEM_BANK_COUNT; // No bank available
}

// Simple memory allocator - this is a placeholder
// In a real implementation, you would manage actual memory banks
void* guikit_autostart_alloc(GUIKitAutostartConfig *config, uint32_t size, const char *name) {
    (void)config; (void)size; (void)name;
    
    // This is a placeholder - implement with your memory management
    // For now, just return NULL
    return NULL;
}

void guikit_autostart_free(GUIKitAutostartConfig *config, void *ptr, const char *name) {
    (void)config; (void)ptr; (void)name;
    // Placeholder - implement with your memory management
}

// ============================================================================
// GUI LOADING
// ============================================================================

bool guikit_autostart_load_gui(const GUIKitAutostartConfig *config) {
    if (!config || !config->gui.auto_start) return false;
    if (config->gui.gui_path[0] == '\0') return false;
    
    // Check if GUI exists
    if (!sd_freeze_file_exists(config->gui.gui_path)) {
        // Try to find GUI in common locations
        const char *search_paths[] = {
            config->gui.gui_path,
            "/gui/chooser.GUIKIT",
            "/gui/webdav.GUIKIT",
            "/gui/users.GUIKIT",
            "/gui/editor.GUIKIT",
            NULL
        };
        
        for (int i = 0; search_paths[i]; i++) {
            if (sd_freeze_file_exists(search_paths[i])) {
                // In a real implementation, you would load the GUI here
                return true;
            }
        }
        return false;
    }
    
    // In a real implementation, you would load the GUI here
    return true;
}

uint8_t guikit_autostart_list_guis(char *path, uint32_t path_size) {
    if (!path || path_size == 0) return 0;
    
    // This is a placeholder - implement with your SD library
    // For now, return a dummy list
    const char *dummy_list = "/gui/chooser.GUIKIT,/gui/webdav.GUIKIT,/gui/users.GUIKIT,/gui/editor.GUIKIT";
    strncpy(path, dummy_list, path_size - 1);
    path[path_size - 1] = '\0';
    
    // Count commas + 1
    uint8_t count = 1;
    for (uint32_t i = 0; i < strlen(path); i++) {
        if (path[i] == ',') count++;
    }
    return count;
}

// ============================================================================
// BOOT PROCESS
// ============================================================================

/// @brief Boot steps
typedef enum {
    BOOT_STEP_LOAD_CONFIG,
    BOOT_STEP_DETECT_BANKS,
    BOOT_STEP_LOAD_KERNEL,
    BOOT_STEP_INIT_MEMORY,
    BOOT_STEP_LOAD_GUI,
    BOOT_STEP_START_GUI,
    BOOT_STEP_COUNT
} BootStep;

bool guikit_autostart_boot(GUIKitAutostartConfig *config) {
    if (!config) return false;
    
    // Step 1: Load configuration
    if (!guikit_autostart_parse_ini(config, config->config_file)) {
        // Use defaults
        guikit_autostart_init_default(config);
    }
    
    guikit_autostart_display_progress(config, BOOT_STEP_LOAD_CONFIG, BOOT_STEP_COUNT,
                                       "Loading configuration");
    
    // Step 2: Detect available memory banks
    guikit_autostart_detect_banks(config);
    guikit_autostart_display_progress(config, BOOT_STEP_DETECT_BANKS, BOOT_STEP_COUNT,
                                       "Detecting memory banks");
    
    // Step 3: Load kernel
    uint8_t *kernel_buffer = NULL;
    // In a real implementation, you would allocate and load the kernel
    // For now, we'll just check if it exists
    char kernel_path[64];
    if (!guikit_autostart_find_kernel(config, kernel_path, sizeof(kernel_path))) {
        return false;
    }
    guikit_autostart_display_progress(config, BOOT_STEP_LOAD_KERNEL, BOOT_STEP_COUNT,
                                       "Loading kernel");
    
    // Step 4: Initialize memory
    guikit_autostart_display_progress(config, BOOT_STEP_INIT_MEMORY, BOOT_STEP_COUNT,
                                       "Initializing memory");
    
    // Step 5: Load GUI
    if (!guikit_autostart_load_gui(config)) {
        // GUI not found, but kernel is loaded - this might be OK
    }
    guikit_autostart_display_progress(config, BOOT_STEP_LOAD_GUI, BOOT_STEP_COUNT,
                                       "Loading GUI");
    
    // Step 6: Start GUI
    guikit_autostart_display_progress(config, BOOT_STEP_START_GUI, BOOT_STEP_COUNT,
                                       "Starting GUI");
    
    return true;
}

void guikit_autostart_display_progress(GUIKitAutostartConfig *config,
                                       uint8_t step, uint8_t total_steps,
                                       const char *message) {
    if (!config) return;
    
    if (config->serial_boot_messages) {
        printf("[AUTOSTART] Step %d/%d: %s\n", step + 1, total_steps, message);
    }
    
    // TFT display would go here
    if (config->tft_boot_messages) {
        // tft_display_progress(step, total_steps, message);
    }
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

const char* guikit_autostart_bank_name(MemBankType bank) {
    switch (bank) {
        case MEM_BANK_INTERNAL: return "Internal RAM";
        case MEM_BANK_EXTERNAL: return "External RAM";
        case MEM_BANK_SD_SWAP: return "SD Swap";
        default: return "Unknown";
    }
}

const char* guikit_autostart_strategy_name(MemStrategyType strategy) {
    switch (strategy) {
        case MEM_STRATEGY_AUTO: return "Auto";
        case MEM_STRATEGY_EXTERNAL_FIRST: return "External First";
        case MEM_STRATEGY_SD_SWAP_FIRST: return "SD Swap First";
        case MEM_STRATEGY_INTERNAL_ONLY: return "Internal Only";
        case MEM_STRATEGY_CUSTOM: return "Custom";
        default: return "Unknown";
    }
}

void guikit_autostart_print_config(const GUIKitAutostartConfig *config) {
    if (!config) return;
    
    printf("\n=== GUIKit Autostart Configuration ===\n");
    
    printf("[Kernel]\n");
    printf("  Path: %s\n", config->kernel.path);
    printf("  Compress: %s\n", config->kernel.compress ? "Yes" : "No");
    printf("  Verify: %s\n", config->kernel.verify ? "Yes" : "No");
    
    printf("\n[Memory]\n");
    printf("  Strategy: %s\n", guikit_autostart_strategy_name(config->strategy));
    printf("  Stop at first success: %s\n", config->strategy_stop_at_first_success ? "Yes" : "No");
    
    for (uint8_t i = 0; i < MEM_BANK_COUNT; i++) {
        printf("  Bank %d (%s): %s, %s\n",
               i, guikit_autostart_bank_name(config->banks[i].type),
               config->banks[i].available ? "Available" : "Not available",
               config->banks[i].enabled ? "Enabled" : "Disabled");
    }
    
    printf("\n[GUI]\n");
    printf("  Path: %s\n", config->gui.gui_path);
    printf("  Auto start: %s\n", config->gui.auto_start ? "Yes" : "No");
    printf("  Theme: %s\n", config->gui.theme);
    
    printf("\n[Flags]\n");
    printf("  Debug: %s\n", config->debug ? "Yes" : "No");
    printf("  TFT messages: %s\n", config->tft_boot_messages ? "Yes" : "No");
    printf("  Serial messages: %s\n", config->serial_boot_messages ? "Yes" : "No");
    
    printf("\n[Allocations]\n");
    for (uint8_t i = 0; i < config->allocation_count; i++) {
        MemAllocationRule *rule = &config->allocations[i];
        printf("  %s: %s -> %s, %lu bytes, replace=%s\n",
               rule->name ? rule->name : "(unnamed)",
               guikit_autostart_bank_name(rule->bank),
               guikit_autostart_bank_name(rule->fallback),
               rule->size,
               rule->replace ? "Yes" : "No");
    }
    
    printf("\n==========================================\n");
}
