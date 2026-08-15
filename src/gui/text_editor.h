/**
 * @file text_editor.h
 * @brief TextEditor extends TextField for complete text editing
 * @author GUIKit for ESP8266
 * @date 2026
 * 
 * Architecture: TextEditor EXTENDS TextField to avoid code duplication.
 * 
 * TextField (widget_textfield.h) handles:
 * - Text buffer and line management (1 line to 4K characters)
 * - Scrolling (via ScrollableProperty)
 * - Basic display settings (font, word wrap, multi-line)
 * - Text styling (colors)
 * 
 * TextEditor extends TextField to add:
 * - Cursor navigation and management
 * - Text selection
 * - Clipboard operations (copy, cut, paste)
 * - Undo/redo history
 * - Line numbers
 * - Syntax highlighting hooks
 * - Input handling
 * 
 * Memory optimization: Uses circular buffers, pooled memory
 * ESP8266-friendly: No dynamic allocation, fixed-size buffers
 */

#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

#include "widget_textfield.h"
#include "clipboard.h"
#include <stdint.h>
#include <stdbool.h>

// Constants
#define TEXT_EDITOR_MAX_HISTORY 32
#define TEXT_EDITOR_MAX_VISIBLE_LINES 20
#define TEXT_EDITOR_TAB_SIZE 4
#define TEXT_EDITOR_GUTTER_WIDTH 40
#define TEXT_EDITOR_CURSOR_BLINK_MS 500
#define TEXT_EDITOR_FONT_WIDTH TEXTFIELD_FONT_WIDTH
#define TEXT_EDITOR_FONT_HEIGHT TEXTFIELD_FONT_HEIGHT

// Editor-specific structures
typedef struct {
    uint16_t line; uint16_t column; uint16_t x_pos; uint16_t y_pos;
    bool visible; uint32_t last_blink;
} TextEditorCursor;

typedef struct {
    uint16_t start_line; uint16_t start_column;
    uint16_t end_line; uint16_t end_column;
    bool active; Color bg_color; Color fg_color;
} TextEditorSelection;

typedef struct { int16_t line_offset; } TextEditorScroll;

typedef struct {
    TextEditorCursor cursor; TextEditorSelection selection;
    uint16_t line; uint16_t column; uint16_t removed_length;
    char removed_text[TEXTFIELD_MAX_LINE_LENGTH];
    char inserted_text[TEXTFIELD_MAX_LINE_LENGTH];
    bool was_insert;
} TextEditorHistoryEntry;

typedef struct {
    TextEditorHistoryEntry entries[TEXT_EDITOR_MAX_HISTORY];
    int8_t current; int8_t top; int8_t redo_top;
} TextEditorHistory;

// TextEditor extends TextField (MUST be first member for polymorphism)
typedef struct TextEditor {
    TextField textfield;  // Base TextField
    
    // Editor-specific additions
    TextEditorCursor cursor;
    TextEditorSelection selection;
    TextEditorScroll scroll;
    TextEditorHistory history;
    
    struct {
        bool show_line_numbers;
        bool syntax_highlight;
    } editor_settings;
    
    struct {
        Color line_number_color;
        Color selection_bg;
        Color selection_fg;
        Color gutter_bg;
    } editor_colors;
    
    struct {
        bool focused;
        bool dirty;
        bool needs_render;
        uint32_t last_activity;
    } state;
    
    // Callbacks
    void (*on_cursor_move)(struct TextEditor* editor);
    void (*on_selection_change)(struct TextEditor* editor);
    void (*on_undo_redo)(struct TextEditor* editor, bool is_undo);
    Color (*syntax_highlight_callback)(struct TextEditor* editor, uint16_t line, uint16_t column, const char* text, uint16_t length);
} TextEditor;

// Accessors (delegate to TextField where appropriate)
static inline WIDGET_TYPE text_editor_get_type(const TextEditor* e) { return WIDGET_TYPE_TEXT_EDITOR; }
static inline uint16_t text_editor_get_x(const TextEditor* e) { return e->textfield.base.x; }
static inline uint16_t text_editor_get_y(const TextEditor* e) { return e->textfield.base.y; }
static inline uint16_t text_editor_get_width(const TextEditor* e) { return e->textfield.base.width; }
static inline uint16_t text_editor_get_height(const TextEditor* e) { return e->textfield.base.height; }
static inline uint16_t text_editor_get_line_count(const TextEditor* e) { return e->textfield.line_count; }
static inline const char* text_editor_get_text(const TextEditor* e) { return textfield_get_text(&e->textfield); }
static inline void text_editor_set_text(TextEditor* e, const char* t) { textfield_set_text(&e->textfield, t); }
static inline void text_editor_clear(TextEditor* e) { textfield_clear(&e->textfield); }
static inline TextFieldLine* text_editor_get_line(TextEditor* e, uint16_t i) { return textfield_get_line(&e->textfield, i); }
static inline void text_editor_scroll_to_line(TextEditor* e, uint16_t l) { textfield_scroll_to_line(&e->textfield, l); }
static inline void text_editor_scroll_up(TextEditor* e) { textfield_scroll_up(&e->textfield); }
static inline void text_editor_scroll_down(TextEditor* e) { textfield_scroll_down(&e->textfield); }
static inline void text_editor_set_multiline(TextEditor* e, bool b) { textfield_set_multiline(&e->textfield, b); }
static inline void text_editor_set_word_wrap(TextEditor* e, bool b) { textfield_set_word_wrap(&e->textfield, b); }
static inline void text_editor_set_read_only(TextEditor* e, bool b) { textfield_set_read_only(&e->textfield, b); }
static inline void text_editor_set_font(TextEditor* e, uint8_t w, uint8_t h) { textfield_set_font(&e->textfield, w, h); }
static inline void text_editor_set_bg_color(TextEditor* e, Color c) { textfield_set_bg_color(&e->textfield, c); }
static inline void text_editor_set_fg_color(TextEditor* e, Color c) { textfield_set_fg_color(&e->textfield, c); }

// Constructor
TextEditor* text_editor_create(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void text_editor_init(TextEditor* editor, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void text_editor_destroy(TextEditor* editor);

// Editor-specific settings
void text_editor_show_line_numbers(TextEditor* editor, bool show);
void text_editor_set_syntax_highlight(TextEditor* editor, bool enabled, Color (*callback)(TextEditor*, uint16_t, uint16_t, const char*, uint16_t));
void text_editor_set_line_number_color(TextEditor* editor, Color color);
void text_editor_set_selection_colors(TextEditor* editor, Color bg_color, Color fg_color);

// Cursor
void text_editor_set_cursor(TextEditor* editor, uint16_t line, uint16_t column);
void text_editor_cursor_up(TextEditor* editor);
void text_editor_cursor_down(TextEditor* editor);
void text_editor_cursor_left(TextEditor* editor);
void text_editor_cursor_right(TextEditor* editor);
void text_editor_cursor_home(TextEditor* editor);
void text_editor_cursor_end(TextEditor* editor);
void text_editor_cursor_document_start(TextEditor* editor);
void text_editor_cursor_document_end(TextEditor* editor);
uint16_t text_editor_get_cursor_line(const TextEditor* editor);
uint16_t text_editor_get_cursor_column(const TextEditor* editor);
void text_editor_update_cursor_position(TextEditor* editor);
void text_editor_blink_cursor(TextEditor* editor);

// Selection
void text_editor_set_selection(TextEditor* editor, uint16_t start_line, uint16_t start_column, uint16_t end_line, uint16_t end_column);
void text_editor_clear_selection(TextEditor* editor);
void text_editor_select_all(TextEditor* editor);
void text_editor_select_line(TextEditor* editor);
void text_editor_select_word(TextEditor* editor);
bool text_editor_has_selection(const TextEditor* editor);
char* text_editor_get_selected_text(const TextEditor* editor);

// Clipboard
void text_editor_copy(TextEditor* editor);
void text_editor_cut(TextEditor* editor);
void text_editor_paste(TextEditor* editor);

// Editing
void text_editor_insert_char(TextEditor* editor, char c);
void text_editor_insert_text(TextEditor* editor, const char* text);
void text_editor_delete_char(TextEditor* editor);
void text_editor_backspace(TextEditor* editor);
void text_editor_delete_selection(TextEditor* editor);
void text_editor_insert_newline(TextEditor* editor);
void text_editor_indent(TextEditor* editor);
void text_editor_unindent(TextEditor* editor);

// Undo/Redo
void text_editor_undo(TextEditor* editor);
void text_editor_redo(TextEditor* editor);
bool text_editor_can_undo(const TextEditor* editor);
bool text_editor_can_redo(const TextEditor* editor);
void text_editor_clear_history(TextEditor* editor);

// Input handling
void text_editor_handle_char(TextEditor* editor, char c, uint8_t modifiers);
void text_editor_handle_key(TextEditor* editor, uint8_t key);
void text_editor_handle_touch(TextEditor* editor, uint16_t x, uint16_t y, bool pressed);

// Rendering
void text_editor_render(const TextEditor* editor);
void text_editor_render_text(const TextEditor* editor, uint16_t x, uint16_t y);
void text_editor_render_cursor(const TextEditor* editor);
void text_editor_render_selection(const TextEditor* editor);
void text_editor_render_line_numbers(const TextEditor* editor);
void text_editor_render_gutter(const TextEditor* editor);

// Focus
void text_editor_set_focus(TextEditor* editor, bool focused);
bool text_editor_is_focused(const TextEditor* editor);

// State
void text_editor_set_dirty(TextEditor* editor, bool dirty);
bool text_editor_is_dirty(const TextEditor* editor);

// Callbacks
void text_editor_set_on_cursor_move(TextEditor* editor, void (*callback)(TextEditor*));
void text_editor_set_on_selection_change(TextEditor* editor, void (*callback)(TextEditor*));
void text_editor_set_on_undo_redo(TextEditor* editor, void (*callback)(TextEditor*, bool));

// Utility
static inline const char* text_editor_get_current_line_text(const TextEditor* e) {
    TextFieldLine* line = textfield_get_line(&e->textfield, e->cursor.line);
    return line ? line->text : "";
}
static inline TextFieldLine* text_editor_get_current_line(TextEditor* e) {
    return textfield_get_line(&e->textfield, e->cursor.line);
}
static inline uint16_t text_editor_get_text_width(const TextEditor* e, const char* t) {
    return textfield_calculate_text_width(t, e->textfield.settings.font_width);
}
static inline uint16_t text_editor_get_line_height(const TextEditor* e) { return e->textfield.settings.font_height; }
static inline uint16_t text_editor_get_visible_lines(const TextEditor* e) { return e->textfield.base.height / e->textfield.settings.font_height; }

#endif // TEXT_EDITOR_H
