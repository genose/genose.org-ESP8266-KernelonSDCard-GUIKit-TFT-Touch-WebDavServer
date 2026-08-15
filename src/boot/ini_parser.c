/**
 * @file ini_parser.c
 * @brief Simple INI file parser implementation for embedded systems
 * 
 * Lightweight INI parser with minimal memory usage
 * No dynamic allocation, works with static buffers
 * 
 * Supports:
 * - Sections: [section]
 * - Key-value pairs: key=value
 * - Comments: ; or #
 * - Whitespace trimming
 * - String and numeric values
 */

#include "ini_parser.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

// ============================================================================
// STATIC HELPER FUNCTIONS
// ============================================================================

/// @brief Skip whitespace characters
/// @param p Pointer to string
/// @return Pointer to first non-whitespace character
static const char* skip_whitespace(const char *p) {
    while (p && *p && isspace((unsigned char)*p)) {
        p++;
    }
    return p;
}

/// @brief Skip to end of line
/// @param p Pointer to string
/// @return Pointer to end of line (\n, \r\n, or \0)
static const char* skip_to_eol(const char *p) {
    while (p && *p && *p != '\n' && *p != '\r') {
        p++;
    }
    return p;
}

/// @brief Check if character is a section delimiter
/// @param c Character to check
/// @return true if it's a section start/end character
static bool is_section_delimiter(char c) {
    return (c == '[' || c == ']');
}

/// @brief Check if character starts a comment
/// @param c Character to check
/// @return true if it's a comment character
static bool is_comment_char(char c) {
    return (c == ';' || c == '#');
}

/// @brief Check if character is a key-value separator
/// @param c Character to check
/// @return true if it's a separator character
static bool is_separator(char c) {
    return (c == '=' || c == ':');
}

/// @brief Extract a token (key, value, or section name) from string
/// @param src Source string
/// @param dst Destination buffer
/// @param dst_size Destination buffer size
/// @return Pointer to end of token in source
static const char* extract_token(const char *src, char *dst, size_t dst_size) {
    if (!src || !dst || dst_size == 0) return src;
    
    size_t i = 0;
    while (src[i] && !isspace((unsigned char)src[i]) && 
           !is_separator(src[i]) && !is_section_delimiter(src[i]) &&
           !is_comment_char(src[i])) {
        if (i < dst_size - 1) {
            dst[i] = src[i];
        }
        i++;
    }
    dst[min(i, dst_size - 1)] = '\0';
    return src + i;
}

/// @brief Convert string to lowercase (in-place)
/// @param str String to convert
static void str_to_lower(char *str) {
    if (str) {
        for (int i = 0; str[i]; i++) {
            str[i] = (char)tolower((unsigned char)str[i]);
        }
    }
}

/// @brief Compare strings case-insensitively
/// @param s1 First string
/// @param s2 Second string
/// @return 0 if equal, non-zero otherwise
static int str_casecmp(const char *s1, const char *s2) {
    if (!s1 || !s2) return s1 == s2 ? 0 : 1;
    
    while (*s1 && *s2) {
        char c1 = (char)tolower((unsigned char)*s1);
        char c2 = (char)tolower((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

/// @brief Convert string to integer
/// @param str String to convert
/// @param default_value Default value on error
/// @return Integer value
static int32_t str_to_int(const char *str, int32_t default_value) {
    if (!str || !*str) return default_value;
    
    char *endptr;
    long val = strtol(str, &endptr, 0);
    
    if (endptr == str) return default_value; // No conversion
    
    if (val < INT32_MIN) return INT32_MIN;
    if (val > INT32_MAX) return INT32_MAX;
    
    return (int32_t)val;
}

/// @brief Convert string to unsigned integer
/// @param str String to convert
/// @param default_value Default value on error
/// @return Unsigned integer value
static uint32_t str_to_uint(const char *str, uint32_t default_value) {
    if (!str || !*str) return default_value;
    
    char *endptr;
    unsigned long val = strtoul(str, &endptr, 0);
    
    if (endptr == str) return default_value; // No conversion
    
    return (uint32_t)val;
}

/// @brief Convert string to float
/// @param str String to convert
/// @param default_value Default value on error
/// @return Float value
static float str_to_float(const char *str, float default_value) {
    if (!str || !*str) return default_value;
    
    char *endptr;
    float val = strtof(str, &endptr);
    
    if (endptr == str) return default_value; // No conversion
    
    return val;
}

/// @brief Convert string to boolean
/// @param str String to convert
/// @param default_value Default value on error
/// @return Boolean value
static bool str_to_bool(const char *str, bool default_value) {
    if (!str || !*str) return default_value;
    
    char lower[16];
    size_t len = strlen(str);
    if (len >= sizeof(lower)) return default_value;
    
    memcpy(lower, str, len + 1);
    str_to_lower(lower);
    
    if (strcmp(lower, "true") == 0 || strcmp(lower, "yes") == 0 || 
        strcmp(lower, "1") == 0 || strcmp(lower, "on") == 0) {
        return true;
    }
    
    if (strcmp(lower, "false") == 0 || strcmp(lower, "no") == 0 || 
        strcmp(lower, "0") == 0 || strcmp(lower, "off") == 0) {
        return false;
    }
    
    return default_value;
}

// ============================================================================
// PARSER IMPLEMENTATION
// ============================================================================

void ini_parser_init(IniParser *parser, const char *data, size_t length,
                     void *user_data, ini_section_callback section_cb,
                     ini_key_callback key_cb) {
    if (!parser) return;
    
    parser->data = data;
    parser->position = 0;
    parser->length = data ? length : 0;
    parser->line_pos = 0;
    parser->current_section = NULL;
    parser->user_data = user_data;
    parser->section_cb = section_cb;
    parser->key_cb = key_cb;
    parser->error = false;
    parser->error_message = NULL;
    
    if (parser->line_pos < sizeof(parser->line_buffer)) {
        parser->line_buffer[0] = '\0';
    }
}

bool ini_parser_parse(IniParser *parser) {
    if (!parser || !parser->data) {
        parser->error = true;
        parser->error_message = "No data";
        return false;
    }
    
    size_t pos = parser->position;
    const char *p = parser->data + pos;
    
    while (pos < parser->length && !parser->error) {
        // Skip leading whitespace
        p = skip_whitespace(parser->data + pos);
        pos = p - parser->data;
        
        if (pos >= parser->length) break;
        
        // Check for section
        if (*p == '[') {
            // Find closing bracket
            const char *end_bracket = strchr(p, ']');
            if (!end_bracket) {
                parser->error = true;
                parser->error_message = "Unclosed section";
                return false;
            }
            
            // Extract section name
            size_t section_len = end_bracket - p - 1;
            if (section_len >= sizeof(parser->line_buffer)) {
                parser->error = true;
                parser->error_message = "Section name too long";
                return false;
            }
            
            memcpy(parser->line_buffer, p + 1, section_len);
            parser->line_buffer[section_len] = '\0';
            
            // Set current section
            parser->current_section = parser->line_buffer;
            
            // Move past closing bracket
            pos = end_bracket - parser->data + 1;
            p = parser->data + pos;
            
            // Call section callback if provided
            if (parser->section_cb) {
                parser->section_cb(parser->user_data, parser->current_section);
            }
            
            // Skip to end of line
            p = skip_to_eol(p);
            pos = p - parser->data;
            continue;
        }
        
        // Check for comment
        if (is_comment_char(*p)) {
            p = skip_to_eol(p);
            pos = p - parser->data;
            continue;
        }
        
        // Check for key-value pair
        if (*p && !is_section_delimiter(*p)) {
            // Extract key
            char key[64];
            p = extract_token(p, key, sizeof(key));
            p = skip_whitespace(p);
            
            // Check for separator
            if (*p && is_separator(*p)) {
                p++; // Skip separator
                p = skip_whitespace(p);
                
                // Extract value
                char value[128];
                p = extract_token(p, value, sizeof(value));
                
                // Remove trailing comment if any
                char *comment = strchr(value, ';');
                if (comment) *comment = '\0';
                comment = strchr(value, '#');
                if (comment) *comment = '\0';
                
                // Trim trailing whitespace from value
                size_t val_len = strlen(value);
                while (val_len > 0 && isspace((unsigned char)value[val_len - 1])) {
                    value[--val_len] = '\0';
                }
                
                // Call key callback if provided
                if (parser->key_cb) {
                    parser->key_cb(parser->user_data, 
                                  parser->current_section, 
                                  key, 
                                  value);
                }
                
                // Skip to end of line
                p = skip_to_eol(p);
                pos = p - parser->data;
                continue;
            }
        }
        
        // Skip to next line
        p = skip_to_eol(p);
        pos = p - parser->data;
    }
    
    parser->position = pos;
    return !parser->error;
}

// ============================================================================
// VALUE GETTERS
// ============================================================================

// Simple callback to store key-value pairs
typedef struct {
    const char *section;
    const char *key;
    char value[128];
    bool found;
} KeyValueFinder;

static void kv_find_cb(void *user_data, const char *section, const char *key, const char *value) {
    KeyValueFinder *finder = (KeyValueFinder*)user_data;
    
    if (finder->found) return;
    
    // Check section
    if (finder->section) {
        if (!section || str_casecmp(section, finder->section) != 0) {
            return;
        }
    }
    
    // Check key
    if (str_casecmp(key, finder->key) == 0) {
        strncpy(finder->value, value, sizeof(finder->value) - 1);
        finder->value[sizeof(finder->value) - 1] = '\0';
        finder->found = true;
    }
}

const char* ini_get_string(IniParser *parser, const char *section, const char *key, const char *default_value) {
    if (!parser || !parser->data) return default_value;
    
    KeyValueFinder finder = { section, key, {0}, false };
    
    IniParser temp_parser;
    ini_parser_init(&temp_parser, parser->data, parser->length,
                    &finder, NULL, kv_find_cb);
    ini_parser_parse(&temp_parser);
    
    return finder.found ? finder.value : default_value;
}

int32_t ini_get_int(IniParser *parser, const char *section, const char *key, int32_t default_value) {
    const char *str = ini_get_string(parser, section, key, NULL);
    if (!str) return default_value;
    return str_to_int(str, default_value);
}

uint32_t ini_get_uint(IniParser *parser, const char *section, const char *key, uint32_t default_value) {
    const char *str = ini_get_string(parser, section, key, NULL);
    if (!str) return default_value;
    return str_to_uint(str, default_value);
}

bool ini_get_bool(IniParser *parser, const char *section, const char *key, bool default_value) {
    const char *str = ini_get_string(parser, section, key, NULL);
    if (!str) return default_value;
    return str_to_bool(str, default_value);
}

float ini_get_float(IniParser *parser, const char *section, const char *key, float default_value) {
    const char *str = ini_get_string(parser, section, key, NULL);
    if (!str) return default_value;
    return str_to_float(str, default_value);
}

// ============================================================================
// FILE PARSING
// ============================================================================

// For file parsing, we need SD card access
// This is a placeholder - implement with your SD library

#include "sd_freeze_wrapper.h"

bool ini_parse_file(const char *filepath, void *user_data,
                    ini_section_callback section_cb, ini_key_callback key_cb) {
    // Read file from SD card
    sd_file_t fp = sd_freeze_fopen(filepath, "r");
    if (!fp) return false;
    
    // Get file size
    uint32_t size = sd_freeze_file_size(filepath);
    if (size == 0) {
        sd_freeze_fclose(fp);
        return false;
    }
    
    // Allocate buffer for file content
    // For embedded systems, you might want to use a static buffer
    char *buffer = (char*)malloc(size + 1);
    if (!buffer) {
        sd_freeze_fclose(fp);
        return false;
    }
    
    // Read file
    size_t read = sd_freeze_fread(buffer, 1, size, fp);
    sd_freeze_fclose(fp);
    
    if (read != size) {
        free(buffer);
        return false;
    }
    
    buffer[size] = '\0';
    
    // Parse
    IniParser parser;
    ini_parser_init(&parser, buffer, size, user_data, section_cb, key_cb);
    bool result = ini_parser_parse(&parser);
    
    free(buffer);
    return result;
}

// ============================================================================
// ERROR HANDLING
// ============================================================================

const char* ini_get_error(IniParser *parser) {
    if (!parser) return "No parser";
    return parser->error_message ? parser->error_message : "No error";
}

bool ini_has_error(IniParser *parser) {
    return parser && parser->error;
}

// ============================================================================
// SIMPLE KEY-VALUE PARSING
// ============================================================================

uint8_t ini_parse_simple(const char *data, const char **keys, const char **values, uint8_t max_pairs) {
    if (!data || !keys || !values) return 0;
    
    uint8_t count = 0;
    const char *p = data;
    
    while (*p && count < max_pairs) {
        p = skip_whitespace(p);
        if (!*p) break;
        
        // Check for comment
        if (is_comment_char(*p)) {
            p = skip_to_eol(p);
            continue;
        }
        
        // Extract key
        char key_buf[64];
        p = extract_token(p, key_buf, sizeof(key_buf));
        p = skip_whitespace(p);
        
        // Check for separator
        if (*p && is_separator(*p)) {
            p++;
            p = skip_whitespace(p);
            
            // Extract value
            char value_buf[128];
            p = extract_token(p, value_buf, sizeof(value_buf));
            
            // Store in output arrays
            keys[count] = strdup(key_buf);
            values[count] = strdup(value_buf);
            count++;
            
            // Skip to end of line
            p = skip_to_eol(p);
        } else {
            p = skip_to_eol(p);
        }
    }
    
    return count;
}

const char* ini_get_simple_value(const char *data, const char *key, const char *default_value) {
    if (!data || !key) return default_value;
    
    const char *p = data;
    const char *key_start;
    
    while ((p = strstr(p, key)) != NULL) {
        // Check if this is a key (not part of another word)
        key_start = p;
        p--;
        
        // Check if preceded by whitespace or separator
        bool is_key = false;
        if (p < data || isspace((unsigned char)*p) || is_separator(*p) || *p == '[') {
            is_key = true;
        }
        
        if (!is_key) {
            p = key_start + 1;
            continue;
        }
        
        // Move to end of key
        p = key_start + strlen(key);
        
        // Check for separator
        p = skip_whitespace(p);
        if (*p && is_separator(*p)) {
            p++;
            p = skip_whitespace(p);
            
            // Extract value
            static char value_buf[128];
            p = extract_token(p, value_buf, sizeof(value_buf));
            
            // Remove trailing comment
            char *comment = strchr(value_buf, ';');
            if (comment) *comment = '\0';
            comment = strchr(value_buf, '#');
            if (comment) *comment = '\0';
            
            return value_buf;
        }
        
        p = key_start + 1;
    }
    
    return default_value;
}
