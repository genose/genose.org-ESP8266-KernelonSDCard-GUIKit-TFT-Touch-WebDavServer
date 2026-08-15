# Style System

> Extracted from discussion_guikit.txt - Complete WidgetStyle implementation

## Overview

The Style System in GUIKit provides a comprehensive way to control widget appearance with support for colors, gradients, borders, custom shapes, effects, and animations. This document covers the complete WidgetStyle structure and its usage.

---

## WidgetStyle Structure

The central structure that holds all style properties for a widget:

```c
typedef struct {
    uint64_t draw_style;  /**< Combination of WIDGET_DRAW_STYLE flags. */

    // === Couleurs et Fond ===
    struct {
        Color primary;   /**< Couleur principale (pour SOLID_FILL). */
        Color secondary; /**< Couleur secondaire (pour degrades). */
        Color tertiary;  /**< Couleur tertiaire (pour effets avances). */
    } colors;

    // === Degrades =====
    struct {
        float start_x;      /**< Position X de depart (0.0 a 1.0). */
        float start_y;      /**< Position Y de depart. */
        float end_x;        /**< Position X de fin. */
        float end_y;        /**< Position Y de fin. */
        float angle;        /**< Angle du degrade (pour DIAGONAL_GRADIENT). */
        struct {
            uint16_t center_x;  /**< Centre X (pour RADIAL_GRADIENT). */
            uint16_t center_y;  /**< Centre Y. */
            float radius;       /**< Rayon du degrade radial. */
        } radial;
    } gradient;

    // === Bordure =====
    struct {
        uint8_t width;          /**< Epaisseur de la bordure. */
        Color color;            /**< Couleur de la bordure (pour SOLID_BORDER). */
        struct {
            Color start;         /**< Couleur de depart (pour GRADIENT_BORDER). */
            Color end;           /**< Couleur de fin. */
        } gradient;
        union {
            uint8_t radius;     /**< Rayon uniforme (pour ROUNDED_BORDER). */
            struct {
                uint8_t top_left;     /**< Rayon du coin haut-gauche. */
                uint8_t top_right;    /**< Rayon du coin haut-droite. */
                uint8_t bottom_left;  /**< Rayon du coin bas-gauche. */
                uint8_t bottom_right; /**< Rayon du coin bas-droite. */
            } variable_radius;  /**< Rayons personnalises (pour ROUNDED_VARIABLE). */
            struct {
                uint8_t on_length;   /**< Longueur des segments visibles. */
                uint8_t off_length;  /**< Longueur des espaces. */
            } pattern;          /**< Motif de bordure (pour DOTTED/DASHED). */
        };
    } border;

    // === Formes Personnalisees =====
    struct {
        uint8_t num_points;     /**< Nombre de points. */
        Point* points;          /**< Pointeur vers les points. */
        uint8_t path_type;      /**< Type de chemin (0=POLYGON, 1=BEZIER_CUBIC). */
    } custom_shape;

    // === Effets Speciaux =====
    struct {
        bool enabled;           /**< Si l'effet est active. */
        uint8_t blur_radius;    /**< Rayon du flou (pour BLUR). */
        uint8_t offset_x;       /**< Decalage X (pour DROP_SHADOW). */
        uint8_t offset_y;       /**< Decalage Y. */
        Color color;            /**< Couleur de l'effet (pour GLOW, NEON, etc.). */
        uint8_t intensity;      /**< Intensite de l'effet (0-255). */
    } effect;

    // === Formes Predefinies =====
    struct {
        uint16_t radius;        /**< Rayon (pour CIRCLE). */
        struct {
            uint16_t a;         /**< Demi-grand axe (pour ELLIPSE). */
            uint16_t b;         /**< Demi-petit axe. */
        } ellipse;
        struct {
            uint16_t start_angle; /**< Angle de depart (pour ARC, PIE). */
            uint16_t end_angle;   /**< Angle de fin. */
        } arc;
    } shape;

    // === Animation =====
    struct {
        bool enabled;           /**< Si l'animation est activee. */
        uint16_t duration;      /**< Duree de l'animation (en ms). */
        uint16_t current_time;  /**< Temps ecoule depuis le debut. */
        float progress;        /**< Progression (0.0 a 1.0). */
    } animation;
} WidgetStyle;
```

---

## Complete WidgetStyle Structure (English Comments)

```c
typedef struct {
    uint64_t draw_style;  /**< Combination of WIDGET_DRAW_STYLE flags (uint64_t for 64 bits). */

    // ===== Colors and Background =====
    struct {
        Color primary;   /**< Primary color (for SOLID_FILL). */
        Color secondary; /**< Secondary color (for gradients). */
        Color tertiary;  /**< Tertiary color (for advanced effects). */
    } colors;

    // ===== Gradients =====
    struct {
        float start_x;      /**< Start X position (0.0 to 1.0) for linear gradients. */
        float start_y;      /**< Start Y position. */
        float end_x;        /**< End X position. */
        float end_y;        /**< End Y position. */
        float angle;        /**< Angle for diagonal gradients. */
        struct {
            uint16_t center_x;  /**< Center X for radial gradients. */
            uint16_t center_y;  /**< Center Y. */
            float radius;       /**< Radius. */
        } radial;
    } gradient;

    // ===== Border =====
    struct {
        uint8_t width;          /**< Border width. */
        Color color;            /**< Border color (for SOLID_BORDER). */
        struct {
            Color start;         /**< Start color (for GRADIENT_BORDER). */
            Color end;           /**< End color. */
        } gradient;
        union {
            uint8_t radius;     /**< Uniform radius (for ROUNDED_BORDER). */
            struct {
                uint8_t top_left;     /**< Top-left corner radius. */
                uint8_t top_right;    /**< Top-right corner radius. */
                uint8_t bottom_left;  /**< Bottom-left corner radius. */
                uint8_t bottom_right; /**< Bottom-right corner radius. */
            } variable_radius;  /**< Per-corner radius (for ROUNDED_VARIABLE). */
            struct {
                uint8_t on_length;   /**< Visible segment length (for DOTTED/DASHED). */
                uint8_t off_length;  /**< Space length. */
            } pattern;          /**< Pattern for dotted/dashed borders. */
        };
    } border;

    // ===== Custom Shapes =====
    struct {
        uint8_t num_points;     /**< Number of points (for POLYGON, BEZIER_PATH). */
        Point* points;          /**< Pointer to points array. */
        uint8_t path_type;      /**< Path type: 0=POLYGON, 1=BEZIER_CUBIC, etc. */
    } custom_shape;

    // ===== Effects =====
    struct {
        bool enabled;           /**< Effect enabled. */
        uint8_t blur_radius;    /**< Blur radius (for BLUR). */
        uint8_t offset_x;       /**< X offset (for shadows). */
        uint8_t offset_y;       /**< Y offset. */
        Color color;            /**< Effect color (for GLOW, NEON, etc.). */
        uint8_t intensity;      /**< Intensity (0-255). */
    } effect;

    // ===== Predefined Shapes =====
    struct {
        uint16_t radius;        /**< Radius (for CIRCLE). */
        struct {
            uint16_t a;         /**< Semi-major axis (for ELLIPSE). */
            uint16_t b;         /**< Semi-minor axis. */
        } ellipse;
        struct {
            uint16_t start_angle; /**< Start angle (for ARC, PIE). */
            uint16_t end_angle;   /**< End angle. */
        } arc;
    } shape;

    // ===== Animation =====
    struct {
        bool enabled;           /**< Animation enabled. */
        uint16_t duration;      /**< Duration in milliseconds. */
        uint16_t current_time;  /**< Elapsed time since start. */
        float progress;        /**< Progress (0.0 to 1.0). */
    } animation;
} WidgetStyle;
```

---

## Rendering Functions

### Draw Solid Fill

```c
void draw_solid_fill(Widget* widget) {
    if (widget->style.draw_style & WIDGET_DRAW_STYLE_TRANSPARENT_FILL) {
        return; // Nothing to draw for transparent
    }
    
    uint16_t color = widget->style.colors.primary;
    
    // Handle different shape types
    if (widget->style.draw_style & WIDGET_DRAW_STYLE_CIRCLE) {
        tft.fillCircle(
            widget->rect.x + widget->rect.width / 2,
            widget->rect.y + widget->rect.height / 2,
            widget->style.shape.radius,
            color
        );
    } else if (widget->style.draw_style & WIDGET_DRAW_STYLE_ROUNDED_RECT) {
        uint8_t radius = widget->style.border.radius;
        if (widget->style.draw_style & WIDGET_DRAW_STYLE_ROUNDED_VARIABLE) {
            // Use average radius for simplified drawing
            radius = (widget->style.border.variable_radius.top_left + 
                     widget->style.border.variable_radius.top_right +
                     widget->style.border.variable_radius.bottom_left +
                     widget->style.border.variable_radius.bottom_right) / 4;
        }
        tft.fillRoundRect(
            widget->rect.x,
            widget->rect.y,
            widget->rect.width,
            widget->rect.height,
            radius,
            color
        );
    } else {
        // Default: rectangle
        tft.fillRect(
            widget->rect.x,
            widget->rect.y,
            widget->rect.width,
            widget->rect.height,
            color
        );
    }
}
```

### Draw Gradients

```c
// Interpolate between two RGB565 colors
uint16_t interpolate_color(uint16_t color1, uint16_t color2, float ratio) {
    uint8_t r1 = (color1 >> 11) & 0x1F;
    uint8_t g1 = (color1 >> 5) & 0x3F;
    uint8_t b1 = color1 & 0x1F;

    uint8_t r2 = (color2 >> 11) & 0x1F;
    uint8_t g2 = (color2 >> 5) & 0x3F;
    uint8_t b2 = color2 & 0x1F;

    uint8_t r = r1 + (r2 - r1) * ratio;
    uint8_t g = g1 + (g2 - g1) * ratio;
    uint8_t b = b1 + (b2 - b1) * ratio;

    return (r << 11) | (g << 5) | b;
}

void draw_horizontal_gradient(Widget* widget) {
    uint16_t color1 = widget->style.colors.primary;
    uint16_t color2 = widget->style.colors.secondary;
    
    for (uint16_t i = 0; i < widget->rect.width; i++) {
        float ratio = (float)i / (widget->rect.width - 1);
        uint16_t color = interpolate_color(color1, color2, ratio);
        tft.drawFastVLine(
            widget->rect.x + i,
            widget->rect.y,
            widget->rect.height,
            color
        );
    }
}

void draw_vertical_gradient(Widget* widget) {
    uint16_t color1 = widget->style.colors.primary;
    uint16_t color2 = widget->style.colors.secondary;
    
    for (uint16_t i = 0; i < widget->rect.height; i++) {
        float ratio = (float)i / (widget->rect.height - 1);
        uint16_t color = interpolate_color(color1, color2, ratio);
        tft.drawFastHLine(
            widget->rect.x,
            widget->rect.y + i,
            widget->rect.width,
            color
        );
    }
}

void draw_radial_gradient(Widget* widget) {
    uint16_t color1 = widget->style.colors.primary;
    uint16_t color2 = widget->style.colors.secondary;
    uint16_t center_x = widget->style.gradient.radial.center_x;
    uint16_t center_y = widget->style.gradient.radial.center_y;
    float radius = widget->style.gradient.radial.radius;
    
    // Simplified radial gradient: draw concentric circles
    for (uint16_t r = 0; r <= radius; r++) {
        float ratio = (float)r / radius;
        uint16_t color = interpolate_color(color1, color2, ratio);
        tft.drawCircle(center_x, center_y, r, color);
    }
}

void draw_diagonal_gradient(Widget* widget) {
    uint16_t color1 = widget->style.colors.primary;
    uint16_t color2 = widget->style.colors.secondary;
    float angle = widget->style.gradient.angle * M_PI / 180.0f;
    
    // Diagonal gradient at specified angle
    for (uint16_t i = 0; i < widget->rect.width; i++) {
        for (uint16_t j = 0; j < widget->rect.height; j++) {
            // Calculate distance along gradient axis
            float dx = i - widget->rect.width / 2.0f;
            float dy = j - widget->rect.height / 2.0f;
            float dist = dx * cos(angle) + dy * sin(angle);
            float max_dist = sqrt(pow(widget->rect.width/2.0f, 2) + pow(widget->rect.height/2.0f, 2));
            float ratio = (dist + max_dist) / (2 * max_dist);
            ratio = fmax(0.0f, fmin(1.0f, ratio));
            
            uint16_t color = interpolate_color(color1, color2, ratio);
            tft.drawPixel(widget->rect.x + i, widget->rect.y + j, color);
        }
    }
}
```

### Draw Borders

```c
void draw_normal_border(Widget* widget) {
    tft.drawRect(
        widget->rect.x,
        widget->rect.y,
        widget->rect.width,
        widget->rect.height,
        widget->style.border.color
    );
}

void draw_rounded_border(Widget* widget) {
    uint8_t radius = widget->style.border.radius;
    tft.drawRoundRect(
        widget->rect.x,
        widget->rect.y,
        widget->rect.width,
        widget->rect.height,
        radius,
        widget->style.border.color
    );
}

void draw_dotted_border(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
                        uint16_t color, uint8_t width, uint8_t on_len, uint8_t off_len) {
    // Draw top and bottom
    for (uint16_t i = 0; i < w; i += on_len + off_len) {
        uint16_t segment_len = (i + on_len <= w) ? on_len : (w - i);
        tft.drawFastHLine(x + i, y, segment_len, color);
        tft.drawFastHLine(x + i, y + h - 1, segment_len, color);
    }
    // Draw left and right
    for (uint16_t i = 0; i < h; i += on_len + off_len) {
        uint16_t segment_len = (i + on_len <= h) ? on_len : (h - i);
        tft.drawFastVLine(x, y + i, segment_len, color);
        tft.drawFastVLine(x + w - 1, y + i, segment_len, color);
    }
}

void draw_dashed_border(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
                        uint16_t color, uint8_t width, uint8_t on_len, uint8_t off_len) {
    // Similar to dotted but with longer segments
    for (uint16_t i = 0; i < w; i += on_len + off_len) {
        uint16_t segment_len = (i + on_len <= w) ? on_len : (w - i);
        tft.drawFastHLine(x + i, y, segment_len, color);
        tft.drawFastHLine(x + i, y + h - 1, segment_len, color);
    }
    for (uint16_t i = 0; i < h; i += on_len + off_len) {
        uint16_t segment_len = (i + on_len <= h) ? on_len : (h - i);
        tft.drawFastVLine(x, y + i, segment_len, color);
        tft.drawFastVLine(x + w - 1, y + i, segment_len, color);
    }
}

void draw_double_border(Widget* widget) {
    uint16_t color = widget->style.border.color;
    uint8_t width = widget->style.border.width;
    
    // Outer border
    tft.drawRect(
        widget->rect.x,
        widget->rect.y,
        widget->rect.width,
        widget->rect.height,
        color
    );
    
    // Inner border
    tft.drawRect(
        widget->rect.x + width,
        widget->rect.y + width,
        widget->rect.width - 2 * width,
        widget->rect.height - 2 * width,
        color
    );
}

void draw_etched_border(Widget* widget) {
    // Etched effect: light color on top/left, dark on bottom/right
    uint16_t light_color = lighten_color(widget->style.border.color, 30);
    uint16_t dark_color = darken_color(widget->style.border.color, 30);
    
    // Top and left: light
    tft.drawFastHLine(widget->rect.x, widget->rect.y, widget->rect.width, light_color);
    tft.drawFastVLine(widget->rect.x, widget->rect.y, widget->rect.height, light_color);
    
    // Bottom and right: dark
    tft.drawFastHLine(widget->rect.x, widget->rect.y + widget->rect.height - 1, 
                     widget->rect.width, dark_color);
    tft.drawFastVLine(widget->rect.x + widget->rect.width - 1, widget->rect.y, 
                     widget->rect.height, dark_color);
}

void draw_embossed_border(Widget* widget) {
    // Embossed effect: dark on top/left, light on bottom/right
    uint16_t light_color = lighten_color(widget->style.border.color, 30);
    uint16_t dark_color = darken_color(widget->style.border.color, 30);
    
    // Top and left: dark
    tft.drawFastHLine(widget->rect.x, widget->rect.y, widget->rect.width, dark_color);
    tft.drawFastVLine(widget->rect.x, widget->rect.y, widget->rect.height, dark_color);
    
    // Bottom and right: light
    tft.drawFastHLine(widget->rect.x, widget->rect.y + widget->rect.height - 1, 
                     widget->rect.width, light_color);
    tft.drawFastVLine(widget->rect.x + widget->rect.width - 1, widget->rect.y, 
                     widget->rect.height, light_color);
}
```

### Color Utility Functions

```c
// Lighten a color by percentage
uint16_t lighten_color(uint16_t color, uint8_t percent) {
    uint8_t r = (color >> 11) & 0x1F;
    uint8_t g = (color >> 5) & 0x3F;
    uint8_t b = color & 0x1F;

    r = min(31, r + (31 - r) * percent / 100);
    g = min(63, g + (63 - g) * percent / 100);
    b = min(31, b + (31 - b) * percent / 100);

    return (r << 11) | (g << 5) | b;
}

// Darken a color by percentage
uint16_t darken_color(uint16_t color, uint8_t percent) {
    uint8_t r = (color >> 11) & 0x1F;
    uint8_t g = (color >> 5) & 0x3F;
    uint8_t b = color & 0x1F;

    r = r * (100 - percent) / 100;
    g = g * (100 - percent) / 100;
    b = b * (100 - percent) / 100;

    return (r << 11) | (g << 5) | b;
}

// Alpha blend two colors
uint16_t alpha_blend(uint16_t bg, uint16_t fg, uint8_t alpha) {
    if (alpha == 0) return bg;
    if (alpha == 255) return fg;

    uint8_t r1 = (bg >> 11) & 0x1F;
    uint8_t g1 = (bg >> 5) & 0x3F;
    uint8_t b1 = bg & 0x1F;

    uint8_t r2 = (fg >> 11) & 0x1F;
    uint8_t g2 = (fg >> 5) & 0x3F;
    uint8_t b2 = fg & 0x1F;

    uint8_t r = r1 + (r2 - r1) * alpha / 255;
    uint8_t g = g1 + (g2 - g1) * alpha / 255;
    uint8_t b = b1 + (b2 - b1) * alpha / 255;

    return (r << 11) | (g << 5) | b;
}
```

### Draw Effects

```c
void draw_drop_shadow(Widget* widget) {
    if (!widget->style.effect.enabled) return;

    uint8_t blur = widget->style.effect.blur_radius;
    uint8_t offset_x = widget->style.effect.offset_x;
    uint8_t offset_y = widget->style.effect.offset_y;
    uint16_t color = widget->style.effect.color;

    // Draw blurred rectangle under widget
    for (uint8_t b = 0; b <= blur; b++) {
        uint8_t alpha = (200 * (blur - b)) / blur; // Decreasing opacity (0-200)
        uint16_t shadow_color = alpha_blend(0x0000, color, alpha);

        tft.drawRect(
            widget->rect.x + offset_x - b,
            widget->rect.y + offset_y - b,
            widget->rect.width + 2 * b,
            widget->rect.height + 2 * b,
            shadow_color
        );
    }
}

void draw_glow_effect(Widget* widget) {
    if (!widget->style.effect.enabled) return;

    uint16_t color = widget->style.effect.color;
    uint8_t intensity = widget->style.effect.intensity;

    // Draw glowing halo around widget
    for (uint8_t r = 1; r <= 5; r++) {
        uint8_t alpha = intensity - (r * 20); // Reduce intensity with distance
        if (alpha <= 0) break;

        uint16_t glow_color = alpha_blend(0x0000, color, alpha);

        // Draw slightly larger rectangle
        uint8_t radius = widget->style.border.radius + r;
        tft.drawRoundRect(
            widget->rect.x - r,
            widget->rect.y - r,
            widget->rect.width + 2 * r,
            widget->rect.height + 2 * r,
            radius,
            glow_color
        );
    }
}

void draw_neon_effect(Widget* widget) {
    if (!widget->style.effect.enabled) return;

    uint16_t color = widget->style.effect.color;
    uint8_t intensity = widget->style.effect.intensity;

    // Neon effect: multiple glowing layers
    for (uint8_t r = 1; r <= 8; r++) {
        uint8_t alpha = intensity - (r * 15);
        if (alpha <= 0) break;

        uint16_t neon_color = alpha_blend(0x0000, color, alpha);

        tft.drawRoundRect(
            widget->rect.x - r,
            widget->rect.y - r,
            widget->rect.width + 2 * r,
            widget->rect.height + 2 * r,
            widget->style.border.radius + r,
            neon_color
        );
    }
}

void draw_metallic_effect(Widget* widget) {
    if (!(widget->style.draw_style & WIDGET_DRAW_STYLE_METALIC)) return;

    // Draw diagonal gradient for metallic reflection
    uint16_t color1 = lighten_color(widget->style.colors.primary, 20);
    uint16_t color2 = darken_color(widget->style.colors.primary, 20);

    for (uint16_t i = 0; i < widget->rect.width; i++) {
        float ratio = (float)i / widget->rect.width;
        uint16_t color = interpolate_color(color1, color2, ratio);
        tft.drawFastVLine(widget->rect.x + i, widget->rect.y, widget->rect.height, color);
    }

    // Add light border for reflection
    tft.drawRoundRect(
        widget->rect.x + 1,
        widget->rect.y + 1,
        widget->rect.width - 2,
        widget->rect.height - 2,
        widget->style.border.radius - 1,
        0xFFFF // White
    );
}

void draw_glass_effect(Widget* widget) {
    if (!(widget->style.draw_style & WIDGET_DRAW_STYLE_GLASS)) return;

    // Semi-transparent fill
    uint16_t base_color = widget->style.colors.primary;
    uint16_t glass_color = alpha_blend(0xFFFF, base_color, 128); // 50% transparent white
    
    tft.fillRoundRect(
        widget->rect.x,
        widget->rect.y,
        widget->rect.width,
        widget->rect.height,
        widget->style.border.radius,
        glass_color
    );

    // Add white highlight on top-left
    tft.drawRoundRect(
        widget->rect.x + 1,
        widget->rect.y + 1,
        widget->rect.width / 2,
        widget->rect.height / 2,
        widget->style.border.radius,
        alpha_blend(0xFFFF, 0xFFFF, 64)
    );
}
```

### Draw Custom Shapes

```c
void draw_polygon(Point* points, uint8_t num_points, uint16_t color) {
    if (num_points < 2) return;
    for (uint8_t i = 0; i < num_points - 1; i++) {
        tft.drawLine(points[i].x, points[i].y, points[i+1].x, points[i+1].y, color);
    }
    tft.drawLine(points[num_points-1].x, points[num_points-1].y, points[0].x, points[0].y, color);
}

void draw_filled_polygon(Point* points, uint8_t num_points, uint16_t fill_color, uint16_t border_color) {
    if (num_points < 3) return;
    
    // Simple flood fill approach (for convex polygons)
    // Find bounding box
    uint16_t min_x = points[0].x, max_x = points[0].x;
    uint16_t min_y = points[0].y, max_y = points[0].y;
    for (uint8_t i = 1; i < num_points; i++) {
        min_x = min(min_x, points[i].x);
        max_x = max(max_x, points[i].x);
        min_y = min(min_y, points[i].y);
        max_y = max(max_y, points[i].y);
    }
    
    // Ray casting algorithm would be better but more complex
    // For simplicity, just draw the border
    draw_polygon(points, num_points, border_color);
}

void draw_ellipse(uint16_t center_x, uint16_t center_y, uint16_t a, uint16_t b, uint16_t color) {
    // Midpoint ellipse algorithm
    float aa = a * a;
    float bb = b * b;
    float x = a;
    float y = 0;
    float dx, dy;
    
    // Region 1
    dx = 2 * bb * x;
    dy = 2 * aa * y;
    float p1 = bb - aa * b + 0.25 * aa;
    while (dx >= dy) {
        tft.drawPixel(center_x + x, center_y + y, color);
        tft.drawPixel(center_x - x, center_y + y, color);
        tft.drawPixel(center_x + x, center_y - y, color);
        tft.drawPixel(center_x - x, center_y - y, color);
        
        if (p1 < 0) {
            p1 += bb * (2 * x - 1);
        } else {
            y++;
            p1 += bb * (2 * x - 1) - aa * (2 * y - 1);
        }
        x--;
        dx = 2 * bb * x;
        dy = 2 * aa * y;
    }
    
    // Region 2
    float p2 = bb * (x + 0.5) * (x + 0.5) + aa * (y - 1) * (y - 1) - aa * bb;
    while (y <= b) {
        tft.drawPixel(center_x + x, center_y + y, color);
        tft.drawPixel(center_x - x, center_y + y, color);
        tft.drawPixel(center_x + x, center_y - y, color);
        tft.drawPixel(center_x - x, center_y - y, color);
        
        if (p2 > 0) {
            p2 -= aa * (2 * y - 1);
        } else {
            x--;
            p2 += bb * (2 * x + 1) - aa * (2 * y - 1);
        }
        y++;
    }
}

void draw_arc(uint16_t center_x, uint16_t center_y, uint16_t radius, 
              uint16_t start_angle, uint16_t end_angle, uint16_t color) {
    // Draw arc from start_angle to end_angle (in degrees)
    for (int angle = start_angle; angle <= end_angle; angle++) {
        float rad = angle * M_PI / 180.0f;
        uint16_t x = center_x + radius * cos(rad);
        uint16_t y = center_y + radius * sin(rad);
        tft.drawPixel(x, y, color);
    }
}

void draw_pie(uint16_t center_x, uint16_t center_y, uint16_t radius,
              uint16_t start_angle, uint16_t end_angle, uint16_t fill_color, uint16_t border_color) {
    // Draw pie slice (filled arc sector)
    // First draw lines from center to arc endpoints
    float rad1 = start_angle * M_PI / 180.0f;
    float rad2 = end_angle * M_PI / 180.0f;
    
    uint16_t x1 = center_x + radius * cos(rad1);
    uint16_t y1 = center_y + radius * sin(rad1);
    uint16_t x2 = center_x + radius * cos(rad2);
    uint16_t y2 = center_y + radius * sin(rad2);
    
    tft.drawLine(center_x, center_y, x1, y1, border_color);
    tft.drawLine(center_x, center_y, x2, y2, border_color);
    draw_arc(center_x, center_y, radius, start_angle, end_angle, border_color);
    
    // Fill would require flood fill or triangle filling
    // Simplified: just draw the arc for now
}
```

---

## Summary

The Style System provides:

- **40+ draw style flags** organized into categories (borders, fills, shapes, effects, animations)
- **Bitmask combination** allowing any style mix via | operator
- **Category masks** for filtering styles
- **Complete WidgetStyle structure** with all properties
- **Rendering functions** for every style type
- **ESP8266 optimizations** (precomputation, pooling, caching)
- **Color utility functions** (interpolation, lighten, darken, alpha blend)

---

*Source: Extracted from discussion_guikit.txt, lines 3270-4260*
*Documentation organized by Mistral Vibe*
