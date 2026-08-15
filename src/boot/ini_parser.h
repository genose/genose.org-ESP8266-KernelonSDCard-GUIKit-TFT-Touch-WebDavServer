#ifndef INI_PARSER_H
#define INI_PARSER_H

/**
 * @brief Simple INI file parser for embedded systems
 * 
 * Lightweight INI parser with minimal memory usage
 * No dynamic allocation, works with static buffers
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// CALLBACK TYPEDEFS
// ============================================================================

/// @brief Callback for INI section start
/// @param user_data User data pointer
/// @param section Section name (NULL for default section)
typedef void (*ini_section_callback)(void *user_data, const char *section);

/// @brief Callback for INI key-value pair
/// @param user_data User data pointer
/// @param section Section name (NULL for default section)
/// @param key Key name
/// @param value Value string
typedef void (*ini_key_callback)(void *user_data, const char *section, const char *key, const char *value);

// ============================================================================
// PARSER STRUCTURE
// ============================================================================

/// @brief INI parser state
typedef struct {
    const char *data;            /// Pointer to INI data
    size_t position;            /// Current position in data
    size_t length;              /// Total data length
    char line_buffer[128];      /// Current line buffer
    uint8_t line_pos;            /// Position in line buffer
    const char *current_section;/// Current section name
    
    // Callbacks
    void *user_data;            /// User data for callbacks
    ini_section_callback section_cb;  /// Section callback
    ini_key_callback key_cb;           /// Key-value callback
    
    // Error tracking
    bool error;                 /// Parse error occurred
    const char *error_message;  /// Error message
} IniParser;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

/// @brief Initialize INI parser
/// @param parser Parser instance
/// @param data INI data (NULL to start fresh)
/// @param length Data length
/// @param user_data User data for callbacks
/// @param section_cb Section callback (can be NULL)
/// @param key_cb Key-value callback (can be NULL)
void ini_parser_init(IniParser *parser, const char *data, size_t length,
                     void *user_data, ini_section_callback section_cb, 
                     ini_key_callback key_cb);

/// @brief Parse INI data
/// @param parser Parser instance
/// @return true on success, false on error
bool ini_parser_parse(IniParser *parser);

/// @brief Parse INI from file
/// @param filepath File path
/// @param user_data User data for callbacks
/// @param section_cb Section callback (can be NULL)
/// @param key_cb Key-value callback (can be NULL)
/// @return true on success, false on error
bool ini_parse_file(const char *filepath, void *user_data,
                    ini_section_callback section_cb, ini_key_callback key_cb);

/// @brief Get string value from INI
/// @param parser Parser instance
/// @param section Section name (NULL for default)
/// @param key Key name
/// @param default_value Default value if not found
/// @return Pointer to value string (internal buffer, not persistent)
const char* ini_get_string(IniParser *parser, const char *section, const char *key, const char *default_value);

/// @brief Get integer value from INI
/// @param parser Parser instance
/// @param section Section name (NULL for default)
/// @param key Key name
/// @param default_value Default value if not found
/// @return Integer value
int32_t ini_get_int(IniParser *parser, const char *section, const char *key, int32_t default_value);

/// @brief Get unsigned integer value from INI
/// @param parser Parser instance
/// @param section Section name (NULL for default)
/// @param key Key name
/// @param default_value Default value if not found
/// @return Unsigned integer value
uint32_t ini_get_uint(IniParser *parser, const char *section, const char *key, uint32_t default_value);

/// @brief Get boolean value from INI
/// @param parser Parser instance
/// @param section Section name (NULL for default)
/// @param key Key name
/// @param default_value Default value if not found
/// @return Boolean value
bool ini_get_bool(IniParser *parser, const char *section, const char *key, bool default_value);

/// @brief Get float value from INI
/// @param parser Parser instance
/// @param section Section name (NULL for default)
/// @param key Key name
/// @param default_value Default value if not found
/// @return Float value
float ini_get_float(IniParser *parser, const char *section, const char *key, float default_value);

// ============================================================================
// INI SECTION ITERATION
// ============================================================================

/// @brief Get list of section names
/// @param data INI data
/// @param sections Array to store section names
/// @param max_sections Maximum number of sections
/// @return Number of sections found
uint8_t ini_get_sections(const char *data, const char **sections, uint8_t max_sections);

/// @brief Get list of keys in a section
/// @param data INI data
/// @param section Section name
/// @param keys Array to store key names
/// @param max_keys Maximum number of keys
/// @return Number of keys found
uint8_t ini_get_keys(const char *data, const char *section, const char **keys, uint8_t max_keys);

// ============================================================================
// INI WRITING
// ============================================================================

/// @brief Write INI to buffer
/// @param buffer Output buffer
/// @param buffer_size Buffer size
/// @param sections Array of section names
/// @param keys Array of arrays of key names
/// @param values Array of arrays of value strings
/// @param section_count Number of sections
/// @param max_keys_per_section Maximum keys per section
/// @return Number of bytes written
size_t ini_write_buffer(char *buffer, size_t buffer_size,
                        const char **sections, const char ***keys, const char ***values,
                        uint8_t section_count, uint8_t max_keys_per_section);

/// @brief Write INI to file
/// @param filepath File path
/// @param sections Array of section names
/// @param keys Array of arrays of key names
/// @param values Array of arrays of value strings
/// @param section_count Number of sections
/// @param max_keys_per_section Maximum keys per section
/// @return true on success
bool ini_write_file(const char *filepath, const char **sections, const char ***keys, const char ***values,
                    uint8_t section_count, uint8_t max_keys_per_section);

// ============================================================================
// SIMPLE KEY-VALUE PARSING
// ============================================================================

/// @brief Parse simple key=value format (no sections)
/// @param data Input string
/// @param keys Array to store key names
/// @param values Array to store value strings
/// @param max_pairs Maximum number of pairs
/// @return Number of pairs parsed
uint8_t ini_parse_simple(const char *data, const char **keys, const char **values, uint8_t max_pairs);

/// @brief Get value from simple key=value data
/// @param data Input string
/// @param key Key to find
/// @param default_value Default value if not found
/// @return Pointer to value (internal buffer)
const char* ini_get_simple_value(const char *data, const char *key, const char *default_value);

// ============================================================================
// ERROR HANDLING
// ============================================================================

/// @brief Get error message from parser
/// @param parser Parser instance
/// @return Error message string
const char* ini_get_error(IniParser *parser);

/// @brief Check if parser has error
/// @param parser Parser instance
/// @return true if error
bool ini_has_error(IniParser *parser);

#endif // INI_PARSER_H
