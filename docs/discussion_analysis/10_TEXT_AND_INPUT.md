# Text and Input System

> Extracted from discussion_guikit.txt - Text field, keyboard, and input systems

## Overview

This document covers the complete text input system for GUIKit, including:
- Text field widget with cursor and selection
- Virtual keyboard (AZERTY/QWERTY switchable)
- Clipboard system for copy/paste operations
- Automatic text correction with dictionary
- Rendering optimizations (double buffering, dirty flags)

All implementations follow Objective-C style memory management (NOT ARC/reference counting) for ESP8266 compatibility.

---

## Text Field Widget

### Structure

```c
typedef struct {
    Widget base;                    // Base widget properties
    char* buffer;                 // Text buffer (dynamic)
    uint16_t buffer_size;         // Buffer capacity
    uint16_t cursor_pos;          // Cursor position
    TextSelection selection;      // Text selection state
    bool has_focus;               // Focus state
    TEXTFIELD_STYLE style;        // Field style (NORMAL, PASSWORD)
    char password_char;           // Password mask character (default: '*')
    CorrectionSuggestion* suggestions;  // Correction suggestions
    uint8_t suggestion_count;    // Number of suggestions
    void (*on_change)(const char*);    // Text change callback
    void (*on_enter)(const char*);      // Enter/return callback
    void (*on_suggestion_select)(uint8_t index); // Suggestion selection callback
} WidgetTextField;
```

### Text Field Style Enum

```c
typedef enum {
    TEXTFIELD_STYLE_NORMAL,       // Standard text field
    TEXTFIELD_STYLE_PASSWORD      // Password field with masking
} TEXTFIELD_STYLE;
```

### Text Selection Structure

```c
typedef struct {
    bool active;          // Selection active flag
    uint16_t start;      // Selection start position
    uint16_t end;        // Selection end position
    Color bg_color;      // Selection background color
} TextSelection;
```

### Correction Suggestion Structure

```c
typedef struct {
    char* word;           // Suggested word
    uint16_t start;      // Start position of word to correct
    uint16_t end;        // End position of word to correct
} CorrectionSuggestion;
```

---

### Constructor

```c
WidgetTextField* new_textfield(uint16_t buffer_size, TEXTFIELD_STYLE style) {
    WidgetTextField* textfield = (WidgetTextField*)malloc(sizeof(WidgetTextField));
    if (!textfield) return NULL;

    textfield->base = *new_widget(WIDGET_TYPE_TEXTFIELD);
    
    // Allocate text buffer
    textfield->buffer = (char*)malloc(buffer_size + 1);
    if (!textfield->buffer) {
        free(textfield);
        return NULL;
    }
    textfield->buffer[0] = '\0';
    textfield->buffer_size = buffer_size;
    
    // Initialize cursor and selection
    textfield->cursor_pos = 0;
    textfield->selection.active = false;
    textfield->selection.start = 0;
    textfield->selection.end = 0;
    textfield->selection.bg_color = 0x5AEB;  // Light blue selection
    
    // Initialize state
    textfield->has_focus = false;
    textfield->style = style;
    textfield->password_char = '*';
    textfield->suggestions = NULL;
    textfield->suggestion_count = 0;
    textfield->on_change = NULL;
    textfield->on_enter = NULL;
    textfield->on_suggestion_select = NULL;

    // Default style
    textfield->base.style.draw_style = 
        WIDGET_DRAW_STYLE_SOLID_BORDER | 
        WIDGET_DRAW_STYLE_SOLID_FILL;
    textfield->base.style.colors.primary = 0xFFFF;   // White background
    textfield->base.style.colors.secondary = 0x0000; // Black text
    textfield->base.style.border.color = 0x8410;     // Gray border
    textfield->base.style.border.width = 1;
    textfield->base.style.border.radius = 3;
    textfield->base.rect.size.height = 30;

    return textfield;
}
```

### Memory Management

```c
// Release text field and its buffer
#define RELEASE_TEXTFIELD(tf) \
    if(tf) { \
        if((tf)->buffer) free((tf)->buffer); \
        if((tf)->suggestions) free((tf)->suggestions); \
        free(tf); \
        tf = NULL; \
    }

// Free suggestions array only
void textfield_free_suggestions(WidgetTextField* textfield) {
    if (!textfield) return;
    
    for (uint8_t i = 0; i < textfield->suggestion_count; i++) {
        if (textfield->suggestions[i].word) {
            free(textfield->suggestions[i].word);
        }
    }
    free(textfield->suggestions);
    textfield->suggestions = NULL;
    textfield->suggestion_count = 0;
}
```

---

## Text Field Operations

### Character Manipulation

```c
// Append a character at cursor position
void textfield_append_char(WidgetTextField* textfield, char c) {
    if (!textfield || !textfield->buffer) return;

    // If text is selected, delete it first
    if (textfield->selection.active) {
        textfield_backspace(textfield);
    }

    if (textfield->cursor_pos < textfield->buffer_size) {
        // Shift characters to make space
        for (uint16_t i = textfield->buffer_size; i > textfield->cursor_pos; i--) {
            textfield->buffer[i] = textfield->buffer[i - 1];
        }
        textfield->buffer[textfield->cursor_pos] = c;
        textfield->cursor_pos++;
        textfield->buffer[textfield->cursor_pos] = '\0';

        if (textfield->on_change) {
            textfield->on_change(textfield->buffer);
        }
    }
}

// Delete character at cursor
void textfield_delete_char(WidgetTextField* textfield) {
    if (!textfield || !textfield->buffer || 
        textfield->cursor_pos >= textfield->buffer_size) return;

    if (textfield->selection.active) {
        textfield_backspace(textfield);
        return;
    }

    // Shift characters to fill gap
    for (uint16_t i = textfield->cursor_pos; i < textfield->buffer_size; i++) {
        textfield->buffer[i] = textfield->buffer[i + 1];
    }
    textfield->buffer[textfield->buffer_size] = '\0';

    if (textfield->on_change) {
        textfield->on_change(textfield->buffer);
    }
}

// Backspace (delete before cursor)
void textfield_backspace(WidgetTextField* textfield) {
    if (!textfield || !textfield->buffer || textfield->cursor_pos == 0) return;

    if (textfield->selection.active) {
        // Delete selected text
        uint16_t start = textfield->selection.start;
        uint16_t end = textfield->selection.end;
        if (start > end) {
            uint16_t tmp = start;
            start = end;
            end = tmp;
        }
        for (uint16_t i = start; i + (end - start) < textfield->buffer_size; i++) {
            textfield->buffer[i] = textfield->buffer[i + (end - start)];
        }
        textfield->cursor_pos = start;
        textfield->selection.active = false;
    } else {
        // Delete single character before cursor
        textfield->cursor_pos--;
        for (uint16_t i = textfield->cursor_pos; i < textfield->buffer_size; i++) {
            textfield->buffer[i] = textfield->buffer[i + 1];
        }
    }
    textfield->buffer[textfield->buffer_size] = '\0';

    if (textfield->on_change) {
        textfield->on_change(textfield->buffer);
    }
}

// Set text programmatically
void textfield_set_text(WidgetTextField* textfield, const char* text) {
    if (!textfield || !textfield->buffer || !text) return;

    strncpy(textfield->buffer, text, textfield->buffer_size);
    textfield->buffer[textfield->buffer_size] = '\0';
    textfield->cursor_pos = strlen(textfield->buffer);
    textfield->selection.active = false;

    if (textfield->on_change) {
        textfield->on_change(textfield->buffer);
    }
}

// Get display text (handles password masking)
const char* textfield_get_display_text(WidgetTextField* textfield) {
    if (!textfield || !textfield->buffer) return "";

    if (textfield->style == TEXTFIELD_STYLE_PASSWORD) {
        static char masked_buffer[256];
        uint16_t len = strlen(textfield->buffer);
        for (uint16_t i = 0; i < len; i++) {
            masked_buffer[i] = textfield->password_char;
        }
        masked_buffer[len] = '\0';
        return masked_buffer;
    } else {
        return textfield->buffer;
    }
}
```

---

### Cursor and Selection Operations

```c
// Set cursor position
void textfield_set_cursor_pos(WidgetTextField* textfield, uint16_t pos) {
    if (!textfield) return;
    if (pos > strlen(textfield->buffer)) 
        pos = strlen(textfield->buffer);
    textfield->cursor_pos = pos;
}

// Select all text
void textfield_select_all(WidgetTextField* textfield) {
    if (!textfield) return;
    textfield->selection.active = true;
    textfield->selection.start = 0;
    textfield->selection.end = strlen(textfield->buffer);
}

// Deselect text
void textfield_deselect(WidgetTextField* textfield) {
    if (!textfield) return;
    textfield->selection.active = false;
    textfield->selection.start = 0;
    textfield->selection.end = 0;
}
```

---

## Clipboard Operations

### Copy, Cut, Paste

```c
// Copy selected text to clipboard
void textfield_copy(WidgetTextField* textfield) {
    if (!textfield || !textfield->selection.active) return;

    uint16_t start = textfield->selection.start;
    uint16_t end = textfield->selection.end;
    if (start > end) {
        uint16_t tmp = start;
        start = end;
        end = tmp;
    }
    uint16_t len = end - start;
    
    char* copied_text = (char*)malloc(len + 1);
    strncpy(copied_text, textfield->buffer + start, len);
    copied_text[len] = '\0';

    // Store in clipboard (implemented in clipboard.h)
    clipboard_set(copied_text);
    free(copied_text);
}

// Cut selected text to clipboard
void textfield_cut(WidgetTextField* textfield) {
    if (!textfield || !textfield->selection.active) return;

    textfield_copy(textfield);
    textfield_backspace(textfield);
}

// Paste text from clipboard
void textfield_paste(WidgetTextField* textfield, const char* text) {
    if (!textfield || !text) return;

    uint16_t len = strlen(text);
    if (textfield->cursor_pos + len > textfield->buffer_size) {
        len = textfield->buffer_size - textfield->cursor_pos;
    }

    if (textfield->selection.active) {
        textfield_backspace(textfield);
    }

    for (uint16_t i = 0; i < len; i++) {
        textfield_append_char(textfield, text[i]);
    }
}
```

---

## Automatic Text Correction

### Dictionary and Suggestions

```c
// Common words dictionary (extend as needed)
const char* dictionary[] = {
    "bonjour", "au revoir", "merci", "s'il vous plait", "oui", "non",
    "esp8266", "arduino", "tft", "ecran", "clavier", "texte", "champ",
    "bouton", "interface", "graphique", "utilisateur", "saisie",
    "hello", "world", "the", "and", "for", "this", "that",
    NULL
};

// Check if word exists in dictionary
bool is_word_in_dictionary(const char* word) {
    for (uint16_t i = 0; dictionary[i] != NULL; i++) {
        if (strcmp(dictionary[i], word) == 0) {
            return true;
        }
    }
    return false;
}

// Extract word at cursor position
void extract_word_at_cursor(WidgetTextField* textfield, 
                            char* word, 
                            uint16_t* start, 
                            uint16_t* end) {
    if (!textfield || !word) return;

    uint16_t pos = textfield->cursor_pos;
    uint16_t len = strlen(textfield->buffer);

    // Find word start
    while (pos > 0 && isalnum(textfield->buffer[pos - 1])) {
        pos--;
    }
    *start = pos;

    // Find word end
    while (pos < len && isalnum(textfield->buffer[pos])) {
        pos++;
    }
    *end = pos;

    // Copy word
    uint16_t word_len = *end - *start;
    strncpy(word, textfield->buffer + *start, word_len);
    word[word_len] = '\0';
}

// Generate correction suggestions
void textfield_handle_correction(WidgetTextField* textfield) {
    if (!textfield) return;

    // Free old suggestions
    textfield_free_suggestions(textfield);

    // Extract word at cursor
    char current_word[64];
    uint16_t start, end;
    extract_word_at_cursor(textfield, current_word, &start, &end);

    if (strlen(current_word) == 0) return;

    // Check if word is in dictionary
    if (is_word_in_dictionary(current_word)) return;

    // Generate suggestions based on edit distance
    // (Implementation uses Levenshtein distance or simple matching)
    
    // For now, add simple suggestions
    for (uint16_t i = 0; dictionary[i] != NULL; i++) {
        // Simple heuristic: similar length
        if (abs((int)strlen(dictionary[i]) - (int)strlen(current_word)) <= 2) {
            // Could add Levenshtein distance check here
            textfield->suggestions = (CorrectionSuggestion*)realloc(
                textfield->suggestions,
                (textfield->suggestion_count + 1) * sizeof(CorrectionSuggestion)
            );
            textfield->suggestions[textfield->suggestion_count].word = 
                strdup(dictionary[i]);
            textfield->suggestions[textfield->suggestion_count].start = start;
            textfield->suggestions[textfield->suggestion_count].end = end;
            textfield->suggestion_count++;
        }
    }
}
```

---

## Virtual Keyboard

### Keyboard Structure

```c
typedef enum {
    KEYBOARD_LAYOUT_QWERTY,
    KEYBOARD_LAYOUT_AZERTY,
    KEYBOARD_LAYOUT_NUMERIC,
    KEYBOARD_LAYOUT_SYMBOLIC
} KEYBOARD_LAYOUT;

typedef enum {
    KEYBOARD_MODE_LOWER,
    KEYBOARD_MODE_UPPER,
    KEYBOARD_MODE_SPECIAL
} KEYBOARD_MODE;

typedef struct {
    Widget base;
    KEYBOARD_LAYOUT layout;        // Current layout
    KEYBOARD_MODE mode;            // Current mode (lower/upper/special)
    bool visible;                  // Visibility state
    WidgetButton** keys;          // Array of key buttons
    uint8_t keys_count;           // Number of keys
    WidgetTextField* target;      // Target text field (NULL = general input)
    void (*on_key_press)(char);   // Key press callback
} WidgetKeyboard;
```

### Keyboard Layouts

```c
// QWERTY layout definition
const char* keyboard_qwerty_lower[] = {
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
    "a", "s", "d", "f", "g", "h", "j", "k", "l",
    "z", "x", "c", "v", "b", "n", "m",
    "SP", "DEL"
};

const char* keyboard_qwerty_upper[] = {
    "!", "@", "#", "$", "%", "^", "&", "*", "(", ")",
    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
    "A", "S", "D", "F", "G", "H", "J", "K", "L",
    "Z", "X", "C", "V", "B", "N", "M",
    "SP", "DEL"
};

// AZERTY layout definition
const char* keyboard_azerty_lower[] = {
    "a", "z", "e", "r", "t", "y", "u", "i", "o", "p",
    "q", "s", "d", "f", "g", "h", "j", "k", "l", "m",
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
    "SP", "DEL"
};

const char* keyboard_azerty_upper[] = {
    "A", "Z", "E", "R", "T", "Y", "U", "I", "O", "P",
    "Q", "S", "D", "F", "G", "H", "J", "K", "L", "M",
    "!", "@", "#", "$", "%", "^", "&", "*", "(", ")",
    "SP", "DEL"
};
```

### Keyboard Constructor

```c
WidgetKeyboard* new_keyboard(KEYBOARD_LAYOUT layout, WidgetTextField* target) {
    WidgetKeyboard* keyboard = (WidgetKeyboard*)malloc(sizeof(WidgetKeyboard));
    if (!keyboard) return NULL;

    keyboard->base = *new_widget(WIDGET_TYPE_KEYBOARD);
    keyboard->layout = layout;
    keyboard->mode = KEYBOARD_MODE_LOWER;
    keyboard->visible = false;
    keyboard->target = target;
    keyboard->on_key_press = NULL;
    keyboard->keys = NULL;
    keyboard->keys_count = 0;

    // Create keys based on layout
    keyboard_create_keys(keyboard);

    // Default style
    keyboard->base.style.draw_style = WIDGET_DRAW_STYLE_SOLID_FILL;
    keyboard->base.style.colors.primary = 0xC618;  // Dark gray
    keyboard->base.style.border.color = 0x8410;
    keyboard->base.rect.size.width = 320;
    keyboard->base.rect.size.height = 120;
    keyboard->base.rect.position.y = 120;  // Bottom of screen

    return keyboard;
}

// Create keyboard keys
void keyboard_create_keys(WidgetKeyboard* keyboard) {
    if (!keyboard) return;

    const char** current_layout = NULL;
    uint8_t rows = 0;
    uint8_t cols = 0;

    // Select layout
    switch (keyboard->layout) {
        case KEYBOARD_LAYOUT_QWERTY:
            current_layout = (keyboard->mode == KEYBOARD_MODE_LOWER) ? 
                keyboard_qwerty_lower : keyboard_qwerty_upper;
            rows = 4;
            cols = 10;
            break;
        case KEYBOARD_LAYOUT_AZERTY:
            current_layout = (keyboard->mode == KEYBOARD_MODE_LOWER) ? 
                keyboard_azerty_lower : keyboard_azerty_upper;
            rows = 4;
            cols = 10;
            break;
        // ... other layouts
    }

    // Calculate key size
    uint8_t key_width = keyboard->base.rect.size.width / cols;
    uint8_t key_height = keyboard->base.rect.size.height / rows;

    // Create keys
    for (uint8_t row = 0; row < rows; row++) {
        for (uint8_t col = 0; col < cols; col++) {
            uint8_t index = row * cols + col;
            const char* label = current_layout[index];
            
            WidgetButton* key = new_button();
            strcpy(key->base.text.text, label);
            key->base.rect.size.width = key_width;
            key->base.rect.size.height = key_height;
            key->base.rect.position.x = col * key_width;
            key->base.rect.position.y = row * key_height;
            key->on_click = keyboard_key_callback;
            key->data = keyboard;  // Store keyboard reference
            
            keyboard->keys = (WidgetButton**)realloc(
                keyboard->keys,
                (keyboard->keys_count + 1) * sizeof(WidgetButton*)
            );
            keyboard->keys[keyboard->keys_count++] = key;
            widget_add_child(&keyboard->base, &key->base);
        }
    }
}

// Toggle keyboard visibility
void keyboard_toggle(WidgetKeyboard* keyboard) {
    if (!keyboard) return;
    keyboard->visible = !keyboard->visible;
    
    if (keyboard->visible) {
        // Show keyboard
        keyboard->base.rect.position.y = 120;  // Slide up
    } else {
        // Hide keyboard
        keyboard->base.rect.position.y = 240;  // Slide down (off-screen)
    }
}

// Switch between AZERTY and QWERTY
void keyboard_switch_layout(WidgetKeyboard* keyboard) {
    if (!keyboard) return;
    
    KEYBOARD_LAYOUT new_layout = 
        (keyboard->layout == KEYBOARD_LAYOUT_QWERTY) ? 
        KEYBOARD_LAYOUT_AZERTY : KEYBOARD_LAYOUT_QWERTY;
    
    // Free old keys
    for (uint8_t i = 0; i < keyboard->keys_count; i++) {
        free_widget(&keyboard->keys[i]->base);
    }
    free(keyboard->keys);
    keyboard->keys = NULL;
    keyboard->keys_count = 0;
    
    // Create new keys with new layout
    keyboard->layout = new_layout;
    keyboard_create_keys(keyboard);
}
```

---

## Memory Management for Text System

### Macros for Memory Management

```c
// Release keyboard
#define RELEASE_KEYBOARD(kb) \
    if(kb) { \
        if((kb)->keys) { \
            for(uint8_t i = 0; i < (kb)->keys_count; i++) { \
                RELEASE_BUTTON((kb)->keys[i]); \
            } \
            free((kb)->keys); \
        } \
        free(kb); \
        kb = NULL; \
    }

// Release clipboard content
#define RELEASE_CLIPBOARD() \
    if(clipboard_buffer) { free(clipboard_buffer); clipboard_buffer = NULL; }
```

### Object Pooling for Text Input

```c
// Text field pool for ESP8266
#define MAX_TEXTFIELD_POOL 10
#define TEXTFIELD_BUFFER_SIZE 256

WidgetTextField textfield_pool[MAX_TEXTFIELD_POOL];
bool textfield_pool_used[MAX_TEXTFIELD_POOL] = {false};
char textfield_buffers[MAX_TEXTFIELD_POOL][TEXTFIELD_BUFFER_SIZE + 1];

// Allocate text field from pool
WidgetTextField* textfield_pool_alloc(TEXTFIELD_STYLE style) {
    for (int i = 0; i < MAX_TEXTFIELD_POOL; i++) {
        if (!textfield_pool_used[i]) {
            textfield_pool_used[i] = true;
            WidgetTextField* tf = &textfield_pool[i];
            memset(tf, 0, sizeof(WidgetTextField));
            
            tf->base = *new_widget(WIDGET_TYPE_TEXTFIELD);
            tf->buffer = textfield_buffers[i];
            tf->buffer[0] = '\0';
            tf->buffer_size = TEXTFIELD_BUFFER_SIZE;
            tf->style = style;
            tf->cursor_pos = 0;
            tf->selection.active = false;
            
            return tf;
        }
    }
    return NULL;
}

// Release text field to pool
void textfield_pool_release(WidgetTextField* textfield) {
    if (textfield >= textfield_pool && textfield < textfield_pool + MAX_TEXTFIELD_POOL) {
        int index = (textfield - textfield_pool);
        textfield_pool_used[index] = false;
        // Clear buffer
        textfield->buffer[0] = '\0';
    }
}
```

---

## Rendering Optimizations

### Double Buffering

To prevent flickering during text input:

```c
// Double buffer for text rendering
uint16_t* text_render_buffer = NULL;

// Initialize double buffer
void text_init_double_buffer(uint16_t width, uint16_t height) {
    if (text_render_buffer) free(text_render_buffer);
    text_render_buffer = (uint16_t*)malloc(width * height * sizeof(uint16_t));
}

// Render text field to buffer
void textfield_render_to_buffer(WidgetTextField* textfield) {
    if (!textfield || !text_render_buffer) return;
    
    // Draw to buffer first
    uint16_t old_color = tft.textcolor;
    uint16_t old_bg = tft.textbgcolor;
    
    // Draw background
    tft.fillRect(
        textfield->base.rect.position.x,
        textfield->base.rect.position.y,
        textfield->base.rect.size.width,
        textfield->base.rect.size.height,
        textfield->base.style.colors.primary
    );
    
    // Draw border
    tft.drawRect(
        textfield->base.rect.position.x,
        textfield->base.rect.position.y,
        textfield->base.rect.size.width,
        textfield->base.rect.size.height,
        textfield->base.style.border.color
    );
    
    // Draw text (handling password masking)
    const char* display_text = textfield_get_display_text(textfield);
    tft.setCursor(
        textfield->base.rect.position.x + 5,
        textfield->base.rect.position.y + 10
    );
    tft.print(display_text);
    
    // Draw cursor (blinking)
    if (textfield->has_focus && (millis() / 500) % 2 == 0) {
        uint16_t cursor_x = textfield->base.rect.position.x + 5 + 
                          tft.textWidth(display_text, textfield->cursor_pos);
        tft.drawLine(
            cursor_x,
            textfield->base.rect.position.y + 5,
            cursor_x,
            textfield->base.rect.position.y + textfield->base.rect.size.height - 5,
            0x0000  // Black cursor
        );
    }
    
    // Draw selection
    if (textfield->selection.active) {
        uint16_t start_x = textfield->base.rect.position.x + 5 +
                         tft.textWidth(display_text, textfield->selection.start);
        uint16_t end_x = textfield->base.rect.position.x + 5 +
                       tft.textWidth(display_text, textfield->selection.end);
        
        tft.fillRect(
            start_x,
            textfield->base.rect.position.y + 5,
            end_x - start_x,
            textfield->base.rect.size.height - 10,
            textfield->selection.bg_color
        );
    }
}

// Flush buffer to screen
void text_flush_buffer(void) {
    if (!text_render_buffer) return;
    
    // Copy buffer to TFT
    // (Implementation depends on TFT_eSPI capabilities)
}
```

### Dirty Flag System

```c
// Dirty flag for text fields
typedef struct {
    bool dirty;                   // Needs redraw
    Rect dirty_rect;             // Region to redraw
} WidgetDirtyState;

// Mark text field as dirty
void textfield_mark_dirty(WidgetTextField* textfield) {
    if (!textfield) return;
    textfield->dirty = true;
    textfield->dirty_rect = textfield->base.rect;
}

// Check if needs redraw
bool textfield_needs_redraw(WidgetTextField* textfield) {
    if (!textfield) return false;
    return textfield->dirty;
}

// Clear dirty flag
void textfield_clear_dirty(WidgetTextField* textfield) {
    if (!textfield) return;
    textfield->dirty = false;
}
```

---

## Usage Example

### Complete Text Input System

```c
// Create text field
WidgetTextField* username_field = new_textfield(256, TEXTFIELD_STYLE_NORMAL);
username_field->base.rect.position.x = 50;
username_field->base.rect.position.y = 50;
username_field->base.rect.size.width = 200;
strcpy(username_field->base.text.text, "Username");

// Create password field
WidgetTextField* password_field = new_textfield(256, TEXTFIELD_STYLE_PASSWORD);
password_field->base.rect.position.x = 50;
password_field->base.rect.position.y = 100;
password_field->base.rect.size.width = 200;
strcpy(password_field->base.text.text, "Password");

// Create keyboard
WidgetKeyboard* keyboard = new_keyboard(KEYBOARD_LAYOUT_AZERTY, username_field);

// Focus management
void focus_textfield(WidgetTextField* textfield) {
    if (currently_focused) {
        currently_focused->has_focus = false;
        textfield_mark_dirty(currently_focused);
    }
    currently_focused = textfield;
    textfield->has_focus = true;
    textfield_mark_dirty(textfield);
    
    // Show keyboard
    keyboard->target = textfield;
    keyboard_toggle(keyboard);
}

// Key press handler
void handle_key_press(char c) {
    if (currently_focused) {
        switch (c) {
            case 'SP':  // Space
                textfield_append_char(currently_focused, ' ');
                break;
            case 'DEL':  // Delete
                textfield_backspace(currently_focused);
                break;
            case 'ENT':  // Enter
                if (currently_focused->on_enter) {
                    currently_focused->on_enter(currently_focused->buffer);
                }
                break;
            default:
                textfield_append_char(currently_focused, c);
                break;
        }
    }
}

// Touch handler for text fields
void handle_textfield_touch(WidgetTextField* textfield, uint16_t x, uint16_t y) {
    if (!textfield) return;
    
    // Check if touch is inside text field
    if (x >= textfield->base.rect.position.x &&
        x < textfield->base.rect.position.x + textfield->base.rect.size.width &&
        y >= textfield->base.rect.position.y &&
        y < textfield->base.rect.position.y + textfield->base.rect.size.height) {
        
        focus_textfield(textfield);
        
        // Calculate cursor position from x
        const char* display_text = textfield_get_display_text(textfield);
        uint16_t text_x = textfield->base.rect.position.x + 5;
        uint16_t char_width = tft.textWidth(display_text, 1);
        
        if (char_width > 0) {
            uint16_t pos = (x - text_x) / char_width;
            if (pos > strlen(display_text)) pos = strlen(display_text);
            textfield_set_cursor_pos(textfield, pos);
        }
    }
}
```

---

## ESP8266-Specific Optimizations

### Memory Usage

- **Text buffer size**: Use fixed-size buffers (256-512 chars) to avoid dynamic allocation
- **Keyboard keys**: Pre-allocate key objects, don't create/destroy dynamically
- **Clipboard**: Single static buffer for clipboard content
- **Suggestions**: Limit to 5-10 suggestions maximum

### Performance Tips

1. **Batch rendering**: Redraw all text fields at once, not individually
2. **Use dirty flags**: Only redraw text fields that have changed
3. **Precompute text widths**: Cache character positions for cursor placement
4. **Limit keyboard animations**: Simple slide-in/out, no complex animations

### No Dynamic Allocation in Touch Handlers

```c
// BAD - allocates memory in touch handler
void bad_touch_handler(void) {
    char* temp = malloc(256);  // Avoid!
    // ...
    free(temp);
}

// GOOD - uses static buffer
void good_touch_handler(void) {
    static char temp_buffer[256];  // Pre-allocated
    // ...
}
```

---

## Cross-References

- **Widget Implementations**: See `09_WIDGET_IMPLEMENTATIONS.md`
- **Rendering System**: See `07_RENDERING_SYSTEM.md`
- **Touch Handling**: See `08_TOUCH_HANDLING.md`
- **Clipboard System**: See `14_ADVANCED_FEATURES.md` (Clipboard section)
- **Memory Management**: See `06_MEMORY_MANAGEMENT.md` and `docs/MEMORY_MANAGEMENT.md`

---

## File Location

Source implementations:
- `src/gui/textfield.h` - Text field definitions
- `src/gui/textfield.cpp` - Text field implementation
- `src/gui/keyboard.h` - Virtual keyboard declarations
- `src/gui/keyboard.cpp` - Virtual keyboard implementation
- `src/gui/corrector.h` - Text corrector declarations
- `src/gui/corrector.cpp` - Text corrector implementation
- `src/gui/clipboard.h` - Clipboard system
- `src/gui/clipboard.cpp` - Clipboard implementation
