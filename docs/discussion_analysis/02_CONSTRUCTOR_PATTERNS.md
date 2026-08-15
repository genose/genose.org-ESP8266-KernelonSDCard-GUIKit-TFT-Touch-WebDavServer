# Constructor Patterns

> Extracted from discussion_guikit.txt - Widget constructor implementations and patterns

## Overview

This document covers various approaches to creating widget constructors in C (which lacks native function overloading). Multiple patterns are explored to achieve flexible, type-safe widget creation with optional parameters.

---

## Constraint: Text Length Limit

> From discussion: "creer un construteur, le char*text ne peut depasser 512 characteres delim '\0'"

All widget text must be limited to 512 characters (511 + null terminator).

---

## Pattern 1: Simple Constructor with Static Text Buffer

The simplest approach using a static text buffer to avoid dynamic allocation:

```c
#define MAX_TEXT_LENGTH 512  // Limit to 512 characters

struct t_widget_base_text {
    char text[MAX_TEXT_LENGTH];  // Static text buffer (avoids malloc)
    struct {
        uint8_t size;           // Font size
        uint16_t color;         // Text color (RGBA565)
    } font;
};

struct t_widget_button {
    t_widget_base base;          // Base common
    t_widget_base_text text;     // Button text
    bool pressed;                // Button state
    void (*on_click)(void);      // Click callback
};

/**
 * @brief Creates a button with text limited to 512 characters.
 * @param text Button text (automatically truncated to 511 characters + '\0').
 * @param x X position.
 * @param y Y position.
 * @param width Width.
 * @param height Height.
 * @param on_click Callback called on click (NULL if none).
 * @return Pointer to the created button (dynamically allocated).
 */
t_widget_button* create_button(
    const char* text,
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    void (*on_click)(void)
) {
    // Allocate memory for the button
    t_widget_button* button = (t_widget_button*)malloc(sizeof(t_widget_button));
    if (!button) {
        return NULL;  // Allocation failed
    }

    // Initialize the base
    button->base.type = WIDGET_TYPE_BUTTON;
    button->base.position.x = x;
    button->base.position.y = y;
    button->base.size.width = width;
    button->base.size.height = height;
    button->base.background.color = 0x0000;  // Black background by default
    button->base.border.color = 0xFFFF;     // White border
    button->base.border.width = 1;
    button->base.children = NULL;
    button->base.children_count = 0;

    // Copy text (truncate to MAX_TEXT_LENGTH-1 if necessary)
    strncpy(button->text.text, text, MAX_TEXT_LENGTH - 1);
    button->text.text[MAX_TEXT_LENGTH - 1] = '\0';  // Ensure termination

    // Initialize font
    button->text.font.size = 12;
    button->text.font.color = 0xFFFF;  // White

    // Initial state
    button->pressed = false;
    button->on_click = on_click;

    return button;
}

/**
 * @brief Frees a button's memory.
 * @param button Pointer to the button to free.
 */
void free_button(t_widget_button* button) {
    if (button) {
        // Free children if necessary (not handled here for simplicity)
        free(button);
    }
}
```

### Key Points for Static Text Buffer

**Text Management:**
- Use `strncpy` to limit copy to MAX_TEXT_LENGTH - 1 characters
- Manually add '\0' to guarantee string termination
- No need for malloc for text: The text array is static in the structure, saving RAM and preventing memory leaks

**ESP8266 Optimization:**
- Avoid `strdup` (which allocates dynamically) to reduce memory fragmentation
- Use default values for uninitialized fields (e.g., background color, border)

**Safety:**
- Check memory allocation (if (!button))
- No risk of buffer overflow thanks to strncpy

---

## Pattern 2: Constructor with va_arg (Variable Arguments)

Using C's variable argument lists to support optional parameters:

```c
#include <stdarg.h>
#include <string.h>

// Basic types
typedef enum {
    WIDGET_TYPE_VIEW = 1,
    WIDGET_TYPE_BUTTON,
    WIDGET_TYPE_LABEL,
} WIDGET_TYPE;

// Rect structure (position + size)
typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
} Rect;

typedef struct {
    uint16_t x;
    uint16_t y;
} Position;

typedef struct {
    uint16_t width;
    uint16_t height;
} Size;

// Style
typedef struct {
    uint16_t color;      // Background color (RGBA565)
    bool gradient;       // Gradient enabled
    uint16_t gradient_color;
} Background;

typedef struct {
    uint16_t color;      // Border color
    uint8_t width;       // Width
} Border;

// Text (limited to 512 characters)
typedef struct {
    char text[512];
    struct {
        uint8_t size;
        uint16_t color;
    } font;
} WidgetText;

// Base structure
typedef struct {
    WIDGET_TYPE type;
    Background background;
    Border border;
    Position position;
    Size size;
    WidgetText text;     // Present for all widgets (can be empty)
    void* data;          // Type-specific data (e.g., callback for button)
} Widget;

// Button widget
typedef struct {
    Widget base;
    bool pressed;
    void (*on_click)(void);
} WidgetButton;

/**
 * @brief Generic constructor to create a widget with optional parameters.
 * @param type Widget type (WIDGET_TYPE_BUTTON, etc.).
 * @param ... Optional parameters (Rect, Size, Position, text, etc.).
 * @return Pointer to the created widget.
 */
Widget* new_widget(WIDGET_TYPE type, ...) {
    // Allocate memory for the base widget
    Widget* widget = (Widget*)malloc(sizeof(Widget));
    if (!widget) return NULL;

    // Initialize default values
    widget->type = type;
    widget->background.color = 0x0000;       // Black
    widget->background.gradient = false;
    widget->border.color = 0xFFFF;          // White
    widget->border.width = 1;
    widget->position.x = 0;
    widget->position.y = 0;
    widget->size.width = 100;             // Default size
    widget->size.height = 50;
    widget->text.text[0] = '\0';             // Empty text
    widget->text.font.size = 12;
    widget->text.font.color = 0xFFFF;       // White
    widget->data = NULL;

    // Read optional arguments
    va_list args;
    va_start(args, type);

    // Process arguments (assume they are passed in specific order)
    for (;;) {
        // Check if next argument is a Rect (Position + Size)
        Rect* rect_arg = va_arg(args, Rect*);
        if (rect_arg) {
            widget->position = rect_arg->position;
            widget->size = rect_arg->size;
            continue;
        }

        // Check if it's a Size
        Size* size_arg = va_arg(args, Size*);
        if (size_arg) {
            widget->size = *size_arg;
            continue;
        }

        // Check if it's a Position
        Position* pos_arg = va_arg(args, Position*);
        if (pos_arg) {
            widget->position = *pos_arg;
            continue;
        }

        // Check if it's text (char*)
        char* text_arg = va_arg(args, char*);
        if (text_arg) {
            strncpy(widget->text.text, text_arg, 511);
            widget->text.text[511] = '\0';
            continue;
        }

        // End of arguments
        break;
    }
    va_end(args);

    // Allocate type-specific data
    switch (type) {
        case WIDGET_TYPE_BUTTON: {
            WidgetButton* button = (WidgetButton*)malloc(sizeof(WidgetButton));
            if (!button) {
                free(widget);
                return NULL;
            }
            button->base = *widget;
            button->pressed = false;
            button->on_click = NULL;
            free(widget);  // Free temporary widget
            return (Widget*)button;
        }
        // Other types...
        default:
            return widget;
    }
}

// Macro to simplify syntax
#define NEW_WIDGET(type, ...) new_widget(type, &(Rect){__VA_ARGS__})
#define NEW_BUTTON(...) ((WidgetButton*)NEW_WIDGET(WIDGET_TYPE_BUTTON, __VA_ARGS__))
```

### Usage Examples (va_arg version)

```c
// Button with default values
WidgetButton* button1 = NEW_BUTTON();

// Button with size
WidgetButton* button2 = NEW_BUTTON({32, 128});  // {width=32, height=128}

// Button with position and size
WidgetButton* button3 = NEW_BUTTON({10, 20, 32, 128});  // {x=10, y=20, width=32, height=128}

// Button with text
WidgetButton* button4 = NEW_BUTTON({10, 20, 32, 128}, "OK");
```

### Limitations of va_arg

- `va_arg` cannot automatically detect argument types
- Arguments must be passed in a precise order (Rect*, Size*, Position*, char*)
- Less type-safe than other approaches

---

## Pattern 3: Constructor with _Generic (C11)

Using C11's `_Generic` for type-safe function overloading:

```c
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

// Widget types
typedef enum {
    WIDGET_TYPE_VIEW,
    WIDGET_TYPE_BUTTON,
    WIDGET_TYPE_LABEL,
} WIDGET_TYPE;

// Rect structure
typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
} Rect;

// Style
typedef struct {
    uint16_t bg_color;
    uint16_t border_color;
    uint8_t border_width;
} Style;

// Text
typedef struct {
    char text[512];
    uint8_t font_size;
    uint16_t font_color;
} Text;

// Base widget
typedef struct {
    WIDGET_TYPE type;
    Rect rect;           // position + size
    Style style;
    Text text;           // Present for all widgets
    void* data;          // Type-specific data
} Widget;

// Button widget
typedef struct {
    Widget base;
    bool pressed;
    void (*on_click)(void);
} WidgetButton;

// Private constructor for base widget
static Widget* _new_widget_base(WIDGET_TYPE type) {
    Widget* widget = malloc(sizeof(Widget));
    if (!widget) return NULL;

    widget->type = type;
    widget->rect = (Rect){0, 0, 100, 50}; // Default values
    widget->style = (Style){0x0000, 0xFFFF, 1}; // Black bg, white border
    widget->text = (Text){"", 12, 0xFFFF}; // Empty text, white font
    widget->data = NULL;
    return widget;
}

// Constructor for WidgetButton with Rect
static WidgetButton* _new_button_with_rect(Rect rect) {
    WidgetButton* button = malloc(sizeof(WidgetButton));
    if (!button) return NULL;

    button->base = *_new_widget_base(WIDGET_TYPE_BUTTON);
    button->base.rect = rect;
    button->pressed = false;
    button->on_click = NULL;
    return button;
}

// Constructor for WidgetButton with Rect + text
static WidgetButton* _new_button_with_rect_and_text(Rect rect, const char* text) {
    WidgetButton* button = _new_button_with_rect(rect);
    if (!button) return NULL;

    strncpy(button->base.text.text, text, 511);
    button->base.text.text[511] = '\0';
    return button;
}

// Constructor for WidgetButton with text only
static WidgetButton* _new_button_with_text(const char* text) {
    WidgetButton* button = _new_button_with_rect((Rect){0, 0, 100, 50});
    if (!button) return NULL;

    strncpy(button->base.text.text, text, 511);
    button->base.text.text[511] = '\0';
    return button;
}

// Default constructor for WidgetButton
static WidgetButton* _new_button_default(void) {
    return _new_button_with_rect((Rect){0, 0, 100, 50});
}

// Generic-based dispatch
#define new_widget(type, ...) \
    _Generic((type), \
        WIDGET_TYPE_BUTTON: _new_button(__VA_ARGS__), \
        WIDGET_TYPE_VIEW:   _new_view(__VA_ARGS__), \
        WIDGET_TYPE_LABEL:  _new_label(__VA_ARGS__) \
    )

// Problem: _Generic cannot match multiple arguments
// new_widget(WIDGET_TYPE_BUTTON, (Rect){10,20,32,128}, "OK") won't work
// because _Generic can only match one type at a time
```

### Limitation of _Generic

`_Generic` cannot handle multiple arguments of different types. The following won't work:
```c
new_widget(WIDGET_TYPE_BUTTON, (Rect){10, 20, 32, 128}, "OK");
```

Because `_Generic` can only match one type at a time.

---

## Pattern 4: Macro-Based Argument Counting (Final Solution)

Combining macros to count arguments and `_Generic` for type dispatch:

```c
// Macro to count number of arguments (up to 4)
#define _NARG(...) _NARG_IMPL(__VA_ARGS__, 4, 3, 2, 1, 0)
#define _NARG_IMPL(_1, _2, _3, _4, N, ...) N

// Macro to select constructor based on number of arguments
#define new_widget(type, ...) \
    _new_widget_dispatch(type, _NARG(__VA_ARGS__), __VA_ARGS__)

// Dispatcher
#define _new_widget_dispatch(type, n, ...) _new_widget_dispatch_##n(type, __VA_ARGS__)

// Case 0: No arguments
#define _new_widget_dispatch_0(type) \
    _Generic((type), \
        WIDGET_TYPE_BUTTON: _new_button_default, \
        WIDGET_TYPE_VIEW:   _new_view_default, \
        WIDGET_TYPE_LABEL:  _new_label_default \
    )()

// Case 1: One argument (Rect or const char*)
#define _new_widget_dispatch_1(type, arg1) \
    _Generic((type), \
        WIDGET_TYPE_BUTTON: _new_button_dispatch_1, \
        WIDGET_TYPE_VIEW:   _new_view_dispatch_1, \
        WIDGET_TYPE_LABEL:  _new_label_dispatch_1 \
    )(arg1)

// Dispatch for WidgetButton with 1 argument
#define _new_button_dispatch_1(arg1) \
    _Generic((arg1), \
        Rect: _new_button_with_rect, \
        const char*: _new_button_with_text, \
        default: _new_button_default \
    )(arg1)

// Case 2: Two arguments (Rect + const char*)
#define _new_widget_dispatch_2(type, arg1, arg2) \
    _Generic((type), \
        WIDGET_TYPE_BUTTON: _new_button_with_rect_and_text, \
        WIDGET_TYPE_VIEW:   _new_view_with_rect_and_style, \
        WIDGET_TYPE_LABEL:  _new_label_with_rect_and_text \
    )(arg1, arg2)
```

### Complete Implementation

```c
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

// --- Base Types ---
typedef enum { WIDGET_TYPE_VIEW, WIDGET_TYPE_BUTTON, WIDGET_TYPE_LABEL } WIDGET_TYPE;

typedef struct { uint16_t x, y, width, height; } Rect;
typedef struct { uint16_t bg_color; uint16_t border_color; uint8_t border_width; } Style;
typedef struct { char text[512]; uint8_t font_size; uint16_t font_color; } Text;

typedef struct {
    WIDGET_TYPE type;
    Rect rect;
    Style style;
    Text text;
    void* data;
} Widget;

typedef struct {
    Widget base;
    bool pressed;
    void (*on_click)(void);
} WidgetButton;

// --- WidgetButton Constructors ---
static WidgetButton* _new_button_default(void) {
    WidgetButton* button = malloc(sizeof(WidgetButton));
    if (!button) return NULL;
    button->base.type = WIDGET_TYPE_BUTTON;
    button->base.rect = (Rect){0, 0, 100, 50};
    button->base.style = (Style){0x0000, 0xFFFF, 1};
    button->base.text = (Text){"", 12, 0xFFFF};
    button->pressed = false;
    button->on_click = NULL;
    return button;
}

static WidgetButton* _new_button_with_rect(Rect rect) {
    WidgetButton* button = _new_button_default();
    if (!button) return NULL;
    button->base.rect = rect;
    return button;
}

static WidgetButton* _new_button_with_text(const char* text) {
    WidgetButton* button = _new_button_default();
    if (!button) return NULL;
    strncpy(button->base.text.text, text, 511);
    button->base.text.text[511] = '\0';
    return button;
}

static WidgetButton* _new_button_with_rect_and_text(Rect rect, const char* text) {
    WidgetButton* button = _new_button_with_rect(rect);
    if (!button) return NULL;
    strncpy(button->base.text.text, text, 511);
    button->base.text.text[511] = '\0';
    return button;
}

// --- Macros for Overloading ---
#define _NARG(...) _NARG_IMPL(__VA_ARGS__, 4, 3, 2, 1, 0)
#define _NARG_IMPL(_1, _2, _3, _4, N, ...) N

#define new_widget(type, ...) \
    _new_widget_dispatch(type, _NARG(__VA_ARGS__), __VA_ARGS__)

#define _new_widget_dispatch(type, n, ...) _new_widget_dispatch_##n(type, __VA_ARGS__)

#define _new_widget_dispatch_0(type) \
    _Generic((type), \
        WIDGET_TYPE_BUTTON: _new_button_default, \
        WIDGET_TYPE_VIEW:   _new_view_default, \
        WIDGET_TYPE_LABEL:  _new_label_default \
    )()

#define _new_widget_dispatch_1(type, arg1) \
    _Generic((type), \
        WIDGET_TYPE_BUTTON: _new_button_dispatch_1, \
        WIDGET_TYPE_VIEW:   _new_view_dispatch_1, \
        WIDGET_TYPE_LABEL:  _new_label_dispatch_1 \
    )(arg1)

#define _new_button_dispatch_1(arg1) \
    _Generic((arg1), \
        Rect: _new_button_with_rect, \
        const char*: _new_button_with_text, \
        default: _new_button_default \
    )(arg1)

#define _new_widget_dispatch_2(type, arg1, arg2) \
    _Generic((type), \
        WIDGET_TYPE_BUTTON: _new_button_with_rect_and_text, \
        WIDGET_TYPE_VIEW:   _new_view_with_rect_and_style, \
        WIDGET_TYPE_LABEL:  _new_label_with_rect_and_text \
    )(arg1, arg2)
```

### Usage Examples (Final Macro Version)

```c
// Button with default values
WidgetButton* button1 = new_widget(WIDGET_TYPE_BUTTON);

// Button with Rect
WidgetButton* button2 = new_widget(WIDGET_TYPE_BUTTON, (Rect){10, 20, 32, 128});

// Button with text
WidgetButton* button3 = new_widget(WIDGET_TYPE_BUTTON, "OK");

// Button with Rect + text
WidgetButton* button4 = new_widget(WIDGET_TYPE_BUTTON, (Rect){10, 20, 32, 128}, "OK");

// Cleanup
free(button1);
free(button2);
free(button3);
free(button4);
```

---

## Pattern 5: Simplified Explicit Overloads

For ESP8266, the recommended approach avoids `malloc` and uses explicit function overloads:

```c
// Default constructor
Widget* new_widget(WIDGET_TYPE type) {
    Widget* widget = malloc(sizeof(Widget));
    // Initialize with default values
    widget->type = type;
    widget->rect = (Rect){0, 0, 100, 50};
    widget->text.text[0] = '\0';
    // ...
    return widget;
}

// Overload with Rect
Widget* new_widget_with_rect(WIDGET_TYPE type, Rect rect) {
    Widget* widget = new_widget(type);
    widget->rect = rect;
    return widget;
}

// Overload with text
Widget* new_widget_with_text(WIDGET_TYPE type, const char* text) {
    Widget* widget = new_widget(type);
    strncpy(widget->text.text, text, 511);
    widget->text.text[511] = '\0';
    return widget;
}

// Overload with Rect + text
Widget* new_widget_full(WIDGET_TYPE type, Rect rect, const char* text) {
    Widget* widget = new_widget_with_rect(type, rect);
    strncpy(widget->text.text, text, 511);
    widget->text.text[511] = '\0';
    return widget;
}

// Macros for simplicity
#define NEW_WIDGET(type) new_widget(type)
#define NEW_BUTTON() ((WidgetButton*)NEW_WIDGET(WIDGET_TYPE_BUTTON))
#define NEW_BUTTON_WITH_RECT(rect) ((WidgetButton*)new_widget_with_rect(WIDGET_TYPE_BUTTON, rect))
```

### Usage Examples (Explicit Overloads)

```c
// Button with default values
WidgetButton* button1 = NEW_BUTTON();

// Button with size
WidgetButton* button2 = NEW_BUTTON_WITH_RECT({10, 20, 32, 128});

// Button with text
WidgetButton* button3 = (WidgetButton*)new_widget_with_text(WIDGET_TYPE_BUTTON, "OK");

// Button with Rect + text
WidgetButton* button4 = (WidgetButton*)new_widget_full(WIDGET_TYPE_BUTTON, {10, 20, 32, 128}, "OK");
```

---

## Comparison of Patterns

| Pattern | Flexibility | Type Safety | ESP8266 Friendly | Complexity | Recommended |
|---------|-------------|-------------|-------------------|------------|-------------|
| Simple with malloc | Low | Medium | No (malloc overhead) | Low | No |
| Static text buffer | Medium | High | **Yes** | Low | **Yes** |
| va_arg | High | Low | No (complex) | Medium | No |
| _Generic | Medium | High | No (C11 required) | Medium | Partial |
| Macro + _Generic | High | High | Partial | **High** | Partial |
| Explicit overloads | Medium | High | **Yes** | Low | **Yes** |
| Object pooling | High | High | **Yes (Best)** | Medium | **Best** |

---

## Best Practices for ESP8266

1. **Avoid malloc**: Use object pools for widgets
2. **Use static buffers**: For text, use fixed-size char arrays
3. **Prefer explicit overloads**: Simpler and more maintainable
4. **Limit text length**: Always enforce 512 character limit
5. **Use macros for convenience**: But keep them simple
6. **Test on hardware**: ESP8266 has limited resources

---

## Final Recommendation

For ESP8266 GUIKit, use **object pooling + static text buffers + explicit constructor functions**:

```c
// Object pool for buttons
#define MAX_BUTTONS 10
t_widget_button button_pool[MAX_BUTTONS];
uint8_t button_pool_index = 0;

t_widget_button* create_button_static(const char* text, uint16_t x, uint16_t y, uint16_t width, uint16_t height, void (*on_click)(void)) {
    if (button_pool_index >= MAX_BUTTONS) return NULL;

    t_widget_button* button = &button_pool[button_pool_index++];
    // Initialize without malloc
    strncpy(button->text.text, text, MAX_TEXT_LENGTH - 1);
    button->text.text[MAX_TEXT_LENGTH - 1] = '\0';
    // ... set other properties
    return button;
}
```

This gives you:
- ✅ No malloc/free overhead
- ✅ No memory fragmentation
- ✅ Predictable memory usage
- ✅ Fast O(1) allocation
- ✅ Safe (no leaks, no double-frees)

---

*Source: Extracted from discussion_guikit.txt, lines 400-1475*
*Documentation organized by Mistral Vibe*
