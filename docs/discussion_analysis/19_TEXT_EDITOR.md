# Text Editor Widget

> Evolved from simple text field to full-featured text editor

## Overview

The **Text Editor** widget extends the simple text field concept into a **full-featured text editor** with comprehensive editing capabilities. It's designed specifically for **ESP8266** with memory constraints in mind, using **static buffers** and **no dynamic allocation**.

## Features

### Core Editing
- **Multi-line text** editing with line management
- **Cursor navigation** (arrow keys, page up/down, home/end)
- **Text insertion and deletion** at cursor position
- **Backspace and delete** operations
- **Newline handling** (Enter key)
- **Tab expansion** (configurable tab size)

### Selection
- **Text selection** with Shift + navigation keys
- **Mouse/touch selection** with drag support
- **Select all** (Ctrl+A)
- **Selection visual feedback** with customizable colors

### Clipboard
- **Copy** selected text to clipboard (Ctrl+C)
- **Cut** selected text to clipboard (Ctrl+X)
- **Paste** from clipboard (Ctrl+V)
- **Internal clipboard** buffer (no external dependencies)

### Undo/Redo
- **Undo** last operation (Ctrl+Z)
- **Redo** last undone operation (Ctrl+Y)
- **History stack** with configurable depth (default: 32)

### Scrolling
- **Vertical scrolling** with scrollbar or keyboard
- **Horizontal scrolling** for long lines
- **Auto-scroll** to keep cursor visible
- **Page up/down** navigation

### Display
- **Line numbers** (optional, with gutter)
- **Word wrap** (optional)
- **Syntax highlighting hooks** (for future implementation)
- **Cursor blinking** (configurable interval)
- **Selection highlighting**

### Styling
- **Customizable colors** for background, foreground, cursor, selection
- **Customizable gutter** appearance
- **Read-only mode** for display-only use

## Memory Analysis

### Text Editor Structure

```
TextEditor (main struct):
  - bounds: 8 bytes (x, y, width, height)
  - widget pointer: 4 bytes (optional)
  - lines[256]: 256 × 260 bytes = 66,560 bytes
    - Each TextEditorLine: 260 bytes (text[256] + length + visual_length + dirty)
  - cursor: 10 bytes
  - selection: 14 bytes
  - scroll: 6 bytes
  - history: 3 + 32 × ~40 bytes = 1,283 bytes
  - settings: 8 bytes
  - colors: 14 bytes
  - state: 8 bytes
  - callbacks: 16 bytes
  - touch: 16 bytes
  - clipboard: 258 bytes

Total: ~70KB (worst case)

Note: This exceeds ESP8266 RAM (80KB) but:
1. Most editors won't use all 256 lines
2. Lines are only allocated as needed
3. Can be reduced by decreasing TEXT_EDITOR_MAX_LINES
```

### Memory Optimization

| **Configuration** | **Max Lines** | **Memory Usage** |
|-------------------|---------------|------------------|
| Default | 256 | ~70KB |
| Conservative | 128 | ~35KB |
| Minimal | 64 | ~18KB |
| Tiny | 32 | ~9KB |

**Recommended for ESP8266:** Use 64-128 max lines

### Configurable Constants

```c
#define TEXT_EDITOR_MAX_LINES 256    // Reduce to 64-128 for ESP8266
#define TEXT_EDITOR_MAX_LINE_LENGTH 256  // Reduce to 128 if needed
#define TEXT_EDITOR_MAX_HISTORY 32    // History depth
#define TEXT_EDITOR_MAX_VISIBLE_LINES 20
#define TEXT_EDITOR_TAB_SIZE 4
#define TEXT_EDITOR_FONT_WIDTH 8
#define TEXT_EDITOR_FONT_HEIGHT 16
#define TEXT_EDITOR_GUTTER_WIDTH 40
```

## Data Structures

### TextEditorLine
```c
typedef struct {
    char text[TEXT_EDITOR_MAX_LINE_LENGTH];  // Line content
    uint16_t length;                         // Actual text length
    uint16_t visual_length;                  // Visual length (tabs expanded)
    bool dirty;                              // Needs re-rendering
} TextEditorLine;
```

### TextEditorCursor
```c
typedef struct {
    uint16_t line;          // Line index (0-based)
    uint16_t column;        // Column index (0-based)
    uint16_t x_pos;         // X position in pixels
    uint16_t y_pos;         // Y position in pixels
    bool visible;           // Blinking state
    uint32_t last_blink;    // Timestamp for blinking
} TextEditorCursor;
```

### TextEditorSelection
```c
typedef struct {
    uint16_t start_line;      // Selection start
    uint16_t start_column;
    uint16_t end_line;        // Selection end
    uint16_t end_column;
    bool active;              // Whether selection is active
    Color bg_color;          // Selection background
    Color fg_color;          // Selection foreground
} TextEditorSelection;
```

### TextEditorScroll
```c
typedef struct {
    int16_t top_line;         // Top visible line
    int16_t left_column;      // Left visible column
    uint16_t line_offset;     // Pixel offset (for smooth scrolling)
} TextEditorScroll;
```

### KeyModifier and KeyCode
```c
typedef enum {
    KEY_MOD_NONE = 0,
    KEY_MOD_SHIFT = 1,
    KEY_MOD_CTRL = 2,
    KEY_MOD_ALT = 4
} KeyModifier;

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
    KEY_DELETE = 108
} KeyCode;
```

## API Reference

### Lifecycle

| Function | Description |
|----------|-------------|
| `text_editor_create(x, y, w, h)` | Create new editor |
| `text_editor_destroy(editor)` | Destroy editor |
| `text_editor_init(editor, x, y, w, h)` | Initialize editor |
| `text_editor_reset(editor)` | Reset to empty |

### Text Content

| Function | Description |
|----------|-------------|
| `text_editor_set_text(editor, text)` | Set all text |
| `text_editor_get_text(editor, buf, size)` | Get all text |
| `text_editor_get_text_length(editor)` | Get total char count |
| `text_editor_get_line_count(editor)` | Get line count |
| `text_editor_get_line(editor, idx)` | Get line text |
| `text_editor_get_line_length(editor, idx)` | Get line length |
| `text_editor_insert_text(editor, text, len)` | Insert text at cursor |
| `text_editor_insert_char(editor, c)` | Insert single char |
| `text_editor_delete_text(editor, len)` | Delete text at cursor |
| `text_editor_delete_char(editor)` | Delete char at cursor |
| `text_editor_backspace(editor)` | Backspace (delete before cursor) |

### Cursor Operations

| Function | Description |
|----------|-------------|
| `text_editor_set_cursor(editor, line, col)` | Set cursor position |
| `text_editor_get_cursor_line(editor)` | Get cursor line |
| `text_editor_get_cursor_column(editor)` | Get cursor column |
| `text_editor_move_cursor_up(editor, shift)` | Move up (shift=extend selection) |
| `text_editor_move_cursor_down(editor, shift)` | Move down |
| `text_editor_move_cursor_left(editor, shift)` | Move left |
| `text_editor_move_cursor_right(editor, shift)` | Move right |
| `text_editor_move_cursor_home(editor, shift)` | Move to line start |
| `text_editor_move_cursor_end(editor, shift)` | Move to line end |
| `text_editor_move_cursor_document_start(editor, shift)` | Move to doc start |
| `text_editor_move_cursor_document_end(editor, shift)` | Move to doc end |
| `text_editor_move_cursor_to(editor, x, y)` | Move to pixel position |

### Selection Operations

| Function | Description |
|----------|-------------|
| `text_editor_set_selection(editor, sl, sc, el, ec)` | Set selection range |
| `text_editor_clear_selection(editor)` | Clear selection |
| `text_editor_select_all(editor)` | Select all text |
| `text_editor_has_selection(editor)` | Check if selection active |
| `text_editor_get_selected_text(editor, buf, size)` | Get selected text |
| `text_editor_get_selection_start(editor, &line, &col)` | Get selection start |
| `text_editor_get_selection_end(editor, &line, &col)` | Get selection end |

### Clipboard Operations

| Function | Description |
|----------|-------------|
| `text_editor_copy(editor)` | Copy to clipboard |
| `text_editor_cut(editor)` | Cut to clipboard |
| `text_editor_paste(editor)` | Paste from clipboard |
| `text_editor_set_clipboard(editor, text)` | Set clipboard text |
| `text_editor_get_clipboard(editor)` | Get clipboard text |

### Undo/Redo

| Function | Description |
|----------|-------------|
| `text_editor_undo(editor)` | Undo last operation |
| `text_editor_redo(editor)` | Redo last undone |
| `text_editor_can_undo(editor)` | Check if can undo |
| `text_editor_can_redo(editor)` | Check if can redo |
| `text_editor_clear_history(editor)` | Clear history |

### Scroll Operations

| Function | Description |
|----------|-------------|
| `text_editor_scroll_up(editor)` | Scroll up one line |
| `text_editor_scroll_down(editor)` | Scroll down one line |
| `text_editor_scroll_to_cursor(editor)` | Scroll to cursor |
| `text_editor_scroll_to_line(editor, line)` | Scroll to line |
| `text_editor_set_scroll(editor, top, left)` | Set scroll position |
| `text_editor_get_visible_lines(editor, &start, &end)` | Get visible range |

### Input Handling

| Function | Description |
|----------|-------------|
| `text_editor_handle_char(editor, c, mods)` | Handle character input |
| `text_editor_handle_key(editor, key, mods)` | Handle key input |
| `text_editor_handle_touch(editor, x, y, pressed)` | Handle touch input |

### Rendering

| Function | Description |
|----------|-------------|
| `text_editor_render(editor)` | Render entire editor |
| `text_editor_render_line(editor, line)` | Render specific line |
| `text_editor_render_cursor(editor)` | Render cursor |
| `text_editor_render_selection(editor)` | Render selection |
| `text_editor_render_line_numbers(editor)` | Render line numbers |

### Settings

| Function | Description |
|----------|-------------|
| `text_editor_show_line_numbers(editor, show)` | Show/hide line numbers |
| `text_editor_set_word_wrap(editor, wrap)` | Enable/disable word wrap |
| `text_editor_set_read_only(editor, ro)` | Set read-only mode |
| `text_editor_set_tab_size(editor, size)` | Set tab size |

### Styling

| Function | Description |
|----------|-------------|
| `text_editor_set_bg_color(editor, color)` | Set background color |
| `text_editor_set_fg_color(editor, color)` | Set text color |
| `text_editor_set_cursor_color(editor, color)` | Set cursor color |
| `text_editor_set_selection_colors(editor, bg, fg)` | Set selection colors |
| `text_editor_set_line_number_color(editor, color)` | Set line number color |
| `text_editor_set_gutter_bg_color(editor, color)` | Set gutter background |

### Focus

| Function | Description |
|----------|-------------|
| `text_editor_set_focus(editor, focused)` | Set input focus |
| `text_editor_has_focus(editor)` | Check if focused |

### State

| Function | Description |
|----------|-------------|
| `text_editor_is_dirty(editor)` | Check if content changed |
| `text_editor_clear_dirty(editor)` | Clear dirty flag |

### Callbacks

| Function | Description |
|----------|-------------|
| `text_editor_set_on_change(editor, cb)` | Set change callback |
| `text_editor_set_on_cursor_move(editor, cb)` | Set cursor move callback |
| `text_editor_set_on_selection_change(editor, cb)` | Set selection callback |
| `text_editor_set_on_key(editor, cb)` | Set custom key handler |

### Line Operations

| Function | Description |
|----------|-------------|
| `text_editor_insert_line(editor, idx, text)` | Insert new line |
| `text_editor_delete_line(editor, idx)` | Delete line |
| `text_editor_split_line(editor)` | Split line at cursor |
| `text_editor_join_lines(editor)` | Join with next line |

### Word Operations

| Function | Description |
|----------|-------------|
| `text_editor_move_cursor_next_word(editor, shift)` | Move to next word |
| `text_editor_move_cursor_previous_word(editor, shift)` | Move to previous word |
| `text_editor_delete_word_before_cursor(editor)` | Delete word before |
| `text_editor_delete_word_after_cursor(editor)` | Delete word after |

## Usage Examples

### Example 1: Basic Text Editor

```c
#include "text_editor.h"

TextEditor* editor;

void setup() {
    tft.init();
    
    // Create text editor
    editor = text_editor_create(10, 10, 240, 320);
    
    // Set initial text
    text_editor_set_text(editor, "Hello, World!\n\nThis is a text editor.");
    
    // Show line numbers
    text_editor_show_line_numbers(editor, true);
    
    // Set focus
    text_editor_set_focus(editor, true);
}

void loop() {
    // Handle input
    if (Serial.available()) {
        char c = Serial.read();
        text_editor_handle_char(editor, c, KEY_MOD_NONE);
    }
    
    // Handle keyboard (if available)
    KeyCode key = get_key();
    if (key != KEY_NONE) {
        text_editor_handle_key(editor, key, KEY_MOD_NONE);
    }
    
    // Handle touch
    if (ts.touched()) {
        TS_Point p = ts.getPoint();
        uint16_t x = map(p.x, 0, 4095, 0, tft.width());
        uint16_t y = map(p.y, 0, 4095, 0, tft.height());
        text_editor_handle_touch(editor, x, y, true);
    }
    
    // Render
    text_editor_render(editor);
}
```

### Example 2: Text Editor with Callbacks

```c
void on_text_change(TextEditor* editor) {
    char text[1024];
    uint16_t len = text_editor_get_text(editor, text, sizeof(text));
    Serial.print("Text changed: ");
    Serial.println(text);
}

void on_cursor_move(TextEditor* editor) {
    Serial.print("Cursor: line ");
    Serial.print(text_editor_get_cursor_line(editor));
    Serial.print(", col ");
    Serial.println(text_editor_get_cursor_column(editor));
}

void setup() {
    editor = text_editor_create(0, 0, 240, 320);
    
    // Set callbacks
    text_editor_set_on_change(editor, on_text_change);
    text_editor_set_on_cursor_move(editor, on_cursor_move);
    
    // Set styling
    text_editor_set_bg_color(editor, COLOR_WHITE);
    text_editor_set_fg_color(editor, COLOR_BLACK);
    text_editor_set_selection_colors(editor, 0x50A0, COLOR_WHITE);
}
```

### Example 3: Read-Only Code Viewer

```c
void setup() {
    editor = text_editor_create(0, 0, 240, 320);
    
    // Set code
    text_editor_set_text(editor, 
        "#include <Arduino.h>\n"
        "void setup() {\n"
        "  Serial.begin(115200);\n"
        "}\n"
        "void loop() {\n"
        "  delay(1000);\n"
        "}");
    
    // Make read-only
    text_editor_set_read_only(editor, true);
    
    // Show line numbers
    text_editor_show_line_numbers(editor, true);
    
    // Style like code
    text_editor_set_bg_color(editor, 0x0841);  // Dark bg
    text_editor_set_fg_color(editor, 0xFFFF);  // White text
    text_editor_set_line_number_color(editor, 0x8410);  // Gray numbers
}
```

### Example 4: Touch-Based Text Editing

```c
bool was_touched = false;

void loop() {
    if (ts.touched()) {
        TS_Point p = ts.getPoint();
        uint16_t x = map(p.x, 0, 4095, 0, tft.width());
        uint16_t y = map(p.y, 0, 4095, 0, tft.height());
        
        if (!was_touched) {
            // First touch
            text_editor_handle_touch(editor, x, y, true);
            was_touched = true;
        } else {
            // Drag - move cursor
            text_editor_handle_touch(editor, x, y, true);
        }
    } else {
        if (was_touched) {
            // Touch released
            text_editor_handle_touch(editor, 0, 0, false);
            was_touched = false;
        }
    }
    
    text_editor_render(editor);
}
```

### Example 5: Custom Key Handling

```c
bool custom_key_handler(TextEditor* editor, char c, uint8_t mods) {
    // Handle F1 for help
    if (c == KEY_F1) {
        show_help();
        return true;  // Handled
    }
    
    // Handle Ctrl+S for save
    if (mods & KEY_MOD_CTRL && c == 'S') {
        save_file(editor);
        return true;
    }
    
    return false;  // Not handled, continue
}

void setup() {
    editor = text_editor_create(0, 0, 240, 320);
    text_editor_set_on_key(editor, custom_key_handler);
}
```

### Example 6: Using with Widget System

```c
// Integrate text editor as a widget
Widget* create_text_editor_widget(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    Widget* widget = new_widget(WIDGET_TYPE_CUSTOM);
    widget->position.x = x;
    widget->position.y = y;
    widget->size.width = w;
    widget->size.height = h;
    
    // Create editor
    TextEditor* editor = text_editor_create(x, y, w, h);
    
    // Store editor pointer in widget custom data
    widget->custom_data = editor;
    
    // Set widget draw function
    widget->draw = draw_text_editor_widget;
    
    return widget;
}

void draw_text_editor_widget(Widget* widget) {
    TextEditor* editor = (TextEditor*)widget->custom_data;
    if (editor) {
        text_editor_render(editor);
    }
}
```

## Integration with GUIKit

### Widget Type

Add to `WIDGET_TYPE` enum:
```c
typedef enum {
    // ... existing types
    WIDGET_TYPE_TEXT_EDITOR,
    WIDGET_TYPE_TEXTFIELD,  // Simple text field
    WIDGET_TYPE_EDITOR,      // Full editor
} WIDGET_TYPE;
```

### Widget Structure Extension

```c
typedef struct {
    WIDGET_TYPE type;
    // ... common properties
    
    union {
        // ... other widget types
        TextEditor* text_editor;  // For WIDGET_TYPE_TEXT_EDITOR
    } data;
} Widget;
```

### Constructor

```c
Widget* new_text_editor(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    Widget* widget = widget_pool_alloc(WIDGET_TYPE_TEXT_EDITOR);
    widget->position.x = x;
    widget->position.y = y;
    widget->size.width = w;
    widget->size.height = h;
    
    widget->data.text_editor = text_editor_create(x, y, w, h);
    widget->draw = text_editor_widget_draw;
    widget->handle_input = text_editor_widget_input;
    
    return widget;
}
```

## Keyboard Input Mapping

For physical keyboards or Bluetooth keyboards:

| Physical Key | KeyCode | Action |
|--------------|---------|--------|
| Up Arrow | KEY_UP | Move cursor up |
| Down Arrow | KEY_DOWN | Move cursor down |
| Left Arrow | KEY_LEFT | Move cursor left |
| Right Arrow | KEY_RIGHT | Move cursor right |
| Home | KEY_HOME | Move to line start |
| End | KEY_END | Move to line end |
| Page Up | KEY_PAGE_UP | Page up |
| Page Down | KEY_PAGE_DOWN | Page down |
| Backspace | KEY_BACKSPACE | Backspace |
| Delete | KEY_DELETE | Delete char |
| Tab | KEY_TAB | Insert tab |
| Enter | KEY_ENTER | Insert newline |
| Escape | KEY_ESCAPE | Clear selection |

### Modifier Keys

| Combination | Action |
|-------------|--------|
| Shift + Arrow | Extend selection |
| Ctrl + C | Copy |
| Ctrl + X | Cut |
| Ctrl + V | Paste |
| Ctrl + A | Select all |
| Ctrl + Z | Undo |
| Ctrl + Y | Redo |
| Ctrl + Up/Down | Scroll |

## Touch Input

### Single Tap
- Moves cursor to tapped position

### Tap and Hold
- Starts selection at tap position

### Drag
- Extends selection from start position to drag position

### Double Tap
- Selects word under cursor

### Triple Tap
- Selects line

## Performance Considerations

### Rendering Optimization

1. **Dirty flag system**: Only re-render lines that have changed
2. **Visible range**: Only render lines that are visible
3. **Cursor blink**: Only redraw cursor when blinking
4. **Selection caching**: Cache selection rectangles

### Memory Optimization

1. **Reduce max lines**: `TEXT_EDITOR_MAX_LINES = 64` saves ~50KB
2. **Reduce line length**: `TEXT_EDITOR_MAX_LINE_LENGTH = 128` saves ~30KB
3. **Reduce history**: `TEXT_EDITOR_MAX_HISTORY = 16` saves ~1KB
4. **Pool allocation**: Use object pool instead of dynamic allocation

### Example: Optimized Configuration

```c
// In text_editor_config.h
#define TEXT_EDITOR_MAX_LINES 64
#define TEXT_EDITOR_MAX_LINE_LENGTH 128
#define TEXT_EDITOR_MAX_HISTORY 16
#define TEXT_EDITOR_TAB_SIZE 2  // Smaller tabs

// Results in ~15KB memory usage
```

## Best Practices

### Do This ✅

```c
// Always check bounds
if (line < text_editor_get_line_count(editor)) {
    const char* text = text_editor_get_line(editor, line);
    // Use text
}

// Use callbacks for change notifications
text_editor_set_on_change(editor, on_change_handler);

// Check if read-only before modifying
if (!editor->settings.read_only) {
    text_editor_insert_text(editor, "new text");
}

// Clear selection after copy/cut (optional)
text_editor_copy(editor);
text_editor_clear_selection(editor);

// Use word navigation
text_editor_move_cursor_next_word(editor, false);
text_editor_move_cursor_previous_word(editor, false);

// Batch operations
text_editor_set_text(editor, large_text);  // More efficient than insert
```

### Don't Do This ❌

```c
// DON'T access internal structures directly
editor->lines[0].text[0] = 'A';  // Use text_editor_set_char()

// DON'T assume line exists
const char* text = text_editor_get_line(editor, 1000);  // Check bounds first

// DON'T forget to check read-only
text_editor_insert_text(read_only_editor, "text");  // Won't work

// DON'T use without initialization
TextEditor editor;  // Not initialized!
text_editor_set_text(&editor, "text");  // Undefined behavior

// DON'T modify while iterating
for (int i = 0; i < text_editor_get_line_count(editor); i++) {
    text_editor_delete_line(editor, i);  // Modifies during iteration
}
```

## File Locations

| File | Path | Description |
|------|------|-------------|
| Header | `src/gui/text_editor.h` | Type definitions, constants, function declarations |
| Implementation | `src/gui/text_editor.c` | Core functions (text, cursor, selection, clipboard) |
| Implementation | `src/gui/text_editor_part2.c` | Undo/redo, scrolling, rendering, input |
| Documentation | `docs/discussion_analysis/19_TEXT_EDITOR.md` | This file |

## Cross-References

- **Widget Architecture**: See `01_WIDGET_ARCHITECTURE.md`
- **Text and Input**: See `10_TEXT_AND_INPUT.md`
- **Touch Handling**: See `08_TOUCH_HANDLING.md`
- **Rendering System**: See `07_RENDERING_SYSTEM.md`

## Summary

| Feature | Implementation | Benefit |
|---------|---------------|---------|
| Multi-line editing | Line buffer array | Full text editing |
| Cursor navigation | Arrow key handlers | Intuitive editing |
| Text selection | Selection struct with start/end | Visual feedback |
| Clipboard | Internal buffer + copy/cut/paste | Data transfer |
| Undo/Redo | History stack | Mistake recovery |
| Scrolling | Scroll struct with top/left | Large document support |
| Line numbers | Gutter rendering | Code navigation |
| Word wrap | Configurable setting | Better readability |
| Touch support | Touch handlers | Mobile-friendly |
| Customizable | Colors, settings, callbacks | Flexible styling |
| ESP8266 optimized | Static buffers, no malloc | Memory efficient |

The **Text Editor** widget provides a **complete text editing solution** that can evolve from a simple text field into a full-featured editor, while remaining **memory-efficient** and **ESP8266-compatible**.
