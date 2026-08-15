# Gradient Group - Multiple Independent Gradients per Widget

> Extracted from implementation - Union-based gradient group for complex shading effects

## Overview

This document describes the **Gradient Group** system, which extends the union-based gradient implementation to support **up to 6 independent gradient definitions per widget**. Each gradient in the group can have its own:

- Gradient type (linear, radial, conic, horizontal, vertical, diagonal)
- Colors (start, end, multi-color)
- Position and size (bounds within the widget)
- Blend mode (how it combines with other gradients)
- Opacity (transparency level)
- Enable/disable state

This enables **complex shading effects** with multiple gradient lines/areas on a single widget, such as:
- Button with horizontal gradient + vertical highlight overlay
- Panel with diagonal shadow gradient + radial glow
- Background with multiple gradient stripes

---

## Memory Analysis

### GradientEntry Structure (27 bytes)
```
Gradient: 15 bytes (max, from union)
  - type: 1 byte
  - color: 2 bytes  
  - union data: up to 12 bytes
  
bounds: 8 bytes
  - x, y: 2 bytes each
  - width, height: 2 bytes each
  
Other: 4 bytes
  - blend_mode: 1 byte
  - opacity: 1 byte
  - enabled: 1 byte
  - padding: 1 byte
  
Total: 27 bytes per gradient entry
```

### GradientGroup Structure
```
GradientEntry[6]: 6 × 27 = 162 bytes
count: 1 byte
padding: 3 bytes
Total: 165 bytes maximum per widget
```

### Comparison
| **Approach** | **Memory per Widget** | **Max Gradients** | **Flexibility** |
|--------------|----------------------|-------------------|----------------|
| Single Gradient | 15 bytes | 1 | Limited |
| Gradient Group | 165 bytes max | 6 | Full |
| Individual Widgets | N/A | Unlimited | Overkill |

**165 bytes is acceptable** for ESP8266 (80KB RAM) as:
- Typical widget already uses 200+ bytes
- Most widgets will use 1-3 gradients, not 6
- Enables rich visual effects without external resources

---

## Data Structures

### 1. Blend Modes

```c
typedef enum {
    BLEND_NORMAL = 0,      // Normal alpha blending (default)
    BLEND_OVERLAY,        // Overlay blend mode
    BLEND_MULTIPLY,       // Multiply blend mode
    BLEND_SCREEN,         // Screen blend mode
    BLEND_ADD,            // Additive blending
    BLEND_DARKEN,         // Darken (min of each channel)
    BLEND_LIGHTEN,        // Lighten (max of each channel)
    BLEND_LINEAR_DODGE,   // Linear dodge (add with clamping)
    BLEND_LINEAR_BURN,    // Linear burn
} BLEND_MODE;
```

### 2. Gradient Entry

```c
typedef struct {
    Gradient gradient;           // The gradient definition (union-based)
    
    struct {
        uint16_t x;              // X offset within widget
        uint16_t y;              // Y offset within widget
        uint16_t width;          // Width of gradient area (0 = full)
        uint16_t height;         // Height of gradient area (0 = full)
    } bounds;
    
    BLEND_MODE blend_mode;      // How this gradient blends
    uint8_t opacity;           // Opacity (0-255, 255 = fully opaque)
    bool enabled;              // Whether this gradient is active
} GradientEntry;
```

### 3. Gradient Group

```c
#define MAX_GRADIENTS_PER_GROUP 6

typedef struct {
    GradientEntry gradients[MAX_GRADIENTS_PER_GROUP];
    uint8_t count;
} GradientGroup;
```

---

## API Reference

### Initialization

| **Function/Macro** | **Description** |
|-------------------|-----------------|
| `gradientGroup_init(group)` | Initialize group (clear all gradients) |
| `gradientGroup_reset(group)` | Reset to empty state |
| `gradientGroup_clear(group)` | Clear all gradients |
| `INIT_GRADIENT_GROUP(g)` | Macro for init |
| `RESET_GRADIENT_GROUP(g)` | Macro for reset |
| `CLEAR_GRADIENT_GROUP(g)` | Macro for clear |

### Gradient Management

| **Function/Macro** | **Description** |
|-------------------|-----------------|
| `gradientGroup_addGradient(group)` | Add new gradient, return entry |
| `gradientGroup_addGradientAt(group, idx)` | Add at specific index |
| `gradientGroup_removeGradient(group, idx)` | Remove gradient at index |
| `gradientGroup_removeLast(group)` | Remove last gradient |
| `ADD_GRADIENT(g)` | Macro to add gradient |
| `REMOVE_GRADIENT(g, i)` | Macro to remove at index |
| `REMOVE_LAST_GRADIENT(g)` | Macro to remove last |

### Convenience Add Functions

| **Function** | **Description** |
|-------------|-----------------|
| `gradientGroup_addHorizontal(g, s, e, x, y, w, h, o, m)` | Add horizontal gradient |
| `gradientGroup_addVertical(g, s, e, x, y, w, h, o, m)` | Add vertical gradient |
| `gradientGroup_addDiagonal(g, s, e, x, y, w, h, o, m)` | Add diagonal gradient |
| `gradientGroup_addLinear(g, s, e, a, x, y, w, h, o, m)` | Add linear with angle |
| `gradientGroup_addRadial(g, s, e, cx, cy, r, x, y, w, h, o, m)` | Add radial gradient |

**Parameters:**
- `g`: GradientGroup pointer
- `s`: Start color (RGB565)
- `e`: End color (RGB565)
- `a`: Angle (for linear, degrees 0-360)
- `cx, cy`: Center coordinates (for radial)
- `r`: Radius (for radial)
- `x, y`: Position offset within widget
- `w, h`: Width and height of gradient area (0 = full widget)
- `o`: Opacity (0-255)
- `m`: Blend mode (BLEND_NORMAL, etc.)

### Accessor Macros

| **Macro** | **Description** |
|-----------|-----------------|
| `GRADIENT_GROUP_COUNT(g)` | Get number of gradients |
| `GRADIENT_GROUP_EMPTY(g)` | Check if empty |
| `GRADIENT_GROUP_FULL(g)` | Check if full (6 gradients) |
| `GRADIENT_GROUP_GET(g, i)` | Get gradient entry at index |
| `GRADIENT_GROUP_FIRST(g)` | Get first gradient entry |
| `GRADIENT_GROUP_LAST(g)` | Get last gradient entry |
| `GRADIENT_ENTRY_ENABLE(e)` | Enable a gradient entry |
| `GRADIENT_ENTRY_DISABLE(e)` | Disable a gradient entry |
| `GRADIENT_ENTRY_IS_ENABLED(e)` | Check if enabled |
| `GRADIENT_ENTRY_SET_OPACITY(e, o)` | Set opacity |
| `GRADIENT_ENTRY_GET_OPACITY(e)` | Get opacity |
| `GRADIENT_ENTRY_SET_BLEND(e, m)` | Set blend mode |
| `GRADIENT_ENTRY_GET_BLEND(e)` | Get blend mode |
| `GRADIENT_ENTRY_SET_BOUNDS(e, x, y, w, h)` | Set bounds |

### Inline Functions

| **Function** | **Description** |
|-------------|-----------------|
| `gradientGroup_getCount(group)` | Get gradient count |
| `gradientGroup_hasGradients(group)` | Check if has gradients |
| `gradientGroup_isFull(group)` | Check if at capacity |
| `gradientGroup_getEntry(group, index)` | Get entry at index |
| `gradientGroup_getEntryConst(group, index)` | Get entry (const) |
| `gradientEntry_enable(entry)` | Enable entry |
| `gradientEntry_disable(entry)` | Disable entry |
| `gradientEntry_isEnabled(entry)` | Check if enabled |
| `gradientEntry_setOpacity(entry, opacity)` | Set opacity |
| `gradientEntry_getOpacity(entry)` | Get opacity |
| `gradientEntry_setBlendMode(entry, mode)` | Set blend mode |
| `gradientEntry_getBlendMode(entry)` | Get blend mode |
| `gradientEntry_setBounds(entry, x, y, w, h)` | Set bounds |

### Rendering

| **Function** | **Description** |
|-------------|-----------------|
| `gradientGroup_getColorAt(group, x, y, w, h)` | Get blended color at position |
| `gradientEntry_getColorAt(entry, x, y, w, h)` | Get color from single entry |
| `GET_GROUP_COLOR(g, x, y, w, h)` | Macro for getColorAt |

### Blending

| **Function** | **Description** |
|-------------|-----------------|
| `blend_colors(base, top, opacity)` | Blend with opacity |
| `blend_colors_with_mode(base, top, mode)` | Blend with mode |
| `blend_colors_ex(base, top, opacity, mode)` | Blend with both |

### Bulk Operations

| **Function** | **Description** |
|-------------|-----------------|
| `gradientGroup_copy(dest, src)` | Copy entire group |
| `gradientGroup_equal(g1, g2)` | Compare two groups |
| `gradientGroup_getBaseColor(group)` | Get first gradient's color |
| `gradientGroup_setBaseColor(group, color)` | Set base color for all |
| `gradientGroup_enableAll(group)` | Enable all gradients |
| `gradientGroup_disableAll(group)` | Disable all gradients |
| `gradientGroup_setOpacityAll(group, opacity)` | Set opacity for all |
| `gradientGroup_setBlendModeAll(group, mode)` | Set blend mode for all |

---

## Usage Examples

### Example 1: Basic Gradient Group

```c
#include "widget_gradient_group.h"

void create_gradient_group_button() {
    Widget* button = new_widget(WIDGET_TYPE_BUTTON);
    button->size.width = 200;
    button->size.height = 80;
    
    // Initialize gradient group
    gradientGroup_init(&button->background.gradient_group);
    
    // Add base horizontal gradient (red to blue)
    ADD_HGRADIENT(&button->background.gradient_group,
                 COLOR_RED, COLOR_BLUE,
                 0, 0, 0, 0,  // Full widget area
                 255, BLEND_NORMAL);
    
    // Add overlay vertical gradient (white to transparent at 50% opacity)
    ADD_VGRADIENT(&button->background.gradient_group,
                 COLOR_WHITE, COLOR_TRANSPARENT,
                 0, 0, 0, 0,
                 128, BLEND_OVERLAY);
}
```

### Example 2: Complex Shading with Multiple Gradients

```c
void create_complex_panel() {
    Widget* panel = new_widget(WIDGET_TYPE_VIEW);
    panel->size.width = 240;
    panel->size.height = 320;
    
    gradientGroup_init(&panel->background.gradient_group);
    
    // 1. Base diagonal gradient (dark gray to light gray)
    ADD_DGRADIENT(&panel->background.gradient_group,
                 COLOR_DARK_GRAY, COLOR_LIGHT_GRAY,
                 0, 0, 0, 0,
                 255, BLEND_NORMAL);
    
    // 2. Top highlight (white to transparent, top 20% of panel)
    ADD_VGRADIENT(&panel->background.gradient_group,
                 COLOR_WHITE, COLOR_TRANSPARENT,
                 0, 0, 240, 64,  // Top 64 pixels
                 128, BLEND_SCREEN);
    
    // 3. Left border shadow (black to transparent, left 10 pixels)
    ADD_HGRADIENT(&panel->background.gradient_group,
                 COLOR_BLACK, COLOR_TRANSPARENT,
                 0, 0, 10, 320,  // Left 10 pixels
                 64, BLEND_MULTIPLY);
}
```

### Example 3: Radial Glow Effect

```c
void create_glow_button() {
    Widget* button = new_widget(WIDGET_TYPE_BUTTON);
    button->size.width = 100;
    button->size.height = 50;
    
    gradientGroup_init(&button->background.gradient_group);
    
    // Base solid color
    GradientEntry* base = ADD_GRADIENT(&button->background.gradient_group);
    gradient_setSolid(&base->gradient, COLOR_DARK_BLUE);
    base->opacity = 255;
    base->blend_mode = BLEND_NORMAL;
    
    // Radial glow from center
    ADD_RGRADIENT(&button->background.gradient_group,
                 COLOR_WHITE, COLOR_TRANSPARENT,
                 50, 25, 40,  // Center at (50, 25), radius 40
                 0, 0, 100, 50,  // Full button area
                 128, BLEND_ADD);  // Additive blending for glow
}
```

### Example 4: Multi-Directional Stripes

```c
void create_striped_background() {
    Widget* background = new_widget(WIDGET_TYPE_VIEW);
    background->size.width = 240;
    background->size.height = 320;
    
    gradientGroup_init(&background->background.gradient_group);
    
    // Horizontal stripes (alternating colors)
    for (int i = 0; i < 3; i++) {
        uint16_t y = i * 40;
        GradientEntry* entry = ADD_GRADIENT(&background->background.gradient_group);
        gradient_setHorizontal(&entry->gradient,
                               (i % 2) ? COLOR_BLUE : COLOR_RED,
                               (i % 2) ? COLOR_BLUE : COLOR_RED);
        entry->bounds.x = 0;
        entry->bounds.y = y;
        entry->bounds.width = 240;
        entry->bounds.height = 20;
        entry->opacity = 255;
        entry->blend_mode = BLEND_NORMAL;
    }
}
```

### Example 5: Dynamic Gradient Group Modification

```c
void update_button_state(Widget* button, ButtonState state) {
    gradientGroup_clear(&button->background.gradient_group);
    
    switch (state) {
        case BUTTON_NORMAL:
            ADD_HGRADIENT(&button->background.gradient_group,
                         COLOR_GRAY, COLOR_SILVER,
                         0, 0, 0, 0, 255, BLEND_NORMAL);
            break;
            
        case BUTTON_HOVER:
            ADD_HGRADIENT(&button->background.gradient_group,
                         COLOR_LIGHT_GRAY, COLOR_WHITE,
                         0, 0, 0, 0, 255, BLEND_NORMAL);
            ADD_VGRADIENT(&button->background.gradient_group,
                         COLOR_WHITE, COLOR_TRANSPARENT,
                         0, 0, 0, 0, 128, BLEND_OVERLAY);
            break;
            
        case BUTTON_PRESSED:
            ADD_VGRADIENT(&button->background.gradient_group,
                         COLOR_DARK_GRAY, COLOR_GRAY,
                         0, 0, 0, 0, 255, BLEND_NORMAL);
            ADD_RGRADIENT(&button->background.gradient_group,
                         COLOR_BLACK, COLOR_TRANSPARENT,
                         50, 50, 30,
                         0, 0, 100, 100, 128, BLEND_MULTIPLY);
            break;
    }
}
```

### Example 6: Rendering with Gradient Group

```c
void render_widget_with_gradient_group(Widget* widget) {
    if (!widget->visible) return;
    
    const GradientGroup* group = &widget->background.gradient_group;
    
    if (!gradientGroup_hasGradients(group)) {
        // No gradients, use solid color
        Color bg = gradientGroup_getBaseColor(group);
        if (bg != 0) {
            tft.fillRect(widget->position.x, widget->position.y,
                        widget->size.width, widget->size.height,
                        bg);
        }
        return;
    }
    
    // Render pixel by pixel with gradient group
    for (uint16_t y = 0; y < widget->size.height; y++) {
        for (uint16_t x = 0; x < widget->size.width; x++) {
            Color c = gradientGroup_getColorAt(group,
                                                x, y,
                                                widget->size.width,
                                                widget->size.height);
            if (c != 0) {  // Skip transparent
                tft.drawPixel(widget->position.x + x,
                             widget->position.y + y,
                             c);
            }
        }
    }
}
```

### Example 7: Optimized Row-Based Rendering

```c
void render_widget_gradient_group_optimized(Widget* widget) {
    if (!widget->visible) return;
    
    const GradientGroup* group = &widget->background.gradient_group;
    
    if (!gradientGroup_hasGradients(group)) {
        Color bg = gradientGroup_getBaseColor(group);
        if (bg != 0) {
            tft.fillRect(widget->position.x, widget->position.y,
                        widget->size.width, widget->size.height,
                        bg);
        }
        return;
    }
    
    // For each row, compute all gradient colors
    for (uint16_t y = 0; y < widget->size.height; y++) {
        for (uint16_t x = 0; x < widget->size.width; x++) {
            Color final = gradientGroup_getColorAt(group,
                                                    x, y,
                                                    widget->size.width,
                                                    widget->size.height);
            tft.drawPixel(widget->position.x + x,
                         widget->position.y + y,
                         final);
        }
    }
}
```

---

## Blend Mode Reference

### BLEND_NORMAL (Default)
- Standard alpha blending
- `result = base + (top - base) * opacity / 255`
- Most common, natural look

### BLEND_OVERLAY
- Preserves highlights and shadows
- Dark areas: multiply (darkens)
- Light areas: screen (lightens)
- Good for adding highlights/shadows

### BLEND_MULTIPLY
- Multiplies color channels
- Always results in darker color
- Good for shadows, darkening

### BLEND_SCREEN
- Inverse of multiply
- Always results in lighter color
- Good for highlights, lightening

### BLEND_ADD
- Adds color channels
- Can exceed 255 (clamped)
- Good for glow effects

### BLEND_DARKEN
- Takes minimum of each channel
- Good for dark overlays

### BLEND_LIGHTEN
- Takes maximum of each channel
- Good for light overlays

### BLEND_LINEAR_DODGE
- Add with clamping
- Similar to ADD but more controlled

### BLEND_LINEAR_BURN
- Subtract with clamping
- Good for deep shadows

---

## Integration with Widget Architecture

### Updated Widget Background Structure

```c
typedef struct {
    // Single gradient (for simple cases)
    Gradient gradient;
    
    // Gradient group (for complex cases)
    GradientGroup gradient_group;
    
    // Flag to indicate which to use
    bool use_gradient_group;  // false = use single gradient
} Background;
```

### Widget Constructor with Gradient Group

```c
Widget* new_widget_with_gradients(WIDGET_TYPE type) {
    Widget* widget = widget_pool_alloc(type);
    if (!widget) return NULL;
    
    widget->type = type;
    
    // Initialize both single and group gradients
    gradient_setSolid(&widget->background.gradient, COLOR_WHITE);
    gradientGroup_init(&widget->background.gradient_group);
    widget->background.use_gradient_group = false;
    
    return widget;
}
```

### Rendering Decision

```c
Color get_widget_background_color(Widget* widget, uint16_t x, uint16_t y) {
    if (widget->background.use_gradient_group) {
        return gradientGroup_getColorAt(&widget->background.gradient_group,
                                          x, y,
                                          widget->size.width,
                                          widget->size.height);
    } else {
        return gradient_getColorAt(&widget->background.gradient,
                                   x, y,
                                   widget->size.width,
                                   widget->size.height);
    }
}
```

### Migration from Single to Group

```c
// Convert single gradient to group
void widget_convert_to_gradient_group(Widget* widget) {
    GradientGroup* group = &widget->background.gradient_group;
    
    gradientGroup_init(group);
    
    // Copy single gradient to group
    GradientEntry* entry = gradientGroup_addGradient(group);
    gradient_copy(&entry->gradient, &widget->background.gradient);
    
    // Set full bounds
    entry->bounds.x = 0;
    entry->bounds.y = 0;
    entry->bounds.width = 0;  // Full width
    entry->bounds.height = 0; // Full height
    entry->opacity = 255;
    entry->blend_mode = BLEND_NORMAL;
    entry->enabled = true;
    
    widget->background.use_gradient_group = true;
}
```

---

## Memory Optimization Tips

### 1. Use Single Gradient When Possible
```c
// For simple cases, don't use group
gradient_setHorizontal(&widget->background.gradient, COLOR_RED, COLOR_BLUE);
widget->background.use_gradient_group = false;  // Saves 165 bytes
```

### 2. Limit Gradient Count
```c
// Most effects only need 2-3 gradients
// 2 gradients = 54 bytes (2 × 27)
// 3 gradients = 81 bytes (3 × 27)
```

### 3. Share Gradient Groups
```c
// For widgets with same gradient configuration
static GradientGroup shared_gradient_group;

void init_shared_gradients() {
    gradientGroup_init(&shared_gradient_group);
    ADD_HGRADIENT(&shared_gradient_group, COLOR_A, COLOR_B, 0, 0, 0, 0, 255, BLEND_NORMAL);
    // ... add more
}

// Use shared group (read-only in rendering)
widget->background.gradient_group = shared_gradient_group;
```

### 4. Use Pointers for Shared Groups (Advanced)
```c
// Store pointer to shared group instead of copying
typedef struct {
    Gradient gradient;
    GradientGroup* gradient_group_ptr;  // Pointer to shared or local
    bool owns_group;  // Whether we own the group (need to free)
} Background;
```

---

## Performance Considerations

### Rendering Cost
- **Single gradient**: O(1) per pixel (one gradient lookup)
- **Gradient group**: O(N) per pixel (N = number of gradients)
- **6 gradients**: ~6× slower than single gradient

### Optimization Strategies

#### 1. Cache Gradient Rows
```c
void render_gradient_group_cached(Widget* widget) {
    GradientGroup* group = &widget->background.gradient_group;
    
    // Pre-allocate row buffer
    static Color row_buffer[240];  // Max width
    
    for (uint16_t y = 0; y < widget->size.height; y++) {
        // Compute entire row
        for (uint16_t x = 0; x < widget->size.width; x++) {
            row_buffer[x] = gradientGroup_getColorAt(group, x, y,
                                                        widget->size.width,
                                                        widget->size.height);
        }
        // Draw row
        tft.drawRGBBitmap(widget->position.x, widget->position.y + y,
                         row_buffer, widget->size.width, 1);
    }
}
```

#### 2. Bounding Box Optimization
```c
void render_gradient_group_bbox(Widget* widget) {
    GradientGroup* group = &widget->background.gradient_group;
    
    // Calculate union of all gradient bounds
    uint16_t min_x = widget->size.width;
    uint16_t min_y = widget->size.height;
    uint16_t max_x = 0;
    uint16_t max_y = 0;
    
    for (uint8_t i = 0; i < group->count; i++) {
        GradientEntry* entry = &group->gradients[i];
        if (!entry->enabled) continue;
        
        uint16_t ex = entry->bounds.x + (entry->bounds.width ? entry->bounds.width : widget->size.width);
        uint16_t ey = entry->bounds.y + (entry->bounds.height ? entry->bounds.height : widget->size.height);
        
        if (entry->bounds.x < min_x) min_x = entry->bounds.x;
        if (entry->bounds.y < min_y) min_y = entry->bounds.y;
        if (ex > max_x) max_x = ex;
        if (ey > max_y) max_y = ey;
    }
    
    // Only render the affected area
    for (uint16_t y = min_y; y < max_y && y < widget->size.height; y++) {
        for (uint16_t x = min_x; x < max_x && x < widget->size.width; x++) {
            Color c = gradientGroup_getColorAt(group, x, y,
                                                widget->size.width,
                                                widget->size.height);
            tft.drawPixel(widget->position.x + x,
                         widget->position.y + y,
                         c);
        }
    }
}
```

#### 3. Skip Transparent Pixels
```c
// In gradientEntry_getColorAt, check opacity first
if (!entry->enabled || entry->opacity == 0) {
    return 0;  // Early exit for disabled/transparent
}
```

---

## Best Practices

### Do This ✅

```c
// Initialize gradient groups
INIT_GRADIENT_GROUP(&widget->background.gradient_group);

// Use macros for common operations
ADD_HGRADIENT(&group, COLOR_RED, COLOR_BLUE, 0, 0, 0, 0, 255, BLEND_NORMAL);

// Check bounds before adding
if (!GRADIENT_GROUP_FULL(&group)) {
    GradientEntry* entry = ADD_GRADIENT(&group);
    // Configure entry...
}

// Use fixed-point for positions
uint16_t half_width = widget->size.width / 2;
ADD_RGRADIENT(&group, COLOR_WHITE, COLOR_TRANSPARENT,
             half_width, half_width, half_width,
             0, 0, 0, 0, 128, BLEND_ADD);

// Disable unused gradients
if (entry->opacity == 0) {
    GRADIENT_ENTRY_DISABLE(entry);
}

// Use appropriate blend modes
// Overlay for highlights, Multiply for shadows, Add for glow
```

### Don't Do This ❌

```c
// DON'T exceed MAX_GRADIENTS_PER_GROUP
for (int i = 0; i < 10; i++) {  // WRONG: max is 6
    ADD_GRADIENT(&group);
}

// DON'T forget to initialize
GradientGroup group;  // Uninitialized!
ADD_GRADIENT(&group);  // Undefined behavior

// DON'T access out of bounds
GradientEntry* entry = GRADIENT_GROUP_GET(&group, 10);  // WRONG: max 5

// DON'T use without checking
if (entry) {  // Always check NULL
    // Configure entry...
}

// DON'T forget enabled flag
entry->enabled = false;  // Remember to disable when not needed
```

---

## File Locations

| **File** | **Path** | **Description** |
|----------|----------|-----------------|
| Header | `src/gui/widget_gradient_group.h` | Type definitions, macros, inline functions |
| Implementation | `src/gui/widget_gradient_group.c` | Function implementations and rendering |
| Documentation | `docs/discussion_analysis/18_GRADIENT_GROUP.md` | This file |

---

## Cross-References

- **Single Gradient**: See `17_GRADIENT_UNION.md` for base gradient implementation
- **Widget Architecture**: See `01_WIDGET_ARCHITECTURE.md` for widget structure
- **Style System**: See `05_STYLE_SYSTEM.md` for style properties
- **Scrollable Union**: See `16_SCROLLABLE_UNION.md` for similar union-based optimization
- **Rendering System**: See `07_RENDERING_SYSTEM.md` for rendering implementation

---

## Summary

| **Feature** | **Implementation** | **Benefit** |
|-------------|-------------------|-------------|
| Multiple Gradients | GradientGroup with 6 GradientEntry slots | Complex shading effects |
| Independent Bounds | Each gradient has x, y, width, height | Precise positioning |
| Blend Modes | 9 blend modes for combining gradients | Professional effects |
| Opacity Control | Per-gradient opacity (0-255) | Layered transparency |
| Memory Efficient | 27 bytes per gradient, 165 bytes max | ESP8266 friendly |
| Type-Safe | Builds on union-based Gradient | Safe access |
| Fast Rendering | Fixed-point math, inline functions | Optimized for ESP8266 |

The **Gradient Group** system provides **professional-quality multi-gradient effects** for GUIKit widgets while maintaining **memory efficiency** and **type safety** suitable for resource-constrained ESP8266 devices.
