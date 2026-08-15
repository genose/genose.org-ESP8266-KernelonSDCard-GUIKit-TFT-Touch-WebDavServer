#ifndef GUIKIT_AUTOSTART_CONFIG_H
#define GUIKIT_AUTOSTART_CONFIG_H

/**
 * @brief GUIKit Autostart Configuration System
 * 
 * Reads /etc/GUIKIT_autostart.ini to determine:
 * - Which kernel.bin to load
 * - Memory strategy configuration
 * - Which GUI to start automatically
 * - Memory bank allocation
 * 
 * Feel like a little OS boot configuration
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// AUTOSTART CONFIGURATION STRUCTURE
// ============================================================================

/// @brief Memory bank types
typedef enum {
    MEM_BANK_INTERNAL = 0,    /// Internal MCU RAM
    MEM_BANK_EXTERNAL,        /// External SRAM/PSRAM
    MEM_BANK_SD_SWAP,        /// SD card swap space
    MEM_BANK_COUNT
} MemBankType;

/// @brief Memory bank configuration
typedef struct {
    MemBankType type;           /// Bank type
    uint32_t start;              /// Start address (0 if not applicable)
    uint32_t size;               /// Size in bytes
    bool available;              /// Is this bank available
    bool enabled;                /// Is this bank enabled in config
    const char* name;            /// Bank name (for display)
} MemBankConfig;

/// @brief Memory strategy for autostart
typedef enum {
    MEM_STRATEGY_AUTO = 0,       /// Auto-select based on available banks
    MEM_STRATEGY_EXTERNAL_FIRST, /// Try external RAM first
    MEM_STRATEGY_SD_SWAP_FIRST,   /// Try SD swap first
    MEM_STRATEGY_INTERNAL_ONLY,   /// Only use internal RAM
    MEM_STRATEGY_CUSTOM           /// Custom strategy from config
} MemStrategyType;

/// @brief Kernel load configuration
typedef struct {
    char path[64];              /// Path to kernel.bin
    bool compress;              /// Is kernel compressed (gzip)
    uint32_t expected_size;      /// Expected size (0 = any)
    uint32_t max_size;           /// Maximum allowed size (0 = any)
    bool verify;                /// Verify checksum after load
} KernelLoadConfig;

/// @brief GUI start configuration
typedef struct {
    char gui_path[64];          /// Path to GUI directory or JSON file
    bool auto_start;            /// Auto-start this GUI on boot
    char theme[32];             /// Theme to use
    uint16_t width;             /// GUI width (0 = auto)
    uint16_t height;            /// GUI height (0 = auto)
} GUISettings;

/// @brief Memory allocation rule
typedef struct {
    const char* name;            /// Name of the resource
    MemBankType bank;           /// Preferred bank
    MemBankType fallback;       /// Fallback bank if preferred not available
    uint32_t size;               /// Size required
    bool replace;               /// Replace existing content if bank full
} MemAllocationRule;

/// @brief Main autostart configuration
typedef struct {
    // Kernel configuration
    KernelLoadConfig kernel;
    
    // Memory bank configuration
    MemBankConfig banks[MEM_BANK_COUNT];
    MemStrategyType strategy;
    bool strategy_stop_at_first_success;
    
    // GUI configuration
    GUISettings gui;
    
    // Memory allocation rules
    MemAllocationRule allocations[16];
    uint8_t allocation_count;
    
    // Flags
    bool debug;                   /// Enable debug output
    bool tft_boot_messages;      /// Show boot messages on TFT
    bool serial_boot_messages;   /// Show boot messages on serial
    
    // Version
    uint16_t config_version;      /// Configuration file version
    char config_file[64];        /// Path to config file
} GUIKitAutostartConfig;

// ============================================================================
// DEFAULT VALUES
// ============================================================================

/// @brief Default configuration
#define GUIKIT_AUTOSTART_DEFAULT_CONFIG \
    { \
        .kernel = { \
            .path = "/kernel.bin", \
            .compress = false, \
            .expected_size = 0, \
            .max_size = 0, \
            .verify = true \
        }, \
        .banks = { \
            [MEM_BANK_INTERNAL] = { MEM_BANK_INTERNAL, 0, 0, false, true, "internal" },\n            [MEM_BANK_EXTERNAL] = { MEM_BANK_EXTERNAL, 0, 0, false, true, "external" },\n            [MEM_BANK_SD_SWAP] = { MEM_BANK_SD_SWAP, 0, 0, false, true, "sd_swap" } \
        },\n        .strategy = MEM_STRATEGY_AUTO,\n        .strategy_stop_at_first_success = true,\n        .gui = { \
            .gui_path = "/gui/chooser.GUIKIT", \
            .auto_start = true,\n            .theme = "default", \
            .width = 0,\n            .height = 0 \
        },\n        .allocation_count = 0,\n        .debug = false,\n        .tft_boot_messages = true,\n        .serial_boot_messages = true,\n        .config_version = 1,\n        .config_file = "/etc/GUIKIT_autostart.ini" \
    }

// ============================================================================
// CONFIG FILE PARSER
// ============================================================================

/// @brief Parse autostart config from INI file
/// @param config Pointer to config struct to fill
/// @param filepath Path to INI file
/// @return true on success
bool guikit_autostart_parse_ini(GUIKitAutostartConfig *config, const char *filepath);

/// @brief Save autostart config to INI file
/// @param config Pointer to config struct
/// @param filepath Path to INI file
/// @return true on success
bool guikit_autostart_save_ini(const GUIKitAutostartConfig *config, const char *filepath);

/// @brief Initialize with default config
/// @param config Pointer to config struct
void guikit_autostart_init_default(GUIKitAutostartConfig *config);

/// @brief Validate config
/// @param config Pointer to config struct
/// @return true if valid
bool guikit_autostart_validate(const GUIKitAutostartConfig *config);

/// @brief Detect available memory banks
/// @param config Pointer to config struct
void guikit_autostart_detect_banks(GUIKitAutostartConfig *config);

// ============================================================================
// KERNEL LOAD FUNCTIONS
// ============================================================================

/// @brief Load kernel from configured path
/// @param config Pointer to config struct
/// @param buffer Buffer to load into (or NULL for direct execution)
/// @param buffer_size Buffer size
/// @return true on success
bool guikit_autostart_load_kernel(const GUIKitAutostartConfig *config, 
                                   uint8_t *buffer, uint32_t buffer_size);

/// @brief Find kernel file on SD card
/// @param config Pointer to config struct
/// @param found_path Buffer to store found path
/// @param found_path_size Size of buffer
/// @return true if found
bool guikit_autostart_find_kernel(const GUIKitAutostartConfig *config,
                                    char *found_path, uint32_t found_path_size);

/// @brief Verify kernel checksum
/// @param buffer Kernel buffer
/// @param size Kernel size
/// @param expected_crc Expected CRC32 (0 = skip verification)
/// @return true if valid
bool guikit_autostart_verify_kernel(const uint8_t *buffer, uint32_t size, uint32_t expected_crc);

// ============================================================================
// MEMORY MANAGEMENT
// ============================================================================

/// @brief Allocate memory using strategy from config
/// @param config Pointer to config struct
/// @param size Size to allocate
/// @param name Name of the allocation (for tracking)
/// @return Pointer to allocated memory, or NULL on failure
void* guikit_autostart_alloc(GUIKitAutostartConfig *config, uint32_t size, const char *name);

/// @brief Free memory
/// @param config Pointer to config struct
/// @param ptr Pointer to free
/// @param name Name of the allocation
void guikit_autostart_free(GUIKitAutostartConfig *config, void *ptr, const char *name);

/// @brief Check if memory can fit in a bank
/// @param bank Bank type
/// @param size Size required
/// @return true if fits
bool guikit_autostart_bank_can_fit(GUIKitAutostartConfig *config, MemBankType bank, uint32_t size);

/// @brief Apply memory strategy
/// @param config Pointer to config struct
/// @param size Size required
/// @return Selected bank type
MemBankType guikit_autostart_apply_strategy(GUIKitAutostartConfig *config, uint32_t size);

// ============================================================================
// GUI LOAD FUNCTIONS
// ============================================================================

/// @brief Load GUI from configured path
/// @param config Pointer to config struct
/// @return true on success
bool guikit_autostart_load_gui(const GUIKitAutostartConfig *config);

/// @brief List available GUIs on SD card
/// @param path Buffer to store list (comma-separated)
/// @param path_size Buffer size
/// @return Number of GUIs found
uint8_t guikit_autostart_list_guis(char *path, uint32_t path_size);

// ============================================================================
// BOOT PROCESS
// ============================================================================

/// @brief Run complete autostart boot process
/// @param config Pointer to config struct (loaded from file)
/// @return true on success
bool guikit_autostart_boot(GUIKitAutostartConfig *config);

/// @brief Display boot progress
/// @param config Pointer to config struct
/// @param step Current step
/// @param total_steps Total steps
/// @param message Message to display
void guikit_autostart_display_progress(GUIKitAutostartConfig *config, 
                                       uint8_t step, uint8_t total_steps,
                                       const char *message);

// ============================================================================
// INI FILE SECTION NAMES
// ============================================================================

#define INI_SECTION_KERNEL      "kernel"
#define INI_SECTION_MEMORY       "memory"
#define INI_SECTION_GUI          "gui"
#define INI_SECTION_ALLOCATIONS  "allocations"
#define INI_SECTION_FLAGS        "flags"

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/// @brief Get bank name string
/// @param bank Bank type
/// @return String name
const char* guikit_autostart_bank_name(MemBankType bank);

/// @brief Get strategy name string
/// @param strategy Strategy type
/// @return String name
const char* guikit_autostart_strategy_name(MemStrategyType strategy);

/// @brief Print config to serial
/// @param config Pointer to config struct
void guikit_autostart_print_config(const GUIKitAutostartConfig *config);

#endif // GUIKIT_AUTOSTART_CONFIG_H
