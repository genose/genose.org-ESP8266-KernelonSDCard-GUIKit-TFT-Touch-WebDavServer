# Widget Types (WIDGET_TYPE)

> Extracted from discussion_guikit.txt - Complete widget type system

## Overview

The widget type system defines all possible widget types in GUIKit. Multiple versions are provided, from minimal to comprehensive, to suit different project needs.

---

## Version 1: Minimal Enum

For projects that want to start with the essential widgets only:

```c
typedef enum {
    WIDGET_TYPE_VIEW = 1,      // Container
    WIDGET_TYPE_BUTTON,       // Button
    WIDGET_TYPE_LABEL,        // Static text
    WIDGET_TYPE_SLIDER,       // Slider
    WIDGET_TYPE_CHECKBOX,     // Checkbox
    WIDGET_TYPE_PROGRESS_BAR, // Progress bar
    WIDGET_TYPE_CUSTOM = 255, // Custom widget type
} WIDGET_TYPE;
```

---

## Version 2: Comprehensive Enum (Recommended)

Complete widget type system organized by categories:

```c
/**
 * @brief Widget types supported by GUIKit.
 * @note Values are organized by categories for better readability.
 *       Numeric values are explicit to avoid conflicts.
 */
typedef enum {
    // ===== Containers (Parent Widgets) =====
    WIDGET_TYPE_VIEW = 1,          ///< Generic container (can contain other widgets).
    WIDGET_TYPE_SCROLL_VIEW,       ///< Container with scrolling (vertical/horizontal).
    WIDGET_TYPE_GRID,              ///< Container with grid layout (rows/columns).
    WIDGET_TYPE_HBOX,              ///< Horizontal container (inline layout).
    WIDGET_TYPE_VBOX,              ///< Vertical container (column layout).

    // ===== Interactive Widgets =====
    WIDGET_TYPE_BUTTON = 10,       ///< Clickable button.
    WIDGET_TYPE_TOGGLE_BUTTON,     ///< Toggle button (ON/OFF).
    WIDGET_TYPE_CHECKBOX,          ///< Checkbox.
    WIDGET_TYPE_RADIO_BUTTON,      ///< Radio button (single choice in group).
    WIDGET_TYPE_SLIDER,            ///< Slider control (for numeric values).
    WIDGET_TYPE_KNOB,              ///< Rotary knob (for circular adjustments).
    WIDGET_TYPE_DROPDOWN,          ///< Dropdown list.
    WIDGET_TYPE_TEXT_INPUT,        ///< Text input field.

    // ===== Display Widgets =====
    WIDGET_TYPE_LABEL = 20,        ///< Static text.
    WIDGET_TYPE_IMAGE,             ///< Image (bitmap, icon, etc.).
    WIDGET_TYPE_PROGRESS_BAR,      ///< Progress bar.
    WIDGET_TYPE_CANVAS,            ///< Custom drawing area.
    WIDGET_TYPE_CHART,             ///< Chart (line, bar, pie, etc.).

    // ===== Notification Widgets =====
    WIDGET_TYPE_ALERT = 30,        ///< Alert popup.
    WIDGET_TYPE_TOOLTIP,           ///< Tooltip (on hover).
    WIDGET_TYPE_NOTIFICATION,      ///< Temporary notification (toast).

    // ===== Advanced Widgets =====
    WIDGET_TYPE_TAB = 40,          ///< Tab (in a TAB_GROUP).
    WIDGET_TYPE_TAB_GROUP,         ///< Tab group container.
    WIDGET_TYPE_MENU,              ///< Menu (for complex interfaces).
    WIDGET_TYPE_MENU_ITEM,        ///< Menu item.
    WIDGET_TYPE_CUSTOM = 255,      ///< Custom widget (for user extensions).
} WIDGET_TYPE;
```

---

## Category Explanations

### Containers

Widgets that can contain other widgets:
- **VIEW**: Generic container, base for most UIs
- **SCROLL_VIEW**: Container with scrollable content
- **GRID**: Layout container with rows and columns
- **HBOX**: Horizontal box layout
- **VBOX**: Vertical box layout

### Interactive Widgets

Widgets that respond to user input (touch/click):
- **BUTTON**: Standard clickable button
- **TOGGLE_BUTTON**: Button that maintains on/off state
- **CHECKBOX**: Checkbox for boolean selection
- **RADIO_BUTTON**: Radio button for single selection from group
- **SLIDER**: Slider for continuous value selection
- **KNOB**: Circular control for adjustment
- **DROPDOWN**: Dropdown list for selection
- **TEXT_INPUT**: Text entry field

### Display Widgets

Widgets for displaying data:
- **LABEL**: Static text display
- **IMAGE**: Image/bitmap display
- **PROGRESS_BAR**: Visual progress indicator
- **CANVAS**: Custom drawing surface
- **CHART**: Data visualization (graphs, charts)

### Notification Widgets

Widgets for user feedback:
- **ALERT**: Modal alert dialog
- **TOOLTIP**: Temporary info display
- **NOTIFICATION**: Non-blocking notification (toast)

### Advanced Widgets

Complex widgets for sophisticated interfaces:
- **TAB**: Individual tab page
- **TAB_GROUP**: Container for tabs
- **MENU**: Menu container
- **MENU_ITEM**: Individual menu item
- **CUSTOM**: User-defined widget type

---

## Usage Examples with Constructor

```c
// Create a button
WidgetButton* button = new_widget(WIDGET_TYPE_BUTTON);

// Create a container view
Widget* container = new_widget(WIDGET_TYPE_VIEW);

// Create a slider
Widget* slider = new_widget(WIDGET_TYPE_SLIDER, (Rect){10, 50, 200, 20});

// Create a progress bar
Widget* progress_bar = new_widget(WIDGET_TYPE_PROGRESS_BAR, (Rect){10, 100, 150, 15});
```

---

## Best Practices for the Enum

### Explicit Values

Each category starts at a multiple of 10 (1, 10, 20, 30, 40) to facilitate adding new types without conflicts.

Example: Add `WIDGET_TYPE_SWITCH` between `WIDGET_TYPE_BUTTON` and `WIDGET_TYPE_CHECKBOX`:
```c
WIDGET_TYPE_SWITCH = 11,  // Between BUTTON (10) and CHECKBOX (12)
```

### Clear Naming

- Use descriptive and consistent names (e.g., `TOGGLE_BUTTON` not `TOGGLE`)
- Avoid ambiguous abbreviations
- Use uppercase with underscores for enum values

### Documentation

Add comments for each type to clarify usage:
```c
WIDGET_TYPE_SCROLL_VIEW,       ///< Container with scrolling (vertical/horizontal).
```

### Custom Type

`WIDGET_TYPE_CUSTOM = 255` allows extending GUIKit with user-defined widgets without modifying the enum.

---

## Extensions: Sub-Types

For widgets with multiple variants, use nested enums:

```c
typedef enum {
    BUTTON_TYPE_PUSH = 1,
    BUTTON_TYPE_TOGGLE,
} BUTTON_TYPE;

// In the button structure:
typedef struct {
    Widget base;
    BUTTON_TYPE button_type;  // Push or toggle
    bool pressed;
    void (*on_click)(void);
} WidgetButton;
```

---

## Extensions: Widget Flags

For additional widget states, use flag enums:

```c
typedef enum {
    WIDGET_FLAG_VISIBLE = 1 << 0,   ///< Widget is visible
    WIDGET_FLAG_ENABLED = 1 << 1,   ///< Widget is enabled (interactive)
    WIDGET_FLAG_FOCUSED = 1 << 2,   ///< Widget has focus
    WIDGET_FLAG_DIRTY = 1 << 3,     ///< Widget needs redrawing
} WIDGET_FLAGS;

// Usage in widget structure:
typedef struct {
    WIDGET_TYPE type;
    uint8_t flags;  // Combination of WIDGET_FLAGS
    // ... other fields
} Widget;

// Check flags:
if (widget->flags & WIDGET_FLAG_VISIBLE) {
    // Widget is visible
}

// Set flags:
widget->flags |= WIDGET_FLAG_DIRTY;

// Clear flags:
widget->flags &= ~WIDGET_FLAG_DIRTY;
```

---

## Summary Table

| Category | Types | Count | Starting Value |
|----------|-------|-------|----------------|
| Containers | VIEW, SCROLL_VIEW, GRID, HBOX, VBOX | 5 | 1 |
| Interactive | BUTTON, TOGGLE_BUTTON, CHECKBOX, RADIO_BUTTON, SLIDER, KNOB, DROPDOWN, TEXT_INPUT | 8 | 10 |
| Display | LABEL, IMAGE, PROGRESS_BAR, CANVAS, CHART | 5 | 20 |
| Notification | ALERT, TOOLTIP, NOTIFICATION | 3 | 30 |
| Advanced | TAB, TAB_GROUP, MENU, MENU_ITEM, CUSTOM | 5 | 40 |
| **Total** | | **26** | - |

---

*Source: Extracted from discussion_guikit.txt, lines 1475-1720*
*Documentation organized by Mistral Vibe*
