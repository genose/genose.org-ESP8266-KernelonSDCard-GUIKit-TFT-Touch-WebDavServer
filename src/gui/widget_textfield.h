/**
 * @file widget_textfield.h
 * @brief TextField widget for GUIKit
 * @author GUIKit for ESP8266
 * @date 2026
 * 
 * TextField is the base widget for text display and input.
 * It supports single or multi-line text (1 line to 4K characters).
 * 
 * TextEditor extends TextField to add editing capabilities.
 * 
 * Memory optimization: Uses circular buffers for lines, pooled memory for text
 * ESP8266-friendly: No dynamic allocation, fixed-size buffers
 * 
 * Features:
 * - Single or multi-line text
 * - Scrollable (inherits scrollable property from base view)
 * - Configurable text buffer size
 * - Text wrapping options
 * - Read-only or editable modes
 * 
 * Architecture:
 * TextField is the base for all text widgets.
 * TextEditor extends TextField with editing features.
 * This avoids code duplication between text display and text editing.
 */

#ifndef WIDGET_TEXTFIELD_H
#define WIDGET_TEXTFIELD_H

#include "widget_pool.h"
#include "widget_scrollable.h"
#include <stdint.h>
#include <stdbool.h>

// Alias for consistency with widget_scrollable.h
typedef Scrollable ScrollableProperty;
typedef WIDGET_SCROLLABLE_FLAGS ScrollableFlags;

// =============================================================================
// CONSTANTS
// =============================================================================

/** Maximum number of lines in textfield */
#define TEXTFIELD_MAX_LINES 256

/** Maximum line length (characters) */
#define TEXTFIELD_MAX_LINE_LENGTH 256

/** Maximum total text size (4K = 4096 characters) */
#define TEXTFIELD_MAX_TEXT_SIZE 4096

/** Default font width (pixels) */
#define TEXTFIELD_FONT_WIDTH 8

/** Default font height (pixels) */
#define TEXTFIELD_FONT_HEIGHT 16

// =============================================================================
// TEXT LINE STRUCTURE
// =============================================================================

/** A single line of text in the textfield */
typedef struct {
    char text[TEXTFIELD_MAX_LINE_LENGTH];  ///< Line content
    uint16_t length;                         ///< Actual text length
    uint16_t visual_length;                  ///< Visual length (accounting for tabs)
    bool dirty;                              ///< Needs re-rendering
} TextFieldLine;

// =============================================================================
// TEXTFIELD STRUCTURE
// =============================================================================

/** TextField widget - base for all text display/input widgets */
typedef struct {
    Widget base;                            ///< Base widget
    
    // Scrollable properties (union-based, from widget_scrollable.h)
    ScrollableProperty scrollable;         ///< Scrollable flags and coords
    
    // Text content
    TextFieldLine lines[TEXTFIELD_MAX_LINES];
    uint16_t line_count;                   ///< Number of lines in use
    char* text_buffer;                     ///< Flat text buffer (optional, for 4K mode)
    uint16_t text_buffer_size;            ///< Size of text buffer
    
    // Display settings
    struct {
        uint8_t font_width;                 ///< Character width in pixels
        uint8_t font_height;                ///< Character height in pixels
        uint8_t tab_size;                   ///< Tab size in spaces
        bool word_wrap;                     ///< Enable word wrapping
        bool multi_line;                    ///< Enable multi-line display
        bool read_only;                     ///< Read-only mode
    } settings;
    
    // Styling
    struct {
        Color bg_color;                     ///< Background color
        Color fg_color;                     ///< Foreground (text) color
        Color cursor_color;                 ///< Cursor color (if applicable)
    } colors;
    
    // State
    struct {
        uint16_t scroll_line;                ///< First visible line (for scrolling)
        uint16_t scroll_column;             ///< First visible column (for horizontal scroll)
    } scroll;
    
    // Callbacks
    void (*on_change)(struct TextField* tf);///< Text changed callback
    void (*on_scroll)(struct TextField* tf);///< Scroll callback
} TextField;

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// --- Constructors ---

/**
 * @brief Create a new TextField widget
 * 
 * @param x X position
 * @param y Y position
 * @param width Width in pixels
 * @param height Height in pixels
 * @return Pointer to new TextField, or NULL on failure
 */
TextField* textfield_create(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

/**
 * @brief Initialize a TextField from pool
 * 
 * @param tf Pointer to TextField to initialize
 * @param x X position
 * @param y Y position
 * @param width Width in pixels
 * @param height Height in pixels
 */
void textfield_init(TextField* tf, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

/**
 * @brief Destroy a TextField
 * 
 * @param tf Pointer to TextField to destroy
 */
void textfield_destroy(TextField* tf);

// --- Text Management ---

/**
 * @brief Set text content
 * 
 * @param tf Pointer to TextField
 * @param text Text to set (NULL to clear)
 */
void textfield_set_text(TextField* tf, const char* text);

/**
 * @brief Get text content
 * 
 * @param tf Pointer to TextField
 * @return Pointer to text (internal buffer, do not modify)
 */
const char* textfield_get_text(const TextField* tf);

/**
 * @brief Append text
 * 
 * @param tf Pointer to TextField
 * @param text Text to append
 */
void textfield_append_text(TextField* tf, const char* text);

/**
 * @brief Clear text
 * 
 * @param tf Pointer to TextField
 */
void textfield_clear(TextField* tf);

/**
 * @brief Set text from formatted string
 * 
 * @param tf Pointer to TextField
 * @param format Format string
 * @param ... Arguments
 */
void textfield_set_format(TextField* tf, const char* format, ...);

// --- Line Management ---

/**
 * @brief Get number of lines
 * 
 * @param tf Pointer to TextField
 * @return Number of lines
 */
uint16_t textfield_get_line_count(const TextField* tf);

/**
 * @brief Get line at index
 * 
 * @param tf Pointer to TextField
 * @param index Line index (0-based)
 * @return Pointer to line, or NULL if invalid
 */
TextFieldLine* textfield_get_line(TextField* tf, uint16_t index);

/**
 * @brief Set line text
 * 
 * @param tf Pointer to TextField
 * @param index Line index
 * @param text Text for the line
 */
void textfield_set_line(TextField* tf, uint16_t index, const char* text);

/**
 * @brief Insert a new line
 * 
 * @param tf Pointer to TextField
 * @param index Insert position (0 = at beginning)
 * @param text Text for new line
 */
void textfield_insert_line(TextField* tf, uint16_t index, const char* text);

/**
 * @brief Remove a line
 * 
 * @param tf Pointer to TextField
 * @param index Line index to remove
 */
void textfield_remove_line(TextField* tf, uint16_t index);

/**
 * @brief Split text into lines based on width
 * 
 * @param tf Pointer to TextField
 * @param max_width Maximum width in pixels
 */
void textfield_wrap_lines(TextField* tf, uint16_t max_width);

// --- Scrolling ---

/**
 * @brief Scroll to line
 * 
 * @param tf Pointer to TextField
 * @param line Line index to scroll to
 */
void textfield_scroll_to_line(TextField* tf, uint16_t line);

/**
 * @brief Scroll up by one line
 * 
 * @param tf Pointer to TextField
 */
void textfield_scroll_up(TextField* tf);

/**
 * @brief Scroll down by one line
 * 
 * @param tf Pointer to TextField
 */
void textfield_scroll_down(TextField* tf);

/**
 * @brief Get first visible line
 * 
 * @param tf Pointer to TextField
 * @return First visible line index
 */
uint16_t textfield_get_first_visible_line(const TextField* tf);

/**
 * @brief Get number of visible lines
 * 
 * @param tf Pointer to TextField
 * @return Number of visible lines
 */
uint16_t textfield_get_visible_line_count(const TextField* tf);

// --- Scrollable Property ---

/**
 * @brief Check if textfield is scrollable
 * 
 * @param tf Pointer to TextField
 * @return true if scrollable
 */
bool textfield_is_scrollable(const TextField* tf);

/**
 * @brief Set scrollable flags
 * 
 * @param tf Pointer to TextField
 * @param flags Bitmask of SCROLLABLE_X and SCROLLABLE_Y
 */
void textfield_set_scrollable(TextField* tf, ScrollableFlags flags);

/**
 * @brief Get scrollable flags
 * 
 * @param tf Pointer to TextField
 * @return Bitmask of scrollable flags
 */
ScrollableFlags textfield_get_scrollable(const TextField* tf);

// --- Settings ---

/**
 * @brief Set multi-line mode
 * 
 * @param tf Pointer to TextField
 * @param enabled true for multi-line, false for single-line
 */
void textfield_set_multiline(TextField* tf, bool enabled);

/**
 * @brief Set word wrap mode
 * 
 * @param tf Pointer to TextField
 * @param enabled true for word wrap
 */
void textfield_set_word_wrap(TextField* tf, bool enabled);

/**
 * @brief Set read-only mode
 * 
 * @param tf Pointer to TextField
 * @param read_only true for read-only
 */
void textfield_set_read_only(TextField* tf, bool read_only);

/**
 * @brief Set font dimensions
 * 
 * @param tf Pointer to TextField
 * @param width Character width in pixels
 * @param height Character height in pixels
 */
void textfield_set_font(TextField* tf, uint8_t width, uint8_t height);

/**
 * @brief Set tab size
 * 
 * @param tf Pointer to TextField
 * @param size Tab size in spaces
 */
void textfield_set_tab_size(TextField* tf, uint8_t size);

// --- Styling ---

/**
 * @brief Set background color
 * 
 * @param tf Pointer to TextField
 * @param color Background color (RGBA565)
 */
void textfield_set_bg_color(TextField* tf, Color color);

/**
 * @brief Set foreground (text) color
 * 
 * @param tf Pointer to TextField
 * @param color Foreground color (RGBA565)
 */
void textfield_set_fg_color(TextField* tf, Color color);

/**
 * @brief Set cursor color
 * 
 * @param tf Pointer to TextField
 * @param color Cursor color (RGBA565)
 */
void textfield_set_cursor_color(TextField* tf, Color color);

// --- Callbacks ---

/**
 * @brief Set text change callback
 * 
 * @param tf Pointer to TextField
 * @param callback Callback function
 */
void textfield_set_on_change(TextField* tf, void (*callback)(TextField*));

/**
 * @brief Set scroll callback
 * 
 * @param tf Pointer to TextField
 * @param callback Callback function
 */
void textfield_set_on_scroll(TextField* tf, void (*callback)(TextField*));

// --- Utility ---

/**
 * @brief Calculate text width in pixels
 * 
 * @param text Text string
 * @param font_width Character width in pixels
 * @return Width in pixels
 */
uint16_t textfield_calculate_text_width(const char* text, uint8_t font_width);

/**
 * @brief Count number of lines needed for text
 * 
 * @param text Text string
 * @param max_width Maximum width in pixels
 * @param font_width Character width in pixels
 * @return Number of lines
 */
uint16_t textfield_count_lines(const char* text, uint16_t max_width, uint8_t font_width);

// =============================================================================
// TEXT EDITOR EXTENSION
// =============================================================================

/**
 * TextEditor extends TextField with editing capabilities.
 * This avoids code duplication - TextField handles text buffer and scrolling,
 * TextEditor adds cursor, selection, clipboard, undo/redo, etc.
 */

// Forward declaration (defined in text_editor.h)
typedef struct TextEditor TextEditor;

/**
 * @brief Convert TextField to TextEditor (upcast)
 * 
 * @param tf Pointer to TextField (must be a TextEditor)
 * @return Pointer to TextEditor
 */
TextEditor* textfield_to_editor(TextField* tf);

/**
 * @brief Check if TextField is actually a TextEditor
 * 
 * @param tf Pointer to TextField
 * @return true if it's a TextEditor
 */
bool textfield_is_editor(const TextField* tf);

#endif // WIDGET_TEXTFIELD_H
