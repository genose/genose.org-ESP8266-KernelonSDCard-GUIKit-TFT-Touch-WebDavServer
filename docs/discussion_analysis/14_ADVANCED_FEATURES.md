# Advanced Features

> Extracted from discussion_guikit.txt - Clipboard, gestures, history, undo/redo, and command pattern

## Overview

This document covers all advanced features of GUIKit that go beyond basic widget functionality:
- Clipboard system for copy/paste operations
- Gesture recognition (tap, double-tap, long-press, swipe)
- History system with undo/redo functionality
- Command pattern for actions
- Dirty flag system for optimized rendering

All implementations follow Objective-C style memory management (NOT ARC/reference counting) for ESP8266 compatibility.

**IMPORTANT**: As per project constraints for ESP8266 (80KB RAM limit, no atomic ops):
- **DO NOT** use ARC (Automatic Reference Counting)
- **DO NOT** use reference counting
- **USE** manual memory management with accessors/macros
- **USE** object pooling where appropriate

---

## Clipboard System

### Overview

The clipboard system provides copy/cut/paste functionality across text fields and other widgets. It uses a static buffer to avoid dynamic memory allocation.

### Implementation

#### Header File (clipboard.h)

```c
#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// ========== DEFINITIONS ==========
#define CLIPBOARD_BUFFER_SIZE 64  // Maximum clipboard content size

// ========== FUNCTIONS ==========
// Initialize clipboard
void init_clipboard(void);

// Set clipboard content
void clipboard_set(const char* text);

// Get clipboard content
const char* clipboard_get(void);

// Check if clipboard has content
bool clipboard_has_content(void);

// Clear clipboard
void clipboard_clear(void);

// Copy text to clipboard
void clipboard_copy(const char* text);

// Cut text to clipboard (copy + delete source)
void clipboard_cut(const char* text);

// Paste from clipboard
const char* clipboard_paste(void);

#endif // CLIPBOARD_H
```

#### Implementation (clipboard.cpp)

```c
#include "clipboard.h"

// ========== GLOBAL VARIABLES ==========
static char clipboard_buffer[CLIPBOARD_BUFFER_SIZE + 1];
static bool clipboard_has_data = false;

// ========== FUNCTIONS ==========
void init_clipboard(void) {
    clipboard_buffer[0] = '\0';
    clipboard_has_data = false;
}

void clipboard_set(const char* text) {
    if (!text) {
        clipboard_clear();
        return;
    }
    
    strncpy(clipboard_buffer, text, CLIPBOARD_BUFFER_SIZE);
    clipboard_buffer[CLIPBOARD_BUFFER_SIZE] = '\0';
    clipboard_has_data = (clipboard_buffer[0] != '\0');
}

const char* clipboard_get(void) {
    return clipboard_has_data ? clipboard_buffer : NULL;
}

bool clipboard_has_content(void) {
    return clipboard_has_data;
}

void clipboard_clear(void) {
    clipboard_buffer[0] = '\0';
    clipboard_has_data = false;
}

void clipboard_copy(const char* text) {
    clipboard_set(text);
}

// Note: clipboard_cut requires the caller to delete the source text
void clipboard_cut(const char* text) {
    clipboard_set(text);
}

const char* clipboard_paste(void) {
    return clipboard_get();
}
```

### Integration with TextField

```c
// In textfield.cpp
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
    if (!copied_text) return;
    
    strncpy(copied_text, textfield->buffer + start, len);
    copied_text[len] = '\0';

    clipboard_set(copied_text);
    free(copied_text);
}

void textfield_cut(WidgetTextField* textfield) {
    if (!textfield || !textfield->selection.active) return;

    // Copy to clipboard
    textfield_copy(textfield);
    
    // Delete selected text
    textfield_backspace(textfield);
}

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

### Usage Example

```c
// Copy selected text to clipboard
if (textfield->selection.active) {
    textfield_copy(textfield);
    textfield_deselect(textfield);
}

// Paste from clipboard
const char* clipboard_text = clipboard_get();
if (clipboard_text) {
    textfield_paste(current_textfield, clipboard_text);
}

// Check if clipboard has content
if (clipboard_has_content()) {
    // Enable paste button
    paste_button->base.style.colors.primary = 0x07E0;  // Green
} else {
    // Disable paste button
    paste_button->base.style.colors.primary = 0x8410;  // Gray
}
```

---

## Gesture Recognition

### Overview

Since XPT2046 touchscreen controller doesn't support multi-touch natively, gestures are simulated using single-point tracking and timing.

### Implementation

#### Header File (gestures.h)

```c
#ifndef GESTURES_H
#define GESTURES_H

#include <stdint.h>
#include <stdbool.h>
#include "touch.h"

// ========== GESTURE TYPES ==========
typedef enum {
    GESTURE_NONE,           // No gesture detected
    GESTURE_TAP,            // Single tap
    GESTURE_DOUBLE_TAP,     // Double tap
    GESTURE_LONG_PRESS,     // Long press (hold > 1 second)
    GESTURE_SWIPE_LEFT,    // Swipe left
    GESTURE_SWIPE_RIGHT,   // Swipe right
    GESTURE_SWIPE_UP,       // Swipe up
    GESTURE_SWIPE_DOWN,     // Swipe down
    GESTURE_PINCH_IN,       // Pinch in (simulated)
    GESTURE_PINCH_OUT       // Pinch out (simulated)
} GestureType;

// ========== GESTURE CALLBACK ==========
typedef void (*GestureCallback)(GestureType gesture, uint16_t x, uint16_t y);

// ========== TOUCH STATE ==========
typedef struct {
    uint16_t x;              // Current X position
    uint16_t y;              // Current Y position
    uint16_t start_x;        // Touch start X
    uint16_t start_y;        // Touch start Y
    uint32_t start_time;     // Touch start time (millis)
    uint32_t last_tap_time;  // Last tap time (for double tap)
    bool pressed;           // Is currently pressed
    bool was_pressed;       // Was pressed in previous frame
    uint16_t tap_count;     // Consecutive tap count
} TouchGestureState;

// ========== FUNCTIONS ==========
// Initialize gesture recognition
void init_gestures(void);

// Process touch input for gestures
GestureType process_gesture(uint16_t x, uint16_t y, bool pressed);

// Register gesture callback
void set_gesture_callback(GestureCallback callback);

// Check if a gesture is currently active
bool is_gesture_active(void);

// Get current gesture state
TouchGestureState* get_gesture_state(void);

// Handle touch event for gestures
void handle_gesture_touch(uint16_t x, uint16_t y, bool pressed);

#endif // GESTURES_H
```

#### Implementation (gestures.cpp)

```c
#include "gestures.h"
#include <Arduino.h>

// ========== GLOBAL VARIABLES ==========
static TouchGestureState gesture_state = {0};
static GestureCallback gesture_callback = NULL;

// ========== THRESHOLDS ==========
#define TAP_MAX_DURATION 300       // ms
#define DOUBLE_TAP_MAX_DELAY 300  // ms between taps
#define LONG_PRESS_DURATION 1000  // ms
#define SWIPE_MIN_DISTANCE 50      // pixels
#define SWIPE_MAX_DEVIATION 20     // pixels (max perpendicular movement)

// ========== FUNCTIONS ==========
void init_gestures(void) {
    memset(&gesture_state, 0, sizeof(TouchGestureState));
    gesture_callback = NULL;
}

void set_gesture_callback(GestureCallback callback) {
    gesture_callback = callback;
}

bool is_gesture_active(void) {
    return gesture_state.pressed;
}

TouchGestureState* get_gesture_state(void) {
    return &gesture_state;
}

GestureType process_gesture(uint16_t x, uint16_t y, bool pressed) {
    static bool was_pressed = false;
    GestureType gesture = GESTURE_NONE;
    
    if (pressed && !was_pressed) {
        // Touch started
        gesture_state.start_x = x;
        gesture_state.start_y = y;
        gesture_state.start_time = millis();
        gesture_state.x = x;
        gesture_state.y = y;
        gesture_state.pressed = true;
        gesture_state.tap_count++;
        
    } else if (!pressed && was_pressed) {
        // Touch ended
        uint32_t duration = millis() - gesture_state.start_time;
        int16_t dx = x - gesture_state.start_x;
        int16_t dy = y - gesture_state.start_y;
        
        // Check for long press
        if (duration >= LONG_PRESS_DURATION) {
            gesture = GESTURE_LONG_PRESS;
        }
        // Check for double tap
        else if (duration < TAP_MAX_DURATION && 
                 abs(dx) < 5 && abs(dy) < 5 &&
                 (millis() - gesture_state.last_tap_time) < DOUBLE_TAP_MAX_DELAY) {
            gesture = GESTURE_DOUBLE_TAP;
            gesture_state.tap_count = 0;
        }
        // Check for tap
        else if (duration < TAP_MAX_DURATION && abs(dx) < 5 && abs(dy) < 5) {
            gesture = GESTURE_TAP;
            gesture_state.last_tap_time = millis();
        }
        // Check for swipes
        else if (abs(dx) > SWIPE_MIN_DISTANCE && abs(dy) < SWIPE_MAX_DEVIATION) {
            if (dx > 0) gesture = GESTURE_SWIPE_RIGHT;
            else gesture = GESTURE_SWIPE_LEFT;
        }
        else if (abs(dy) > SWIPE_MIN_DISTANCE && abs(dx) < SWIPE_MAX_DEVIATION) {
            if (dy > 0) gesture = GESTURE_SWIPE_DOWN;
            else gesture = GESTURE_SWIPE_UP;
        }
        
        gesture_state.pressed = false;
    }
    
    if (pressed) {
        // Update current position
        gesture_state.x = x;
        gesture_state.y = y;
    }
    
    was_pressed = pressed;
    gesture_state.was_pressed = was_pressed;
    
    // Call callback if gesture detected
    if (gesture != GESTURE_NONE && gesture_callback) {
        gesture_callback(gesture, x, y);
    }
    
    return gesture;
}

void handle_gesture_touch(uint16_t x, uint16_t y, bool pressed) {
    GestureType gesture = process_gesture(x, y, pressed);
    
    // Additional gesture-specific handling can go here
    switch (gesture) {
        case GESTURE_TAP:
            break;
        case GESTURE_DOUBLE_TAP:
            // Select all text in focused text field
            if (currently_focused_textfield) {
                textfield_select_all(currently_focused_textfield);
            }
            break;
        case GESTURE_LONG_PRESS:
            // Copy selected text to clipboard
            if (currently_focused_textfield) {
                textfield_copy(currently_focused_textfield);
            }
            break;
        case GESTURE_SWIPE_LEFT:
        case GESTURE_SWIPE_RIGHT:
            // Switch between UIs
            if (gesture == GESTURE_SWIPE_LEFT) {
                show_next_ui();
            } else {
                show_prev_ui();
            }
            break;
        default:
            break;
    }
}
```

### Gesture-Based Actions

```c
// Gesture callbacks for different contexts
void handle_textfield_gestures(GestureType gesture, uint16_t x, uint16_t y) {
    switch (gesture) {
        case GESTURE_TAP:
            // Position cursor
            if (currently_focused_textfield) {
                // Calculate cursor position from x
                textfield_set_cursor_from_x(currently_focused_textfield, x);
            }
            break;
            
        case GESTURE_DOUBLE_TAP:
            // Select all text
            if (currently_focused_textfield) {
                textfield_select_all(currently_focused_textfield);
            }
            break;
            
        case GESTURE_LONG_PRESS:
            // Copy selected text
            if (currently_focused_textfield) {
                textfield_copy(currently_focused_textfield);
            }
            break;
    }
}

void handle_widget_gestures(GestureType gesture, uint16_t x, uint16_t y) {
    switch (gesture) {
        case GESTURE_SWIPE_LEFT:
            // Scroll view left
            if (current_view) {
                view_scroll(current_view, -50, 0);
            }
            break;
            
        case GESTURE_SWIPE_RIGHT:
            // Scroll view right
            if (current_view) {
                view_scroll(current_view, 50, 0);
            }
            break;
    }
}
```

---

## History System (Undo/Redo)

### Overview

The history system provides undo/redo functionality for text fields and other editable widgets. It uses a static stack to store previous states without dynamic memory allocation.

### Implementation

#### Header File (history.h)

```c
#ifndef HISTORY_H
#define HISTORY_H

#include <stdint.h>
#include <stdbool.h>
#include "textfield.h"

// ========== DEFINITIONS ==========
#define MAX_HISTORY_STATES 10      // Maximum history states
#define MAX_STATE_SIZE 32         // Maximum text length per state

// ========== HISTORY STATE ==========
typedef struct {
    char text[MAX_STATE_SIZE + 1];  // Text content
    uint16_t cursor_pos;            // Cursor position
    uint16_t selection_start;      // Selection start
    uint16_t selection_end;        // Selection end
} HistoryState;

// ========== HISTORY SYSTEM ==========
typedef struct {
    HistoryState states[MAX_HISTORY_STATES];
    int8_t current_index;          // Current state index (-1 = no history)
    int8_t top_index;             // Top of history stack
    WidgetTextField* textfield;    // Associated text field
} History;

// ========== FUNCTIONS ==========
// Initialize history for a text field
void init_history(WidgetTextField* textfield);

// Save current state to history
void history_save_state(WidgetTextField* textfield);

// Undo (go back one state)
void history_undo(WidgetTextField* textfield);

// Redo (go forward one state)
void history_redo(WidgetTextField* textfield);

// Clear history
void history_clear(WidgetTextField* textfield);

// Check if undo is available
bool history_can_undo(WidgetTextField* textfield);

// Check if redo is available
bool history_can_redo(WidgetTextField* textfield);

// Set history callback
void history_set_callback(WidgetTextField* textfield, void (*on_change)(void));

#endif // HISTORY_H
```

#### Implementation (history.cpp)

```c
#include "history.h"
#include <string.h>

// ========== GLOBAL VARIABLES ==========
static History history_pool[MAX_HISTORY_STATES];
static bool history_initialized = false;

// ========== FUNCTIONS ==========
void init_history(WidgetTextField* textfield) {
    if (!textfield) return;
    
    // Find or create history for this textfield
    for (uint8_t i = 0; i < MAX_HISTORY_STATES; i++) {
        if (history_pool[i].textfield == NULL) {
            history_pool[i].textfield = textfield;
            history_pool[i].current_index = -1;
            history_pool[i].top_index = -1;
            history_initialized = true;
            return;
        }
    }
}

void history_save_state(WidgetTextField* textfield) {
    if (!textfield || !history_initialized) return;
    
    // Find history for this textfield
    History* history = NULL;
    for (uint8_t i = 0; i < MAX_HISTORY_STATES; i++) {
        if (history_pool[i].textfield == textfield) {
            history = &history_pool[i];
            break;
        }
    }
    if (!history) return;
    
    // If we're not at the top, discard future states
    if (history->current_index < history->top_index) {
        history->top_index = history->current_index;
    }
    
    // Move forward and save state
    if (history->top_index < MAX_HISTORY_STATES - 1) {
        history->current_index++;
        history->top_index = history->current_index;
        
        HistoryState* state = &history->states[history->current_index];
        strncpy(state->text, textfield->buffer, MAX_STATE_SIZE);
        state->text[MAX_STATE_SIZE] = '\0';
        state->cursor_pos = textfield->cursor_pos;
        state->selection_start = textfield->selection.start;
        state->selection_end = textfield->selection.end;
    }
}

void history_undo(WidgetTextField* textfield) {
    if (!textfield || !history_initialized) return;
    
    History* history = NULL;
    for (uint8_t i = 0; i < MAX_HISTORY_STATES; i++) {
        if (history_pool[i].textfield == textfield) {
            history = &history_pool[i];
            break;
        }
    }
    if (!history || !history_can_undo(textfield)) return;
    
    // Go back one state
    history->current_index--;
    HistoryState* state = &history->states[history->current_index];
    
    // Restore state
    strncpy(textfield->buffer, state->text, textfield->buffer_size);
    textfield->buffer[textfield->buffer_size] = '\0';
    textfield->cursor_pos = state->cursor_pos;
    textfield->selection.start = state->selection_start;
    textfield->selection.end = state->selection_end;
    
    // Update dirty flag
    widget_mark_dirty(&textfield->base);
}

void history_redo(WidgetTextField* textfield) {
    if (!textfield || !history_initialized) return;
    
    History* history = NULL;
    for (uint8_t i = 0; i < MAX_HISTORY_STATES; i++) {
        if (history_pool[i].textfield == textfield) {
            history = &history_pool[i];
            break;
        }
    }
    if (!history || !history_can_redo(textfield)) return;
    
    // Go forward one state
    history->current_index++;
    HistoryState* state = &history->states[history->current_index];
    
    // Restore state
    strncpy(textfield->buffer, state->text, textfield->buffer_size);
    textfield->buffer[textfield->buffer_size] = '\0';
    textfield->cursor_pos = state->cursor_pos;
    textfield->selection.start = state->selection_start;
    textfield->selection.end = state->selection_end;
    
    // Update dirty flag
    widget_mark_dirty(&textfield->base);
}

void history_clear(WidgetTextField* textfield) {
    if (!textfield) return;
    
    for (uint8_t i = 0; i < MAX_HISTORY_STATES; i++) {
        if (history_pool[i].textfield == textfield) {
            history_pool[i].current_index = -1;
            history_pool[i].top_index = -1;
            return;
        }
    }
}

bool history_can_undo(WidgetTextField* textfield) {
    if (!textfield) return false;
    
    for (uint8_t i = 0; i < MAX_HISTORY_STATES; i++) {
        if (history_pool[i].textfield == textfield) {
            return (history_pool[i].current_index > 0);
        }
    }
    return false;
}

bool history_can_redo(WidgetTextField* textfield) {
    if (!textfield) return false;
    
    for (uint8_t i = 0; i < MAX_HISTORY_STATES; i++) {
        if (history_pool[i].textfield == textfield) {
            return (history_pool[i].current_index < history_pool[i].top_index);
        }
    }
    return false;
}

// Integration with text field
void textfield_undo(WidgetTextField* textfield) {
    if (history_can_undo(textfield)) {
        history_undo(textfield);
    }
}

void textfield_redo(WidgetTextField* textfield) {
    if (history_can_redo(textfield)) {
        history_redo(textfield);
    }
}
```

### Usage Example

```c
void setup_textfield_with_history(WidgetTextField* textfield) {
    // Initialize history
    init_history(textfield);
    
    // Save initial state
    history_save_state(textfield);
    
    // Set up undo/redo buttons
    WidgetButton* undo_btn = new_button();
    strcpy(undo_btn->base.text.text, "Undo");
    undo_btn->on_click = []() {
        if (history_can_undo(textfield)) {
            history_undo(textfield);
            draw_widget(&textfield->base);
        }
    };
    
    WidgetButton* redo_btn = new_button();
    strcpy(redo_btn->base.text.text, "Redo");
    redo_btn->on_click = []() {
        if (history_can_redo(textfield)) {
            history_redo(textfield);
            draw_widget(&textfield->base);
        }
    };
    
    // Update button states
    undo_btn->base.enabled = history_can_undo(textfield);
    redo_btn->base.enabled = history_can_redo(textfield);
}

// When text changes, save state
void on_textfield_change(const char* text) {
    history_save_state(currently_focused_textfield);
    
    // Update undo/redo button states
    update_history_buttons();
}
```

---

## Command Pattern

### Overview

The command pattern provides a way to encapsulate actions as objects, enabling undo/redo, queuing, and logging of actions.

### Implementation

#### Header File (command.h)

```c
#ifndef COMMAND_H
#define COMMAND_H

#include <stdint.h>
#include <stdbool.h>

// ========== COMMAND TYPES ==========
typedef enum {
    COMMAND_TYPE_SET_TEXT,
    COMMAND_TYPE_INSERT_TEXT,
    COMMAND_TYPE_DELETE_TEXT,
    COMMAND_TYPE_SET_VALUE,
    COMMAND_TYPE_STYLE_CHANGE,
    COMMAND_TYPE_WIDGET_ADD,
    COMMAND_TYPE_WIDGET_REMOVE
} CommandType;

// ========== COMMAND STRUCTURE ==========
typedef struct Command Command;

// Command function type
typedef void (*CommandExecuteFunc)(Command* cmd);
typedef void (*CommandUndoFunc)(Command* cmd);
typedef void (*CommandFreeFunc)(Command* cmd);

struct Command {
    CommandType type;            // Command type
    CommandExecuteFunc execute;  // Execute function
    CommandUndoFunc undo;        // Undo function
    CommandFreeFunc free;        // Free function
    void* data;                  // Command-specific data
    Command* next;               // Next command in queue
    Command* prev;               // Previous command in queue
};

// ========== COMMAND QUEUE ==========
typedef struct {
    Command* head;               // First command
    Command* tail;               // Last command
    uint8_t count;              // Number of commands
    uint8_t max_count;          // Maximum commands
} CommandQueue;

// ========== FUNCTIONS ==========
// Initialize command queue
void init_command_queue(CommandQueue* queue, uint8_t max_commands);

// Create command
Command* command_create(CommandType type, void* data,
                       CommandExecuteFunc execute,
                       CommandUndoFunc undo,
                       CommandFreeFunc free);

// Execute command
bool command_execute(Command* cmd);

// Undo command
bool command_undo(Command* cmd);

// Free command
void command_free(Command* cmd);

// Queue command
bool command_queue(CommandQueue* queue, Command* cmd);

// Execute and queue command
bool command_execute_and_queue(CommandQueue* queue, Command* cmd);

// Undo last command
bool command_undo_last(CommandQueue* queue);

// Redo last undone command
bool command_redo_last(CommandQueue* queue);

// Clear command queue
void command_queue_clear(CommandQueue* queue);

#endif // COMMAND_H
```

#### Implementation (command.cpp)

```c
#include "command.h"
#include <stdlib.h>

// ========== COMMAND IMPLEMENTATIONS ==========

// Set text command
typedef struct {
    WidgetTextField* textfield;
    char* old_text;
    char* new_text;
} SetTextCommandData;

void command_set_text_execute(Command* cmd) {
    SetTextCommandData* data = (SetTextCommandData*)cmd->data;
    if (data->textfield && data->new_text) {
        data->old_text = strdup(data->textfield->buffer);
        textfield_set_text(data->textfield, data->new_text);
    }
}

void command_set_text_undo(Command* cmd) {
    SetTextCommandData* data = (SetTextCommandData*)cmd->data;
    if (data->textfield && data->old_text) {
        textfield_set_text(data->textfield, data->old_text);
    }
}

void command_set_text_free(Command* cmd) {
    SetTextCommandData* data = (SetTextCommandData*)cmd->data;
    if (data->old_text) free(data->old_text);
    free(data);
    free(cmd);
}

Command* command_create_set_text(WidgetTextField* textfield, const char* new_text) {
    SetTextCommandData* data = (SetTextCommandData*)malloc(sizeof(SetTextCommandData));
    data->textfield = textfield;
    data->old_text = NULL;
    data->new_text = strdup(new_text);
    
    return command_create(COMMAND_TYPE_SET_TEXT, data,
                          command_set_text_execute,
                          command_set_text_undo,
                          command_set_text_free);
}

// ========== COMMAND QUEUE ==========

void init_command_queue(CommandQueue* queue, uint8_t max_commands) {
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
    queue->max_count = max_commands;
}

Command* command_create(CommandType type, void* data,
                       CommandExecuteFunc execute,
                       CommandUndoFunc undo,
                       CommandFreeFunc free) {
    Command* cmd = (Command*)malloc(sizeof(Command));
    if (!cmd) return NULL;
    
    cmd->type = type;
    cmd->data = data;
    cmd->execute = execute;
    cmd->undo = undo;
    cmd->free = free;
    cmd->next = NULL;
    cmd->prev = NULL;
    
    return cmd;
}

bool command_execute(Command* cmd) {
    if (!cmd || !cmd->execute) return false;
    cmd->execute(cmd);
    return true;
}

bool command_undo(Command* cmd) {
    if (!cmd || !cmd->undo) return false;
    cmd->undo(cmd);
    return true;
}

void command_free(Command* cmd) {
    if (!cmd) return;
    if (cmd->free) {
        cmd->free(cmd);
    } else {
        free(cmd->data);
        free(cmd);
    }
}

bool command_queue(CommandQueue* queue, Command* cmd) {
    if (!queue || !cmd) return false;
    
    // Remove oldest command if queue is full
    if (queue->count >= queue->max_count && queue->max_count > 0) {
        Command* old = queue->head;
        if (old) {
            if (old->prev) old->prev->next = NULL;
            queue->head = old->next;
            command_free(old);
            queue->count--;
        }
    }
    
    // Add to tail
    cmd->prev = queue->tail;
    cmd->next = NULL;
    
    if (queue->tail) {
        queue->tail->next = cmd;
    } else {
        queue->head = cmd;
    }
    queue->tail = cmd;
    queue->count++;
    
    return true;
}

bool command_execute_and_queue(CommandQueue* queue, Command* cmd) {
    if (!command_execute(cmd)) return false;
    if (!command_queue(queue, cmd)) {
        command_undo(cmd);
        return false;
    }
    return true;
}

bool command_undo_last(CommandQueue* queue) {
    if (!queue || !queue->tail) return false;
    return command_undo(queue->tail);
}

bool command_redo_last(CommandQueue* queue) {
    if (!queue || !queue->tail) return false;
    // Redo is undo of undo, so we need to track redo stack separately
    // For simplicity, we'll just re-execute the command
    return command_execute(queue->tail);
}

void command_queue_clear(CommandQueue* queue) {
    if (!queue) return;
    
    Command* cmd = queue->head;
    while (cmd) {
        Command* next = cmd->next;
        command_free(cmd);
        cmd = next;
    }
    
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
}
```

### Usage with TextField

```c
// Command queue for text field actions
CommandQueue textfield_queue;

void setup_textfield_commands(void) {
    init_command_queue(&textfield_queue, 20);
}

// Execute a text change with undo support
void textfield_set_text_with_command(WidgetTextField* textfield, const char* text) {
    Command* cmd = command_create_set_text(textfield, text);
    if (cmd) {
        command_execute_and_queue(&textfield_queue, cmd);
    }
}

// Undo last text change
void textfield_undo_with_command(void) {
    command_undo_last(&textfield_queue);
}
```

---

## Dirty Flag System

### Overview

The dirty flag system optimizes rendering by only redrawing widgets that have changed since the last render.

### Implementation

#### Dirty State Structure

```c
typedef struct {
    bool dirty;              // Needs redraw
    Rect dirty_rect;        // Region to redraw (for partial updates)
    bool children_dirty;    // Children need redraw
} DirtyState;

// Extended widget structure with dirty state
typedef struct Widget {
    // ... existing fields ...
    DirtyState dirty_state;  // Dirty state
} Widget;
```

#### Dirty Flag Functions

```c
// Mark widget as dirty
void widget_mark_dirty(Widget* widget) {
    if (!widget) return;
    widget->dirty_state.dirty = true;
    widget->dirty_state.dirty_rect = widget->rect;
    
    // Mark all parents as having dirty children
    Widget* parent = widget->parent;
    while (parent) {
        parent->dirty_state.children_dirty = true;
        parent = parent->parent;
    }
}

// Clear dirty flag
void widget_clear_dirty(Widget* widget) {
    if (!widget) return;
    widget->dirty_state.dirty = false;
    widget->dirty_state.children_dirty = false;
    
    // Clear children
    for (uint8_t i = 0; i < widget->children_count; i++) {
        widget_clear_dirty(widget->children[i]);
    }
}

// Check if widget needs redraw
bool widget_needs_redraw(Widget* widget) {
    if (!widget) return false;
    if (widget->dirty_state.dirty) return true;
    if (widget->dirty_state.children_dirty) return true;
    
    // Check children
    for (uint8_t i = 0; i < widget->children_count; i++) {
        if (widget_needs_redraw(widget->children[i])) {
            return true;
        }
    }
    return false;
}
```

#### Optimized Rendering

```c
// Draw only dirty widgets
void draw_widget_tree_optimized(Widget* root) {
    if (!root) return;
    
    if (!widget_needs_redraw(root)) return;
    
    // Draw this widget
    draw_widget(root);
    
    // Clear dirty flag
    widget_clear_dirty(root);
    
    // Draw children
    for (uint8_t i = 0; i < root->children_count; i++) {
        draw_widget_tree_optimized(root->children[i]);
    }
}

// Force full redraw
void widget_force_redraw(Widget* widget) {
    if (!widget) return;
    widget_mark_dirty(widget);
    widget->dirty_state.dirty = true;
    
    for (uint8_t i = 0; i < widget->children_count; i++) {
        widget_force_redraw(widget->children[i]);
    }
}
```

---

## Memory Management for Advanced Features

### Clipboard Macros

```c
// Clipboard management macros
#define CLIPBOARD_SET(txt) \
    if(txt) { clipboard_set(txt); }

#define CLIPBOARD_GET() \
    (clipboard_has_content() ? clipboard_get() : NULL)

#define CLIPBOARD_CLEAR() \
    clipboard_clear()

#define CLIPBOARD_COPY(tf) \
    if(tf && (tf)->selection.active) { textfield_copy(tf); }

#define CLIPBOARD_CUT(tf) \
    if(tf && (tf)->selection.active) { textfield_cut(tf); }

#define CLIPBOARD_PASTE(tf) \
    if(tf && clipboard_has_content()) { textfield_paste(tf, clipboard_get()); }
```

### History Macros

```c
// History management macros
#define HISTORY_SAVE(tf) \
    if(tf) { history_save_state(tf); }

#define HISTORY_UNDO(tf) \
    if(tf && history_can_undo(tf)) { history_undo(tf); }

#define HISTORY_REDO(tf) \
    if(tf && history_can_redo(tf)) { history_redo(tf); }

#define HISTORY_CLEAR(tf) \
    if(tf) { history_clear(tf); }

#define HISTORY_CAN_UNDO(tf) \
    (tf && history_can_undo(tf))

#define HISTORY_CAN_REDO(tf) \
    (tf && history_can_redo(tf))
```

---

## Usage Examples

### Complete Text Editing System

```c
WidgetTextField* textfield;

void setup() {
    // Initialize systems
    init_clipboard();
    init_gestures();
    init_history(textfield);
    
    // Set gesture callback
    set_gesture_callback(handle_gestures);
    
    // Create text field
    textfield = new_textfield(512, TEXTFIELD_STYLE_NORMAL);
    init_history(textfield);
    
    // Set callbacks
    textfield->on_change = on_textfield_change;
    textfield->on_enter = on_textfield_enter;
    
    // Save initial state
    history_save_state(textfield);
}

void handle_gestures(GestureType gesture, uint16_t x, uint16_t y) {
    switch (gesture) {
        case GESTURE_TAP:
            // Position cursor
            textfield_set_cursor_from_x(textfield, x);
            break;
            
        case GESTURE_DOUBLE_TAP:
            // Select all text
            textfield_select_all(textfield);
            break;
            
        case GESTURE_LONG_PRESS:
            // Copy selected text
            textfield_copy(textfield);
            break;
            
        case GESTURE_SWIPE_LEFT:
        case GESTURE_SWIPE_RIGHT:
            // Switch UIs or scroll
            break;
    }
}

void on_textfield_change(const char* text) {
    // Save state for undo
    history_save_state(textfield);
    
    // Update clipboard/paste button state
    update_ui_buttons();
}

void handle_key_press(char key) {
    switch (key) {
        case 'C':  // Ctrl+C equivalent
            textfield_copy(textfield);
            break;
            
        case 'X':  // Ctrl+X equivalent
            textfield_cut(textfield);
            break;
            
        case 'V':  // Ctrl+V equivalent
            if (clipboard_has_content()) {
                textfield_paste(textfield, clipboard_get());
            }
            break;
            
        case 'Z':  // Ctrl+Z equivalent
            history_undo(textfield);
            break;
            
        case 'Y':  // Ctrl+Y equivalent
            history_redo(textfield);
            break;
            
        default:
            // Handle regular key press
            textfield_append_char(textfield, key);
            break;
    }
}
```

---

## ESP8266-Specific Considerations

### Memory Constraints

| Feature | Memory Usage | Notes |
|---------|--------------|-------|
| Clipboard | ~64 bytes | Static buffer |
| Gesture State | ~20 bytes | Single state |
| History | ~320 bytes | 10 states * 32 bytes |
| Command Queue | Variable | Limited to 20 commands |

### Performance Tips

1. **Limit history depth**: Use 10-20 states maximum
2. **Use static buffers**: Avoid dynamic allocation in critical paths
3. **Batch operations**: Process gestures during idle time, not in touch handlers
4. **Debounce gestures**: Add small delay to prevent multiple gesture detections

### No Dynamic Allocation

```c
// BAD - allocates in gesture handler
void bad_gesture_handler(GestureType gesture) {
    char* temp = malloc(256);  // Avoid!
    // Process gesture
    free(temp);
}

// GOOD - uses static buffer
void good_gesture_handler(GestureType gesture) {
    static char temp_buffer[256];  // Pre-allocated
    // Process gesture
}
```

---

## Cross-References

- **Widget Implementations**: See `09_WIDGET_IMPLEMENTATIONS.md`
- **Text and Input**: See `10_TEXT_AND_INPUT.md`
- **Memory Management**: See `06_MEMORY_MANAGEMENT.md` and `docs/MEMORY_MANAGEMENT.md`
- **Optimizations**: See `12_OPTIMIZATIONS.md`
- **SD Card/WebDAV**: See `11_SD_CARD_WEBDAV.md`

---

## File Locations

Source implementations:
- `src/gui/clipboard.h` - Clipboard system declarations
- `src/gui/clipboard.cpp` - Clipboard implementation
- `src/gui/gestures.h` - Gesture recognition declarations
- `src/gui/gestures.cpp` - Gesture implementation
- `src/gui/history.h` - History system declarations
- `src/gui/history.cpp` - History implementation
- `src/gui/command.h` - Command pattern declarations
- `src/gui/command.cpp` - Command pattern implementation
- `src/gui/scope_guard.h` - RAII-style scope guards
