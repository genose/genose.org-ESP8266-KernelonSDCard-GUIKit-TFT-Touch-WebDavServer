/**
 * @file text_editor.h
 * @brief Full-featured text editor widget for GUIKit
 * @author GUIKit for ESP8266
 * @date 2026
 * 
 * This module provides a complete text editor widget that evolves from a simple
 * text field. It supports multi-line editing, cursor navigation, selection,
 * clipboard operations, undo/redo, and scrolling.
 * 
 * Memory optimization: Uses circular buffers for lines, pooled memory for text
 * ESP8266-friendly: No dynamic allocation, fixed-size buffers
 * 
 * Features:
 * - Multi-line text editing
 * - Cursor navigation (arrow keys, touch)
 * - Text selection (shift + navigation)
 * - Copy, cut, paste operations
 * - Undo/redo history (limited depth)
 * - Line numbers (optional)
 * - Word wrap (optional)
 * - Scrollable with touch
 * - Syntax highlighting hooks
 * 
 * Usage:
 * @code
 * TextEditor* editor = text_editor_create(10, 10, 240, 320);
 * text_editor_set_text(editor, "Hello\nWorld");
 * text_editor_show_line_numbers(editor, true);
 * 
 * // In main loop:
 * text_editor_handle_input(editor, input_char, key_modifiers);
 * text_editor_render(editor);
 * @endcode
 */

#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

#include "widget.h"
#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// CONSTANTS
// =============================================================================

/** Maximum number of lines in the editor */
#define TEXT_EDITOR_MAX_LINES 256

/** Maximum line length (characters) */
#define TEXT_EDITOR_MAX_LINE_LENGTH 256

/** Maximum undo/redo history depth */
#define TEXT_EDITOR_MAX_HISTORY 32

/** Maximum visible lines on screen */
#define TEXT_EDITOR_MAX_VISIBLE_LINES 20

/** Tab size in spaces */
#define TEXT_EDITOR_TAB_SIZE 4

/** Default font width (pixels) */
#define TEXT_EDITOR_FONT_WIDTH 8

/** Default font height (pixels) */
#define TEXT_EDITOR_FONT_HEIGHT 16

/** Line number gutter width (pixels) */
#define TEXT_EDITOR_GUTTER_WIDTH 40

/** Cursor blink interval (ms) */
#define TEXT_EDITOR_CURSOR_BLINK_MS 500

// =============================================================================
// TEXT EDITOR STRUCTURES
// =============================================================================

/**
 * @brief A single line of text in the editor
 */
typedef struct {
    char text[TEXT_EDITOR_MAX_LINE_LENGTH];  ///< Line content
    uint16_t length;                         ///< Actual text length
    uint16_t visual_length;                  ///< Visual length (accounting for tabs)
    bool dirty;                              ///< Needs re-rendering
} TextEditorLine;

/**
 * @brief Cursor position
 */
typedef struct {
    uint16_t line;      ///< Line index (0-based)
    uint16_t column;    ///< Column index (0-based, character position)
    uint16_t x_pos;      ///< X position in pixels (for rendering)
    uint16_t y_pos;      ///< Y position in pixels (for rendering)
    bool visible;       ///< Whether cursor is visible (blinking)
    uint32_t last_blink; ///< Last blink timestamp
} TextEditorCursor;

/**
 * @brief Text selection range
 */
typedef struct {
    uint16_t start_line;      ///< Selection start line
    uint16_t start_column;    ///< Selection start column
    uint16_t end_line;        ///< Selection end line
    uint16_t end_column;      ///< Selection end column
    bool active;              ///< Whether selection is active
    Color bg_color;          ///< Selection background color
    Color fg_color;          ///< Selection foreground color
} TextEditorSelection;

/**
 * @brief Scroll position
 */
typedef struct {
    int16_t top_line;         ///< Top visible line (0 = first line)
    int16_t left_column;      ///< Left visible column (0 = first column)
    uint16_t line_offset;     ///< Pixel offset for sub-line scrolling
} TextEditorScroll;

/**
 * @brief History entry for undo/redo
 */
typedef struct {
    TextEditorCursor cursor;  ///< Cursor position before change
    TextEditorSelection selection;  ///< Selection before change
    uint16_t line;           ///< Line that was modified
    uint16_t column;         ///< Column where modification started
    uint16_t removed_length;  ///< Length of removed text
    char removed_text[TEXT_EDITOR_MAX_LINE_LENGTH];  ///< Removed text
    char inserted_text[TEXT_EDITOR_MAX_LINE_LENGTH]; ///< Inserted text
    bool was_insert;         ///< Whether this was an insert operation
} TextEditorHistoryEntry;

/**
 * @brief Undo/redo history stack
 */
typedef struct {
    TextEditorHistoryEntry entries[TEXT_EDITOR_MAX_HISTORY];
    int8_t current;  ///< Current position (-1 = no history)
    int8_t top;      ///< Top of undo stack
    int8_t redo_top; ///< Top of redo stack
} TextEditorHistory;

/**
 * @brief Text editor widget structure
 */
typedef struct TextEditor {
    // Widget base
    Widget* widget;                  ///< Parent widget (optional)
    
    // Position and size
    struct {
        uint16_t x;
        uint16_t y;
        uint16_t width;
        uint16_t height;
    } bounds;
    
    // Text content
    TextEditorLine lines[TEXT_EDITOR_MAX_LINES];
    uint16_t line_count;  ///< Number of lines in use
    
    // Cursor
    TextEditorCursor cursor;
    
    // Selection
    TextEditorSelection selection;
    
    // Scroll
    TextEditorScroll scroll;
    
    // History
    TextEditorHistory history;
    
    // Display settings
    struct {
        uint8_t font_width;     ///< Character width in pixels
        uint8_t font_height;    ///< Character height in pixels
        uint8_t tab_size;       ///< Tab size in spaces
        bool show_line_numbers; ///< Display line numbers
        bool word_wrap;         ///< Enable word wrapping
        bool read_only;         ///< Read-only mode
        bool syntax_highlight;  ///< Enable syntax highlighting
    } settings;
    
    // Styling
    struct {
        Color bg_color;         ///< Background color
        Color fg_color;         ///< Foreground (text) color
        Color cursor_color;     ///< Cursor color
        Color line_number_color; ///< Line number color
        Color selection_bg;     ///< Selection background
        Color selection_fg;     ///< Selection foreground
        Color gutter_bg;        ///< Gutter background
    } colors;
    
    // State
    struct {
        bool focused;           ///< Has input focus
        bool dirty;            ///< Content has changed
        bool needs_render;     ///< Needs re-rendering
        uint32_t last_input;    ///< Last input timestamp
    } state;
    
    // Callbacks
    struct {
        void (*on_change)(struct TextEditor*);        ///< Text changed callback
        void (*on_cursor_move)(struct TextEditor*);   ///< Cursor moved callback
        void (*on_selection_change)(struct TextEditor*); ///< Selection changed callback
        bool (*on_key)(struct TextEditor*, char, uint8_t); ///< Custom key handler
    } callbacks;
    
    // Touch handling
    struct {
        uint16_t touch_x;
        uint16_t touch_y;
        uint32_t touch_start_time;
        bool touch_active;
        bool touch_dragging;
    } touch;
    
    // Clipboard
    struct {
        char text[TEXT_EDITOR_MAX_LINE_LENGTH];
        uint16_t length;
    } clipboard;
} TextEditor;

// =============================================================================
// KEY MODIFIERS
// =============================================================================

typedef enum {
    KEY_MOD_NONE = 0,
    KEY_MOD_SHIFT = 1,
    KEY_MOD_CTRL = 2,
    KEY_MOD_ALT = 4
} KeyModifier;

// =============================================================================
// KEY CODES
// =============================================================================

typedef enum {
    KEY_NONE = 0,
    KEY_BACKSPACE = 8,
    KEY_TAB = 9,
    KEY_ENTER = 13,
    KEY_ESCAPE = 27,
    KEY_UP = 100,
    KEY_DOWN = 101,
    KEY_LEFT = 102,
    KEY_RIGHT = 103,
    KEY_PAGE_UP = 104,
    KEY_PAGE_DOWN = 105,
    KEY_HOME = 106,
    KEY_END = 107,
    KEY_DELETE = 108,
    KEY_INSERT = 109,
    KEY_F1 = 110,
    KEY_F2 = 111,
    KEY_F3 = 112,
    KEY_F4 = 113
} KeyCode;

// =============================================================================
// TEXT EDITOR EVENTS
// =============================================================================

typedef enum {
    TEXT_EDITOR_EVENT_CHANGE,
    TEXT_EDITOR_EVENT_CURSOR_MOVE,
    TEXT_EDITOR_EVENT_SELECTION_CHANGE,
    TEXT_EDITOR_EVENT_FOCUS_GAIN,
    TEXT_EDITOR_EVENT_FOCUS_LOSE
} TextEditorEvent;

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// --- Lifecycle ---

/**
 * @brief Create a new text editor
 * @param x X position
 * @param y Y position
 * @param width Width in pixels
 * @param height Height in pixels
 * @return Pointer to new TextEditor, or NULL on failure
 */
TextEditor* text_editor_create(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

/**
 * @brief Destroy a text editor
 * @param editor TextEditor pointer
 */
void text_editor_destroy(TextEditor* editor);

/**
 * @brief Initialize an existing TextEditor structure
 * @param editor TextEditor pointer
 * @param x X position
 * @param y Y position
 * @param width Width in pixels
 * @param height Height in pixels
 */
void text_editor_init(TextEditor* editor, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

/**
 * @brief Reset text editor to empty state
 * @param editor TextEditor pointer
 */
void text_editor_reset(TextEditor* editor);

// --- Text Content ---

/**
 * @brief Set the entire text content
 * @param editor TextEditor pointer
 * @param text Text to set (can be multi-line)
 */
void text_editor_set_text(TextEditor* editor, const char* text);

/**
 * @brief Get the entire text content
 * @param editor TextEditor pointer
 * @param buffer Buffer to write to
 * @param buffer_size Buffer size
 * @return Number of characters written
 */
uint16_t text_editor_get_text(TextEditor* editor, char* buffer, uint16_t buffer_size);

/**
 * @brief Get text length
 * @param editor TextEditor pointer
 * @return Total number of characters
 */
uint32_t text_editor_get_text_length(const TextEditor* editor);

/**
 * @brief Get number of lines
 * @param editor TextEditor pointer
 * @return Number of lines
 */
uint16_t text_editor_get_line_count(const TextEditor* editor);

/**
 * @brief Get a specific line
 * @param editor TextEditor pointer
 * @param line_index Line index (0-based)
 * @return Pointer to line text, or NULL if invalid
 */
const char* text_editor_get_line(const TextEditor* editor, uint16_t line_index);

/**
 * @brief Get line length
 * @param editor TextEditor pointer
 * @param line_index Line index
 * @return Length of line, or 0 if invalid
 */
uint16_t text_editor_get_line_length(const TextEditor* editor, uint16_t line_index);

/**
 * @brief Insert text at cursor position
 * @param editor TextEditor pointer
 * @param text Text to insert
 * @param length Length of text (0 = strlen)
 */
void text_editor_insert_text(TextEditor* editor, const char* text, uint16_t length);

/**
 * @brief Insert a single character at cursor position
 * @param editor TextEditor pointer
 * @param c Character to insert
 */
void text_editor_insert_char(TextEditor* editor, char c);

/**
 * @brief Delete text at cursor position
 * @param editor TextEditor pointer
 * @param length Number of characters to delete (0 = delete selection or 1 char)
 */
void text_editor_delete_text(TextEditor* editor, uint16_t length);

/**
 * @brief Delete a single character at cursor position
 * @param editor TextEditor pointer
 */
void text_editor_delete_char(TextEditor* editor);

/**
 * @brief Delete a single character before cursor (backspace)
 * @param editor TextEditor pointer
 */
void text_editor_backspace(TextEditor* editor);

// --- Cursor Operations ---

/**
 * @brief Set cursor position
 * @param editor TextEditor pointer
 * @param line Line index
 * @param column Column index
 */
void text_editor_set_cursor(TextEditor* editor, uint16_t line, uint16_t column);

/**
 * @brief Get cursor line
 * @param editor TextEditor pointer
 * @return Cursor line
 */
uint16_t text_editor_get_cursor_line(const TextEditor* editor);

/**
 * @brief Get cursor column
 * @param editor TextEditor pointer
 * @return Cursor column
 */
uint16_t text_editor_get_cursor_column(const TextEditor* editor);

/**
 * @brief Move cursor up
 * @param editor TextEditor pointer
 * @param shift If true, extend selection
 */
void text_editor_move_cursor_up(TextEditor* editor, bool shift);

/**
 * @brief Move cursor down
 * @param editor TextEditor pointer
 * @param shift If true, extend selection
 */
void text_editor_move_cursor_down(TextEditor* editor, bool shift);

/**
 * @brief Move cursor left
 * @param editor TextEditor pointer
 * @param shift If true, extend selection
 */
void text_editor_move_cursor_left(TextEditor* editor, bool shift);

/**
 * @brief Move cursor right
 * @param editor TextEditor pointer
 * @param shift If true, extend selection
 */
void text_editor_move_cursor_right(TextEditor* editor, bool shift);

/**
 * @brief Move cursor to line start
 * @param editor TextEditor pointer
 * @param shift If true, extend selection
 */
void text_editor_move_cursor_home(TextEditor* editor, bool shift);

/**
 * @brief Move cursor to line end
 * @param editor TextEditor pointer
 * @param shift If true, extend selection
 */
void text_editor_move_cursor_end(TextEditor* editor, bool shift);

/**
 * @brief Move cursor to document start
 * @param editor TextEditor pointer
 * @param shift If true, extend selection
 */
void text_editor_move_cursor_document_start(TextEditor* editor, bool shift);

/**
 * @brief Move cursor to document end
 * @param editor TextEditor pointer
 * @param shift If true, extend selection
 */
void text_editor_move_cursor_document_end(TextEditor* editor, bool shift);

/**
 * @brief Move cursor to specific position (pixel-based)
 * @param editor TextEditor pointer
 * @param x X pixel position
 * @param y Y pixel position
 */
void text_editor_move_cursor_to(TextEditor* editor, uint16_t x, uint16_t y);

// --- Selection Operations ---

/**
 * @brief Set selection range
 * @param editor TextEditor pointer
 * @param start_line Start line
 * @param start_column Start column
 * @param end_line End line
 * @param end_column End column
 */
void text_editor_set_selection(TextEditor* editor, uint16_t start_line, uint16_t start_column,
                              uint16_t end_line, uint16_t end_column);

/**
 * @brief Clear selection
 * @param editor TextEditor pointer
 */
void text_editor_clear_selection(TextEditor* editor);

/**
 * @brief Select all text
 * @param editor TextEditor pointer
 */
void text_editor_select_all(TextEditor* editor);

/**
 * @brief Check if there is an active selection
 * @param editor TextEditor pointer
 * @return true if selection is active
 */
bool text_editor_has_selection(const TextEditor* editor);

/**
 * @brief Get selected text
 * @param editor TextEditor pointer
 * @param buffer Buffer to write to
 * @param buffer_size Buffer size
 * @return Number of characters written
 */
uint16_t text_editor_get_selected_text(const TextEditor* editor, char* buffer, uint16_t buffer_size);

/**
 * @brief Get selection start position
 * @param editor TextEditor pointer
 * @param line Output line
 * @param column Output column
 */
void text_editor_get_selection_start(const TextEditor* editor, uint16_t* line, uint16_t* column);

/**
 * @brief Get selection end position
 * @param editor TextEditor pointer
 * @param line Output line
 * @param column Output column
 */
void text_editor_get_selection_end(const TextEditor* editor, uint16_t* line, uint16_t* column);

// --- Clipboard Operations ---

/**
 * @brief Copy selected text to clipboard
 * @param editor TextEditor pointer
 */
void text_editor_copy(TextEditor* editor);

/**
 * @brief Cut selected text to clipboard
 * @param editor TextEditor pointer
 */
void text_editor_cut(TextEditor* editor);

/**
 * @brief Paste from clipboard
 * @param editor TextEditor pointer
 */
void text_editor_paste(TextEditor* editor);

/**
 * @brief Set clipboard text
 * @param editor TextEditor pointer
 * @param text Text to set in clipboard
 */
void text_editor_set_clipboard(TextEditor* editor, const char* text);

/**
 * @brief Get clipboard text
 * @param editor TextEditor pointer
 * @return Clipboard text (internal buffer, don't free)
 */
const char* text_editor_get_clipboard(const TextEditor* editor);

// --- Undo/Redo ---

/**
 * @brief Undo last operation
 * @param editor TextEditor pointer
 */
void text_editor_undo(TextEditor* editor);

/**
 * @brief Redo last undone operation
 * @param editor TextEditor pointer
 */
void text_editor_redo(TextEditor* editor);

/**
 * @brief Check if undo is available
 * @param editor TextEditor pointer
 * @return true if undo is available
 */
bool text_editor_can_undo(const TextEditor* editor);

/**
 * @brief Check if redo is available
 * @param editor TextEditor pointer
 * @return true if redo is available
 */
bool text_editor_can_redo(const TextEditor* editor);

/**
 * @brief Clear undo/redo history
 * @param editor TextEditor pointer
 */
void text_editor_clear_history(TextEditor* editor);

// --- Scroll Operations ---

/**
 * @brief Scroll up by one line
 * @param editor TextEditor pointer
 */
void text_editor_scroll_up(TextEditor* editor);

/**
 * @brief Scroll down by one line
 * @param editor TextEditor pointer
 */
void text_editor_scroll_down(TextEditor* editor);

/**
 * @brief Scroll to cursor
 * @param editor TextEditor pointer
 */
void text_editor_scroll_to_cursor(TextEditor* editor);

/**
 * @brief Scroll to line
 * @param editor TextEditor pointer
 * @param line Line to scroll to
 */
void text_editor_scroll_to_line(TextEditor* editor, uint16_t line);

/**
 * @brief Set scroll position
 * @param editor TextEditor pointer
 * @param top_line Top visible line
 * @param left_column Left visible column
 */
void text_editor_set_scroll(TextEditor* editor, int16_t top_line, int16_t left_column);

/**
 * @brief Get visible line range
 * @param editor TextEditor pointer
 * @param start_line Output start line
 * @param end_line Output end line
 */
void text_editor_get_visible_lines(const TextEditor* editor, uint16_t* start_line, uint16_t* end_line);

// --- Input Handling ---

/**
 * @brief Handle character input
 * @param editor TextEditor pointer
 * @param c Character
 * @param modifiers Key modifiers (SHIFT, CTRL, ALT)
 */
void text_editor_handle_char(TextEditor* editor, char c, KeyModifier modifiers);

/**
 * @brief Handle key input
 * @param editor TextEditor pointer
 * @param key Key code
 * @param modifiers Key modifiers
 */
void text_editor_handle_key(TextEditor* editor, KeyCode key, KeyModifier modifiers);

/**
 * @brief Handle touch input
 * @param editor TextEditor pointer
 * @param x X touch position
 * @param y Y touch position
 * @param pressed Whether touch is pressed or released
 */
void text_editor_handle_touch(TextEditor* editor, uint16_t x, uint16_t y, bool pressed);

// --- Rendering ---

/**
 * @brief Render the entire text editor
 * @param editor TextEditor pointer
 */
void text_editor_render(TextEditor* editor);

/**
 * @brief Render a specific line
 * @param editor TextEditor pointer
 * @param line_index Line to render
 */
void text_editor_render_line(TextEditor* editor, uint16_t line_index);

/**
 * @brief Render the cursor
 * @param editor TextEditor pointer
 */
void text_editor_render_cursor(TextEditor* editor);

/**
 * @brief Render the selection
 * @param editor TextEditor pointer
 */
void text_editor_render_selection(TextEditor* editor);

/**
 * @brief Render line numbers
 * @param editor TextEditor pointer
 */
void text_editor_render_line_numbers(TextEditor* editor);

/**
 * @brief Render the gutter (line number area)
 * @param editor TextEditor pointer
 */
void text_editor_render_gutter(TextEditor* editor);

// --- Settings ---

/**
 * @brief Show or hide line numbers
 * @param editor TextEditor pointer
 * @param show Whether to show line numbers
 */
void text_editor_show_line_numbers(TextEditor* editor, bool show);

/**
 * @brief Enable or disable word wrap
 * @param editor TextEditor pointer
 * @param wrap Whether to enable word wrap
 */
void text_editor_set_word_wrap(TextEditor* editor, bool wrap);

/**
 * @brief Set read-only mode
 * @param editor TextEditor pointer
 * @param read_only Whether to set read-only
 */
void text_editor_set_read_only(TextEditor* editor, bool read_only);

/**
 * @brief Set tab size
 * @param editor TextEditor pointer
 * @param size Tab size in spaces
 */
void text_editor_set_tab_size(TextEditor* editor, uint8_t size);

// --- Styling ---

/**
 * @brief Set background color
 * @param editor TextEditor pointer
 * @param color RGB565 color
 */
void text_editor_set_bg_color(TextEditor* editor, Color color);

/**
 * @brief Set foreground (text) color
 * @param editor TextEditor pointer
 * @param color RGB565 color
 */
void text_editor_set_fg_color(TextEditor* editor, Color color);

/**
 * @brief Set cursor color
 * @param editor TextEditor pointer
 * @param color RGB565 color
 */
void text_editor_set_cursor_color(TextEditor* editor, Color color);

/**
 * @brief Set selection colors
 * @param editor TextEditor pointer
 * @param bg_color Background color
 * @param fg_color Foreground color
 */
void text_editor_set_selection_colors(TextEditor* editor, Color bg_color, Color fg_color);

/**
 * @brief Set line number color
 * @param editor TextEditor pointer
 * @param color RGB565 color
 */
void text_editor_set_line_number_color(TextEditor* editor, Color color);

/**
 * @brief Set gutter background color
 * @param editor TextEditor pointer
 * @param color RGB565 color
 */
void text_editor_set_gutter_bg_color(TextEditor* editor, Color color);

// --- Focus ---

/**
 * @brief Set input focus
 * @param editor TextEditor pointer
 * @param focused Whether to set focus
 */
void text_editor_set_focus(TextEditor* editor, bool focused);

/**
 * @brief Check if editor has focus
 * @param editor TextEditor pointer
 * @return true if focused
 */
bool text_editor_has_focus(const TextEditor* editor);

// --- State ---

/**
 * @brief Check if editor content has changed
 * @param editor TextEditor pointer
 * @return true if dirty
 */
bool text_editor_is_dirty(const TextEditor* editor);

/**
 * @brief Clear dirty flag
 * @param editor TextEditor pointer
 */
void text_editor_clear_dirty(TextEditor* editor);

// --- Callbacks ---

/**
 * @brief Set change callback
 * @param editor TextEditor pointer
 * @param callback Callback function
 */
void text_editor_set_on_change(TextEditor* editor, void (*callback)(TextEditor*));

/**
 * @brief Set cursor move callback
 * @param editor TextEditor pointer
 * @param callback Callback function
 */
void text_editor_set_on_cursor_move(TextEditor* editor, void (*callback)(TextEditor*));

/**
 * @brief Set selection change callback
 * @param editor TextEditor pointer
 * @param callback Callback function
 */
void text_editor_set_on_selection_change(TextEditor* editor, void (*callback)(TextEditor*));

/**
 * @brief Set custom key handler
 * @param editor TextEditor pointer
 * @param callback Callback function (return true to handle, false to continue)
 */
void text_editor_set_on_key(TextEditor* editor, bool (*callback)(TextEditor*, char, uint8_t));

// --- Line Operations ---

/**
 * @brief Insert a new line at position
 * @param editor TextEditor pointer
 * @param line_index Line index to insert before
 * @param text Initial text for new line
 */
void text_editor_insert_line(TextEditor* editor, uint16_t line_index, const char* text);

/**
 * @brief Delete a line
 * @param editor TextEditor pointer
 * @param line_index Line index to delete
 */
void text_editor_delete_line(TextEditor* editor, uint16_t line_index);

/**
 * @brief Split current line at cursor
 * @param editor TextEditor pointer
 */
void text_editor_split_line(TextEditor* editor);

/**
 * @brief Join current line with next
 * @param editor TextEditor pointer
 */
void text_editor_join_lines(TextEditor* editor);

// --- Word Operations ---

/**
 * @brief Move cursor to next word
 * @param editor TextEditor pointer
 * @param shift If true, extend selection
 */
void text_editor_move_cursor_next_word(TextEditor* editor, bool shift);

/**
 * @brief Move cursor to previous word
 * @param editor TextEditor pointer
 * @param shift If true, extend selection
 */
void text_editor_move_cursor_previous_word(TextEditor* editor, bool shift);

/**
 * @brief Delete word before cursor
 * @param editor TextEditor pointer
 */
void text_editor_delete_word_before_cursor(TextEditor* editor);

/**
 * @brief Delete word after cursor
 * @param editor TextEditor pointer
 */
void text_editor_delete_word_after_cursor(TextEditor* editor);

// --- Utility Functions ---

/**
 * @brief Count characters in a line
 * @param line TextEditorLine pointer
 * @return Character count
 */
uint16_t text_editor_line_length(const TextEditorLine* line);

/**
 * @brief Get character at position
 * @param editor TextEditor pointer
 * @param line Line index
 * @param column Column index
 * @return Character at position, or 0 if out of bounds
 */
char text_editor_get_char(const TextEditor* editor, uint16_t line, uint16_t column);

/**
 * @brief Set character at position
 * @param editor TextEditor pointer
 * @param line Line index
 * @param column Column index
 * @param c Character to set
 */
void text_editor_set_char(TextEditor* editor, uint16_t line, uint16_t column, char c);

/**
 * @brief Find word boundary
 * @param line Line text
 * @param start_pos Starting position
 * @param forward Whether to search forward
 * @return Word boundary position
 */
int16_t text_editor_find_word_boundary(const char* line, uint16_t start_pos, bool forward);

/**
 * @brief Get visible width in characters
 * @param editor TextEditor pointer
 * @return Number of visible characters per line
 */
uint16_t text_editor_get_visible_width(const TextEditor* editor);

/**
 * @brief Get visible height in lines
 * @param editor TextEditor pointer
 * @return Number of visible lines
 */
uint16_t text_editor_get_visible_height(const TextEditor* editor);

/**
 * @brief Check if a character is a word character
 * @param c Character
 * @return true if word character
 */
bool text_editor_is_word_char(char c);

/**
 * @brief Expand tab to spaces
 * @param editor TextEditor pointer
 * @param line Line index
 */
void text_editor_expand_tabs(TextEditor* editor, uint16_t line);

// =============================================================================
// INLINE FUNCTIONS
// =============================================================================

/**
 * @brief Check if cursor is at line start
 * @param editor TextEditor pointer
 * @return true if at line start
 */
static inline bool text_editor_cursor_at_line_start(const TextEditor* editor) {
    return editor->cursor.column == 0;
}

/**
 * @brief Check if cursor is at line end
 * @param editor TextEditor pointer
 * @return true if at line end
 */
static inline bool text_editor_cursor_at_line_end(const TextEditor* editor) {
    if (editor->cursor.line >= editor->line_count) return true;
    return editor->cursor.column >= editor->lines[editor->cursor.line].length;
}

/**
 * @brief Check if cursor is at document start
 * @param editor TextEditor pointer
 * @return true if at document start
 */
static inline bool text_editor_cursor_at_document_start(const TextEditor* editor) {
    return editor->cursor.line == 0 && editor->cursor.column == 0;
}

/**
 * @brief Check if cursor is at document end
 * @param editor TextEditor pointer
 * @return true if at document end
 */
static inline bool text_editor_cursor_at_document_end(const TextEditor* editor) {
    if (editor->line_count == 0) return true;
    uint16_t last_line = editor->line_count - 1;
    return editor->cursor.line == last_line && 
           editor->cursor.column >= editor->lines[last_line].length;
}

/**
 * @brief Get current line
 * @param editor TextEditor pointer
 * @return Current line pointer
 */
static inline TextEditorLine* text_editor_get_current_line(TextEditor* editor) {
    if (editor->cursor.line >= editor->line_count) return NULL;
    return &editor->lines[editor->cursor.line];
}

/**
 * @brief Get current line (const)
 * @param editor TextEditor pointer
 * @return Current line pointer
 */
static inline const TextEditorLine* text_editor_get_current_line_const(const TextEditor* editor) {
    if (editor->cursor.line >= editor->line_count) return NULL;
    return &editor->lines[editor->cursor.line];
}

/**
 * @brief Get line at index
 * @param editor TextEditor pointer
 * @param index Line index
 * @return Line pointer or NULL
 */
static inline TextEditorLine* text_editor_get_line_mut(TextEditor* editor, uint16_t index) {
    if (index >= editor->line_count) return NULL;
    return &editor->lines[index];
}

/**
 * @brief Get character under cursor
 * @param editor TextEditor pointer
 * @return Character or 0
 */
static inline char text_editor_get_cursor_char(const TextEditor* editor) {
    if (editor->cursor.line >= editor->line_count) return 0;
    if (editor->cursor.column >= editor->lines[editor->cursor.line].length) return 0;
    return editor->lines[editor->cursor.line].text[editor->cursor.column];
}

/**
 * @brief Check if editor is empty
 * @param editor TextEditor pointer
 * @return true if empty
 */
static inline bool text_editor_is_empty(const TextEditor* editor) {
    return editor->line_count == 0 || 
           (editor->line_count == 1 && editor->lines[0].length == 0);
}

/**
 * @brief Get clipboard text length
 * @param editor TextEditor pointer
 * @return Clipboard length
 */
static inline uint16_t text_editor_get_clipboard_length(const TextEditor* editor) {
    return editor->clipboard.length;
}

/**
 * @brief Check if clipboard has content
 * @param editor TextEditor pointer
 * @return true if clipboard has content
 */
static inline bool text_editor_clipboard_has_content(const TextEditor* editor) {
    return editor->clipboard.length > 0;
}

#endif // TEXT_EDITOR_H
