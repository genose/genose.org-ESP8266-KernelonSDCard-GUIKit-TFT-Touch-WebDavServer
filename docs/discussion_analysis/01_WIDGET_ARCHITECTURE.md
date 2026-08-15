# Widget Architecture

> Extracted from discussion_guikit.txt - Base widget structures and hierarchy

## Overview

GUIKit is a development kit for creating GUI applications using C/C++ structures for ESP8266 with a 3.2" TFT display. The architecture is based on a modular, object-oriented approach using C structures with inheritance via composition.

## Initial Structure Concepts

### Widget Type Enum

The foundational enum that describes widget types:

```c
enum WIDGET_TYPE {
    WIDGET_TYPE_VIEW = 1,
    WIDGET_TYPE_BUTTON,
    // ... other types
};
```

### Base Text Structure

Common text properties shared across widgets:

```c
struct t_widget_base_text {
    char* text;        // Dynamic text pointer
    struct {
        uint8_t size;   // Font size
        uint16_t color; // Text color (RGBA565)
        // Other font properties
    } font;
};
```

### Base Widget Structure

The foundational structure that all widgets inherit from:

```c
struct t_widget_base {
    uint8_t UUID[16];    // Unique identifier for the widget
    WIDGET_TYPE type;   // Widget type

    // Style properties (CSS-like)
    struct {
        uint16_t color;           // Background color (RGBA565 for TFT)
        bool gradient;            // Gradient enabled flag
        uint16_t gradient_color;  // Secondary color for gradient
    } background;

    struct {
        uint16_t color;           // Border color
        uint8_t width;            // Border width
    } border;

    struct {
        uint16_t width;           // Width
        uint16_t height;          // Height
    } size;

    struct {
        uint16_t x;               // X position
        uint16_t y;               // Y position
    } position;

    struct {
        struct {
            uint8_t top, right, bottom, left;  // Padding
        } padding;
        struct {
            uint8_t top, right, bottom, left;  // Margin
        } margin;
    } bound;

    // Scrollable support (union-based, memory optimized)
    Scrollable scroll;           // Scrollable property with bitmask flags

    // Child widgets (for container widgets like VIEW)
    struct t_widget_base** children;  // Pointer to array of widgets
    uint8_t children_count;            // Number of children
};
```

**Memory Optimization**: The union-based scrollable design saves ~46% memory on average by only storing data for active scroll directions. See `16_SCROLLABLE_UNION.md` for complete details.

### Button Widget Structure

Example of a widget that extends the base:

```c
struct t_widget_button : t_widget_base {
    // Inherits all fields from t_widget_base
    // Additional button-specific fields would go here
    // node[]: {t_widget_base} - can contain child widgets
};
```

### Generic Widget Union

Union-based approach to handle different widget types:

```c
typedef struct {
    WIDGET_TYPE type;
    union {
        t_widget_base base;
        t_widget_button button;
        // Other widget types...
    } widget;
} t_widget;
```

---

## Proposed Architecture

### 1. Class/Structure Hierarchy

The structures are designed for an inheritance approach using `t_widget_base` as a common base:

```c
// Widget types enum
enum WIDGET_TYPE {
    WIDGET_TYPE_VIEW = 1,
    WIDGET_TYPE_BUTTON,
    WIDGET_TYPE_LABEL,
    WIDGET_TYPE_SLIDER,
    // ...
};

// Base structure for all widgets
struct t_widget_base {
    uint8_t UUID[16];           // UUID to identify the widget
    WIDGET_TYPE type;          // Widget type

    // CSS-like styling
    struct {
        uint16_t color;          // Background color (RGBA565 for TFT)
        bool gradient;           // Gradient enabled
        uint16_t gradient_color; // Secondary color for gradient
    } background;

    struct {
        uint16_t color;          // Border color
        uint8_t width;           // Border width
    } border;

    struct {
        uint16_t width;           // Width
        uint16_t height;          // Height
    } size;

    struct {
        uint16_t x;               // X position
        uint16_t y;               // Y position
    } position;

    struct {
        struct {
            uint8_t top, right, bottom, left;  // Padding
        } padding;
        struct {
            uint8_t top, right, bottom, left;  // Margin
        } margin;
    } bound;

    // Child widgets array (for containers like VIEW)
    struct t_widget_base* children;    // Pointer to widget array
    uint8_t children_count;            // Number of children
};

// Text structure for widgets with text
struct t_widget_base_text {
    char* text;                    // Displayed text
    struct {
        uint8_t size;               // Font size
        uint16_t color;             // Text color (RGBA565)
        // Other font properties (alignment, etc.)
    } font;
};

// Button widget (implicit inheritance via composition)
struct t_widget_button {
    t_widget_base base;            // Inheritance from base
    t_widget_base_text text;       // Button text
    bool pressed;                  // Button state (pressed/released)
    void (*on_click)(void);        // Click callback
};

// Union to manage all widget types
typedef struct {
    WIDGET_TYPE type;
    union {
        t_widget_base base;
        t_widget_button button;
        // Other widget types...
    } widget;
} t_widget;
```

### 2. Widget Management

#### Creation and Destruction

```c
// Function to create a base widget
t_widget_base* create_widget_base(WIDGET_TYPE type, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    t_widget_base* widget = (t_widget_base*)malloc(sizeof(t_widget_base));
    widget->type = type;
    widget->position.x = x;
    widget->position.y = y;
    widget->size.width = width;
    widget->size.height = height;
    widget->children = NULL;
    widget->children_count = 0;
    // Initialize other fields with defaults
    return widget;
}

// Function to create a button
t_widget_button* create_button(const char* text, uint16_t x, uint16_t y, uint16_t width, uint16_t height, void (*on_click)(void)) {
    t_widget_button* button = (t_widget_button*)malloc(sizeof(t_widget_button));
    button->base = *create_widget_base(WIDGET_TYPE_BUTTON, x, y, width, height);
    button->text.text = strdup(text);
    button->text.font.size = 12;
    button->text.font.color = 0xFFFF; // White by default
    button->pressed = false;
    button->on_click = on_click;
    return button;
}

// Function to free a widget
void free_widget(t_widget* widget) {
    if (widget->type == WIDGET_TYPE_BUTTON) {
        free(widget->widget.button.text.text);
    }
    free((void*)widget);
}
```

#### Adding Children (for Containers)

```c
// Add a child to a container widget (e.g., VIEW)
void add_child(t_widget_base* parent, t_widget_base* child) {
    parent->children_count++;
    parent->children = (t_widget_base**)realloc(
        parent->children,
        parent->children_count * sizeof(t_widget_base*)
    );
    parent->children[parent->children_count - 1] = child;
}
```

### 3. Graphic Rendering (TFT)

Using TFT_eSPI library for drawing widgets on the TFT display:

```c
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

// Draw a button
void draw_button(t_widget_button* button) {
    // Draw background
    tft.fillRoundRect(
        button->base.position.x,
        button->base.position.y,
        button->base.size.width,
        button->base.size.height,
        5, // Corner radius
        button->pressed ? 0x8410 : button->base.background.color // Pressed/normal color
    );

    // Draw border
    tft.drawRoundRect(
        button->base.position.x,
        button->base.position.y,
        button->base.size.width,
        button->base.size.height,
        5,
        button->base.border.color
    );

    // Draw text (centered)
    tft.setTextColor(button->text.font.color);
    tft.setTextSize(button->text.font.size / 8); // Adapt to font size
    uint16_t text_width = tft.textWidth(button->text.text);
    uint16_t text_x = button->base.position.x + (button->base.size.width - text_width) / 2;
    uint16_t text_y = button->base.position.y + (button->base.size.height - 8) / 2; // 8 = text height
    tft.setCursor(text_x, text_y);
    tft.print(button->text.text);
}
```

### 4. Touch Event Handling

Using XPT2046 library for touchscreen management:

```c
#include <XPT2046_Touchscreen.h>
XPT2046_Touchscreen ts(CS_PIN);

// Check if a point is within a widget
bool is_touched(t_widget_base* widget, uint16_t touch_x, uint16_t touch_y) {
    return (touch_x >= widget->position.x &&
            touch_x <= widget->position.x + widget->size.width &&
            touch_y >= widget->position.y &&
            touch_y <= widget->position.y + widget->size.height);
}

// Handle touch for a button
void handle_touch(t_widget_button* button, uint16_t touch_x, uint16_t touch_y) {
    if (is_touched(&button->base, touch_x, touch_y)) {
        button->pressed = true;
        draw_button(button); // Redraw
        if (button->on_click) {
            button->on_click(); // Call callback
        }
    } else {
        button->pressed = false;
        draw_button(button);
    }
}
```

### 5. Main Loop

```c
void setup() {
    tft.init();
    ts.begin();

    // Create a button
    t_widget_button* my_button = create_button("OK", 50, 50, 100, 40, []() {
        Serial.println("Button clicked!");
    });
}

void loop() {
    if (ts.touched()) {
        TS_Point p = ts.getPoint();
        uint16_t touch_x = map(p.x, 0, 4095, 0, tft.width());
        uint16_t touch_y = map(p.y, 0, 4095, 0, tft.height());
        handle_touch(my_button, touch_x, touch_y);
    }
}
```

---

## Project Structure

```
GUIKit/
├── src/
│   ├── widget.h          # Structure definitions
│   ├── widget.cpp        # Creation/destruction functions
│   ├── renderer.h        # Rendering functions (TFT)
│   ├── renderer.cpp
│   ├── touch.h           # Touch handling
│   ├── touch.cpp
│   └── main.cpp          # Usage example
├── lib/
│   ├── TFT_eSPI/         # TFT library
│   └── XPT2046/          # Touch library
└── platformio.ini        # PlatformIO configuration
```

---

## ESP8266 Optimizations

### Memory

Use malloc sparingly (ESP8266 has limited RAM). Prefer object pools to avoid fragmentation:

```c
#define MAX_WIDGETS 20
t_widget_base widget_pool[MAX_WIDGETS];
uint8_t widget_pool_index = 0;

t_widget_base* allocate_widget() {
    if (widget_pool_index >= MAX_WIDGETS) return NULL;
    return &widget_pool[widget_pool_index++];
}
```

### Performance

Avoid redrawing all widgets every frame. Use a dirty flag system:

```c
struct t_widget_base {
    bool dirty; // If widget needs redrawing
};

void mark_dirty(t_widget_base* widget) {
    widget->dirty = true;
}

void render_all() {
    for (int i = 0; i < widget_pool_index; i++) {
        if (widget_pool[i].dirty) {
            draw_widget(&widget_pool[i]);
            widget_pool[i].dirty = false;
        }
    }
}
```

### Events

Use interrupts for touch to avoid blocking the CPU.

---

## Next Steps

- Implement other widgets (Label, Slider, Checkbox, etc.)
- Add global styles (themes, default colors)
- Optimize rendering (double buffering if RAM allows)
- Test on hardware with ESP8266 + 3.2" TFT display

---

---

## Related Documentation

- **Scrollable Support**: See `16_SCROLLABLE_UNION.md` for union-based scrollable implementation
- **Memory Management**: See `docs/MEMORY_MANAGEMENT.md` for memory optimization strategies
- **Style System**: See `05_STYLE_SYSTEM.md` for widget styling

*Source: Extracted from discussion_guikit.txt, lines 1-400*
*Documentation organized by Mistral Vibe*
*Updated: Added union-based scrollable support*
