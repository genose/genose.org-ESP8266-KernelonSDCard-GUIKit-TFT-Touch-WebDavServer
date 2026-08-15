# Gradient Color with Union-Based Memory Optimization

> Extracted from implementation - Union-based gradient property for ESP8266 GUIKit

## Overview

This document describes the **union-based gradient implementation** for GUIKit widgets, which optimizes memory usage by only storing the data needed for the active gradient type. This design is specifically tailored for **ESP8266 memory constraints** (80KB RAM limit) and defaults to **RGB565** color format.

### Key Features

- **Union-based storage** - only allocates memory for the active gradient type
- **RGB565 default** - 16-bit color (5 red, 6 green, 5 blue) as the baseline
- **Type-safe accessors** - all access is through inline functions that check gradient type
- **Macro-based syntax** - Objective-C style convenience macros
- **Fixed-point math** - no floating-point operations for ESP8266 compatibility
- **Memory savings** - ~50% average reduction vs. storing all fields

---

## Memory Optimization

### Before (All Fields Always)
```c
typedef struct {
    bool gradient;            // 1 byte
    Color color;             // 2 bytes
    Color end_color;         // 2 bytes
    int16_t angle;           // 2 bytes
    Point center;            // 4 bytes
    uint16_t radius;         // 2 bytes
    int16_t start_angle;     // 2 bytes
    int16_t end_angle;       // 2 bytes
    // + padding = ~18 bytes ALWAYS
} GradientData;
```

### After (Union-Based)
```c
typedef struct Gradient {
    GRADIENT_TYPE type;      // 1 byte
    Color color;             // 2 bytes
    union {
        struct { uint8_t _padding[2]; } none;              // +2 = 5 bytes total
        struct { Color end_color; } simple;                // +2 = 5 bytes total
        struct { Color end_color; uint8_t _padding[2]; } horizontal; // +4 = 7 bytes
        struct { Color end_color; int16_t angle; uint8_t _padding[2]; } linear; // +8 = 11 bytes
        struct { Color end_color; Point center; uint16_t radius; } radial; // +10 = 13 bytes
        struct { Color end_color; Point center; int16_t start_angle; int16_t end_angle; } conic; // +12 = 15 bytes
    } data;
} Gradient;
```

### Memory Comparison

| **Gradient Type** | **Union-Based** | **Savings vs Full** |
|-------------------|-----------------|----------------------|
| GRADIENT_NONE/SOLID | 5 bytes | 72% |
| GRADIENT_HORIZONTAL/VERTICAL/DIAGONAL | 7 bytes | 61% |
| GRADIENT_LINEAR | 11 bytes | 39% |
| GRADIENT_RADIAL | 13 bytes | 28% |
| GRADIENT_CONIC | 15 bytes | 17% |
| **Average** | **~9 bytes** | **~44%** |

For 50 widgets with mixed gradient types (50% solid, 30% simple, 20% complex):
- **Before**: 50 × 18 = 900 bytes
- **After**: ~450 bytes
- **Savings**: **450 bytes (50%)**

---

## Data Structures

### 1. Gradient Type Enum

```c
typedef enum {
    GRADIENT_NONE = 0,      // No gradient (solid color only)
    GRADIENT_SOLID,        // Solid color (explicit, same as NONE)
    GRADIENT_LINEAR,       // Linear gradient (two colors, custom angle)
    GRADIENT_RADIAL,       // Radial gradient (center, radius, two colors)
    GRADIENT_CONIC,        // Conic gradient (center, start/end angles, two colors)
    GRADIENT_HORIZONTAL,   // Horizontal linear gradient (simplified)
    GRADIENT_VERTICAL,     // Vertical linear gradient (simplified)
    GRADIENT_DIAGONAL      // Diagonal linear gradient (45 degrees, simplified)
} GRADIENT_TYPE;
```

### 2. Supporting Types

```c
// RGB565 color (16-bit: 5 red, 6 green, 5 blue)
typedef uint16_t Color;

// 2D point
typedef struct {
    uint16_t x;
    uint16_t y;
} Point;
```

### 3. Union-Based Gradient Structure

```c
typedef struct Gradient {
    GRADIENT_TYPE type;      // Type of gradient (tag for union)
    Color color;             // Primary/fallback color (RGB565)
    
    union {
        // No gradient / solid color - minimal storage
        struct { uint8_t _padding[2]; } none;
        
        // Solid color only (explicit)
        struct { uint8_t _padding[2]; } solid;
        
        // Simplified gradients (horizontal, vertical, diagonal)
        struct { Color end_color; } simple;
        
        // Horizontal gradient
        struct { Color end_color; uint8_t _padding[2]; } horizontal;
        
        // Vertical gradient
        struct { Color end_color; uint8_t _padding[2]; } vertical;
        
        // Diagonal gradient
        struct { Color end_color; uint8_t _padding[2]; } diagonal;
        
        // Linear gradient with custom angle
        struct {
            Color end_color;       // End color
            int16_t angle;         // Angle in degrees (0-360)
            uint8_t _padding[2];
        } linear;
        
        // Radial gradient
        struct {
            Color end_color;       // Outer color
            Point center;         // Center point
            uint16_t radius;      // Radius
        } radial;
        
        // Conic gradient
        struct {
            Color end_color;       // End color
            Point center;         // Center point
            int16_t start_angle;   // Start angle in degrees
            int16_t end_angle;     // End angle in degrees
        } conic;
        
        // Multi-color gradient (future expansion)
        struct {
            Color colors[4];      // Up to 4 colors
            uint8_t color_count;  // Number of colors (1-4)
            int16_t angle;         // Angle for linear gradients
            uint8_t _padding[1];
        } multi;
    } data;
} Gradient;
```

### 4. Integration with Widget Background

```c
typedef struct {
    // Gradient support (union-based)
    Gradient gradient;       // Gradient data with union optimization
    
    // Legacy support (optional, for backward compatibility)
    // bool has_gradient;    // Use IS_GRADIENT(&gradient) instead
    // Color gradient_color; // Use gradient_getEndColor(&gradient) instead
} Background;
```

---

## API Reference

### Type and Check Macros

| **Macro** | **Description** | **Example** |
|-----------|-----------------|-------------|
| `GET_GRADIENT_TYPE(g)` | Get gradient type | `GRADIENT_TYPE t = GET_GRADIENT_TYPE(g);` |
| `IS_GRADIENT_ENABLED(g)` | Check if gradient is enabled | `if (IS_GRADIENT_ENABLED(g)) { ... }` |
| `IS_SOLID_COLOR(g)` | Check if solid color only | `if (IS_SOLID_COLOR(g)) { ... }` |
| `IS_LINEAR(g)` | Check if linear gradient | `if (IS_LINEAR(g)) { ... }` |
| `IS_RADIAL(g)` | Check if radial gradient | `if (IS_RADIAL(g)) { ... }` |
| `IS_CONIC(g)` | Check if conic gradient | `if (IS_CONIC(g)) { ... }` |

### Color Macros

| **Macro** | **Description** | **Example** |
|-----------|-----------------|-------------|
| `SET_COLOR(g, c)` | Set primary color | `SET_COLOR(&bg.gradient, COLOR_RED);` |
| `GET_COLOR(g)` | Get primary color | `Color c = GET_COLOR(&bg.gradient);` |
| `SET_END_COLOR(g, c)` | Set end color | `SET_END_COLOR(&bg.gradient, COLOR_BLUE);` |
| `GET_END_COLOR(g)` | Get end color | `Color e = GET_END_COLOR(&bg.gradient);` |

### Gradient Setup Macros

| **Macro** | **Description** | **Example** |
|-----------|-----------------|-------------|
| `SET_SOLID(g, c)` | Set to solid color | `SET_SOLID(&bg.gradient, COLOR_GREEN);` |
| `SET_HGRADIENT(g, s, e)` | Horizontal gradient | `SET_HGRADIENT(&bg.gradient, COLOR_RED, COLOR_BLUE);` |
| `SET_VGRADIENT(g, s, e)` | Vertical gradient | `SET_VGRADIENT(&bg.gradient, COLOR_RED, COLOR_BLUE);` |
| `SET_DGRADIENT(g, s, e)` | Diagonal gradient | `SET_DGRADIENT(&bg.gradient, COLOR_RED, COLOR_BLUE);` |
| `SET_LGRADIENT(g, s, e, a)` | Linear with angle | `SET_LGRADIENT(&bg.gradient, COLOR_RED, COLOR_BLUE, 45);` |
| `SET_RGRADIENT(g, s, e, cx, cy, r)` | Radial gradient | `SET_RGRADIENT(&bg.gradient, COLOR_RED, COLOR_BLUE, 100, 100, 50);` |
| `SET_CGRADIENT(g, s, e, cx, cy, sa, ea)` | Conic gradient | `SET_CGRADIENT(&bg.gradient, COLOR_RED, COLOR_BLUE, 100, 100, 0, 180);` |

### Parameter Macros

| **Macro** | **Description** | **Example** |
|-----------|-----------------|-------------|
| `SET_ANGLE(g, a)` | Set gradient angle | `SET_ANGLE(&bg.gradient, 45);` |
| `GET_ANGLE(g)` | Get gradient angle | `int16_t a = GET_ANGLE(&bg.gradient);` |
| `SET_CENTER(g, x, y)` | Set center point | `SET_CENTER(&bg.gradient, 100, 100);` |
| `GET_CENTER_X(g)` | Get center X | `uint16_t x = GET_CENTER_X(&bg.gradient);` |
| `GET_CENTER_Y(g)` | Get center Y | `uint16_t y = GET_CENTER_Y(&bg.gradient);` |
| `SET_RADIUS(g, r)` | Set radius | `SET_RADIUS(&bg.gradient, 50);` |
| `GET_RADIUS(g)` | Get radius | `uint16_t r = GET_RADIUS(&bg.gradient);` |

### Utility Macros

| **Macro** | **Description** | **Example** |
|-----------|-----------------|-------------|
| `RESET_GRADIENT(g, c)` | Reset to solid color | `RESET_GRADIENT(&bg.gradient, COLOR_WHITE);` |
| `COPY_GRADIENT(d, s)` | Copy gradient | `COPY_GRADIENT(&dest, &src);` |

### Color Constants (RGB565)

| **Constant** | **Value** | **Color** |
|--------------|-----------|-----------|
| `COLOR_BLACK` | 0x0000 | Black |
| `COLOR_WHITE` | 0xFFFF | White |
| `COLOR_RED` | 0xF800 | Red |
| `COLOR_GREEN` | 0x07E0 | Green |
| `COLOR_BLUE` | 0x001F | Blue |
| `COLOR_YELLOW` | 0xFFE0 | Yellow |
| `COLOR_CYAN` | 0x07FF | Cyan |
| `COLOR_MAGENTA` | 0xF81F | Magenta |
| `COLOR_GRAY` | 0x8410 | Gray |
| `COLOR_SILVER` | 0xC618 | Silver |
| `COLOR_ORANGE` | 0xFD20 | Orange |
| `COLOR_PURPLE` | 0x8010 | Purple |

### Color Creation Macro

```c
// Create RGB565 color from R, G, B components (0-255)
#define RGB565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

// Examples:
Color red = RGB565(255, 0, 0);    // Pure red
Color green = RGB565(0, 255, 0);  // Pure green
Color blue = RGB565(0, 0, 255);   // Pure blue
Color orange = RGB565(255, 165, 0); // Orange
```

---

## Function Reference

### Core Functions

#### `void gradient_setType(Gradient* g, GRADIENT_TYPE type)`

Set gradient type and initialize the appropriate union member.

**Parameters:**
- `g` - Gradient pointer
- `type` - GRADIENT_TYPE enum value

**Example:**
```c
gradient_setType(&widget->background.gradient, GRADIENT_LINEAR);
```

#### `void gradient_setSolid(Gradient* g, Color color)`

Set gradient to solid color (no gradient).

**Parameters:**
- `g` - Gradient pointer
- `color` - RGB565 color

**Example:**
```c
gradient_setSolid(&widget->background.gradient, COLOR_RED);
```

#### `void gradient_setHorizontal(Gradient* g, Color start_color, Color end_color)`

Set gradient to horizontal linear gradient.

**Parameters:**
- `g` - Gradient pointer
- `start_color` - Start color (left side, RGB565)
- `end_color` - End color (right side, RGB565)

**Example:**
```c
gradient_setHorizontal(&widget->background.gradient, COLOR_RED, COLOR_BLUE);
```

#### `void gradient_setVertical(Gradient* g, Color start_color, Color end_color)`

Set gradient to vertical linear gradient.

**Parameters:**
- `g` - Gradient pointer
- `start_color` - Start color (top, RGB565)
- `end_color` - End color (bottom, RGB565)

#### `void gradient_setDiagonal(Gradient* g, Color start_color, Color end_color)`

Set gradient to diagonal linear gradient (45 degrees).

#### `void gradient_setLinear(Gradient* g, Color start_color, Color end_color, int16_t angle)`

Set gradient to linear with custom angle (0-360 degrees).

**Parameters:**
- `g` - Gradient pointer
- `start_color` - Start color (RGB565)
- `end_color` - End color (RGB565)
- `angle` - Angle in degrees (0-360)

**Example:**
```c
// Gradient from top-left to bottom-right at 45 degrees
gradient_setLinear(&widget->background.gradient, COLOR_RED, COLOR_BLUE, 45);
```

#### `void gradient_setRadial(Gradient* g, Color start_color, Color end_color, uint16_t center_x, uint16_t center_y, uint16_t radius)`

Set gradient to radial gradient.

**Parameters:**
- `g` - Gradient pointer
- `start_color` - Center color (RGB565)
- `end_color` - Outer color (RGB565)
- `center_x` - Center X coordinate
- `center_y` - Center Y coordinate
- `radius` - Radius of gradient

**Example:**
```c
// Radial gradient centered at (100, 100) with radius 50
gradient_setRadial(&widget->background.gradient, COLOR_YELLOW, COLOR_RED, 100, 100, 50);
```

#### `void gradient_setConic(Gradient* g, Color start_color, Color end_color, uint16_t center_x, uint16_t center_y, int16_t start_angle, int16_t end_angle)`

Set gradient to conic (pie slice) gradient.

**Parameters:**
- `g` - Gradient pointer
- `start_color` - Start angle color (RGB565)
- `end_color` - End angle color (RGB565)
- `center_x` - Center X coordinate
- `center_y` - Center Y coordinate
- `start_angle` - Start angle in degrees (0-360)
- `end_angle` - End angle in degrees (0-360)

**Example:**
```c
// Conic gradient from 0 to 180 degrees (top half)
gradient_setConic(&widget->background.gradient, COLOR_RED, COLOR_BLUE, 100, 100, 0, 180);
```

#### `void gradient_copy(Gradient* dest, const Gradient* src)`

Copy gradient from one to another.

#### `void gradient_reset(Gradient* g, Color color)`

Reset gradient to solid color.

### Inline Accessor Functions

#### `Color gradient_getColor(const Gradient* g)`
#### `void gradient_setColor(Gradient* g, Color color)`

Get/set primary color (works for all gradient types).

#### `Color gradient_getEndColor(const Gradient* g)`
#### `void gradient_setEndColor(Gradient* g, Color color)`

Get/set end color (for gradient types, returns primary for non-gradients).

#### `int16_t gradient_getAngle(const Gradient* g)`
#### `void gradient_setAngle(Gradient* g, int16_t angle)`

Get/set angle (for linear/conic gradients).

#### `void gradient_getCenter(const Gradient* g, uint16_t* x, uint16_t* y)`
#### `void gradient_setCenter(Gradient* g, uint16_t x, uint16_t y)`

Get/set center point (for radial/conic gradients).

#### `uint16_t gradient_getRadius(const Gradient* g)`
#### `void gradient_setRadius(Gradient* g, uint16_t radius)`

Get/set radius (for radial gradients).

#### `int16_t gradient_getStartAngle(const Gradient* g)`
#### `void gradient_setStartAngle(Gradient* g, int16_t angle)`
#### `int16_t gradient_getEndAngle(const Gradient* g)`
#### `void gradient_setEndAngle(Gradient* g, int16_t angle)`

Get/set start/end angles (for conic gradients).

### Rendering Functions

#### `Color gradient_interpolate(Color color1, Color color2, uint16_t ratio)`

Interpolate between two colors using fixed-point arithmetic.

**Parameters:**
- `color1` - First color (RGB565)
- `color2` - Second color (RGB565)
- `ratio` - Interpolation ratio (0-256, fixed point 8.8)

**Returns:** Interpolated color (RGB565)

#### `Color gradient_getColorAt(const Gradient* g, uint16_t x, uint16_t y, uint16_t width, uint16_t height)`

Get the color at a specific position within a gradient.

**Parameters:**
- `g` - Gradient pointer
- `x` - X position (0 to width-1)
- `y` - Y position (0 to height-1)
- `width` - Total width of area
- `height` - Total height of area

**Returns:** Color at position (RGB565)

**Example:**
```c
// Get color at position (50, 100) in a 240x320 widget
Color c = gradient_getColorAt(&widget->background.gradient, 50, 100, 240, 320);
```

---

## Usage Examples

### Example 1: Solid Color (Default)

```c
#include "widget.h"
#include "widget_gradient.h"

void create_solid_button() {
    Widget* button = new_widget(WIDGET_TYPE_BUTTON);
    
    // Set solid red background
    SET_SOLID(&button->background.gradient, COLOR_RED);
    // or: gradient_setSolid(&button->background.gradient, COLOR_RED);
    
    // Alternative: use RGB565 macro
    SET_SOLID(&button->background.gradient, RGB565(255, 0, 0));
}
```

### Example 2: Horizontal Gradient

```c
void create_horizontal_gradient() {
    Widget* view = new_widget(WIDGET_TYPE_VIEW);
    
    // Red to blue horizontal gradient
    SET_HGRADIENT(&view->background.gradient, COLOR_RED, COLOR_BLUE);
    // or: gradient_setHorizontal(&view->background.gradient, COLOR_RED, COLOR_BLUE);
    
    view->size.width = 240;
    view->size.height = 320;
}
```

### Example 3: Vertical Gradient

```c
void create_vertical_gradient() {
    Widget* panel = new_widget(WIDGET_TYPE_VIEW);
    
    // White to gray vertical gradient (top to bottom)
    SET_VGRADIENT(&panel->background.gradient, COLOR_WHITE, COLOR_GRAY);
    
    panel->size.width = 240;
    panel->size.height = 320;
}
```

### Example 4: Diagonal Gradient

```c
void create_diagonal_gradient() {
    Widget* background = new_widget(WIDGET_TYPE_VIEW);
    
    // Diagonal gradient from top-left to bottom-right
    SET_DGRADIENT(&background->background.gradient, COLOR_BLUE, COLOR_YELLOW);
    
    background->size.width = 240;
    background->size.height = 320;
}
```

### Example 5: Linear Gradient with Custom Angle

```c
void create_custom_linear_gradient() {
    Widget* view = new_widget(WIDGET_TYPE_VIEW);
    
    // Linear gradient at 30 degrees
    SET_LGRADIENT(&view->background.gradient, COLOR_GREEN, COLOR_PURPLE, 30);
    
    view->size.width = 240;
    view->size.height = 320;
}
```

### Example 6: Radial Gradient

```c
void create_radial_gradient() {
    Widget* button = new_widget(WIDGET_TYPE_BUTTON);
    
    // Radial gradient centered in button
    SET_RGRADIENT(&button->background.gradient, 
                  COLOR_YELLOW, COLOR_RED, 
                  button->size.width / 2, button->size.height / 2,
                  button->size.width / 2);
    
    button->size.width = 100;
    button->size.height = 50;
}
```

### Example 7: Conic Gradient (Pie Slice)

```c
void create_conic_gradient() {
    Widget* gauge = new_widget(WIDGET_TYPE_VIEW);
    
    // Conic gradient for gauge (0-180 degrees = semicircle)
    SET_CGRADIENT(&gauge->background.gradient,
                  COLOR_RED, COLOR_GREEN,
                  120, 120,    // Center at (120, 120)
                  0, 180);     // From 0 to 180 degrees
    
    gauge->size.width = 240;
    gauge->size.height = 240;
}
```

### Example 8: Dynamic Gradient Changes

```c
void update_gradient_based_on_state(Widget* widget, bool active) {
    if (active) {
        // Active state: blue gradient
        SET_HGRADIENT(&widget->background.gradient, COLOR_BLUE, COLOR_CYAN);
    } else {
        // Inactive state: gray solid
        SET_SOLID(&widget->background.gradient, COLOR_GRAY);
    }
}
```

### Example 9: Get Color at Position

```c
void render_gradient_pixel(Widget* widget, uint16_t x, uint16_t y) {
    // Get color at position (x, y) within widget
    Color c = gradient_getColorAt(&widget->background.gradient,
                                  x, y,
                                  widget->size.width,
                                  widget->size.height);
    
    // Render pixel with color c
    tft.drawPixel(widget->position.x + x, widget->position.y + y, c);
}
```

### Example 10: Full Widget Rendering with Gradient

```c
void render_widget_with_gradient(Widget* widget) {
    if (!widget->visible) return;
    
    // Check if widget has gradient background
    if (IS_GRADIENT_ENABLED(&widget->background.gradient)) {
        // Render gradient background
        for (uint16_t y = 0; y < widget->size.height; y++) {
            for (uint16_t x = 0; x < widget->size.width; x++) {
                Color c = gradient_getColorAt(&widget->background.gradient,
                                              x, y,
                                              widget->size.width,
                                              widget->size.height);
                tft.drawPixel(widget->position.x + x,
                             widget->position.y + y,
                             c);
            }
        }
    } else {
        // Render solid background
        Color bg_color = GET_COLOR(&widget->background.gradient);
        tft.fillRect(widget->position.x, widget->position.y,
                    widget->size.width, widget->size.height,
                    bg_color);
    }
    
    // Render children...
}
```

---

## Implementation Details

### Memory Layout

The union ensures that only the necessary data is stored:

```
GRADIENT_NONE/SOLID (5 bytes):
  [type: 1][color: 2][none._padding: 2]
  Total: 5 bytes

GRADIENT_HORIZONTAL/VERTICAL/DIAGONAL (7 bytes):
  [type: 1][color: 2][horizontal.end_color: 2][horizontal._padding: 2]
  Total: 7 bytes

GRADIENT_LINEAR (11 bytes):
  [type: 1][color: 2][linear.end_color: 2][linear.angle: 2][linear._padding: 2]
  Total: 11 bytes

GRADIENT_RADIAL (13 bytes):
  [type: 1][color: 2][radial.end_color: 2][radial.center: 4][radial.radius: 2]
  Total: 13 bytes

GRADIENT_CONIC (15 bytes):
  [type: 1][color: 2][conic.end_color: 2][conic.center: 4][conic.start_angle: 2][conic.end_angle: 2]
  Total: 15 bytes
```

### Type Safety

All accessor functions check the gradient type before accessing the union:

```c
static inline Color gradient_getEndColor(const Gradient* g) {
    if (!IS_GRADIENT(g)) {
        return g->color;  // Return primary color if not a gradient
    }
    
    switch (g->type) {
        case GRADIENT_LINEAR: return g->data.linear.end_color;
        case GRADIENT_HORIZONTAL: return g->data.horizontal.end_color;
        // ... handle all types
        default: return g->color;
    }
}
```

### Performance Optimizations

1. **Inline Functions**: All getters are `static inline` - no function call overhead
2. **Fixed-Point Math**: No floating-point operations (critical for ESP8266)
3. **Macro Alternatives**: Macros available for maximum performance
4. **Lookup Tables**: Approximation functions use lookup tables where possible
5. **Early Returns**: Checks for non-gradient cases first

### Fixed-Point Arithmetic

The implementation uses fixed-point arithmetic (8.8 format) for color interpolation:

```c
Color gradient_interpolate(Color color1, Color color2, uint16_t ratio) {
    // ratio is 0-256 (8.8 fixed point)
    uint8_t r1 = (color1 >> 11) & 0x1F;
    uint8_t g1 = (color1 >> 5) & 0x3F;
    uint8_t b1 = color1 & 0x1F;
    
    uint8_t r2 = (color2 >> 11) & 0x1F;
    uint8_t g2 = (color2 >> 5) & 0x3F;
    uint8_t b2 = color2 & 0x1F;
    
    // Fixed-point interpolation: r = r1 + (r2 - r1) * ratio / 256
    uint8_t r = r1 + (((int16_t)(r2 - r1) * ratio) >> 8);
    uint8_t g = g1 + (((int16_t)(g2 - g1) * ratio) >> 8);
    uint8_t b = b1 + (((int16_t)(b2 - b1) * ratio) >> 8);
    
    return (r << 11) | (g << 5) | b;
}
```

---

## Integration with Existing Widgets

### Updated Background Structure

```c
// Old structure (wasteful):
typedef struct {
    uint16_t color;           // Background color
    bool gradient;            // Gradient enabled flag
    uint16_t gradient_color;  // Secondary color for gradient
} Background;

// New structure (optimized):
typedef struct {
    Gradient gradient;       // Union-based gradient with primary color
} Background;
```

### Widget Constructor Updates

```c
Widget* new_widget(WIDGET_TYPE type) {
    Widget* widget = widget_pool_alloc(type);
    if (!widget) return NULL;
    
    widget->type = type;
    
    // Initialize gradient to solid color (default)
    gradient_setSolid(&widget->background.gradient, COLOR_WHITE);
    
    // ... other initialization
    
    return widget;
}
```

### Backward Compatibility

For code using the old gradient system:

```c
// Old way:
widget->background.gradient = true;
widget->background.color = COLOR_RED;
widget->background.gradient_color = COLOR_BLUE;

// New way (equivalent):
SET_HGRADIENT(&widget->background.gradient, COLOR_RED, COLOR_BLUE);

// Old check:
if (widget->background.gradient) { ... }

// New check:
if (IS_GRADIENT_ENABLED(&widget->background.gradient)) { ... }

// Old access:
Color c = widget->background.gradient_color;

// New access:
Color c = GET_END_COLOR(&widget->background.gradient);
```

---

## Best Practices

### Do This ✅

```c
// Use macros for common operations
SET_HGRADIENT(&bg.gradient, COLOR_RED, COLOR_BLUE);
SET_LGRADIENT(&bg.gradient, COLOR_RED, COLOR_BLUE, 45);

// Check gradient type before accessing specific data
if (IS_RADIAL(&bg.gradient)) {
    uint16_t r = GET_RADIUS(&bg.gradient);
}

// Use RGB565 macro for color creation
Color custom = RGB565(200, 100, 50);

// Use inline accessors for maximum performance
Color c = gradient_getColor(&bg.gradient);

// Reset when no longer needed
RESET_GRADIENT(&bg.gradient, COLOR_WHITE);
```

### Don't Do This ❌

```c
// DON'T access union members directly
bg.gradient.data.radial.radius = 100;  // UNSAFE!

// DON'T use with non-gradient widgets
SET_HGRADIENT(&non_widget.gradient, ...);  // Wrong type

// DON'T forget to check type before accessing
uint16_t r = GET_RADIUS(&bg.gradient);  // Returns 0 if not radial
if (IS_RADIAL(&bg.gradient)) {
    uint16_t r = GET_RADIUS(&bg.gradient);  // Safe
}

// DON'T use floating-point colors
Color c = some_float_color;  // Not RGB565
```

---

## RGB565 Color Reference

### Format

RGB565 is a 16-bit color format:
- **Bits 15-11**: Red (5 bits, 0-31)
- **Bits 10-5**: Green (6 bits, 0-63)
- **Bits 4-0**: Blue (5 bits, 0-31)

### Color Creation

```c
// From R, G, B (0-255):
#define RGB565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

// From separate components:
Color color = (red_5bit << 11) | (green_6bit << 5) | blue_5bit;
```

### Extracting Components

```c
uint8_t r = (color >> 11) & 0x1F;    // 0-31
uint8_t g = (color >> 5) & 0x3F;    // 0-63
uint8_t b = color & 0x1F;          // 0-31

// To 0-255 range:
uint8_t r8 = (r * 255) / 31;      // Scale 0-31 to 0-255
uint8_t g8 = (g * 255) / 63;      // Scale 0-63 to 0-255
uint8_t b8 = (b * 255) / 31;      // Scale 0-31 to 0-255
```

---

## File Locations

| **File** | **Path** | **Description** |
|----------|----------|-----------------|
| Header | `src/gui/widget_gradient.h` | Type definitions, macros, inline functions |
| Implementation | `src/gui/widget_gradient.c` | Function implementations and rendering helpers |
| Documentation | `docs/discussion_analysis/17_GRADIENT_UNION.md` | This file |

---

## Cross-References

- **Widget Architecture**: See `01_WIDGET_ARCHITECTURE.md` for base widget structure
- **Style System**: See `05_STYLE_SYSTEM.md` for style properties
- **Memory Management**: See `docs/MEMORY_MANAGEMENT.md` for memory optimization strategies
- **Scrollable Union**: See `16_SCROLLABLE_UNION.md` for similar union-based optimization
- **Rendering System**: See `07_RENDERING_SYSTEM.md` for rendering implementation

---

## Summary

| **Feature** | **Implementation** | **Benefit** |
|-------------|-------------------|-------------|
| Union Storage | Tagged union | Memory efficient (5-15 bytes) |
| Type Safety | Inline accessors | Prevents invalid access |
| Performance | Inline functions + macros | Zero function call overhead |
| API Style | Macros + functions | Flexible usage |
| Default Type | RGB565 | ESP8266 compatible |
| Memory Savings | ~44% average | Critical for ESP8266 |

The union-based gradient implementation provides **memory-efficient, type-safe gradient support** for GUIKit widgets while maintaining a **clean, performant API** suitable for resource-constrained ESP8266 devices.
