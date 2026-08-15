/**
 * @file text_editor.c
 * @brief TextEditor implementation - extends TextField
 * @author GUIKit for ESP8266
 * @date 2026
 */

#include "text_editor.h"
#include "renderer.h"
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

// =============================================================================
// LOCAL DEFINITIONS
// =============================================================================

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(value, min_val, max_val) MAX(min_val, MIN(value, max_val))

// =============================================================================
// CONSTRUCTORS
// =============================================================================

TextEditor* text_editor_create(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    // Allocate from pool or malloc
    TextEditor* editor = (TextEditor*)malloc(sizeof(TextEditor));
    if (!editor) {
        return NULL;
    }
    
    text_editor_init(editor, x, y, width, height);
    return editor;
}

void text_editor_init(TextEditor* editor, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    if (!editor) return;
    
    // Initialize base TextField
    textfield_init(&editor->textfield, x, y, width, height);
    editor->textfield.base.type = WIDGET_TYPE_TEXT_EDITOR;
    
    // Set TextField to multi-line mode
    textfield_set_multiline(&editor->textfield, true);
    textfield_set_word_wrap(&editor->textfield, true);
    
    // Initialize cursor
    editor->cursor.line = 0;
    editor->cursor.column = 0;
    editor->cursor.x_pos = 0;
    editor->cursor.y_pos = 0;
    editor->cursor.visible = true;
    editor->cursor.last_blink = 0;
    
    // Initialize selection
    editor->selection.start_line = 0;
    editor->selection.start_column = 0;
    editor->selection.end_line = 0;
    editor->selection.end_column = 0;
    editor->selection.active = false;
    editor->selection.bg_color = 0xA514;  // Light blue
    editor->selection.fg_color = 0x0000;  // Black
    
    // Initialize scroll
    editor->scroll.line_offset = 0;
    
    // Initialize history
    editor->history.current = -1;
    editor->history.top = -1;
    editor->history.redo_top = -1;
    
    // Initialize editor settings
    editor->editor_settings.show_line_numbers = false;
    editor->editor_settings.syntax_highlight = false;
    
    // Initialize editor colors
    editor->editor_colors.line_number_color = 0x8410;  // Gray
    editor->editor_colors.selection_bg = 0xA514;       // Light blue
    editor->editor_colors.selection_fg = 0x0000;       // Black
    editor->editor_colors.gutter_bg = 0xE71C;          // Light gray
    
    // Initialize state
    editor->state.focused = false;
    editor->state.dirty = false;
    editor->state.needs_render = true;
    editor->state.last_activity = 0;
    
    // Initialize callbacks
    editor->on_cursor_move = NULL;
    editor->on_selection_change = NULL;
    editor->on_undo_redo = NULL;
    editor->syntax_highlight_callback = NULL;
}

void text_editor_destroy(TextEditor* editor) {
    if (!editor) return;
    
    // Clean up textfield
    textfield_destroy(&editor->textfield);
    
    free(editor);
}

// =============================================================================
// EDITOR-SPECIFIC SETTINGS
// =============================================================================

void text_editor_show_line_numbers(TextEditor* editor, bool show) {
    if (!editor) return;
    editor->editor_settings.show_line_numbers = show;
    editor->state.needs_render = true;
}

void text_editor_set_syntax_highlight(TextEditor* editor, bool enabled, 
                                     Color (*callback)(TextEditor*, uint16_t, uint16_t, const char*, uint16_t)) {
    if (!editor) return;
    editor->editor_settings.syntax_highlight = enabled;
    editor->syntax_highlight_callback = callback;
    editor->state.needs_render = true;
}

void text_editor_set_line_number_color(TextEditor* editor, Color color) {
    if (!editor) return;
    editor->editor_colors.line_number_color = color;
    editor->state.needs_render = true;
}

void text_editor_set_selection_colors(TextEditor* editor, Color bg_color, Color fg_color) {
    if (!editor) return;
    editor->editor_colors.selection_bg = bg_color;
    editor->editor_colors.selection_fg = fg_color;
    editor->selection.bg_color = bg_color;
    editor->selection.fg_color = fg_color;
    editor->state.needs_render = true;
}

// =============================================================================
// CURSOR MANAGEMENT
// =============================================================================

void text_editor_set_cursor(TextEditor* editor, uint16_t line, uint16_t column) {
    if (!editor) return;
    
    line = CLAMP(line, 0, editor->textfield.line_count - 1);
    
    TextFieldLine* text_line = textfield_get_line(&editor->textfield, line);
    if (text_line) {
        column = CLAMP(column, 0, text_line->length);
    } else {
        column = 0;
    }
    
    // Save old position for selection tracking
    if (editor->selection.active) {
        // Extend selection
        editor->selection.end_line = line;
        editor->selection.end_column = column;
    }
    
    editor->cursor.line = line;
    editor->cursor.column = column;
    editor->cursor.x_pos = 0;  // Will be calculated in update_cursor_position
    editor->cursor.y_pos = 0;
    
    text_editor_update_cursor_position(editor);
    
    if (editor->on_cursor_move) {
        editor->on_cursor_move(editor);
    }
}

void text_editor_cursor_up(TextEditor* editor) {
    if (!editor) return;
    if (editor->cursor.line > 0) {
        text_editor_set_cursor(editor, editor->cursor.line - 1, editor->cursor.column);
    }
}

void text_editor_cursor_down(TextEditor* editor) {
    if (!editor) return;
    if (editor->cursor.line < editor->textfield.line_count - 1) {
        text_editor_set_cursor(editor, editor->cursor.line + 1, editor->cursor.column);
    }
}

void text_editor_cursor_left(TextEditor* editor) {
    if (!editor) return;
    
    TextFieldLine* line = textfield_get_line(&editor->textfield, editor->cursor.line);
    if (line && editor->cursor.column > 0) {
        text_editor_set_cursor(editor, editor->cursor.line, editor->cursor.column - 1);
    } else if (editor->cursor.line > 0) {
        // Move to end of previous line
        line = textfield_get_line(&editor->textfield, editor->cursor.line - 1);
        if (line) {
            text_editor_set_cursor(editor, editor->cursor.line - 1, line->length);
        }
    }
}

void text_editor_cursor_right(TextEditor* editor) {
    if (!editor) return;
    
    TextFieldLine* line = textfield_get_line(&editor->textfield, editor->cursor.line);
    if (line && editor->cursor.column < line->length) {
        text_editor_set_cursor(editor, editor->cursor.line, editor->cursor.column + 1);
    } else if (editor->cursor.line < editor->textfield.line_count - 1) {
        // Move to start of next line
        text_editor_set_cursor(editor, editor->cursor.line + 1, 0);
    }
}

void text_editor_cursor_home(TextEditor* editor) {
    if (!editor) return;
    text_editor_set_cursor(editor, editor->cursor.line, 0);
}

void text_editor_cursor_end(TextEditor* editor) {
    if (!editor) return;
    
    TextFieldLine* line = textfield_get_line(&editor->textfield, editor->cursor.line);
    if (line) {
        text_editor_set_cursor(editor, editor->cursor.line, line->length);
    }
}

void text_editor_cursor_document_start(TextEditor* editor) {
    if (!editor) return;
    text_editor_set_cursor(editor, 0, 0);
    text_editor_scroll_to_line(editor, 0);
}

void text_editor_cursor_document_end(TextEditor* editor) {
    if (!editor) return;
    uint16_t last_line = editor->textfield.line_count - 1;
    if (last_line > 0) {
        TextFieldLine* line = textfield_get_line(&editor->textfield, last_line);
        text_editor_set_cursor(editor, last_line, line ? line->length : 0);
        text_editor_scroll_to_line(editor, last_line);
    }
}

uint16_t text_editor_get_cursor_line(const TextEditor* editor) {
    return editor ? editor->cursor.line : 0;
}

uint16_t text_editor_get_cursor_column(const TextEditor* editor) {
    return editor ? editor->cursor.column : 0;
}

void text_editor_update_cursor_position(TextEditor* editor) {
    if (!editor) return;
    
    uint16_t visible_lines = text_editor_get_visible_lines(editor);
    uint16_t first_visible = text_editor_get_first_visible_line(editor);
    
    // Calculate cursor Y position
    uint16_t line_height = text_editor_get_line_height(editor);
    uint16_t relative_line = editor->cursor.line - first_visible;
    editor->cursor.y_pos = relative_line * line_height + editor->textfield.base.y - editor->scroll.line_offset;
    
    // Calculate cursor X position
    TextFieldLine* line = textfield_get_line(&editor->textfield, editor->cursor.line);
    if (line) {
        uint16_t char_width = editor->textfield.settings.font_width;
        editor->cursor.x_pos = editor->cursor.column * char_width + editor->textfield.base.x;
    }
    
    // Auto-scroll to keep cursor visible
    if (relative_line >= visible_lines) {
        text_editor_scroll_to_line(editor, editor->cursor.line - visible_lines + 1);
    } else if (relative_line < 0) {
        text_editor_scroll_to_line(editor, editor->cursor.line);
    }
}

void text_editor_blink_cursor(TextEditor* editor) {
    if (!editor) return;
    
    uint32_t now = 0;  // Would use millis() or similar
    // For now, just toggle visibility
    editor->cursor.visible = !editor->cursor.visible;
}

// =============================================================================
// SELECTION MANAGEMENT
// =============================================================================

void text_editor_set_selection(TextEditor* editor, uint16_t start_line, uint16_t start_column,
                               uint16_t end_line, uint16_t end_column) {
    if (!editor) return;
    
    start_line = CLAMP(start_line, 0, editor->textfield.line_count - 1);
    end_line = CLAMP(end_line, 0, editor->textfield.line_count - 1);
    
    TextFieldLine* start_line_ptr = textfield_get_line(&editor->textfield, start_line);
    TextFieldLine* end_line_ptr = textfield_get_line(&editor->textfield, end_line);
    
    if (start_line_ptr) {
        start_column = CLAMP(start_column, 0, start_line_ptr->length);
    }
    if (end_line_ptr) {
        end_column = CLAMP(end_column, 0, end_line_ptr->length);
    }
    
    editor->selection.start_line = start_line;
    editor->selection.start_column = start_column;
    editor->selection.end_line = end_line;
    editor->selection.end_column = end_column;
    editor->selection.active = true;
    
    if (editor->on_selection_change) {
        editor->on_selection_change(editor);
    }
    
    editor->state.needs_render = true;
}

void text_editor_clear_selection(TextEditor* editor) {
    if (!editor) return;
    
    editor->selection.active = false;
    editor->state.needs_render = true;
    
    if (editor->on_selection_change) {
        editor->on_selection_change(editor);
    }
}

void text_editor_select_all(TextEditor* editor) {
    if (!editor) return;
    
    uint16_t last_line = editor->textfield.line_count - 1;
    TextFieldLine* last_line_ptr = textfield_get_line(&editor->textfield, last_line);
    
    text_editor_set_selection(editor, 
                             0, 0,
                             last_line, last_line_ptr ? last_line_ptr->length : 0);
}

void text_editor_select_line(TextEditor* editor) {
    if (!editor) return;
    
    TextFieldLine* line = textfield_get_line(&editor->textfield, editor->cursor.line);
    if (line) {
        text_editor_set_selection(editor,
                                 editor->cursor.line, 0,
                                 editor->cursor.line, line->length);
    }
}

void text_editor_select_word(TextEditor* editor) {
    if (!editor) return;
    
    uint16_t line = editor->cursor.line;
    uint16_t col = editor->cursor.column;
    TextFieldLine* line_ptr = textfield_get_line(&editor->textfield, line);
    if (!line_ptr) return;
    
    // Find start of word
    uint16_t start_col = col;
    while (start_col > 0 && isalnum(line_ptr->text[start_col - 1])) {
        start_col--;
    }
    
    // Find end of word
    uint16_t end_col = col;
    while (end_col < line_ptr->length && isalnum(line_ptr->text[end_col])) {
        end_col++;
    }
    
    text_editor_set_selection(editor, line, start_col, line, end_col);
}

bool text_editor_has_selection(const TextEditor* editor) {
    return editor && editor->selection.active;
}

char* text_editor_get_selected_text(const TextEditor* editor) {
    if (!editor || !editor->selection.active) {
        return NULL;
    }
    
    // Calculate total length needed
    size_t total_length = 0;
    
    if (editor->selection.start_line == editor->selection.end_line) {
        // Single line selection
        total_length = editor->selection.end_column - editor->selection.start_column;
    } else {
        // Multi-line selection
        // First line
        TextFieldLine* first_line = textfield_get_line(&editor->textfield, editor->selection.start_line);
        if (first_line) {
            total_length += first_line->length - editor->selection.start_column;
        }
        
        // Middle lines (full lines)
        for (uint16_t i = editor->selection.start_line + 1; i < editor->selection.end_line; i++) {
            TextFieldLine* line = textfield_get_line(&editor->textfield, i);
            if (line) {
                total_length += line->length;
            }
        }
        
        // Last line
        TextFieldLine* last_line = textfield_get_line(&editor->textfield, editor->selection.end_line);
        if (last_line) {
            total_length += editor->selection.end_column;
        }
        
        // Add newline characters
        total_length += (editor->selection.end_line - editor->selection.start_line);
    }
    
    // Allocate buffer (+1 for null terminator)
    char* buffer = (char*)malloc(total_length + 1);
    if (!buffer) {
        return NULL;
    }
    
    // Copy selected text
    char* ptr = buffer;
    
    if (editor->selection.start_line == editor->selection.end_line) {
        TextFieldLine* line = textfield_get_line(&editor->textfield, editor->selection.start_line);
        if (line) {
            memcpy(ptr, line->text + editor->selection.start_column, 
                   editor->selection.end_column - editor->selection.start_column);
            ptr += editor->selection.end_column - editor->selection.start_column;
        }
    } else {
        // First line
        TextFieldLine* first_line = textfield_get_line(&editor->textfield, editor->selection.start_line);
        if (first_line) {
            memcpy(ptr, first_line->text + editor->selection.start_column, 
                   first_line->length - editor->selection.start_column);
            ptr += first_line->length - editor->selection.start_column;
        }
        
        // Middle lines
        for (uint16_t i = editor->selection.start_line + 1; i < editor->selection.end_line; i++) {
            *ptr++ = '\n';
            TextFieldLine* line = textfield_get_line(&editor->textfield, i);
            if (line) {
                memcpy(ptr, line->text, line->length);
                ptr += line->length;
            }
        }
        
        // Last line
        if (editor->selection.end_line > editor->selection.start_line + 1) {
            *ptr++ = '\n';
        }
        TextFieldLine* last_line = textfield_get_line(&editor->textfield, editor->selection.end_line);
        if (last_line) {
            memcpy(ptr, last_line->text, editor->selection.end_column);
            ptr += editor->selection.end_column;
        }
    }
    
    *ptr = '\0';
    return buffer;
}

// =============================================================================
// CLIPBOARD OPERATIONS
// =============================================================================

void text_editor_copy(TextEditor* editor) {
    if (!editor) return;
    
    char* selected = text_editor_get_selected_text(editor);
    if (selected) {
        clipboard_copy(selected);
        free(selected);
    }
}

void text_editor_cut(TextEditor* editor) {
    if (!editor) return;
    
    text_editor_copy(editor);
    text_editor_delete_selection(editor);
}

void text_editor_paste(TextEditor* editor) {
    if (!editor) return;
    
    const char* text = clipboard_paste();
    if (text) {
        text_editor_insert_text(editor, text);
    }
}

// =============================================================================
// TEXT EDITING
// =============================================================================

void text_editor_insert_char(TextEditor* editor, char c) {
    if (!editor) return;
    
    // Save state for undo
    text_editor_save_state(editor);
    
    // Clear selection and replace with inserted char
    if (editor->selection.active) {
        text_editor_delete_selection(editor);
    }
    
    TextFieldLine* line = textfield_get_line(&editor->textfield, editor->cursor.line);
    if (!line) return;
    
    // Check if line is full
    if (line->length >= TEXTFIELD_MAX_LINE_LENGTH - 1) {
        return;  // Line full
    }
    
    // Make space for new character
    memmove(line->text + editor->cursor.column + 1, 
            line->text + editor->cursor.column,
            line->length - editor->cursor.column);
    
    // Insert character
    line->text[editor->cursor.column] = c;
    line->length++;
    line->dirty = true;
    
    // Move cursor right
    text_editor_cursor_right(editor);
    
    editor->state.dirty = true;
}

void text_editor_insert_text(TextEditor* editor, const char* text) {
    if (!editor || !text) return;
    
    text_editor_save_state(editor);
    
    if (editor->selection.active) {
        text_editor_delete_selection(editor);
    }
    
    TextFieldLine* line = textfield_get_line(&editor->textfield, editor->cursor.line);
    if (!line) return;
    
    size_t text_len = strlen(text);
    size_t available = TEXTFIELD_MAX_LINE_LENGTH - line->length - 1;
    
    if (text_len > available) {
        text_len = available;
    }
    
    // Make space
    memmove(line->text + editor->cursor.column + text_len,
            line->text + editor->cursor.column,
            line->length - editor->cursor.column);
    
    // Insert text
    memcpy(line->text + editor->cursor.column, text, text_len);
    line->length += text_len;
    line->dirty = true;
    
    // Move cursor
    editor->cursor.column += text_len;
    text_editor_update_cursor_position(editor);
    
    editor->state.dirty = true;
}

void text_editor_delete_char(TextEditor* editor) {
    if (!editor) return;
    
    text_editor_save_state(editor);
    
    TextFieldLine* line = textfield_get_line(&editor->textfield, editor->cursor.line);
    if (!line || editor->cursor.column >= line->length) return;
    
    // Delete character at cursor
    memmove(line->text + editor->cursor.column,
            line->text + editor->cursor.column + 1,
            line->length - editor->cursor.column - 1);
    line->length--;
    line->dirty = true;
    
    editor->state.dirty = true;
}

void text_editor_backspace(TextEditor* editor) {
    if (!editor) return;
    
    text_editor_save_state(editor);
    
    if (editor->cursor.column > 0) {
        text_editor_cursor_left(editor);
        text_editor_delete_char(editor);
    } else if (editor->cursor.line > 0) {
        // Join with previous line
        text_editor_cursor_up(editor);
        text_editor_cursor_end(editor);
        
        TextFieldLine* current_line = textfield_get_line(&editor->textfield, editor->cursor.line);
        TextFieldLine* next_line = textfield_get_line(&editor->textfield, editor->cursor.line + 1);
        
        if (current_line && next_line) {
            size_t combined_len = current_line->length + next_line->length;
            if (combined_len < TEXTFIELD_MAX_LINE_LENGTH) {
                memcpy(current_line->text + current_line->length, next_line->text, next_line->length);
                current_line->length = combined_len;
                current_line->dirty = true;
                
                // Remove next line
                textfield_remove_line(&editor->textfield, editor->cursor.line + 1);
            }
        }
    }
    
    editor->state.dirty = true;
}

void text_editor_delete_selection(TextEditor* editor) {
    if (!editor || !editor->selection.active) return;
    
    text_editor_save_state(editor);
    
    uint16_t start_line = editor->selection.start_line;
    uint16_t start_col = editor->selection.start_column;
    uint16_t end_line = editor->selection.end_line;
    uint16_t end_col = editor->selection.end_column;
    
    if (start_line == end_line) {
        // Single line selection
        TextFieldLine* line = textfield_get_line(&editor->textfield, start_line);
        if (line) {
            size_t delete_len = end_col - start_col;
            memmove(line->text + start_col,
                    line->text + end_col,
                    line->length - end_col);
            line->length -= delete_len;
            line->dirty = true;
        }
    } else {
        // Multi-line selection
        TextFieldLine* first_line = textfield_get_line(&editor->textfield, start_line);
        TextFieldLine* last_line = textfield_get_line(&editor->textfield, end_line);
        
        if (first_line && last_line) {
            // Keep text after end_col in last line
            size_t keep_len = last_line->length - end_col;
            
            // Combine first line (before start_col) with last line (after end_col)
            size_t new_len = start_col + keep_len;
            if (new_len < TEXTFIELD_MAX_LINE_LENGTH) {
                memmove(first_line->text + start_col,
                        last_line->text + end_col,
                        keep_len);
                first_line->length = new_len;
                first_line->dirty = true;
            }
            
            // Remove middle lines
            for (uint16_t i = start_line + 1; i <= end_line; i++) {
                textfield_remove_line(&editor->textfield, start_line + 1);
            }
        }
    }
    
    text_editor_clear_selection(editor);
    text_editor_set_cursor(editor, start_line, start_col);
    text_editor_update_cursor_position(editor);
    
    editor->state.dirty = true;
}

void text_editor_insert_newline(TextEditor* editor) {
    if (!editor) return;
    
    text_editor_save_state(editor);
    
    TextFieldLine* current_line = textfield_get_line(&editor->textfield, editor->cursor.line);
    if (!current_line) return;
    
    // Split current line at cursor
    char after_cursor[TEXTFIELD_MAX_LINE_LENGTH];
    size_t after_len = current_line->length - editor->cursor.column;
    
    if (after_len > 0) {
        memcpy(after_cursor, current_line->text + editor->cursor.column, after_len);
    }
    
    // Truncate current line
    current_line->length = editor->cursor.column;
    current_line->dirty = true;
    
    // Insert new line with remaining text
    textfield_insert_line(&editor->textfield, editor->cursor.line + 1, after_cursor);
    
    // Move cursor to new line
    text_editor_cursor_down(editor);
    text_editor_cursor_home(editor);
    
    editor->state.dirty = true;
}

void text_editor_indent(TextEditor* editor) {
    if (!editor) return;
    
    if (editor->selection.active) {
        // Indent all selected lines
        uint16_t start = editor->selection.start_line;
        uint16_t end = editor->selection.end_line;
        
        for (uint16_t i = start; i <= end; i++) {
            TextFieldLine* line = textfield_get_line(&editor->textfield, i);
            if (line && line->length < TEXTFIELD_MAX_LINE_LENGTH - TEXT_EDITOR_TAB_SIZE) {
                memmove(line->text + TEXT_EDITOR_TAB_SIZE, line->text, line->length);
                memset(line->text, ' ', TEXT_EDITOR_TAB_SIZE);
                line->length += TEXT_EDITOR_TAB_SIZE;
                line->dirty = true;
            }
        }
    } else {
        // Indent current line
        TextFieldLine* line = textfield_get_line(&editor->textfield, editor->cursor.line);
        if (line && line->length < TEXTFIELD_MAX_LINE_LENGTH - TEXT_EDITOR_TAB_SIZE) {
            memmove(line->text + TEXT_EDITOR_TAB_SIZE, line->text, line->length);
            memset(line->text, ' ', TEXT_EDITOR_TAB_SIZE);
            line->length += TEXT_EDITOR_TAB_SIZE;
            line->dirty = true;
            editor->cursor.column += TEXT_EDITOR_TAB_SIZE;
        }
    }
    
    editor->state.dirty = true;
}

void text_editor_unindent(TextEditor* editor) {
    if (!editor) return;
    
    uint8_t tab_size = TEXT_EDITOR_TAB_SIZE;
    
    if (editor->selection.active) {
        // Unindent all selected lines
        uint16_t start = editor->selection.start_line;
        uint16_t end = editor->selection.end_line;
        
        for (uint16_t i = start; i <= end; i++) {
            TextFieldLine* line = textfield_get_line(&editor->textfield, i);
            if (line && line->length >= tab_size) {
                // Check if line starts with spaces/tabs
                bool all_whitespace = true;
                for (uint16_t j = 0; j < tab_size; j++) {
                    if (line->text[j] != ' ' && line->text[j] != '\t') {
                        all_whitespace = false;
                        break;
                    }
                }
                
                if (all_whitespace) {
                    memmove(line->text, line->text + tab_size, line->length - tab_size);
                    line->length -= tab_size;
                    line->dirty = true;
                    
                    // Adjust cursor if on this line
                    if (i == editor->cursor.line && editor->cursor.column >= tab_size) {
                        editor->cursor.column -= tab_size;
                    }
                }
            }
        }
    } else {
        // Unindent current line
        TextFieldLine* line = textfield_get_line(&editor->textfield, editor->cursor.line);
        if (line && line->length >= tab_size) {
            bool all_whitespace = true;
            for (uint16_t j = 0; j < tab_size; j++) {
                if (line->text[j] != ' ' && line->text[j] != '\t') {
                    all_whitespace = false;
                    break;
                }
            }
            
            if (all_whitespace) {
                memmove(line->text, line->text + tab_size, line->length - tab_size);
                line->length -= tab_size;
                line->dirty = true;
                
                if (editor->cursor.column >= tab_size) {
                    editor->cursor.column -= tab_size;
                } else {
                    editor->cursor.column = 0;
                }
            }
        }
    }
    
    editor->state.dirty = true;
}

// =============================================================================
// UNDO/REDO
// =============================================================================

void text_editor_save_state(TextEditor* editor) {
    if (!editor) return;
    
    // Clear redo stack when making new changes
    editor->history.redo_top = -1;
    
    // Check if we can add to history
    if (editor->history.top >= (int8_t)TEXT_EDITOR_MAX_HISTORY - 1) {
        // Shift history down (lose oldest)
        for (int i = 0; i < TEXT_EDITOR_MAX_HISTORY - 1; i++) {
            editor->history.entries[i] = editor->history.entries[i + 1];
        }
        editor->history.top--;
        editor->history.current--;
    }
    
    TextEditorHistoryEntry* entry = &editor->history.entries[++editor->history.top];
    
    // Save cursor
    entry->cursor = editor->cursor;
    
    // Save selection
    entry->selection = editor->selection;
    
    // Save modification info
    entry->line = editor->cursor.line;
    entry->column = editor->cursor.column;
    entry->was_insert = true;  // Will be updated by specific operations
    
    // Note: Text buffer is saved by TextField's own state
}

void text_editor_undo(TextEditor* editor) {
    if (!editor || editor->history.current <= -1) return;
    
    // For now, just restore cursor position
    TextEditorHistoryEntry* entry = &editor->history.entries[editor->history.current];
    editor->cursor = entry->cursor;
    editor->selection = entry->selection;
    text_editor_update_cursor_position(editor);
    
    editor->history.current--;
    
    if (editor->on_undo_redo) {
        editor->on_undo_redo(editor, true);
    }
}

void text_editor_redo(TextEditor* editor) {
    if (!editor || editor->history.current >= editor->history.top) return;
    
    editor->history.current++;
    TextEditorHistoryEntry* entry = &editor->history.entries[editor->history.current];
    editor->cursor = entry->cursor;
    editor->selection = entry->selection;
    text_editor_update_cursor_position(editor);
    
    if (editor->on_undo_redo) {
        editor->on_undo_redo(editor, false);
    }
}

bool text_editor_can_undo(const TextEditor* editor) {
    return editor && editor->history.current > -1;
}

bool text_editor_can_redo(const TextEditor* editor) {
    return editor && editor->history.current < editor->history.top;
}

void text_editor_clear_history(TextEditor* editor) {
    if (!editor) return;
    editor->history.current = -1;
    editor->history.top = -1;
    editor->history.redo_top = -1;
}

// =============================================================================
// INPUT HANDLING
// =============================================================================

void text_editor_handle_char(TextEditor* editor, char c, uint8_t modifiers) {
    if (!editor) return;
    
    editor->state.last_activity = 0;  // Would use millis()
    
    switch (c) {
        case '\n':
            text_editor_insert_newline(editor);
            break;
        case '\t':
            text_editor_indent(editor);
            break;
        case '\b':  // Backspace
            text_editor_backspace(editor);
            break;
        case 127:    // Delete
            text_editor_delete_char(editor);
            break;
        default:
            if (isprint(c)) {
                text_editor_insert_char(editor, c);
            }
            break;
    }
}

void text_editor_handle_key(TextEditor* editor, uint8_t key) {
    if (!editor) return;
    
    editor->state.last_activity = 0;
    
    switch (key) {
        case KEY_UP:
            text_editor_cursor_up(editor);
            break;
        case KEY_DOWN:
            text_editor_cursor_down(editor);
            break;
        case KEY_LEFT:
            text_editor_cursor_left(editor);
            break;
        case KEY_RIGHT:
            text_editor_cursor_right(editor);
            break;
        case KEY_HOME:
            text_editor_cursor_home(editor);
            break;
        case KEY_END:
            text_editor_cursor_end(editor);
            break;
        case KEY_PAGE_UP:
            text_editor_scroll_up(editor);
            break;
        case KEY_PAGE_DOWN:
            text_editor_scroll_down(editor);
            break;
    }
}

void text_editor_handle_touch(TextEditor* editor, uint16_t x, uint16_t y, bool pressed) {
    if (!editor) return;
    
    editor->state.last_activity = 0;
    
    if (pressed) {
        // Calculate which line was touched
        uint16_t first_visible = text_editor_get_first_visible_line(editor);
        uint16_t line_height = text_editor_get_line_height(editor);
        uint16_t relative_y = y - editor->textfield.base.y + editor->scroll.line_offset;
        uint16_t touched_line = first_visible + (relative_y / line_height);
        
        touched_line = CLAMP(touched_line, 0, editor->textfield.line_count - 1);
        
        // Calculate column
        uint16_t char_width = editor->textfield.settings.font_width;
        uint16_t relative_x = x - editor->textfield.base.x;
        uint16_t touched_column = relative_x / char_width;
        
        TextFieldLine* line = textfield_get_line(&editor->textfield, touched_line);
        if (line) {
            touched_column = CLAMP(touched_column, 0, line->length);
        }
        
        if (editor->selection.active) {
            // Extend selection
            text_editor_set_selection(editor,
                                     editor->selection.start_line, editor->selection.start_column,
                                     touched_line, touched_column);
        } else {
            // Move cursor and start new selection
            text_editor_set_cursor(editor, touched_line, touched_column);
            editor->selection.start_line = touched_line;
            editor->selection.start_column = touched_column;
            editor->selection.active = true;
        }
    }
}

// =============================================================================
// RENDERING
// =============================================================================

void text_editor_render(const TextEditor* editor) {
    if (!editor || !editor->textfield.base.visible) return;
    
    // Render gutter (line numbers) if enabled
    if (editor->editor_settings.show_line_numbers) {
        text_editor_render_gutter(editor);
    }
    
    // Render text area
    uint16_t text_x = editor->textfield.base.x;
    if (editor->editor_settings.show_line_numbers) {
        text_x += TEXT_EDITOR_GUTTER_WIDTH;
    }
    
    text_editor_render_text(editor, text_x, editor->textfield.base.y);
    
    // Render selection
    if (editor->selection.active) {
        text_editor_render_selection(editor);
    }
    
    // Render cursor
    if (editor->state.focused && editor->cursor.visible) {
        text_editor_render_cursor(editor);
    }
}

void text_editor_render_text(const TextEditor* editor, uint16_t x, uint16_t y) {
    if (!editor) return;
    
    uint16_t line_height = text_editor_get_line_height(editor);
    uint16_t first_visible = text_editor_get_first_visible_line(editor);
    uint16_t visible_lines = text_editor_get_visible_lines(editor);
    uint16_t char_width = editor->textfield.settings.font_width;
    
    // Render visible lines
    for (uint16_t i = 0; i < visible_lines; i++) {
        uint16_t line_index = first_visible + i;
        if (line_index >= editor->textfield.line_count) break;
        
        TextFieldLine* line = textfield_get_line(&editor->textfield, line_index);
        if (!line) continue;
        
        uint16_t text_y = y + i * line_height - editor->scroll.line_offset;
        
        // Render line text
        renderer_draw_text(x, text_y, line->text, editor->textfield.colors.fg_color, 
                         editor->textfield.colors.bg_color, 1);
    }
}

void text_editor_render_cursor(const TextEditor* editor) {
    if (!editor || !editor->cursor.visible) return;
    
    uint16_t char_width = editor->textfield.settings.font_width;
    uint16_t char_height = editor->textfield.settings.font_height;
    
    // Draw cursor as vertical line
    renderer_fill_rect(editor->cursor.x_pos, editor->cursor.y_pos,
                     2, char_height, editor->textfield.colors.cursor_color);
}

void text_editor_render_selection(const TextEditor* editor) {
    if (!editor || !editor->selection.active) return;
    
    uint16_t char_width = editor->textfield.settings.font_width;
    uint16_t char_height = editor->textfield.settings.font_height;
    uint16_t first_visible = text_editor_get_first_visible_line(editor);
    uint16_t visible_lines = text_editor_get_visible_lines(editor);
    
    uint16_t start_line = editor->selection.start_line;
    uint16_t end_line = editor->selection.end_line;
    
    for (uint16_t line = start_line; line <= end_line; line++) {
        if (line < first_visible) continue;
        if (line >= first_visible + visible_lines) break;
        
        TextFieldLine* line_ptr = textfield_get_line(&editor->textfield, line);
        if (!line_ptr) continue;
        
        uint16_t start_col = (line == start_line) ? editor->selection.start_column : 0;
        uint16_t end_col = (line == end_line) ? editor->selection.end_column : line_ptr->length;
        
        uint16_t x = editor->textfield.base.x + start_col * char_width;
        uint16_t y = editor->textfield.base.y + (line - first_visible) * char_height - editor->scroll.line_offset;
        uint16_t width = (end_col - start_col) * char_width;
        
        renderer_fill_rect(x, y, width, char_height, editor->selection.bg_color);
        
        // Re-render selected text
        char temp[TEXTFIELD_MAX_LINE_LENGTH];
        size_t len = end_col - start_col;
        memcpy(temp, line_ptr->text + start_col, len);
        temp[len] = '\0';
        
        renderer_draw_text(x, y, temp, editor->selection.fg_color, editor->selection.bg_color, 1);
    }
}

void text_editor_render_line_numbers(const TextEditor* editor) {
    if (!editor) return;
    
    uint16_t line_height = text_editor_get_line_height(editor);
    uint16_t first_visible = text_editor_get_first_visible_line(editor);
    uint16_t visible_lines = text_editor_get_visible_lines(editor);
    
    for (uint16_t i = 0; i < visible_lines; i++) {
        uint16_t line_number = first_visible + i + 1;  // 1-based
        uint16_t y = editor->textfield.base.y + i * line_height - editor->scroll.line_offset;
        
        char buffer[8];
        snprintf(buffer, sizeof(buffer), "%u", line_number);
        
        renderer_draw_text(editor->textfield.base.x, y, buffer, 
                         editor->editor_colors.line_number_color, 
                         editor->editor_colors.gutter_bg, 1);
    }
}

void text_editor_render_gutter(const TextEditor* editor) {
    if (!editor) return;
    
    uint16_t gutter_width = TEXT_EDITOR_GUTTER_WIDTH;
    uint16_t height = editor->textfield.base.height;
    
    // Draw gutter background
    renderer_fill_rect(editor->textfield.base.x, editor->textfield.base.y,
                     gutter_width, height, editor->editor_colors.gutter_bg);
    
    // Draw line numbers
    text_editor_render_line_numbers(editor);
}

// =============================================================================
// FOCUS MANAGEMENT
// =============================================================================

void text_editor_set_focus(TextEditor* editor, bool focused) {
    if (!editor) return;
    editor->state.focused = focused;
    editor->state.needs_render = true;
}

bool text_editor_is_focused(const TextEditor* editor) {
    return editor && editor->state.focused;
}

// =============================================================================
// STATE MANAGEMENT
// =============================================================================

void text_editor_set_dirty(TextEditor* editor, bool dirty) {
    if (!editor) return;
    editor->state.dirty = dirty;
}

bool text_editor_is_dirty(const TextEditor* editor) {
    return editor && editor->state.dirty;
}

// =============================================================================
// CALLBACKS
// =============================================================================

void text_editor_set_on_cursor_move(TextEditor* editor, void (*callback)(TextEditor*)) {
    if (!editor) return;
    editor->on_cursor_move = callback;
}

void text_editor_set_on_selection_change(TextEditor* editor, void (*callback)(TextEditor*)) {
    if (!editor) return;
    editor->on_selection_change = callback;
}

void text_editor_set_on_undo_redo(TextEditor* editor, void (*callback)(TextEditor*, bool)) {
    if (!editor) return;
    editor->on_undo_redo = callback;
}
