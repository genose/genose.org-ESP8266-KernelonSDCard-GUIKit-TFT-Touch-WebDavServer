# Rendering System

> Extracted from discussion_guikit.txt - TFT_eSPI rendering implementation

## Overview

The rendering system handles drawing widgets onto the TFT display using the TFT_eSPI library. It implements a layered rendering pipeline that processes widgets based on their style flags and properties.

---

## Rendering Pipeline

The recommended rendering order for maximum visual quality:

```c
void draw_widget(Widget* widget) {
    // 1. Draw background effects (shadows that go behind)
    if (widget->style.draw_style & WIDGET_DRAW_STYLE_DROP_SHADOW) {
        draw_drop_shadow(widget);
    }

    // 2. Draw fill (based on fill style)
    draw_fill(widget);

    // 3. Draw shape/border (based on border and shape styles)
    draw_border(widget);
    draw_shape(widget);

    // 4. Draw front effects (glow, neon, etc.)
    if (widget->style.draw_style & WIDGET_DRAW_STYLE_GLOW) {
        draw_glow(widget);
    }
    if (widget->style.draw_style & WIDGET_DRAW_STYLE_NEON) {
        draw_neon_effect(widget);
    }
    if (widget->style.draw_style & WIDGET_DRAW_STYLE_METALIC) {
        draw_metallic_effect(widget);
    }

    // 5. Draw content (text, icon, etc.)
    if (widget->text.text[0] != '\0') {
        draw_text(widget);
    }
}
```

### Fill Rendering

```c
void draw_fill(Widget* widget) {
    uint64_t fill_style = widget->style.draw_style & WIDGET_DRAW_STYLE_FILL_MASK;

    if (fill_style == WIDGET_DRAW_STYLE_TRANSPARENT_FILL) {
        return; // Nothing to draw
    }

    switch (fill_style) {
        case WIDGET_DRAW_STYLE_SOLID_FILL:
            draw_solid_fill(widget);
            break;
        case WIDGET_DRAW_STYLE_HORIZONTAL_GRADIENT:
            draw_horizontal_gradient(widget);
            break;
        case WIDGET_DRAW_STYLE_VERTICAL_GRADIENT:
            draw_vertical_gradient(widget);
            break;
        case WIDGET_DRAW_STYLE_RADIAL_GRADIENT:
            draw_radial_gradient(widget);
            break;
        case WIDGET_DRAW_STYLE_DIAGONAL_GRADIENT:
            draw_diagonal_gradient(widget);
            break;
        case WIDGET_DRAW_STYLE_HATCHED_FILL:
            draw_hatched_fill(widget);
            break;
        case WIDGET_DRAW_STYLE_CROSSHATCH_FILL:
            draw_crosshatched_fill(widget);
            break;
        case WIDGET_DRAW_STYLE_DOT_FILL:
            draw_dot_fill(widget);
            break;
        case WIDGET_DRAW_STYLE_PATTERN_FILL:
            draw_pattern_fill(widget);
            break;
        default:
            // Unknown fill style: draw solid
            if (!(widget->style.draw_style & WIDGET_DRAW_STYLE_TRANSPARENT_FILL)) {
                draw_solid_fill(widget);
            }
            break;
    }
}
```

### Border Rendering

```c
void draw_border(Widget* widget) {
    uint64_t border_style = widget->style.draw_style & WIDGET_DRAW_STYLE_BORDER_MASK;

    if (border_style == WIDGET_DRAW_STYLE_NO_BORDER) {
        return; // No border
    }

    if (border_style & WIDGET_DRAW_STYLE_ROUNDED_BORDER) {
        if (border_style & WIDGET_DRAW_STYLE_DOTTED_BORDER) {
            draw_rounded_dotted_border(widget);
        }
        else if (border_style & WIDGET_DRAW_STYLE_DASHED_BORDER) {
            draw_rounded_dashed_border(widget);
        }
        else if (border_style & WIDGET_DRAW_STYLE_GRADIENT_BORDER) {
            draw_rounded_gradient_border(widget);
        }
        else {
            draw_rounded_border(widget);
        }
    }
    else if (border_style & WIDGET_DRAW_STYLE_DOTTED_BORDER) {
        draw_dotted_border(widget);
    }
    else if (border_style & WIDGET_DRAW_STYLE_DASHED_BORDER) {
        draw_dashed_border(widget);
    }
    else if (border_style & WIDGET_DRAW_STYLE_DOUBLE_BORDER) {
        draw_double_border(widget);
    }
    else if (border_style & WIDGET_DRAW_STYLE_ETCHED_BORDER) {
        draw_etched_border(widget);
    }
    else if (border_style & WIDGET_DRAW_STYLE_EMBOSSED_BORDER) {
        draw_embossed_border(widget);
    }
    else {
        // Default: solid border
        draw_solid_border(widget);
    }
}
```

### Shape Rendering

```c
void draw_shape(Widget* widget) {
    uint64_t shape_style = widget->style.draw_style & WIDGET_DRAW_STYLE_SHAPE_MASK;

    if (shape_style == WIDGET_DRAW_STYLE_CIRCLE) {
        tft.fillCircle(
            widget->rect.x + widget->rect.width / 2,
            widget->rect.y + widget->rect.height / 2,
            widget->style.shape.radius,
            widget->style.colors.primary
        );
    }
    else if (shape_style == WIDGET_DRAW_STYLE_ELLIPSE) {
        draw_filled_ellipse(widget);
    }
    else if (shape_style == WIDGET_DRAW_STYLE_TRIANGLE) {
        draw_filled_triangle(widget);
    }
    else if (shape_style == WIDGET_DRAW_STYLE_POLYGON) {
        draw_filled_polygon(widget);
    }
    else if (shape_style == WIDGET_DRAW_STYLE_BEZIER_PATH) {
        draw_bezier_path(widget);
    }
    else if (shape_style == WIDGET_DRAW_STYLE_ARC) {
        draw_filled_arc(widget);
    }
    else if (shape_style == WIDGET_DRAW_STYLE_PIE) {
        draw_pie(widget);
    }
    else if (shape_style == WIDGET_DRAW_STYLE_ROUNDED_RECT) {
        draw_rounded_rect(widget);
    }
    else {
        // Default: rectangle
        draw_rectangle(widget);
    }
}
```

---

## TFT_eSPI Integration

### Basic Setup

```c
#include <TFT_eSPI.h>

// Create TFT instance
TFT_eSPI tft = TFT_eSPI();

// Initialize in setup()
void setup() {
    tft.init();
    tft.setRotation(3); // Adjust based on your display orientation
    tft.fillScreen(TFT_BLACK);
    
    // Optional: set text parameters
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
}
```

### Color Definitions

RGB565 color format (16-bit):
- Bits 15-11: Red (5 bits, 0-31)
- Bits 10-5: Green (6 bits, 0-63)
- Bits 4-0: Blue (5 bits, 0-31)

```c
typedef uint16_t Color;

// Common colors
#define COLOR_BLACK      0x0000
#define COLOR_WHITE      0xFFFF
#define COLOR_RED        0xF800
#define COLOR_GREEN      0x07E0
#define COLOR_BLUE       0x001F
#define COLOR_CYAN       0x07FF
#define COLOR_MAGENTA    0xF81F
#define COLOR_YELLOW     0xFFE0
#define COLOR_GRAY       0x8410
#define COLOR_ORANGE     0xFD20
#define COLOR_PURPLE     0x8010
#define COLOR_PINK       0xFC18
#define COLOR_BROWN      0xA145
```

### Drawing Primitives

```c
// Draw pixel
void draw_pixel(uint16_t x, uint16_t y, Color color) {
    tft.drawPixel(x, y, color);
}

// Draw line
void draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, Color color) {
    tft.drawLine(x0, y0, x1, y1, color);
}

// Draw rectangle
void draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, Color color) {
    tft.drawRect(x, y, w, h, color);
}

// Fill rectangle
void fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, Color color) {
    tft.fillRect(x, y, w, h, color);
}

// Draw rounded rectangle
void draw_rounded_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
                       uint16_t radius, Color color) {
    tft.drawRoundRect(x, y, w, h, radius, color);
}

// Fill rounded rectangle
void fill_rounded_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
                       uint16_t radius, Color color) {
    tft.fillRoundRect(x, y, w, h, radius, color);
}

// Draw circle
void draw_circle(uint16_t x, uint16_t y, uint16_t radius, Color color) {
    tft.drawCircle(x, y, radius, color);
}

// Fill circle
void fill_circle(uint16_t x, uint16_t y, uint16_t radius, Color color) {
    tft.fillCircle(x, y, radius, color);
}

// Draw triangle
void draw_triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, 
                   uint16_t x2, uint16_t y2, Color color) {
    tft.drawTriangle(x0, y0, x1, y1, x2, y2, color);
}

// Fill triangle
void fill_triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, 
                   uint16_t x2, uint16_t y2, Color color) {
    tft.fillTriangle(x0, y0, x1, y1, x2, y2, color);
}
```

### Text Rendering

```c
// Draw text at cursor position
void draw_text_at_cursor(const char* text, Color color, Color bg, uint8_t size) {
    tft.setTextColor(color, bg);
    tft.setTextSize(size);
    tft.print(text);
}

// Draw text at specific position
void draw_text(uint16_t x, uint16_t y, const char* text, Color color, Color bg, uint8_t size) {
    tft.setCursor(x, y);
    tft.setTextColor(color, bg);
    tft.setTextSize(size);
    tft.print(text);
}

// Draw centered text in a rectangle
void draw_centered_text(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
                        const char* text, Color color, Color bg, uint8_t size) {
    tft.setTextSize(size);
    uint16_t text_width = tft.textWidth(text);
    uint16_t text_height = tft.fontHeight();
    
    uint16_t text_x = x + (w - text_width) / 2;
    uint16_t text_y = y + (h - text_height) / 2;
    
    tft.setCursor(text_x, text_y);
    tft.setTextColor(color, bg);
    tft.print(text);
}

// Get text width (for centering)
uint16_t get_text_width(const char* text, uint8_t size) {
    tft.setTextSize(size);
    return tft.textWidth(text);
}

// Get text height
uint16_t get_text_height(uint8_t size) {
    tft.setTextSize(size);
    return tft.fontHeight();
}
```

---

## Widget-Specific Rendering

### Button Rendering

```c
void draw_button(WidgetButton* button) {
    Widget* widget = &button->base;
    
    // Handle pressed state
    Color bg_color = widget->style.colors.primary;
    if (button->pressed) {
        // Darken color when pressed
        bg_color = darken_color(bg_color, 20);
    }
    
    // Draw background
    uint64_t fill_style = widget->style.draw_style & WIDGET_DRAW_STYLE_FILL_MASK;
    uint64_t shape_style = widget->style.draw_style & WIDGET_DRAW_STYLE_SHAPE_MASK;
    
    if (shape_style == WIDGET_DRAW_STYLE_CIRCLE) {
        fill_circle(widget->rect.x + widget->rect.width/2, 
                    widget->rect.y + widget->rect.height/2,
                    widget->style.shape.radius, bg_color);
    } else if (shape_style == WIDGET_DRAW_STYLE_ROUNDED_RECT) {
        fill_rounded_rect(widget->rect.x, widget->rect.y,
                          widget->rect.width, widget->rect.height,
                          widget->style.border.radius, bg_color);
    } else {
        fill_rect(widget->rect.x, widget->rect.y,
                  widget->rect.width, widget->rect.height, bg_color);
    }
    
    // Draw border
    uint64_t border_style = widget->style.draw_style & WIDGET_DRAW_STYLE_BORDER_MASK;
    if (border_style != WIDGET_DRAW_STYLE_NO_BORDER) {
        if (shape_style == WIDGET_DRAW_STYLE_CIRCLE) {
            draw_circle(widget->rect.x + widget->rect.width/2,
                       widget->rect.y + widget->rect.height/2,
                       widget->style.shape.radius, widget->style.border.color);
        } else if (shape_style == WIDGET_DRAW_STYLE_ROUNDED_RECT) {
            draw_rounded_rect(widget->rect.x, widget->rect.y,
                             widget->rect.width, widget->rect.height,
                             widget->style.border.radius, widget->style.border.color);
        } else {
            draw_rect(widget->rect.x, widget->rect.y,
                      widget->rect.width, widget->rect.height, widget->style.border.color);
        }
    }
    
    // Draw shadow if enabled
    if (widget->style.draw_style & WIDGET_DRAW_STYLE_DROP_SHADOW) {
        draw_drop_shadow(widget);
    }
    
    // Draw glow if enabled
    if (widget->style.draw_style & WIDGET_DRAW_STYLE_GLOW) {
        draw_glow(widget);
    }
    
    // Draw text
    if (widget->text.text[0] != '\0') {
        draw_centered_text(widget->rect.x, widget->rect.y,
                          widget->rect.width, widget->rect.height,
                          widget->text.text, widget->text.font.color,
                          bg_color, widget->text.font.size / 8);
    }
}
```

### Label Rendering

```c
void draw_label(WidgetLabel* label) {
    Widget* widget = &label->base;
    
    // Labels are typically transparent with just text
    if (!(widget->style.draw_style & WIDGET_DRAW_STYLE_TRANSPARENT_FILL)) {
        // Draw background if not transparent
        fill_rect(widget->rect.x, widget->rect.y,
                  widget->rect.width, widget->rect.height,
                  widget->style.colors.primary);
    }
    
    // Draw text
    if (widget->text.text[0] != '\0') {
        Color text_color = widget->text.font.color;
        uint8_t text_size = widget->text.font.size / 8;
        
        // Handle auto-resize if enabled
        if (label->auto_resize) {
            uint16_t text_width = get_text_width(widget->text.text, text_size);
            uint16_t text_height = get_text_height(text_size);
            widget->rect.width = text_width;
            widget->rect.height = text_height;
        }
        
        // Draw text at position
        tft.setCursor(widget->rect.x, widget->rect.y);
        tft.setTextColor(text_color, 
                        (widget->style.draw_style & WIDGET_DRAW_STYLE_TRANSPARENT_FILL) 
                        ? widget->style.colors.primary : widget->style.colors.primary);
        tft.setTextSize(text_size);
        tft.print(widget->text.text);
    }
}
```

### Slider Rendering

```c
void draw_slider(WidgetSlider* slider) {
    Widget* widget = &slider->base;
    
    // Draw track (background)
    uint16_t track_height = 8;
    uint16_t track_y = widget->rect.y + (widget->rect.height - track_height) / 2;
    
    if (slider->vertical) {
        // Vertical slider
        uint16_t track_width = 8;
        uint16_t track_x = widget->rect.x + (widget->rect.width - track_width) / 2;
        
        // Draw track
        fill_rounded_rect(track_x, widget->rect.y, track_width, widget->rect.height,
                         4, widget->style.colors.primary);
        
        // Calculate thumb position
        float ratio = (slider->current_value - slider->min_value) / 
                     (slider->max_value - slider->min_value);
        uint16_t thumb_y = widget->rect.y + ratio * (widget->rect.height - 20);
        
        // Draw thumb
        fill_circle(track_x + track_width/2, thumb_y + 10,
                   10, widget->style.colors.secondary);
        draw_circle(track_x + track_width/2, thumb_y + 10,
                   10, widget->style.border.color);
    } else {
        // Horizontal slider
        // Draw track
        fill_rounded_rect(widget->rect.x, track_y, widget->rect.width, track_height,
                         4, widget->style.colors.primary);
        
        // Calculate thumb position
        float ratio = (slider->current_value - slider->min_value) / 
                     (slider->max_value - slider->min_value);
        uint16_t thumb_x = widget->rect.x + ratio * (widget->rect.width - 20);
        
        // Draw thumb
        fill_circle(thumb_x + 10, track_y + track_height/2,
                   10, widget->style.colors.secondary);
        draw_circle(thumb_x + 10, track_y + track_height/2,
                   10, widget->style.border.color);
    }
}
```

### Checkbox Rendering

```c
void draw_checkbox(WidgetCheckbox* checkbox) {
    Widget* widget = &checkbox->base;
    
    // Draw box
    uint16_t box_size = min(widget->rect.width, widget->rect.height);
    uint16_t box_x = widget->rect.x;
    uint16_t box_y = widget->rect.y;
    
    Color box_color = widget->style.colors.primary;
    Color check_color = widget->text.font.color;
    
    if (checkbox->checked) {
        // Checked state: filled box
        fill_rounded_rect(box_x, box_y, box_size, box_size,
                         widget->style.border.radius, box_color);
        draw_rounded_rect(box_x, box_y, box_size, box_size,
                         widget->style.border.radius, widget->style.border.color);
        
        // Draw check mark
        uint16_t check_x1 = box_x + box_size * 0.2;
        uint16_t check_y1 = box_y + box_size * 0.5;
        uint16_t check_x2 = box_x + box_size * 0.4;
        uint16_t check_y2 = box_y + box_size * 0.7;
        uint16_t check_x3 = box_x + box_size * 0.8;
        uint16_t check_y3 = box_y + box_size * 0.3;
        
        draw_line(check_x1, check_y1, check_x2, check_y2, check_color, 2);
        draw_line(check_x2, check_y2, check_x3, check_y3, check_color, 2);
    } else {
        // Unchecked state: empty box
        draw_rounded_rect(box_x, box_y, box_size, box_size,
                         widget->style.border.radius, widget->style.border.color);
    }
    
    // Draw label if present
    if (widget->text.text[0] != '\0') {
        uint16_t label_x = box_x + box_size + 5;
        draw_text(label_x, box_y, widget->text.text, 
                  widget->text.font.color, widget->style.colors.primary,
                  widget->text.font.size / 8);
    }
}
```

### Progress Bar Rendering

```c
void draw_progress_bar(WidgetProgressBar* progress_bar) {
    Widget* widget = &progress_bar->base;
    
    // Draw background track
    fill_rounded_rect(widget->rect.x, widget->rect.y,
                     widget->rect.width, widget->rect.height,
                     widget->style.border.radius, widget->style.colors.primary);
    
    // Draw border
    draw_rounded_rect(widget->rect.x, widget->rect.y,
                     widget->rect.width, widget->rect.height,
                     widget->style.border.radius, widget->style.border.color);
    
    // Calculate fill width
    float ratio = (progress_bar->current_value - progress_bar->min_value) / 
                 (progress_bar->max_value - progress_bar->min_value);
    ratio = max(0.0f, min(1.0f, ratio));
    
    uint16_t fill_width = widget->rect.width * ratio;
    
    // Draw fill
    uint16_t fill_x = widget->rect.x + 2; // Inset by border
    uint16_t fill_y = widget->rect.y + 2;
    uint16_t fill_h = widget->rect.height - 4;
    
    if (progress_bar->vertical) {
        fill_width = widget->rect.height * ratio;
        fill_rounded_rect(widget->rect.x + 2, widget->rect.y + 2,
                         widget->rect.width - 4, fill_width,
                         widget->style.border.radius, progress_bar->bar_color);
    } else {
        fill_rounded_rect(fill_x, fill_y, fill_width, fill_h,
                         widget->style.border.radius, progress_bar->bar_color);
    }
    
    // Draw percentage text if there's room
    if (widget->rect.width > 50 && widget->rect.height > 20) {
        char percent_str[8];
        snprintf(percent_str, 8, "%d%%", (int)(ratio * 100));
        draw_centered_text(widget->rect.x, widget->rect.y,
                          widget->rect.width, widget->rect.height,
                          percent_str, 0xFFFF, progress_bar->bar_color, 1);
    }
}
```

---

## Dirty Flag Optimization

To avoid redrawing widgets that haven't changed:

```c
// In widget structure
struct Widget {
    // ... other fields
    bool dirty;  /**< Needs redrawing */
};

// Mark a widget as needing redraw
void widget_mark_dirty(Widget* widget) {
    widget->dirty = true;
    
    // Also mark parent as dirty (if it caches children)
    if (widget->parent) {
        widget->parent->dirty = true;
    }
}

// Clear dirty flag after rendering
void widget_clear_dirty(Widget* widget) {
    widget->dirty = false;
}

// Render all dirty widgets
void render_dirty_widgets(Widget* root) {
    render_widget_recursive(root);
}

// Recursive render
void render_widget_recursive(Widget* widget) {
    if (!widget->dirty) {
        return; // Skip if not dirty
    }
    
    // Render this widget
    draw_widget(widget);
    widget_clear_dirty(widget);
    
    // Render children
    for (uint8_t i = 0; i < widget->children_count; i++) {
        render_widget_recursive(widget->children[i]);
    }
}
```

---

## Clipping Optimization

Only draw pixels within the widget's bounds:

```c
// Set clipping region
void set_clip_region(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    tft.setClipRect(x, y, w, h);
}

// Clear clipping
void clear_clip_region() {
    tft.setClipRect();
}

// Draw widget with clipping
void draw_widget_clipped(Widget* widget) {
    // Save current clip region
    uint16_t old_x, old_y, old_w, old_h;
    tft.getClipRect(&old_x, &old_y, &old_w, &old_h);
    
    // Set new clip region
    set_clip_region(widget->rect.x, widget->rect.y, 
                    widget->rect.width, widget->rect.height);
    
    // Draw widget (all drawing will be clipped)
    draw_widget(widget);
    
    // Restore clip region
    tft.setClipRect(old_x, old_y, old_w, old_h);
}
```

---

## Double Buffering (If RAM Allows)

For flicker-free rendering on ESP8266 with sufficient RAM:

```c
// Note: ESP8266 has limited RAM (~80KB), so double buffering may not be feasible
// for large displays. Consider partial buffering or no buffering.

// For small displays (e.g., 320x240 with 16-bit color = 150KB), double buffering
// is NOT possible on ESP8266. Use direct drawing instead.

// Alternative: Buffer only changed regions
#define MAX_BUFFER_REGIONS 5
#define MAX_BUFFER_SIZE 1024  // 1KB per region

uint16_t buffer_regions[MAX_BUFFER_REGIONS][MAX_BUFFER_SIZE];
Rect buffer_rects[MAX_BUFFER_REGIONS];

// For ESP8266, it's better to avoid double buffering and use
// the dirty flag system + fast drawing primitives instead.
```

---

## Summary

The rendering system provides:
- ✅ Layered rendering pipeline (shadow → fill → border → effects → text)
- ✅ Support for all 40+ draw style flags
- ✅ TFT_eSPI integration with all primitives
- ✅ Widget-specific rendering functions
- ✅ Dirty flag optimization to minimize redrawing
- ✅ Clipping support for efficient drawing
- ✅ ESP8266-optimized (no double buffering, minimal RAM usage)

---

*Source: Extracted from discussion_guikit.txt*
*Documentation organized by Mistral Vibe*
