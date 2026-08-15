# Widget Implementations

> Extracted from discussion_guikit.txt - Complete widget implementations for all widget types

## Overview

This document covers the concrete implementations of all widget types in GUIKit, including their structures, constructors, default styles, and memory management patterns. All implementations follow Objective-C style memory management principles (NOT ARC/reference counting) for ESP8266 compatibility.

## Memory Management Philosophy

**IMPORTANT**: As per project constraints for ESP8266 (80KB RAM limit, no atomic ops):
- **DO NOT** use ARC (Automatic Reference Counting)
- **DO NOT** use reference counting
- **USE** manual memory management with accessors/macros
- **USE** object pooling where appropriate
- **USE** explicit `free()` calls or pool release macros

```c
// Preferred pattern for releasing objects/widgets
define RELEASE(obj) if(obj) { free(obj); obj = NULL; }
```

---

## Widget Structure Hierarchy

### Base Widget Structure

The foundational structure that all widgets extend:

```c
typedef struct Widget {
    WIDGET_TYPE type;              // Type identifier
    Rect rect;                    // Position and dimensions
    WidgetStyle style;           // Drawing style (border, fill, effects)
    WidgetText text;              // Text properties (if applicable)
    struct Widget* parent;        // Parent widget pointer
    struct Widget** children;     // Array of child widgets
    uint8_t children_count;       // Number of children
    void* data;                   // Type-specific data pointer
} Widget;
```

### Common Properties

All widgets share:
- **Position**: `Point` with x, y coordinates
- **Size**: `Size` with width, height dimensions
- **Style**: `WidgetStyle` with draw flags, colors, borders, effects
- **Text**: `WidgetText` with character buffer (MAX_TEXT_LENGTH = 512)
- **Hierarchy**: Parent pointer and children array
- **Type-specific data**: Void pointer for widget-specific extensions

---

## Button Widget

### Structure

```c
typedef struct {
    Widget base;                    // Base widget properties
    bool pressed;                 // Press state (true = pressed, false = released)
    void (*on_click)(void);      // Click callback function
    void (*on_release)(void);     // Release callback function
} WidgetButton;
```

### Constructor

```c
WidgetButton* new_button(void) {
    WidgetButton* button = (WidgetButton*)malloc(sizeof(WidgetButton));
    if (!button) return NULL;

    // Initialize base widget
    button->base = *new_widget(WIDGET_TYPE_BUTTON);
    button->pressed = false;
    button->on_click = NULL;
    button->on_release = NULL;

    // Apply default button style
    button->base.style.draw_style = 
        WIDGET_DRAW_STYLE_ROUNDED_BORDER | 
        WIDGET_DRAW_STYLE_SOLID_FILL | 
        WIDGET_DRAW_STYLE_DROP_SHADOW;
    button->base.style.colors.primary = 0x001F;    // Blue
    button->base.style.colors.secondary = 0x001F;
    button->base.style.border.radius = 5;
    button->base.style.border.width = 2;

    return button;
}
```

### Memory Management

```c
// Using accessor/macro pattern for memory release
define RELEASE_BUTTON(btn) \
    if(btn) { \
        if((btn)->on_click) (btn)->on_click = NULL; \
        if((btn)->on_release) (btn)->on_release = NULL; \
        free(btn); \
        btn = NULL; \
    }
```

### Usage Example

```c
WidgetButton* myButton = new_button();
strcpy(myButton->base.text.text, "Click Me");
myButton->base.rect.position.x = 50;
myButton->base.rect.position.y = 50;
myButton->on_click = handle_button_click;

// Later, release properly
RELEASE_BUTTON(myButton);
```

---

## Label Widget

### Structure

```c
typedef struct {
    Widget base;                    // Base widget properties
    bool auto_resize;             // Auto-size to fit text
} WidgetLabel;
```

### Constructor

```c
WidgetLabel* new_label(const char* text) {
    WidgetLabel* label = (WidgetLabel*)malloc(sizeof(WidgetLabel));
    if (!label) return NULL;

    label->base = *new_widget(WIDGET_TYPE_LABEL);
    label->auto_resize = true;

    if (text) {
        strncpy(label->base.text.text, text, MAX_TEXT_LENGTH - 1);
        label->base.text.text[MAX_TEXT_LENGTH - 1] = '\0';
    }

    // Default label style (transparent background)
    label->base.style.draw_style = WIDGET_DRAW_STYLE_TRANSPARENT_FILL;
    label->base.style.colors.primary = 0xFFFF;  // White text

    return label;
}
```

### Memory Management

```c
#define RELEASE_LABEL(lbl) \
    if(lbl) { free(lbl); lbl = NULL; }
```

---

## Slider Widget

### Structure

```c
typedef struct {
    Widget base;                    // Base widget properties
    float min_value;              // Minimum value
    float max_value;              // Maximum value
    float current_value;          // Current value
    bool vertical;                // Orientation (true = vertical, false = horizontal)
    void (*on_change)(float);     // Value change callback
} WidgetSlider;
```

### Constructor

```c
WidgetSlider* new_slider(float min, float max, float value) {
    WidgetSlider* slider = (WidgetSlider*)malloc(sizeof(WidgetSlider));
    if (!slider) return NULL;

    slider->base = *new_widget(WIDGET_TYPE_SLIDER);
    slider->min_value = min;
    slider->max_value = max;
    slider->current_value = value;
    slider->vertical = false;
    slider->on_change = NULL;

    // Default slider style
    slider->base.style.draw_style = 
        WIDGET_DRAW_STYLE_SOLID_FILL | 
        WIDGET_DRAW_STYLE_ROUNDED_BORDER;
    slider->base.style.colors.primary = 0x8410;    // Gray background
    slider->base.style.colors.secondary = 0x07E0; // Green fill
    slider->base.style.border.radius = 3;
    slider->base.rect.size.height = 20;           // Default height

    return slider;
}
```

---

## Checkbox Widget

### Structure

```c
typedef struct {
    Widget base;                    // Base widget properties
    bool checked;                 // Check state (true = checked, false = unchecked)
    void (*on_toggle)(bool);      // Toggle state change callback
} WidgetCheckbox;
```

### Constructor

```c
WidgetCheckbox* new_checkbox(bool checked) {
    WidgetCheckbox* checkbox = (WidgetCheckbox*)malloc(sizeof(WidgetCheckbox));
    if (!checkbox) return NULL;

    checkbox->base = *new_widget(WIDGET_TYPE_CHECKBOX);
    checkbox->checked = checked;
    checkbox->on_toggle = NULL;

    // Default checkbox style
    checkbox->base.style.draw_style = 
        WIDGET_DRAW_STYLE_ROUNDED_BORDER | 
        WIDGET_DRAW_STYLE_SOLID_FILL;
    checkbox->base.style.colors.primary = 0xFFFF;  // White
    checkbox->base.style.border.radius = 3;
    checkbox->base.rect.size.width = 20;
    checkbox->base.rect.size.height = 20;

    return checkbox;
}
```

---

## Progress Bar Widget

### Structure

```c
typedef struct {
    Widget base;                    // Base widget properties
    float min_value;              // Minimum value
    float max_value;              // Maximum value
    float current_value;          // Current value
    bool vertical;                // Orientation
    Color bar_color;              // Progress bar color
} WidgetProgressBar;
```

### Constructor

```c
WidgetProgressBar* new_progress_bar(float min, float max, float value) {
    WidgetProgressBar* progress_bar = (WidgetProgressBar*)malloc(sizeof(WidgetProgressBar));
    if (!progress_bar) return NULL;

    progress_bar->base = *new_widget(WIDGET_TYPE_PROGRESS_BAR);
    progress_bar->min_value = min;
    progress_bar->max_value = max;
    progress_bar->current_value = value;
    progress_bar->vertical = false;
    progress_bar->bar_color = 0x07E0;  // Green

    // Default progress bar style
    progress_bar->base.style.draw_style = 
        WIDGET_DRAW_STYLE_SOLID_FILL | 
        WIDGET_DRAW_STYLE_ROUNDED_BORDER;
    progress_bar->base.style.colors.primary = 0x8410;    // Gray background
    progress_bar->base.style.border.radius = 2;
    progress_bar->base.rect.size.height = 10;           // Default height

    return progress_bar;
}
```

---

## Generic Widget Constructor

### Structure

The generic widget constructor creates a base widget of any type:

```c
Widget* new_widget(WIDGET_TYPE type) {
    Widget* widget = (Widget*)malloc(sizeof(Widget));
    if (!widget) return NULL;

    widget->type = type;
    widget->rect = (Rect){{0, 0}, {100, 50}};  // Default position and size
    widget->style.draw_style = STYLE_DEFAULT;
    widget->text.text[0] = '\0';
    widget->text.font.size = 12;
    widget->text.font.color = 0xFFFF;  // White
    widget->text.font.wrap = false;
    widget->parent = NULL;
    widget->children = NULL;
    widget->children_count = 0;
    widget->data = NULL;

    // Initialize default style
    widget->style.colors.primary = 0x0000;    // Black
    widget->style.colors.secondary = 0xFFFF;  // White
    widget->style.border.width = 1;
    widget->style.border.color = 0xFFFF;      // White
    widget->style.border.radius = 0;
    widget->style.effect.enabled = false;

    return widget;
}
```

---

## Child Management Functions

### Adding Children

```c
void widget_add_child(Widget* parent, Widget* child) {
    if (!parent || !child) return;

    parent->children_count++;
    parent->children = (Widget**)realloc(
        parent->children, 
        parent->children_count * sizeof(Widget*)
    );
    if (!parent->children) {
        parent->children_count--;  // Reallocation failed
        return;
    }
    parent->children[parent->children_count - 1] = child;
    child->parent = parent;
}
```

### Removing Children

```c
void widget_remove_child(Widget* parent, Widget* child) {
    if (!parent || !child) return;

    for (uint8_t i = 0; i < parent->children_count; i++) {
        if (parent->children[i] == child) {
            // Shift remaining elements
            for (uint8_t j = i; j < parent->children_count - 1; j++) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->children_count--;
            parent->children = (Widget**)realloc(
                parent->children, 
                parent->children_count * sizeof(Widget*)
            );
            child->parent = NULL;
            return;
        }
    }
}
```

### Freeing All Children

```c
void widget_free_children(Widget* parent) {
    if (!parent) return;

    for (uint8_t i = 0; i < parent->children_count; i++) {
        free_widget(parent->children[i]);
    }
    free(parent->children);
    parent->children = NULL;
    parent->children_count = 0;
}
```

---

## Memory Deallocation

### Free Widget Function

**IMPORTANT**: Uses manual memory management (NOT reference counting)

```c
void free_widget(Widget* widget) {
    if (!widget) return;

    // Recursively free children first
    widget_free_children(widget);

    // Type-specific deallocation
    switch (widget->type) {
        case WIDGET_TYPE_BUTTON:
            free((WidgetButton*)widget);
            break;
        case WIDGET_TYPE_LABEL:
            free((WidgetLabel*)widget);
            break;
        case WIDGET_TYPE_SLIDER:
            free((WidgetSlider*)widget);
            break;
        case WIDGET_TYPE_CHECKBOX:
            free((WidgetCheckbox*)widget);
            break;
        case WIDGET_TYPE_PROGRESS_BAR:
            free((WidgetProgressBar*)widget);
            break;
        default:
            free(widget);
            break;
    }
}
```

### Memory Management Macros

For Objective-C style memory management:

```c
// General release macro
#define RELEASE(obj) \
    if(obj) { free(obj); obj = NULL; }

// Widget-specific release macros
#define RELEASE_WIDGET(w) \
    if(w) { free_widget(w); w = NULL; }

#define RELEASE_BUTTON(b) \
    if(b) { free_widget(&(b)->base); b = NULL; }

#define RELEASE_LABEL(l) \
    if(l) { free_widget(&(l)->base); l = NULL; }

#define RELEASE_SLIDER(s) \
    if(s) { free_widget(&(s)->base); s = NULL; }
```

---

## Object Pooling for ESP8266

For better memory management on constrained devices:

```c
// Widget pool definition
#define MAX_WIDGET_POOL 50
#define WIDGET_POOL_SIZE (MAX_WIDGET_POOL * sizeof(Widget))

Widget widget_pool[MAX_WIDGET_POOL];
bool widget_pool_used[MAX_WIDGET_POOL] = {false};

// Allocate widget from pool
Widget* widget_pool_alloc(void) {
    for (int i = 0; i < MAX_WIDGET_POOL; i++) {
        if (!widget_pool_used[i]) {
            widget_pool_used[i] = true;
            memset(&widget_pool[i], 0, sizeof(Widget));
            return &widget_pool[i];
        }
    }
    return NULL;  // Pool exhausted
}

// Release widget to pool
void widget_pool_release(Widget* widget) {
    if (widget >= widget_pool && widget < widget_pool + MAX_WIDGET_POOL) {
        int index = (widget - widget_pool) / sizeof(Widget);
        widget_pool_used[index] = false;
    }
}

// Pool-based constructors
WidgetButton* new_button_pooled(void) {
    WidgetButton* button = (WidgetButton*)widget_pool_alloc();
    if (!button) return NULL;
    // Initialize button...
    return button;
}
```

---

## Default Style Assignments

Each widget type has appropriate default styles:

| Widget Type | Default Draw Style | Default Colors | Default Dimensions |
|-------------|-------------------|----------------|-------------------|
| Button | ROUNDED_BORDER + SOLID_FILL + DROP_SHADOW | Primary: Blue (0x001F) | Width: 100, Height: 50 |
| Label | TRANSPARENT_FILL | Primary: White (0xFFFF) | Auto-sized |
| Slider | SOLID_FILL + ROUNDED_BORDER | Primary: Gray (0x8410), Secondary: Green (0x07E0) | Height: 20 |
| Checkbox | ROUNDED_BORDER + SOLID_FILL | Primary: White (0xFFFF) | 20x20 |
| ProgressBar | SOLID_FILL + ROUNDED_BORDER | Primary: Gray (0x8410), Bar: Green (0x07E0) | Height: 10 |

---

## Usage Patterns

### Creating a Complete UI

```c
// Create parent view
Widget* root = new_widget(WIDGET_TYPE_VIEW);
root->rect.size.width = 320;
root->rect.size.height = 240;

// Add button
WidgetButton* btn = new_button();
strcpy(btn->base.text.text, "OK");
btn->base.rect.position.x = 100;
btn->base.rect.position.y = 100;
widget_add_child(root, &btn->base);

// Add label
WidgetLabel* lbl = new_label("Hello, World!");
lbl->base.rect.position.x = 50;
lbl->base.rect.position.y = 50;
widget_add_child(root, &lbl->base);

// Add slider
WidgetSlider* slider = new_slider(0.0f, 100.0f, 50.0f);
slider->base.rect.position.x = 20;
slider->base.rect.position.y = 150;
slider->base.rect.size.width = 200;
widget_add_child(root, &slider->base);

// Render
init_renderer();
draw_widget_tree(root);

// Cleanup (manual memory management)
RELEASE_WIDGET(root);  // Recursively frees all children
```

---

## ESP8266-Specific Considerations

### Memory Constraints

- **RAM Limit**: 80KB total
- **Avoid**: Dynamic allocation in loops
- **Prefer**: Object pooling for frequently created/destroyed widgets
- **Use**: Static buffers for text (MAX_TEXT_LENGTH = 512)

### Performance Optimizations

1. **Precompute** gradient and style calculations
2. **Cache** rendered widget bitmaps when possible
3. **Batch** draw operations to minimize TFT_eSPI calls
4. **Use** dirty flag system to only redraw changed widgets

### No Atomic Operations

Since ESP8266 lacks hardware atomic support:
- **Avoid** reference counting (requires atomic ops)
- **Use** manual memory management with clear ownership
- **Use** pool-based allocation with simple flags

---

## Cross-References

- **Widget Architecture**: See `01_WIDGET_ARCHITECTURE.md`
- **Constructor Patterns**: See `02_CONSTRUCTOR_PATTERNS.md`
- **Widget Types**: See `03_WIDGET_TYPES.md`
- **Draw Styles**: See `04_DRAW_STYLES.md`
- **Style System**: See `05_STYLE_SYSTEM.md`
- **Memory Management**: See `06_MEMORY_MANAGEMENT.md` and `docs/MEMORY_MANAGEMENT.md`
- **Rendering System**: See `07_RENDERING_SYSTEM.md`
- **Touch Handling**: See `08_TOUCH_HANDLING.md`

---

## File Location

Source implementations:
- `src/gui/widget.h` - Widget definitions and types
- `src/gui/widget.cpp` - Widget constructors and utilities
- `src/gui/widget_macros.h` - Constructor and memory management macros
- `src/gui/widget_pool.h` - Object pool declarations
- `src/gui/widget_pool.c` - Object pool implementation
