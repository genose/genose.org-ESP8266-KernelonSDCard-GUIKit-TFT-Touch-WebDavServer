/**
 * @file text_editor.c
 * @brief Full-featured text editor widget implementation for GUIKit
 * @author GUIKit for ESP8266
 * @date 2026
 */

#include "text_editor.h"
#include "widget_gradient.h"
#include <string.h>
#include <stdlib.h>

// =============================================================================
// STATIC HELPERS
// =============================================================================

/**
 * @brief Get current timestamp (for cursor blink)
 * @return Timestamp in milliseconds
 */
static uint32_t get_timestamp() {
    // For ESP8266, use millis() from Arduino
    // This is a placeholder - actual implementation depends on platform
    extern uint32_t millis();
    return millis();
}

/**
 * @brief Clamp value to range
 */
static uint16_t clamp_uint16(uint16_t value, uint16_t min_val, uint16_t max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

/**
 * @brief Clamp int16 to range
 */
static int16_t clamp_int16(int16_t value, int16_t min_val, int16_t max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

/**
 * @brief Shift lines down starting from index
 */
static void shift_lines_down(TextEditor* editor, uint16_t start_index) {
    if (start_index >= TEXT_EDITOR_MAX_LINES - 1) return;
    
    uint16_t count = editor->line_count - start_index;
    if (count == 0) return;
    
    memmove(&editor->lines[start_index + 1], 
            &editor->lines[start_index],
            count * sizeof(TextEditorLine));
}

/**
 * @brief Shift lines up starting from index
 */
static void shift_lines_up(TextEditor* editor, uint16_t start_index) {
    if (start_index >= editor->line_count) return;
    
    uint16_t count = editor->line_count - start_index - 1;
    if (count > 0) {
        memmove(&editor->lines[start_index],
                &editor->lines[start_index + 1],
                count * sizeof(TextEditorLine));
    }
    
    // Clear the last line
    if (editor->line_count > 0) {
        memset(&editor->lines[editor->line_count - 1], 0, sizeof(TextEditorLine));
    }
}

/**
 * @brief Shift text in a line
 */
static void shift_text_right(TextEditorLine* line, uint16_t start_pos, uint16_t count) {
    if (start_pos + count >= TEXT_EDITOR_MAX_LINE_LENGTH) return;
    
    uint16_t length = line->length;
    uint16_t to_move = length - start_pos;
    
    if (to_move > 0) {
        memmove(&line->text[start_pos + count],
                &line->text[start_pos],
                to_move * sizeof(char));
    }
    
    line->length += count;
    line->text[line->length] = '\0';
}

/**
 * @brief Shift text in a line left (delete)
 */
static void shift_text_left(TextEditorLine* line, uint16_t start_pos, uint16_t count) {
    if (start_pos >= line->length) return;
    if (count == 0) return;
    
    uint16_t to_move = line->length - start_pos - count;
    if (to_move > 0) {
        memmove(&line->text[start_pos],
                &line->text[start_pos + count],
                to_move * sizeof(char));
    }
    
    line->length -= count;
    line->text[line->length] = '\0';
}

/**
 * @brief Insert text into a line at position
 */
static void line_insert_text(TextEditorLine* line, uint16_t pos, const char* text, uint16_t len) {
    if (len == 0) return;
    if (pos > line->length) pos = line->length;
    
    // Make sure we don't overflow
    uint16_t available = TEXT_EDITOR_MAX_LINE_LENGTH - line->length;
    if (len > available) len = available;
    
    shift_text_right(line, pos, len);
    memcpy(&line->text[pos], text, len * sizeof(char));
}

/**
 * @brief Delete text from a line
 */
static void line_delete_text(TextEditorLine* line, uint16_t pos, uint16_t len) {
    if (len == 0) return;
    if (pos >= line->length) return;
    
    if (pos + len > line->length) {
        len = line->length - pos;
    }
    
    shift_text_left(line, pos, len);
}

/**
 * @brief Save state to history
 */
static void save_to_history(TextEditor* editor) {
    if (editor->settings.read_only) return;
    
    TextEditorHistory* history = &editor->history;
    
    // Check if we can save
    if (history->top >= TEXT_EDITOR_MAX_HISTORY - 1) {
        // Shift history up
        memmove(&history->entries[0], 
                &history->entries[1],
                (TEXT_EDITOR_MAX_HISTORY - 1) * sizeof(TextEditorHistoryEntry));
        history->top = TEXT_EDITOR_MAX_HISTORY - 1;
    } else {
        history->top++;
    }
    
    history->current = history->top;
    history->redo_top = -1;  // Clear redo stack
    
    TextEditorHistoryEntry* entry = &history->entries[history->top];
    
    // Save cursor and selection
    memcpy(&entry->cursor, &editor->cursor, sizeof(TextEditorCursor));
    memcpy(&entry->selection, &editor->selection, sizeof(TextEditorSelection));
    
    // Save modification info
    entry->line = editor->cursor.line;
    entry->column = editor->cursor.column;
    entry->was_insert = false;
    entry->removed_length = 0;
    entry->removed_text[0] = '\0';
    entry->inserted_text[0] = '\0';
}

/**
 * @brief Calculate visual column from character column (accounting for tabs)
 */
static uint16_t calculate_visual_column(const TextEditor* editor, uint16_t line, uint16_t column) {
    const TextEditorLine* l = &editor->lines[line];
    uint16_t visual = 0;
    
    for (uint16_t i = 0; i < column && i < l->length; i++) {
        if (l->text[i] == '\t') {
            visual += editor->settings.tab_size;
        } else {
            visual++;
        }
    }
    
    return visual;
}

/**
 * @brief Calculate character column from visual column
 */
static uint16_t calculate_character_column(const TextEditor* editor, uint16_t line, uint16_t visual) {
    const TextEditorLine* l = &editor->lines[line];
    uint16_t character = 0;
    uint16_t current_visual = 0;
    
    while (character < l->length && current_visual < visual) {
        if (l->text[character] == '\t') {
            current_visual += editor->settings.tab_size;
        } else {
            current_visual++;
        }
        character++;
    }
    
    return character;
}

/**
 * @brief Update cursor visual position
 */
static void update_cursor_visual_position(TextEditor* editor) {
    if (editor->cursor.line >= editor->line_count) {
        editor->cursor.x_pos = 0;
        editor->cursor.y_pos = 0;
        return;
    }
    
    editor->cursor.x_pos = calculate_visual_column(editor, editor->cursor.line, editor->cursor.column) * editor->settings.font_width;
    editor->cursor.y_pos = editor->cursor.line * editor->settings.font_height;
}

/**
 * @brief Ensure cursor is visible (scroll if needed)
 */
static void ensure_cursor_visible(TextEditor* editor) {
    uint16_t visible_lines = text_editor_get_visible_height(editor);
    
    // Check if cursor is above visible area
    if (editor->cursor.line < editor->scroll.top_line) {
        editor->scroll.top_line = editor->cursor.line;
    }
    
    // Check if cursor is below visible area
    if (editor->cursor.line >= editor->scroll.top_line + visible_lines) {
        editor->scroll.top_line = editor->cursor.line - visible_lines + 1;
        if (editor->scroll.top_line < 0) editor->scroll.top_line = 0;
    }
    
    // Check horizontal scroll
    uint16_t visible_chars = text_editor_get_visible_width(editor);
    uint16_t cursor_visual = calculate_visual_column(editor, editor->cursor.line, editor->cursor.column);
    
    if (cursor_visual < editor->scroll.left_column) {
        editor->scroll.left_column = cursor_visual;
    }
    
    if (cursor_visual >= editor->scroll.left_column + visible_chars) {
        editor->scroll.left_column = cursor_visual - visible_chars + 1;
        if (editor->scroll.left_column < 0) editor->scroll.left_column = 0;
    }
    
    editor->state.needs_render = true;
}

/**
 * @brief Update selection to match cursor movement
 */
static void update_selection(TextEditor* editor, uint16_t new_line, uint16_t new_column) {
    if (!editor->selection.active) {
        // Start new selection
        editor->selection.start_line = editor->cursor.line;
        editor->selection.start_column = editor->cursor.column;
        editor->selection.end_line = new_line;
        editor->selection.end_column = new_column;
        editor->selection.active = true;
    } else {
        // Extend selection
        editor->selection.end_line = new_line;
        editor->selection.end_column = new_column;
    }
    
    // Normalize selection (start <= end)
    if (editor->selection.start_line > editor->selection.end_line ||
        (editor->selection.start_line == editor->selection.end_line && 
         editor->selection.start_column > editor->selection.end_column)) {
        uint16_t tmp_line = editor->selection.start_line;
        uint16_t tmp_col = editor->selection.start_column;
        editor->selection.start_line = editor->selection.end_line;
        editor->selection.start_column = editor->selection.end_column;
        editor->selection.end_line = tmp_line;
        editor->selection.end_column = tmp_col;
    }
    
    if (editor->callbacks.on_selection_change) {
        editor->callbacks.on_selection_change(editor);
    }
    
    editor->state.needs_render = true;
}

/**
 * @brief Clear selection
 */
static void clear_selection_internal(TextEditor* editor) {
    editor->selection.active = false;
    editor->selection.start_line = 0;
    editor->selection.start_column = 0;
    editor->selection.end_line = 0;
    editor->selection.end_column = 0;
}

/**
 * @brief Delete selected text
 */
static void delete_selected_text(TextEditor* editor) {
    if (!editor->selection.active) return;
    
    uint16_t start_line = editor->selection.start_line;
    uint16_t start_col = editor->selection.start_column;
    uint16_t end_line = editor->selection.end_line;
    uint16_t end_col = editor->selection.end_column;
    
    if (start_line > end_line || (start_line == end_line && start_col > end_col)) {
        return;
    }
    
    // Save to history
    save_to_history(editor);
    
    if (start_line == end_line) {
        // Single line selection
        TextEditorLine* line = &editor->lines[start_line];
        uint16_t len = end_col - start_col;
        line_delete_text(line, start_col, len);
        
        // Move cursor to start
        editor->cursor.line = start_line;
        editor->cursor.column = start_col;
    } else {
        // Multi-line selection
        // Delete from start to end of first line
        TextEditorLine* first_line = &editor->lines[start_line];
        uint16_t first_len = first_line->length - start_col;
        line_delete_text(first_line, start_col, first_len);
        
        // Delete middle lines
        if (end_line > start_line + 1) {
            uint16_t lines_to_delete = end_line - start_line - 1;
            for (uint16_t i = 0; i < lines_to_delete; i++) {
                text_editor_delete_line(editor, start_line + 1);
            }
        }
        
        // Delete from start of last line to end
        TextEditorLine* last_line = &editor->lines[start_line + 1];
        line_delete_text(last_line, 0, end_col);
        
        // Join the first and last lines
        text_editor_join_lines(editor);
        
        // Move cursor to start
        editor->cursor.line = start_line;
        editor->cursor.column = start_col;
    }
    
    clear_selection_internal(editor);
    editor->state.dirty = true;
}

// =============================================================================
// LIFECYCLE
// =============================================================================

TextEditor* text_editor_create(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    // For now, use static allocation (ESP8266 friendly)
    // In production, use pool allocation
    static TextEditor editors[4];
    static uint8_t editor_count = 0;
    
    if (editor_count >= 4) return NULL;
    
    TextEditor* editor = &editors[editor_count++];
    text_editor_init(editor, x, y, width, height);
    return editor;
}

void text_editor_destroy(TextEditor* editor) {
    // Just reset for now
    if (editor) {
        text_editor_reset(editor);
    }
}

void text_editor_init(TextEditor* editor, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    memset(editor, 0, sizeof(TextEditor));
    
    // Set bounds
    editor->bounds.x = x;
    editor->bounds.y = y;
    editor->bounds.width = width;
    editor->bounds.height = height;
    
    // Initialize first line
    editor->line_count = 1;
    editor->lines[0].length = 0;
    editor->lines[0].visual_length = 0;
    editor->lines[0].dirty = false;
    editor->lines[0].text[0] = '\0';
    
    // Initialize cursor
    editor->cursor.line = 0;
    editor->cursor.column = 0;
    editor->cursor.x_pos = 0;
    editor->cursor.y_pos = 0;
    editor->cursor.visible = true;
    editor->cursor.last_blink = get_timestamp();
    
    // Initialize scroll
    editor->scroll.top_line = 0;
    editor->scroll.left_column = 0;
    editor->scroll.line_offset = 0;
    
    // Initialize selection
    editor->selection.active = false;
    editor->selection.start_line = 0;
    editor->selection.start_column = 0;
    editor->selection.end_line = 0;
    editor->selection.end_column = 0;
    editor->selection.bg_color = 0x8410;  // Dark gray
    editor->selection.fg_color = 0xFFFF;  // White
    
    // Initialize settings
    editor->settings.font_width = TEXT_EDITOR_FONT_WIDTH;
    editor->settings.font_height = TEXT_EDITOR_FONT_HEIGHT;
    editor->settings.tab_size = TEXT_EDITOR_TAB_SIZE;
    editor->settings.show_line_numbers = false;
    editor->settings.word_wrap = false;
    editor->settings.read_only = false;
    editor->settings.syntax_highlight = false;
    
    // Initialize colors
    editor->colors.bg_color = COLOR_WHITE;
    editor->colors.fg_color = COLOR_BLACK;
    editor->colors.cursor_color = COLOR_BLACK;
    editor->colors.line_number_color = 0x8410;  // Dark gray
    editor->colors.selection_bg = 0x50A0;  // Selected color
    editor->colors.selection_fg = COLOR_WHITE;
    editor->colors.gutter_bg = 0xE71C;  // Light gray
    
    // Initialize history
    editor->history.current = -1;
    editor->history.top = -1;
    editor->history.redo_top = -1;
    
    // Initialize touch
    editor->touch.touch_active = false;
    editor->touch.touch_dragging = false;
    
    // Initialize clipboard
    editor->clipboard.length = 0;
    editor->clipboard.text[0] = '\0';
    
    // Initialize callbacks
    editor->callbacks.on_change = NULL;
    editor->callbacks.on_cursor_move = NULL;
    editor->callbacks.on_selection_change = NULL;
    editor->callbacks.on_key = NULL;
    
    // Initialize state
    editor->state.focused = false;
    editor->state.dirty = false;
    editor->state.needs_render = true;
    editor->state.last_input = 0;
    
    // Widget pointer is NULL (optional)
    editor->widget = NULL;
}

void text_editor_reset(TextEditor* editor) {
    text_editor_init(editor, editor->bounds.x, editor->bounds.y, editor->bounds.width, editor->bounds.height);
}

// =============================================================================
// TEXT CONTENT
// =============================================================================

void text_editor_set_text(TextEditor* editor, const char* text) {
    if (!text) return;
    
    text_editor_clear_history(editor);
    text_editor_reset(editor);
    
    uint16_t line_start = 0;
    uint16_t text_len = strlen(text);
    
    for (uint16_t i = 0; i < text_len; i++) {
        if (text[i] == '\n' || editor->line_count >= TEXT_EDITOR_MAX_LINES) {
            // End of line
            uint16_t line_len = i - line_start;
            if (line_len > 0) {
                if (editor->line_count >= TEXT_EDITOR_MAX_LINES) break;
                
                TextEditorLine* line = &editor->lines[editor->line_count];
                if (line_len >= TEXT_EDITOR_MAX_LINE_LENGTH) line_len = TEXT_EDITOR_MAX_LINE_LENGTH - 1;
                
                memcpy(line->text, &text[line_start], line_len);
                line->text[line_len] = '\0';
                line->length = line_len;
                line->visual_length = line_len;  // Will be calculated later
                line->dirty = true;
                editor->line_count++;
            }
            
            // Start new line
            if (editor->line_count >= TEXT_EDITOR_MAX_LINES) break;
            
            TextEditorLine* new_line = &editor->lines[editor->line_count];
            new_line->length = 0;
            new_line->visual_length = 0;
            new_line->text[0] = '\0';
            new_line->dirty = false;
            editor->line_count++;
            
            line_start = i + 1;
        }
    }
    
    // Handle last line
    if (line_start < text_len) {
        if (editor->line_count >= TEXT_EDITOR_MAX_LINES) {
            editor->line_count = TEXT_EDITOR_MAX_LINES;
        } else {
            uint16_t line_len = text_len - line_start;
            if (line_len >= TEXT_EDITOR_MAX_LINE_LENGTH) line_len = TEXT_EDITOR_MAX_LINE_LENGTH - 1;
            
            TextEditorLine* line = &editor->lines[editor->line_count];
            memcpy(line->text, &text[line_start], line_len);
            line->text[line_len] = '\0';
            line->length = line_len;
            line->visual_length = line_len;
            line->dirty = true;
            editor->line_count++;
        }
    }
    
    // Ensure at least one line
    if (editor->line_count == 0) {
        editor->line_count = 1;
        editor->lines[0].length = 0;
        editor->lines[0].text[0] = '\0';
    }
    
    // Reset cursor
    editor->cursor.line = 0;
    editor->cursor.column = 0;
    update_cursor_visual_position(editor);
    
    // Clear scroll
    editor->scroll.top_line = 0;
    editor->scroll.left_column = 0;
    
    editor->state.dirty = true;
    editor->state.needs_render = true;
    
    if (editor->callbacks.on_change) {
        editor->callbacks.on_change(editor);
    }
}

uint16_t text_editor_get_text(TextEditor* editor, char* buffer, uint16_t buffer_size) {
    uint16_t total = 0;
    
    for (uint16_t i = 0; i < editor->line_count; i++) {
        TextEditorLine* line = &editor->lines[i];
        
        if (total + line->length + 1 > buffer_size) break;
        
        if (i > 0) {
            buffer[total++] = '\n';
            if (total >= buffer_size) break;
        }
        
        memcpy(&buffer[total], line->text, line->length);
        total += line->length;
    }
    
    if (total < buffer_size) {
        buffer[total] = '\0';
    }
    
    return total;
}

uint32_t text_editor_get_text_length(const TextEditor* editor) {
    uint32_t total = 0;
    
    for (uint16_t i = 0; i < editor->line_count; i++) {
        total += editor->lines[i].length;
    }
    
    // Add newlines
    if (editor->line_count > 1) {
        total += editor->line_count - 1;
    }
    
    return total;
}

uint16_t text_editor_get_line_count(const TextEditor* editor) {
    return editor->line_count;
}

const char* text_editor_get_line(const TextEditor* editor, uint16_t line_index) {
    if (line_index >= editor->line_count) return NULL;
    return editor->lines[line_index].text;
}

uint16_t text_editor_get_line_length(const TextEditor* editor, uint16_t line_index) {
    if (line_index >= editor->line_count) return 0;
    return editor->lines[line_index].length;
}

void text_editor_insert_text(TextEditor* editor, const char* text, uint16_t length) {
    if (editor->settings.read_only) return;
    if (!text || length == 0) return;
    if (length == 0) length = strlen(text);
    if (length == 0) return;
    
    save_to_history(editor);
    
    // Delete selection if active
    if (editor->selection.active) {
        delete_selected_text(editor);
    }
    
    // Insert text at cursor
    TextEditorLine* line = &editor->lines[editor->cursor.line];
    line_insert_text(line, editor->cursor.column, text, length);
    
    // Move cursor
    editor->cursor.column += length;
    update_cursor_visual_position(editor);
    ensure_cursor_visible(editor);
    
    editor->state.dirty = true;
    editor->state.needs_render = true;
    
    if (editor->callbacks.on_change) {
        editor->callbacks.on_change(editor);
    }
    
    if (editor->callbacks.on_cursor_move) {
        editor->callbacks.on_cursor_move(editor);
    }
}

void text_editor_insert_char(TextEditor* editor, char c) {
    text_editor_insert_text(editor, &c, 1);
}

void text_editor_delete_text(TextEditor* editor, uint16_t length) {
    if (editor->settings.read_only) return;
    
    if (editor->selection.active) {
        delete_selected_text(editor);
        return;
    }
    
    if (length == 0) length = 1;
    
    save_to_history(editor);
    
    TextEditorLine* line = &editor->lines[editor->cursor.line];
    line_delete_text(line, editor->cursor.column, length);
    
    editor->state.dirty = true;
    editor->state.needs_render = true;
    
    if (editor->callbacks.on_change) {
        editor->callbacks.on_change(editor);
    }
}

void text_editor_delete_char(TextEditor* editor) {
    text_editor_delete_text(editor, 1);
}

void text_editor_backspace(TextEditor* editor) {
    if (editor->settings.read_only) return;
    
    if (editor->selection.active) {
        delete_selected_text(editor);
        return;
    }
    
    if (text_editor_cursor_at_document_start(editor)) return;
    
    save_to_history(editor);
    
    if (text_editor_cursor_at_line_start(editor)) {
        // Join with previous line
        if (editor->cursor.line > 0) {
            text_editor_move_cursor_up(editor, false);
            text_editor_move_cursor_end(editor, false);
            text_editor_join_lines(editor);
        }
    } else {
        // Delete character before cursor
        TextEditorLine* line = &editor->lines[editor->cursor.line];
        line_delete_text(line, editor->cursor.column - 1, 1);
        editor->cursor.column--;
        update_cursor_visual_position(editor);
    }
    
    editor->state.dirty = true;
    editor->state.needs_render = true;
    
    if (editor->callbacks.on_change) {
        editor->callbacks.on_change(editor);
    }
    
    if (editor->callbacks.on_cursor_move) {
        editor->callbacks.on_cursor_move(editor);
    }
}

// =============================================================================
// CURSOR OPERATIONS
// =============================================================================

void text_editor_set_cursor(TextEditor* editor, uint16_t line, uint16_t column) {
    line = clamp_uint16(line, 0, editor->line_count - 1);
    
    if (line < editor->line_count) {
        column = clamp_uint16(column, 0, editor->lines[line].length);
    } else {
        column = 0;
    }
    
    editor->cursor.line = line;
    editor->cursor.column = column;
    update_cursor_visual_position(editor);
    ensure_cursor_visible(editor);
    
    if (!editor->selection.active) {
        clear_selection_internal(editor);
    }
    
    if (editor->callbacks.on_cursor_move) {
        editor->callbacks.on_cursor_move(editor);
    }
    
    editor->state.needs_render = true;
}

uint16_t text_editor_get_cursor_line(const TextEditor* editor) {
    return editor->cursor.line;
}

uint16_t text_editor_get_cursor_column(const TextEditor* editor) {
    return editor->cursor.column;
}

void text_editor_move_cursor_up(TextEditor* editor, bool shift) {
    if (editor->cursor.line == 0) {
        // At top, try to move to line start
        editor->cursor.column = 0;
    } else {
        uint16_t prev_line = editor->cursor.line - 1;
        uint16_t target_column = editor->cursor.column;
        
        // Clamp column to previous line length
        if (target_column >= editor->lines[prev_line].length) {
            target_column = editor->lines[prev_line].length;
        }
        
        uint16_t old_line = editor->cursor.line;
        uint16_t old_column = editor->cursor.column;
        
        editor->cursor.line = prev_line;
        editor->cursor.column = target_column;
        
        if (shift) {
            update_selection(editor, old_line, old_column);
        } else {
            clear_selection_internal(editor);
        }
    }
    
    update_cursor_visual_position(editor);
    ensure_cursor_visible(editor);
    
    if (editor->callbacks.on_cursor_move) {
        editor->callbacks.on_cursor_move(editor);
    }
    
    editor->state.needs_render = true;
}

void text_editor_move_cursor_down(TextEditor* editor, bool shift) {
    if (editor->cursor.line >= editor->line_count - 1) {
        // At bottom, try to move to line end
        if (editor->line_count > 0) {
            editor->cursor.column = editor->lines[editor->cursor.line].length;
        }
    } else {
        uint16_t next_line = editor->cursor.line + 1;
        uint16_t target_column = editor->cursor.column;
        
        // Clamp column to next line length
        if (target_column >= editor->lines[next_line].length) {
            target_column = editor->lines[next_line].length;
        }
        
        uint16_t old_line = editor->cursor.line;
        uint16_t old_column = editor->cursor.column;
        
        editor->cursor.line = next_line;
        editor->cursor.column = target_column;
        
        if (shift) {
            update_selection(editor, old_line, old_column);
        } else {
            clear_selection_internal(editor);
        }
    }
    
    update_cursor_visual_position(editor);
    ensure_cursor_visible(editor);
    
    if (editor->callbacks.on_cursor_move) {
        editor->callbacks.on_cursor_move(editor);
    }
    
    editor->state.needs_render = true;
}

void text_editor_move_cursor_left(TextEditor* editor, bool shift) {
    if (text_editor_cursor_at_document_start(editor)) return;
    
    uint16_t old_line = editor->cursor.line;
    uint16_t old_column = editor->cursor.column;
    
    if (text_editor_cursor_at_line_start(editor)) {
        // Move to end of previous line
        if (editor->cursor.line > 0) {
            editor->cursor.line--;
            editor->cursor.column = editor->lines[editor->cursor.line].length;
        }
    } else {
        editor->cursor.column--;
    }
    
    if (shift) {
        update_selection(editor, old_line, old_column);
    } else {
        clear_selection_internal(editor);
    }
    
    update_cursor_visual_position(editor);
    ensure_cursor_visible(editor);
    
    if (editor->callbacks.on_cursor_move) {
        editor->callbacks.on_cursor_move(editor);
    }
    
    editor->state.needs_render = true;
}

void text_editor_move_cursor_right(TextEditor* editor, bool shift) {
    if (text_editor_cursor_at_document_end(editor)) return;
    
    uint16_t old_line = editor->cursor.line;
    uint16_t old_column = editor->cursor.column;
    
    if (text_editor_cursor_at_line_end(editor)) {
        // Move to start of next line
        if (editor->cursor.line < editor->line_count - 1) {
            editor->cursor.line++;
            editor->cursor.column = 0;
        }
    } else {
        editor->cursor.column++;
    }
    
    if (shift) {
        update_selection(editor, old_line, old_column);
    } else {
        clear_selection_internal(editor);
    }
    
    update_cursor_visual_position(editor);
    ensure_cursor_visible(editor);
    
    if (editor->callbacks.on_cursor_move) {
        editor->callbacks.on_cursor_move(editor);
    }
    
    editor->state.needs_render = true;
}

void text_editor_move_cursor_home(TextEditor* editor, bool shift) {
    uint16_t old_line = editor->cursor.line;
    uint16_t old_column = editor->cursor.column;
    
    editor->cursor.column = 0;
    
    if (shift) {
        update_selection(editor, old_line, old_column);
    } else {
        clear_selection_internal(editor);
    }
    
    update_cursor_visual_position(editor);
    ensure_cursor_visible(editor);
    
    if (editor->callbacks.on_cursor_move) {
        editor->callbacks.on_cursor_move(editor);
    }
    
    editor->state.needs_render = true;
}

void text_editor_move_cursor_end(TextEditor* editor, bool shift) {
    uint16_t old_line = editor->cursor.line;
    uint16_t old_column = editor->cursor.column;
    
    if (editor->cursor.line < editor->line_count) {
        editor->cursor.column = editor->lines[editor->cursor.line].length;
    }
    
    if (shift) {
        update_selection(editor, old_line, old_column);
    } else {
        clear_selection_internal(editor);
    }
    
    update_cursor_visual_position(editor);
    ensure_cursor_visible(editor);
    
    if (editor->callbacks.on_cursor_move) {
        editor->callbacks.on_cursor_move(editor);
    }
    
    editor->state.needs_render = true;
}

void text_editor_move_cursor_document_start(TextEditor* editor, bool shift) {
    uint16_t old_line = editor->cursor.line;
    uint16_t old_column = editor->cursor.column;
    
    editor->cursor.line = 0;
    editor->cursor.column = 0;
    
    if (shift) {
        update_selection(editor, old_line, old_column);
    } else {
        clear_selection_internal(editor);
    }
    
    update_cursor_visual_position(editor);
    ensure_cursor_visible(editor);
    
    if (editor->callbacks.on_cursor_move) {
        editor->callbacks.on_cursor_move(editor);
    }
    
    editor->state.needs_render = true;
}

void text_editor_move_cursor_document_end(TextEditor* editor, bool shift) {
    uint16_t old_line = editor->cursor.line;
    uint16_t old_column = editor->cursor.column;
    
    if (editor->line_count > 0) {
        editor->cursor.line = editor->line_count - 1;
        editor->cursor.column = editor->lines[editor->cursor.line].length;
    }
    
    if (shift) {
        update_selection(editor, old_line, old_column);
    } else {
        clear_selection_internal(editor);
    }
    
    update_cursor_visual_position(editor);
    ensure_cursor_visible(editor);
    
    if (editor->callbacks.on_cursor_move) {
        editor->callbacks.on_cursor_move(editor);
    }
    
    editor->state.needs_render = true;
}

void text_editor_move_cursor_to(TextEditor* editor, uint16_t x, uint16_t y) {
    // Convert pixel position to line and column
    uint16_t line = y / editor->settings.font_height + editor->scroll.top_line;
    line = clamp_uint16(line, 0, editor->line_count - 1);
    
    uint16_t pixel_x = x - editor->bounds.x;
    if (pixel_x < 0) pixel_x = 0;
    
    // Account for gutter if line numbers are shown
    uint16_t text_x = pixel_x;
    if (editor->settings.show_line_numbers) {
        text_x -= TEXT_EDITOR_GUTTER_WIDTH;
        if (text_x < 0) text_x = 0;
    }
    
    // Account for scroll
    text_x += editor->scroll.left_column * editor->settings.font_width;
    
    uint16_t column = text_x / editor->settings.font_width;
    column = clamp_uint16(column, 0, editor->lines[line].length);
    
    text_editor_set_cursor(editor, line, column);
}

// =============================================================================
// SELECTION OPERATIONS
// =============================================================================

void text_editor_set_selection(TextEditor* editor, uint16_t start_line, uint16_t start_column,
                              uint16_t end_line, uint16_t end_column) {
    // Clamp values
    start_line = clamp_uint16(start_line, 0, editor->line_count - 1);
    end_line = clamp_uint16(end_line, 0, editor->line_count - 1);
    
    if (start_line == end_line) {
        start_column = clamp_uint16(start_column, 0, editor->lines[start_line].length);
        end_column = clamp_uint16(end_column, 0, editor->lines[end_line].length);
    } else {
        start_column = clamp_uint16(start_column, 0, editor->lines[start_line].length);
        end_column = clamp_uint16(end_column, 0, editor->lines[end_line].length);
    }
    
    // Set selection
    editor->selection.start_line = start_line;
    editor->selection.start_column = start_column;
    editor->selection.end_line = end_line;
    editor->selection.end_column = end_column;
    editor->selection.active = true;
    
    // Normalize
    if (start_line > end_line || (start_line == end_line && start_column > end_column)) {
        uint16_t tmp_line = editor->selection.start_line;
        uint16_t tmp_col = editor->selection.start_column;
        editor->selection.start_line = editor->selection.end_line;
        editor->selection.start_column = editor->selection.end_column;
        editor->selection.end_line = tmp_line;
        editor->selection.end_column = tmp_col;
    }
    
    editor->state.needs_render = true;
    
    if (editor->callbacks.on_selection_change) {
        editor->callbacks.on_selection_change(editor);
    }
}

void text_editor_clear_selection(TextEditor* editor) {
    clear_selection_internal(editor);
    editor->state.needs_render = true;
    
    if (editor->callbacks.on_selection_change) {
        editor->callbacks.on_selection_change(editor);
    }
}

void text_editor_select_all(TextEditor* editor) {
    if (editor->line_count == 0) return;
    
    editor->selection.start_line = 0;
    editor->selection.start_column = 0;
    editor->selection.end_line = editor->line_count - 1;
    editor->selection.end_column = editor->lines[editor->selection.end_line].length;
    editor->selection.active = true;
    
    editor->state.needs_render = true;
    
    if (editor->callbacks.on_selection_change) {
        editor->callbacks.on_selection_change(editor);
    }
}

bool text_editor_has_selection(const TextEditor* editor) {
    return editor->selection.active;
}

uint16_t text_editor_get_selected_text(const TextEditor* editor, char* buffer, uint16_t buffer_size) {
    if (!editor->selection.active) return 0;
    
    uint16_t total = 0;
    uint16_t start_line = editor->selection.start_line;
    uint16_t start_col = editor->selection.start_column;
    uint16_t end_line = editor->selection.end_line;
    uint16_t end_col = editor->selection.end_column;
    
    for (uint16_t line = start_line; line <= end_line; line++) {
        uint16_t start = (line == start_line) ? start_col : 0;
        uint16_t end = (line == end_line) ? end_col : editor->lines[line].length;
        uint16_t len = end - start;
        
        if (total + len + 1 > buffer_size) break;
        
        if (line > start_line) {
            buffer[total++] = '\n';
            if (total >= buffer_size) break;
        }
        
        memcpy(&buffer[total], &editor->lines[line].text[start], len);
        total += len;
    }
    
    if (total < buffer_size) {
        buffer[total] = '\0';
    }
    
    return total;
}

void text_editor_get_selection_start(const TextEditor* editor, uint16_t* line, uint16_t* column) {
    if (line) *line = editor->selection.start_line;
    if (column) *column = editor->selection.start_column;
}

void text_editor_get_selection_end(const TextEditor* editor, uint16_t* line, uint16_t* column) {
    if (line) *line = editor->selection.end_line;
    if (column) *column = editor->selection.end_column;
}

// =============================================================================
// CLIPBOARD OPERATIONS
// =============================================================================

void text_editor_copy(TextEditor* editor) {
    if (!editor->selection.active) return;
    
    text_editor_get_selected_text(editor, editor->clipboard.text, sizeof(editor->clipboard.text));
    editor->clipboard.length = strlen(editor->clipboard.text);
    
    // Clear selection after copy (optional behavior)
    // clear_selection_internal(editor);
}

void text_editor_cut(TextEditor* editor) {
    if (!editor->selection.active) return;
    if (editor->settings.read_only) return;
    
    text_editor_copy(editor);
    text_editor_delete_text(editor, 0);  // Delete selection
}

void text_editor_paste(TextEditor* editor) {
    if (editor->settings.read_only) return;
    if (editor->clipboard.length == 0) return;
    
    text_editor_insert_text(editor, editor->clipboard.text, editor->clipboard.length);
}

void text_editor_set_clipboard(TextEditor* editor, const char* text) {
    if (!text) {
        editor->clipboard.length = 0;
        editor->clipboard.text[0] = '\0';
        return;
    }
    
    strncpy(editor->clipboard.text, text, sizeof(editor->clipboard.text) - 1);
    editor->clipboard.text[sizeof(editor->clipboard.text) - 1] = '\0';
    editor->clipboard.length = strlen(editor->clipboard.text);
}

const char* text_editor_get_clipboard(const TextEditor* editor) {
    return editor->clipboard.text;
}

