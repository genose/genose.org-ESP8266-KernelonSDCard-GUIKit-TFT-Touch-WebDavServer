# Software Architecture

This document describes the software components, widget system, and module architecture.

---

## Table of Contents

1. [Widget System Architecture](#widget-system-architecture)
2. [GUIKit Module Structure](#guikit-module-structure)
3. [Widget Type System](#widget-type-system)
4. [Widget Creation Interface](#widget-creation-interface)
5. [Rendering Pipeline](#rendering-pipeline)
6. [Touch Event Flow](#touch-event-flow)
7. [Design Patterns](#design-patterns)
8. [Performance Optimizations](#performance-optimizations)

---

## Widget System Architecture

### Widget Type Hierarchy

```
┌─────────────────────────────────────────────────────────────┐
│                        Widget Type System                       │
├─────────────────────────────────────────────────────────────┤
│                                                                  │
│  WIDGET_TYPE (enum)                                              │
│  ├── WIDGET_TYPE_VIEW        # Container widget                │
│  │   ├── Can have child widgets                                 │
│  │   ├── Used for layout grouping                                │
│  │   └── Supports nesting                                        │
│  │                                                                  │
│  ├── WIDGET_TYPE_BUTTON      # Clickable button                │
│  │   ├── Has text (t_widget_base_text)                          │
│  │   ├── Has pressed state                                     │
│  │   ├── Has on_click callback                                 │
│  │   └── Supports visual feedback                              │
│  │                                                                  │
│  ├── WIDGET_TYPE_LABEL       # Text display                    │
│  │   ├── Has text (t_widget_base_text)                          │
│  │   ├── No interaction                                        │
│  │   └── Used for static/dynamic text                          │
│  │                                                                  │
│  ├── WIDGET_TYPE_SLIDER      # Slider control                  │
│  │   ├── Has value range                                       │
│  │   ├── Has on_change callback                                │
│  │   └── Supports horizontal/vertical                          │
│  │                                                                  │
│  └── ... (extensible for new widget types)                     │
│                                                                  │
└─────────────────────────────────────────────────────────────┘
                              │
          ┌───────────────────┴───────────────────┐
          │                                       │
  ┌───────▼───────┐                   ┌────────▼────────┐
  │ t_widget_base  │                   │  Specialized     │
  │  (Common)      │                   │  Widgets        │
  └───────────────┘                   └─────────────────┘
```

---

## GUIKit Module Structure

```
GUIKit Module/
├── widget.h/cpp          # Core widget definitions and management
│   ├── WIDGET_TYPE enum
│   ├── t_widget_base struct
│   ├── t_widget_button struct
│   ├── t_widget_label struct
│   ├── t_widget_slider struct
│   ├── t_widget_view struct
│   ├── create_widget_base()
│   ├── create_button()
│   ├── create_label()
│   ├── create_slider()
│   ├── add_child()
│   ├── free_widget()
│   └── Widget pool management
│
├── renderer.h/cpp        # TFT rendering engine
│   ├── draw_widget()
│   ├── draw_button()
│   ├── draw_label()
│   ├── draw_slider()
│   ├── mark_dirty()
│   ├── render_all()
│   └── TFT_eSPI integration
│
├── touch.h/cpp           # Touch event handling
│   ├── TouchPoint struct
│   ├── TouchState struct
│   ├── init_touch()
│   ├── handle_touch()
│   ├── is_touched()
│   └── XPT2046 integration
│
├── ui_loader.h/cpp       # JSON UI loading
│   ├── load_ui_from_file()
│   ├── parse_ui_json()
│   ├── create_widget_from_json()
│   └── JSON parsing
│
└── gui.h/cpp             # Main GUIKit interface
    ├── init_gui()
    ├── update_gui()
    ├── get_root_view()
    ├── load_and_display_ui()
    └── navigate_to_ui()
```

---

## Widget Type System

### Widget Type Enum

```c
typedef enum {
    WIDGET_TYPE_VIEW = 1,      // Container widget
    WIDGET_TYPE_BUTTON,       // Clickable button
    WIDGET_TYPE_LABEL,        // Text display
    WIDGET_TYPE_SLIDER,       // Slider control
    // ... future widget types
} WIDGET_TYPE;
```

### Base Widget Structure

```c
struct t_widget_base {
    // Identification
    uint8_t UUID[16];            // Unique identifier
    WIDGET_TYPE type;           // Widget type

    // Style (similar to CSS)
    struct {
        uint16_t color;          // Background color (RGBA565 for TFT)
        bool gradient;           // Gradient enabled
        uint16_t gradient_color; // Secondary gradient color
    } background;

    struct {
        uint16_t color;          // Border color
        uint8_t width;           // Border thickness
    } border;

    // Geometry
    struct {
        uint16_t width;          // Widget width
        uint16_t height;         // Widget height
    } size;

    struct {
        uint16_t x;              // X position
        uint16_t y;              // Y position
    } position;

    // Layout (similar to CSS box model)
    struct {
        struct {
            uint8_t top, right, bottom, left;  // Padding
        } padding;
        struct {
            uint8_t top, right, bottom, left;  // Margin
        } margin;
    } bound;

    // Children (for container widgets like VIEW)
    struct t_widget_base** children;   // Array of child widgets
    uint8_t children_count;           // Number of children

    // Rendering optimization
    bool dirty;                       // Needs redrawing
};
```

### Text Structure (Memory-Optimized)

```c
#define MAX_TEXT_LENGTH 512  // Maximum text length for any widget

struct t_widget_base_text {
    char text[MAX_TEXT_LENGTH];  // Static buffer (no malloc needed)
    struct {
        uint8_t size;           // Font size
        uint16_t color;         // Text color (RGBA565)
        uint8_t align;          // Text alignment (left/center/right)
    } font;
};
```

### Button Widget

```c
struct t_widget_button {
    t_widget_base base;          // Inheritance via composition
    t_widget_base_text text;     // Button text
    bool pressed;                // Button state (pressed/released)
    void (*on_click)(void);      // Callback function for click events
};
```

### Label Widget

```c
struct t_widget_label {
    t_widget_base base;          // Inheritance via composition
    t_widget_base_text text;     // Display text
};
```

### Slider Widget

```c
struct t_widget_slider {
    t_widget_base base;          // Inheritance via composition
    uint16_t min_value;          // Minimum value
    uint16_t max_value;          // Maximum value
    uint16_t value;             // Current value
    bool vertical;              // Orientation (true = vertical)
    void (*on_change)(uint16_t); // Callback for value changes
};
```

### View Widget (Container)

```c
struct t_widget_view {
    t_widget_base base;          // Inheritance via composition
    // Children are stored in base.children
};
```

### Widget Union Type

```c
typedef struct {
    WIDGET_TYPE type;           // Discriminator for union
    union {
        t_widget_base base;
        t_widget_button button;
        t_widget_label label;
        t_widget_slider slider;
        t_widget_view view;
        // ... other widget types
    } widget;
} t_widget;
```

---

## Widget Creation Interface

### Basic Creation Functions

```c
// Create base widget with type
Widget* create_widget(WIDGET_TYPE type);

// Create widget with position and size
Widget* create_widget_with_rect(WIDGET_TYPE type, Rect rect);

// Create widget with text
Widget* create_widget_with_text(WIDGET_TYPE type, const char* text);

// Create widget with position, size, and text
Widget* create_widget_full(WIDGET_TYPE type, Rect rect, const char* text);
```

### Button-Specific Creation

```c
// Create a button with all parameters
t_widget_button* create_button(
    const char* text,           // Button text (max 511 chars + null)
    uint16_t x, uint16_t y,      // Position
    uint16_t width, uint16_t height, // Dimensions
    void (*on_click)(void)      // Click callback (NULL if none)
);
```

### Widget Management

```c
// Add child to container widget
void add_child(Widget* parent, Widget* child);

// Remove child from container
void remove_child(Widget* parent, Widget* child);

// Free widget memory
void free_widget(Widget* widget);

// Free widget and all children recursively
void free_widget_tree(Widget* widget);
```

---

## Rendering Pipeline

### Rendering Flow

```
┌─────────────────────────────────────────────────────────────┐
│                    RENDERING PIPELINE                            │
├─────────────────────────────────────────────────────────────┤
│                                                                  │
│  Widget Tree (Hierarchy)                                       │
│      │                                                         │
│      ▼                                                         │
│  ┌─────────────────┐                                           │
│  │  Update State    │  Handle touch events, update values      │
│  │                 │                                           │
│  └────────┬────────┘                                           │
│           │                                                     │
│           ▼                                                     │
│  ┌─────────────────┐                                           │
│  │  Mark Dirty      │  Set dirty flag on modified widgets     │
│  │                 │  Cascade to children if needed            │
│  └────────┬────────┘                                           │
│           │                                                     │
│           ▼                                                     │
│  ┌─────────────────┐     ┌─────────────────┐                 │
│  │  Render All      │────▶│  TFT_eSPI       │                 │
│  │  (Iterate all   │     │  - Draw shapes   │                 │
│  │   widgets)      │     │  - Draw text     │                 │
│  │                 │     │  - Optimized    │                 │
│  └─────────────────┘     │    operations    │                 │
│                           └─────────────────┘                 │
│                                                                  │
└─────────────────────────────────────────────────────────────┘
```

### Rendering Code Example

```c
void draw_button(t_widget_button* button) {
    // Draw background (rounded rectangle)
    tft.fillRoundRect(
        button->base.position.x,
        button->base.position.y,
        button->base.size.width,
        button->base.size.height,
        5, // Corner radius
        button->pressed ? 0x8410 : button->base.background.color
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
    tft.setTextSize(button->text.font.size / 8);
    uint16_t text_width = tft.textWidth(button->text.text);
    uint16_t text_x = button->base.position.x + 
                     (button->base.size.width - text_width) / 2;
    uint16_t text_y = button->base.position.y + 
                     (button->base.size.height - 8) / 2;
    tft.setCursor(text_x, text_y);
    tft.print(button->text.text);
}
```

---

## Touch Event Flow

### Touch Handling Sequence

```
┌─────────────────────────────────────────────────────────────┐
│                    TOUCH EVENT FLOW                              │
├─────────────────────────────────────────────────────────────┤
│                                                                  │
│  XPT2046 Touchscreen                                           │
│      │                                                         │
│      ▼                                                         │
│  ┌─────────────────┐                                           │
│  │  Touch Event     │  TouchPoint (x, y, pressed)               │
│  └────────┬────────┘                                           │
│           │                                                     │
│           ▼                                                     │
│  ┌─────────────────┐                                           │
│  │  Find Touched    │  Traverse widget tree from root         │
│  │  Widget         │  Check bounds of each widget             │
│  └────────┬────────┘                                           │
│           │                                                     │
│           ▼                                                     │
│  ┌─────────────────┐                                           │
│  │  Widget         │  Button/Slider/etc.                        │
│  │  Handler        │                                           │
│  └────────┬────────┘                                           │
│           │                                                     │
│     ┌─────▼─────┐                                              │
│     │           │                                              │
│     ▼           ▼                                              │
│  ┌─────┐   ┌─────────┐                                        │
│  │NO   │   │ YES      │                                        │
│  │     │   │ Update   │  Update widget state                    │
│  │Exit │   │ State    │  pressed=true, value changed            │
│  └─────┘   │ Mark     │                                        │
│           │ Dirty    │  Mark for redrawing                     │
│           │ Redraw   │  Immediately redraw if needed            │
│           │ Execute  │  on_click(), on_change() callbacks        │
│           │ Callback │                                        │
│           └─────────┘                                        │
│                                                                  │
└─────────────────────────────────────────────────────────────┘
```

### Hit Detection

```c
// Check if a point is within a widget's bounds
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
        draw_button(button); // Redraw pressed state
        if (button->on_click) {
            button->on_click(); // Execute callback
        }
    } else {
        button->pressed = false;
        draw_button(button); // Redraw normal state
    }
}
```

---

## Design Patterns

### 1. Composite Pattern

**Purpose:** Treat individual widgets and compositions uniformly.

**Implementation:** Container widgets (VIEW) can have child widgets, and all widgets share the same base interface.

```c
// All widgets have the same base structure
struct t_widget_base {
    Widget** children;
    uint8_t children_count;
};

// Functions work on any widget type
void draw_widget(Widget* widget);
void handle_touch(Widget* widget, uint16_t x, uint16_t y);
```

### 2. Strategy Pattern

**Purpose:** Different rendering strategies for different widget types.

**Implementation:** Type-based dispatch in rendering functions.

```c
void draw_widget(Widget* widget) {
    switch (widget->type) {
        case WIDGET_TYPE_BUTTON:
            draw_button(&widget->widget.button);
            break;
        case WIDGET_TYPE_LABEL:
            draw_label(&widget->widget.label);
            break;
        case WIDGET_TYPE_SLIDER:
            draw_slider(&widget->widget.slider);
            break;
        // ...
    }
}
```

### 3. Observer Pattern

**Purpose:** Event-driven UI interactions.

**Implementation:** Callback functions for touch events.

```c
struct t_widget_button {
    void (*on_click)(void);  // Called when button is clicked
};

// Usage
void my_button_handler() {
    Serial.println("Button clicked!");
}

t_widget_button* btn = create_button(
    "Click me", 0, 0, 100, 50, my_button_handler
);
```

### 4. Object Pooling Pattern

**Purpose:** Avoid memory fragmentation on ESP8266.

**Implementation:** Pre-allocated arrays of objects.

```c
#define MAX_WIDGETS 20
#define MAX_BUTTONS 10

t_widget_base widget_pool[MAX_WIDGETS];
t_widget_button button_pool[MAX_BUTTONS];

uint8_t widget_pool_index = 0;
uint8_t button_pool_index = 0;

Widget* allocate_widget() {
    if (widget_pool_index >= MAX_WIDGETS) return NULL;
    return &widget_pool[widget_pool_index++];
}
```

---

## Performance Optimizations

### Memory Optimizations

| Optimization | Technique | Benefit |
|--------------|-----------|---------|
| Static Text Buffers | Fixed-size char arrays | No malloc/free overhead |
| Object Pooling | Pre-allocated pools | Avoids fragmentation |
| Dirty Flags | Only redraw changed | Reduces TFT updates |
| Lazy Loading | Load on-demand | Saves RAM |

### Rendering Optimizations

1. **Selective Redraw**: Only widgets marked as `dirty` are redrawn
2. **Batch Drawing**: Group TFT operations to minimize SPI transactions
3. **Background Caching**: Don't redraw backgrounds if unchanged
4. **Double Buffering**: (If RAM allows) Render to buffer, then copy to display

### ESP8266-Specific Tips

- Use `strncpy` instead of `strdup` for text
- Avoid dynamic allocation in loops
- Use static buffers for strings
- Minimize use of `malloc` and `free`
- Consider using `PROGMEM` for constant data

---

*See [ARCHITECTURE.md](ARCHITECTURE.md) for overall system architecture*  
*See [NETWORK.md](NETWORK.md) for network and WebDAV architecture*  
*See [DATA_FLOW.md](DATA_FLOW.md) for data flow diagrams*  

---

*Generated from architecture analysis of discussion_guikit.txt*
*Documentation extracted and organized by Mistral Vibe*
