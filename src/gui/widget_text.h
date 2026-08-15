/**
 * widget_text.h - Static Text Buffer Management
 * 
 * Objective-C-style text management for ESP8266 GUIKit
 * Uses fixed-size buffers to avoid malloc/free overhead
 * 
 * Generated for ESP8266 by Mistral Vibe
 */

#ifndef WIDGET_TEXT_H
#define WIDGET_TEXT_H

#include <stdint.h>
#include <stdbool.h>

// Maximum text length for all widgets (512 chars + null terminator)
#define MAX_TEXT_LENGTH 512

// Text structure with static buffer
// No malloc needed - text is stored directly in the struct
 typedef struct {
    char text[MAX_TEXT_LENGTH];  // Fixed buffer for text
    uint8_t font_size;           // Font size (in pixels or points)
    uint16_t font_color;         // Text color (RGBA565 format)
} WidgetText;


// ============================================================================
// SAFE TEXT ASSIGNMENT
// ============================================================================

/**
 * @brief Safely copy text to a WidgetText buffer
 * 
 * Automatically:
 * - Handles NULL input (clears text)
 * - Truncates to MAX_TEXT_LENGTH-1 characters
 * - Always null-terminates
 * - No malloc/free overhead
 * 
 * @param wt Pointer to WidgetText struct
 * @param str String to copy (can be NULL)
 */
#define WidgetSetText(wt, str) do { \
    if ((str)) { \
        strncpy((wt)->text, (str), MAX_TEXT_LENGTH - 1); \
        (wt)->text[MAX_TEXT_LENGTH - 1] = '\0'; \
    } else { \
        (wt)->text[0] = '\0'; \
    } \
} while(0)


// ============================================================================
// TEXT ACCESSORS
// ============================================================================

/**
 * @brief Get text from WidgetText (safe, never returns NULL)
 * 
 * @param wt Pointer to WidgetText struct
 * @return Pointer to text string (empty string if wt is NULL)
 */
static inline const char* WidgetGetText(const WidgetText* wt) {
    return (wt && wt->text) ? wt->text : "";
}

/**
 * @brief Check if WidgetText is empty
 * 
 * @param wt Pointer to WidgetText struct
 * @return true if text is empty or NULL
 */
static inline bool WidgetTextIsEmpty(const WidgetText* wt) {
    return !wt || !wt->text || wt->text[0] == '\0';
}

/**
 * @brief Get text length
 * 
 * @param wt Pointer to WidgetText struct
 * @return Length of text (0 if empty or NULL)
 */
static inline size_t WidgetTextLength(const WidgetText* wt) {
    return (wt && wt->text) ? strlen(wt->text) : 0;
}

/**
 * @brief Clear text (set to empty string)
 * 
 * @param wt Pointer to WidgetText struct
 */
static inline void WidgetClearText(WidgetText* wt) {
    if (wt) {
        wt->text[0] = '\0';
    }
}


// ============================================================================
// TEXT UTILITIES
// ============================================================================

/**
 * @brief Append text to WidgetText (with bounds checking)
 * 
 * @param wt Pointer to WidgetText struct
 * @param str String to append
 */
void WidgetAppendText(WidgetText* wt, const char* str);

/**
 * @brief Format text into WidgetText (like sprintf)
 * 
 * @param wt Pointer to WidgetText struct
 * @param format Format string
 * @param ... Arguments
 */
void WidgetFormatText(WidgetText* wt, const char* format, ...);

/**
 * @brief Compare WidgetText with string
 * 
 * @param wt Pointer to WidgetText struct
 * @param str String to compare with
 * @return 0 if equal, <0 or >0 otherwise
 */
int WidgetTextCompare(const WidgetText* wt, const char* str);


// ============================================================================
// FONT STYLE ACCESSORS
// ============================================================================

/**
 * @brief Set font size
 * 
 * @param wt Pointer to WidgetText struct
 * @param size Font size
 */
static inline void WidgetSetFontSize(WidgetText* wt, uint8_t size) {
    if (wt) wt->font_size = size;
}

/**
 * @brief Get font size
 * 
 * @param wt Pointer to WidgetText struct
 * @return Font size
 */
static inline uint8_t WidgetGetFontSize(const WidgetText* wt) {
    return wt ? wt->font_size : 0;
}

/**
 * @brief Set font color
 * 
 * @param wt Pointer to WidgetText struct
 * @param color Color in RGBA565 format
 */
static inline void WidgetSetFontColor(WidgetText* wt, uint16_t color) {
    if (wt) wt->font_color = color;
}

/**
 * @brief Get font color
 * 
 * @param wt Pointer to WidgetText struct
 * @return Font color in RGBA565 format
 */
static inline uint16_t WidgetGetFontColor(const WidgetText* wt) {
    return wt ? wt->font_color : 0;
}


// ============================================================================
// PREDEFINED COLORS (RGBA565)
// ============================================================================

#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_GRAY    0x7BEF


#endif // WIDGET_TEXT_H
