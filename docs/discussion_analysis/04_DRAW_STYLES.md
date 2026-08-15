# Draw Styles (WIDGET_DRAW_STYLE)

> Extracted from discussion_guikit.txt - Complete widget drawing style system

## Overview

The draw style system controls how widgets are rendered on the TFT display. Three versions evolved in the discussion, culminating in a bitmask flag system that allows combining multiple styles.

---

## Version 1: Basic Enum

Initial simple enum with core style options:

```c
/**
 * @brief Draw styles for widgets (borders, background, etc.).
 * @note Used to define how a widget should be drawn on the TFT screen.
 */
typedef enum {
    // ===== Border Styles =====
    WIDGET_DRAW_STYLE_NORMAL = 0,      /**< Square border (default). */
    WIDGET_DRAW_STYLE_NO_BORDER,        /**< No border (background only). */
    WIDGET_DRAW_STYLE_ROUNDED,         /**< Rounded border (fixed or customizable radius). */
    WIDGET_DRAW_STYLE_CUSTOM_BEZIER,   /**< Custom border via Bezier path. */

    // ===== Background Styles =====
    WIDGET_DRAW_STYLE_SOLID_COLOR = 10, /**< Solid color background. */
    WIDGET_DRAW_STYLE_COLOR_GRADIENT,  /**< Gradient background (linear or radial). */

    // ===== Combined Styles =====
    WIDGET_DRAW_STYLE_ROUNDED_GRADIENT = 20, /**< Rounded border + gradient. */
    WIDGET_DRAW_STYLE_CUSTOM_BEZIER_GRADIENT, /**< Bezier path + gradient. */

    // ===== Custom =====
    WIDGET_DRAW_STYLE_CUSTOM = 255,    /**< Custom style (user-defined). */
} WIDGET_DRAW_STYLE;
```

---

## Version 2: Extended Enum

Expanded with more border, fill, and effect options:

```c
/**
 * @brief Draw styles for widgets (borders, background, shadows, effects).
 * @note Organized by categories for better readability.
 */
typedef enum {
    // ===== Basic Border Styles =====
    WIDGET_DRAW_STYLE_NORMAL = 0,       /**< Square border (default). */
    WIDGET_DRAW_STYLE_NO_BORDER,        /**< No border (background only). */
    WIDGET_DRAW_STYLE_ROUNDED,          /**< Rounded border (uniform radius). */
    WIDGET_DRAW_STYLE_ROUNDED_VARIABLE, /**< Rounded border with per-corner radius. */

    // ===== Advanced Border Styles =====
    WIDGET_DRAW_STYLE_DOTTED = 10,      /**< Dotted border. */
    WIDGET_DRAW_STYLE_DASHED,            /**< Dashed border. */
    WIDGET_DRAW_STYLE_DOUBLE,            /**< Double border. */
    WIDGET_DRAW_STYLE_ETCHED,            /**< Etched border (3D recessed effect). */
    WIDGET_DRAW_STYLE_EMBOSSED,          /**< Embossed border (3D raised effect). */

    // ===== Background Styles =====
    WIDGET_DRAW_STYLE_SOLID_COLOR = 20, /**< Solid color. */
    WIDGET_DRAW_STYLE_COLOR_GRADIENT,   /**< Linear gradient. */
    WIDGET_DRAW_STYLE_RADIAL_GRADIENT,   /**< Radial gradient. */
    WIDGET_DRAW_STYLE_CONIC_GRADIENT,    /**< Conic gradient (circular sector). */
    WIDGET_DRAW_STYLE_HATCHED,          /**< Hatched pattern. */
    WIDGET_DRAW_STYLE_PATTERN,          /**< Custom pattern. */

    // ===== Custom Path Styles =====
    WIDGET_DRAW_STYLE_CUSTOM_BEZIER = 30, /**< Bezier curve path. */
    WIDGET_DRAW_STYLE_CUSTOM_POLYGON,    /**< Polygon shape. */
    WIDGET_DRAW_STYLE_CUSTOM_SVG_PATH,   /**< SVG-style path. */

    // ===== Combined Styles =====
    WIDGET_DRAW_STYLE_ROUNDED_GRADIENT = 40,       /**< Rounded + gradient. */
    WIDGET_DRAW_STYLE_ROUNDED_RADIAL_GRADIENT,     /**< Rounded + radial gradient. */
    WIDGET_DRAW_STYLE_DOTTED_GRADIENT,              /**< Dotted + gradient. */
    WIDGET_DRAW_STYLE_CUSTOM_BEZIER_GRADIENT,       /**< Bezier + gradient. */

    // ===== Special Effects =====
    WIDGET_DRAW_STYLE_DROP_SHADOW = 50, /**< Drop shadow. */
    WIDGET_DRAW_STYLE_INNER_SHADOW,      /**< Inner shadow (inset). */
    WIDGET_DRAW_STYLE_GLOW,               /**< Glow effect. */
    WIDGET_DRAW_STYLE_BLUR,               /**< Blur effect. */
    WIDGET_DRAW_STYLE_TRANSPARENT,        /**< Transparent. */

    // ===== Special Shapes =====
    WIDGET_DRAW_STYLE_CIRCULAR = 60,    /**< Circular shape. */
    WIDGET_DRAW_STYLE_ELLIPSE,           /**< Elliptical shape. */
    WIDGET_DRAW_STYLE_PIE,               /**< Pie slice (for charts). */
    WIDGET_DRAW_STYLE_ARC,               /**< Arc segment. */

    // ===== Dynamic Styles (Animations) =====
    WIDGET_DRAW_STYLE_PULSE = 70,        /**< Pulsing animation. */
    WIDGET_DRAW_STYLE_ANIMATED_GRADIENT, /**< Animated gradient. */

    // ===== Custom =====
    WIDGET_DRAW_STYLE_CUSTOM = 255,      /**< User-defined. */
} WIDGET_DRAW_STYLE;
```

### Style Explanations

#### Border Styles
| Style | Description | Typical Use |
|-------|-------------|-------------|
| NORMAL | Square border | Default for most widgets |
| NO_BORDER | No border, background only | Minimalist designs |
| ROUNDED | Uniform rounded corners | Modern buttons |
| ROUNDED_VARIABLE | Different radius per corner | Custom rounded designs |
| DOTTED | Dotted border pattern | Light separators |
| DASHED | Dashed border pattern | Discontinuous borders |
| DOUBLE | Two parallel borders | Emphasized elements |
| ETCHED | 3D recessed look | Classic GUI elements |
| EMBOSSED | 3D raised look | Classic GUI elements |

#### Fill Styles
| Style | Description | Typical Use |
|-------|-------------|-------------|
| SOLID_COLOR | Single color fill | Most widgets |
| COLOR_GRADIENT | Linear color transition | Styled buttons |
| RADIAL_GRADIENT | Color from center out | Circular elements |
| CONIC_GRADIENT | Color in circular sector | Special effects |
| HATCHED | Cross-hatch pattern | Textured backgrounds |
| PATTERN | Custom repeated pattern | Complex backgrounds |

#### Special Effects
| Style | Description | Typical Use |
|-------|-------------|-------------|
| DROP_SHADOW | Shadow behind widget | Floating elements |
| INNER_SHADOW | Shadow inside widget | Inset/pressed look |
| GLOW | Light halo around widget | Active states |
| BLUR | Blurred effect | Backgrounds |
| TRANSPARENT | No fill, optional border | Overlays |

#### Special Shapes
| Style | Description | Typical Use |
|-------|-------------|-------------|
| CIRCULAR | Perfect circle | Round buttons, indicators |
| ELLIPSE | Oval/elliptical | Special buttons |
| PIE | Pie slice | Charts, gauges |
| ARC | Circular arc | Progress indicators |

#### Dynamic Styles
| Style | Description | Typical Use |
|-------|-------------|-------------|
| PULSE | Size/opacity pulsation | Attention animations |
| ANIMATED_GRADIENT | Moving color gradient | Dynamic backgrounds |

---

## Version 3: Bitmask Flags (Final - Recommended)

The key insight: **styles need to be combinable**. A button can be ROUNDED + COLOR_GRADIENT + DROP_SHADOW simultaneously.

Solution: Use bitmask flags where each style is a unique bit in a uint32_t (or uint64_t):

```c
/**
 * @brief Draw style flags for widgets.
 * @note Each flag is a unique bit, allowing combinations via | (OR).
 *       Example: WIDGET_DRAW_STYLE_ROUNDED | WIDGET_DRAW_STYLE_COLOR_GRADIENT | WIDGET_DRAW_STYLE_DROP_SHADOW
 */
typedef enum {
    // ========== BORDER STYLES ==========
    WIDGET_DRAW_STYLE_NO_BORDER        = 0,            /**< No border (default if no border flag). */
    WIDGET_DRAW_STYLE_SOLID_BORDER      = 1 << 0,       /**< Solid border (default). */
    WIDGET_DRAW_STYLE_ROUNDED_BORDER    = 1 << 1,       /**< Rounded border (uniform radius). */
    WIDGET_DRAW_STYLE_ROUNDED_VARIABLE  = 1 << 2,       /**< Rounded with per-corner radius. */
    WIDGET_DRAW_STYLE_DOTTED_BORDER     = 1 << 3,       /**< Dotted border pattern. */
    WIDGET_DRAW_STYLE_DASHED_BORDER     = 1 << 4,       /**< Dashed border pattern. */
    WIDGET_DRAW_STYLE_DOUBLE_BORDER     = 1 << 5,       /**< Double border. */
    WIDGET_DRAW_STYLE_ETCHED_BORDER     = 1 << 6,       /**< Etched (3D recessed). */
    WIDGET_DRAW_STYLE_EMBOSSED_BORDER   = 1 << 7,       /**< Embossed (3D raised). */
    WIDGET_DRAW_STYLE_DOTTED_ROUNDED    = 1 << 8,       /**< Rounded + dotted. */
    WIDGET_DRAW_STYLE_DASHED_ROUNDED    = 1 << 9,       /**< Rounded + dashed. */

    // ========== FILL STYLES ==========
    WIDGET_DRAW_STYLE_SOLID_FILL        = 1 << 10,      /**< Solid color fill (default). */
    WIDGET_DRAW_STYLE_HORIZONTAL_GRADIENT = 1 << 11,    /**< Horizontal gradient. */
    WIDGET_DRAW_STYLE_VERTICAL_GRADIENT = 1 << 12,      /**< Vertical gradient. */
    WIDGET_DRAW_STYLE_DIAGONAL_GRADIENT = 1 << 13,      /**< Diagonal gradient (45°). */
    WIDGET_DRAW_STYLE_RADIAL_GRADIENT   = 1 << 14,      /**< Radial gradient (centered). */
    WIDGET_DRAW_STYLE_CONIC_GRADIENT    = 1 << 15,      /**< Conic gradient (sector). */
    WIDGET_DRAW_STYLE_HATCHED_FILL       = 1 << 16,      /**< Hatched pattern. */
    WIDGET_DRAW_STYLE_CROSSHATCH_FILL    = 1 << 17,      /**< Cross-hatched (X pattern). */
    WIDGET_DRAW_STYLE_DOT_FILL           = 1 << 18,      /**< Dot pattern. */
    WIDGET_DRAW_STYLE_PATTERN_FILL       = 1 << 19,      /**< Custom pattern. */
    WIDGET_DRAW_STYLE_TRANSPARENT_FILL   = 1 << 20,      /**< Transparent fill. */

    // ========== CUSTOM SHAPES ==========
    WIDGET_DRAW_STYLE_RECTANGLE         = 1 << 21,      /**< Rectangle (default). */
    WIDGET_DRAW_STYLE_CIRCLE            = 1 << 22,      /**< Circle. */
    WIDGET_DRAW_STYLE_ELLIPSE           = 1 << 23,      /**< Ellipse. */
    WIDGET_DRAW_STYLE_TRIANGLE          = 1 << 24,      /**< Triangle. */
    WIDGET_DRAW_STYLE_POLYGON           = 1 << 25,      /**< Polygon. */
    WIDGET_DRAW_STYLE_BEZIER_PATH       = 1 << 26,      /**< Bezier curve path. */
    WIDGET_DRAW_STYLE_ARC               = 1 << 27,      /**< Arc. */
    WIDGET_DRAW_STYLE_PIE               = 1 << 28,      /**< Pie slice. */
    WIDGET_DRAW_STYLE_ROUNDED_RECT      = 1 << 29,      /**< Rounded rectangle. */

    // ========== VISUAL EFFECTS ==========
    WIDGET_DRAW_STYLE_DROP_SHADOW       = 1ULL << 31,   /**< External drop shadow. */
    WIDGET_DRAW_STYLE_INNER_SHADOW       = 1ULL << 32,   /**< Internal shadow. */
    WIDGET_DRAW_STYLE_GLOW               = 1ULL << 33,   /**< Glow effect. */
    WIDGET_DRAW_STYLE_BLUR               = 1ULL << 34,   /**< Blur effect. */
    WIDGET_DRAW_STYLE_METALIC            = 1ULL << 35,   /**< Metallic effect. */
    WIDGET_DRAW_STYLE_GLASS              = 1ULL << 36,   /**< Glass effect. */
    WIDGET_DRAW_STYLE_PLASTIC            = 1ULL << 37,   /**< Plastic effect. */
    WIDGET_DRAW_STYLE_NEON               = 1ULL << 38,   /**< Neon effect. */
    WIDGET_DRAW_STYLE_GRADIENT_BORDER    = 1ULL << 39,   /**< Gradient border. */

    // ========== ANIMATION EFFECTS ==========
    WIDGET_DRAW_STYLE_PULSE              = 1ULL << 40,   /**< Pulsing animation. */
    WIDGET_DRAW_STYLE_ANIMATED_GRADIENT  = 1ULL << 41,   /**< Animated gradient. */
    WIDGET_DRAW_STYLE_ROTATE             = 1ULL << 42,   /**< Rotation animation. */
    WIDGET_DRAW_STYLE_SHAKE              = 1ULL << 43,   /**< Shake animation. */
    WIDGET_DRAW_STYLE_FADE_IN_OUT        = 1ULL << 44,   /**< Fade animation. */

    // ========== CATEGORY MASKS ==========
    WIDGET_DRAW_STYLE_BORDER_MASK        = 0x000003FF,   /**< Mask for border styles (bits 0-9). */
    WIDGET_DRAW_STYLE_FILL_MASK          = 0x000FFC00,   /**< Mask for fill styles (bits 10-19). */
    WIDGET_DRAW_STYLE_SHAPE_MASK         = 0x01F00000,   /**< Mask for shapes (bits 20-29). */
    WIDGET_DRAW_STYLE_EFFECT_MASK        = 0xFFFF00000,  /**< Mask for visual effects (bits 30-44). */
} WIDGET_DRAW_STYLE;
```

### Combining Styles

```c
// Button with rounded border, horizontal gradient, and drop shadow
uint32_t style = WIDGET_DRAW_STYLE_ROUNDED_BORDER 
              | WIDGET_DRAW_STYLE_HORIZONTAL_GRADIENT 
              | WIDGET_DRAW_STYLE_DROP_SHADOW;

// Transparent widget with dotted border
uint32_t style2 = WIDGET_DRAW_STYLE_TRANSPARENT_FILL 
               | WIDGET_DRAW_STYLE_DOTTED_BORDER;

// Circular button with glow effect
uint32_t style3 = WIDGET_DRAW_STYLE_CIRCLE 
               | WIDGET_DRAW_STYLE_GLOW;
```

### Checking Styles

```c
// Check if widget has a specific style
if (widget->style.draw_style & WIDGET_DRAW_STYLE_ROUNDED_BORDER) {
    // Widget has rounded border
}

// Check if fill style is specifically a gradient
if ((widget->style.draw_style & WIDGET_DRAW_STYLE_FILL_MASK) 
    == WIDGET_DRAW_STYLE_COLOR_GRADIENT) {
    // Fill is a color gradient
}

// Extract only border styles
uint32_t border_style = widget->style.draw_style & WIDGET_DRAW_STYLE_BORDER_MASK;

// Extract only fill styles
uint32_t fill_style = widget->style.draw_style & WIDGET_DRAW_STYLE_FILL_MASK;
```

### Utility Functions

```c
// Add a style to a widget
void widget_add_style(Widget* widget, WIDGET_DRAW_STYLE style) {
    widget->style.draw_style |= style;
}

// Remove a style from a widget
void widget_remove_style(Widget* widget, WIDGET_DRAW_STYLE style) {
    widget->style.draw_style &= ~style;
}

// Check if widget has a specific style
bool widget_has_style(Widget* widget, WIDGET_DRAW_STYLE style) {
    return (widget->style.draw_style & style) == style;
}

// Reset all styles to default
void widget_reset_style(Widget* widget) {
    widget->style.draw_style = WIDGET_DRAW_STYLE_SOLID_BORDER 
                             | WIDGET_DRAW_STYLE_SOLID_FILL 
                             | WIDGET_DRAW_STYLE_RECTANGLE;
}

// Check border style
bool widget_has_border_style(Widget* widget, WIDGET_DRAW_STYLE style) {
    return (widget->style.draw_style & WIDGET_DRAW_STYLE_BORDER_MASK & style) == style;
}

// Check fill style
bool widget_has_fill_style(Widget* widget, WIDGET_DRAW_STYLE style) {
    return (widget->style.draw_style & WIDGET_DRAW_STYLE_FILL_MASK & style) == style;
}
```

### Common Style Macros

```c
// Predefined style combinations
#define STYLE_DEFAULT                   (WIDGET_DRAW_STYLE_SOLID_BORDER | WIDGET_DRAW_STYLE_SOLID_FILL | WIDGET_DRAW_STYLE_RECTANGLE)
#define STYLE_ROUNDED_BUTTON            (WIDGET_DRAW_STYLE_ROUNDED_BORDER | WIDGET_DRAW_STYLE_HORIZONTAL_GRADIENT | WIDGET_DRAW_STYLE_DROP_SHADOW)
#define STYLE_METALIC_BUTTON            (WIDGET_DRAW_STYLE_ROUNDED_BORDER | WIDGET_DRAW_STYLE_METALIC | WIDGET_DRAW_STYLE_DROP_SHADOW)
#define STYLE_GLOW_CIRCLE               (WIDGET_DRAW_STYLE_CIRCLE | WIDGET_DRAW_STYLE_GLOW | WIDGET_DRAW_STYLE_SOLID_FILL)
#define STYLE_TRANSPARENT_DOTTED         (WIDGET_DRAW_STYLE_TRANSPARENT_FILL | WIDGET_DRAW_STYLE_DOTTED_BORDER)
#define STYLE_ANIMATED_GRADIENT_BUTTON   (WIDGET_DRAW_STYLE_ROUNDED_BORDER | WIDGET_DRAW_STYLE_ANIMATED_GRADIENT | WIDGET_DRAW_STYLE_GLOW)

// More combinations
#define STYLE_SOLID_RED_FILL            (WIDGET_DRAW_STYLE_SOLID_FILL | WIDGET_DRAW_STYLE_RECTANGLE)
#define STYLE_GLASS_PANEL               (WIDGET_DRAW_STYLE_RECTANGLE | WIDGET_DRAW_STYLE_GLASS | WIDGET_DRAW_STYLE_DROP_SHADOW)
#define STYLE_NEON_BUTTON               (WIDGET_DRAW_STYLE_CIRCLE | WIDGET_DRAW_STYLE_NEON | WIDGET_DRAW_STYLE_SOLID_FILL)
#define STYLE_PULSING_BUTTON            (WIDGET_DRAW_STYLE_ROUNDED_BORDER | WIDGET_DRAW_STYLE_PULSE | WIDGET_DRAW_STYLE_SOLID_FILL)
```

---

## WidgetStyle Structure

The complete structure to support all style options:

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
        uint8_t path_type;      /**< Type: 0=POLYGON, 1=BEZIER_CUBIC, etc. */
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

    // ===== Special Shapes =====
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
        uint16_t current_time;  /**< Elapsed time. */
        float progress;        /**< Progress (0.0 to 1.0). */
    } animation;
} WidgetStyle;
```

---

## Usage Examples

### Example 1: Rounded Button with Horizontal Gradient and Drop Shadow

```c
WidgetButton* button = new_widget(WIDGET_TYPE_BUTTON);
button->base.style.draw_style =
    WIDGET_DRAW_STYLE_ROUNDED_BORDER |
    WIDGET_DRAW_STYLE_HORIZONTAL_GRADIENT |
    WIDGET_DRAW_STYLE_DROP_SHADOW;

// Configure gradient
button->base.style.colors.primary = 0xF800;   // Red
button->base.style.colors.secondary = 0x001F; // Blue
button->base.style.gradient.start_x = 0;
button->base.style.gradient.start_y = 0;
button->base.style.gradient.end_x = 1;
button->base.style.gradient.end_y = 0; // Horizontal gradient

// Configure rounded border
button->base.style.border.radius = 10;
button->base.style.border.width = 2;
button->base.style.border.color = 0xFFFF; // White

// Configure shadow
button->base.style.effect.enabled = true;
button->base.style.effect.offset_x = 2;
button->base.style.effect.offset_y = 2;
button->base.style.effect.color = 0x8410; // Gray
button->base.style.effect.blur_radius = 3;
```

### Example 2: Transparent Widget with Dotted Border and Neon Effect

```c
Widget* widget = new_widget(WIDGET_TYPE_VIEW);
widget->style.draw_style =
    WIDGET_DRAW_STYLE_TRANSPARENT_FILL |
    WIDGET_DRAW_STYLE_DOTTED_BORDER |
    WIDGET_DRAW_STYLE_NEON;

// Configure dotted border
widget->style.border.width = 1;
widget->style.border.color = 0x07E0; // Green
widget->style.border.pattern.on_length = 2;
widget->style.border.pattern.off_length = 2;

// Configure neon effect
widget->style.effect.enabled = true;
widget->style.effect.color = 0x07E0; // Green (same as border)
widget->style.effect.intensity = 200; // High intensity
```

### Example 3: Circular Button with Radial Gradient and Metallic Effect

```c
WidgetButton* button = new_widget(WIDGET_TYPE_BUTTON);
button->base.style.draw_style =
    WIDGET_DRAW_STYLE_CIRCLE |
    WIDGET_DRAW_STYLE_RADIAL_GRADIENT |
    WIDGET_DRAW_STYLE_METALIC;

// Configure circular shape
button->base.style.shape.radius = 30;

// Configure radial gradient
button->base.style.colors.primary = 0xC618;   // Gold
button->base.style.colors.secondary = 0x8410; // Gray
button->base.style.gradient.radial.center_x = 30;
button->base.style.gradient.radial.center_y = 30;
button->base.style.gradient.radial.radius = 30;

// Configure metallic effect
button->base.style.effect.enabled = true;
button->base.style.effect.intensity = 150;
```

### Example 4: Polygon Widget with Hatched Fill

```c
// Define a triangle
Point triangle_points[] = {
    {10, 10},  // Point 1
    {50, 10},  // Point 2
    {30, 50}   // Point 3
};

Widget* widget = new_widget(WIDGET_TYPE_CUSTOM);
widget->style.draw_style =
    WIDGET_DRAW_STYLE_POLYGON |
    WIDGET_DRAW_STYLE_HATCHED_FILL;

// Configure polygon
widget->style.custom_shape.num_points = 3;
widget->style.custom_shape.points = triangle_points;
widget->style.custom_shape.path_type = 0; // POLYGON

// Configure hatched fill
widget->style.colors.primary = 0xFFFF; // White
widget->style.colors.secondary = 0x0000; // Black
```

### Example 5: Rounded Rect Button with Glass Effect and Pulse Animation

```c
WidgetButton* button = new_widget(WIDGET_TYPE_BUTTON);
button->base.style.draw_style =
    WIDGET_DRAW_STYLE_ROUNDED_RECT |
    WIDGET_DRAW_STYLE_GLASS |
    WIDGET_DRAW_STYLE_PULSE;

// Configure shape
button->base.style.border.radius = 15;

// Configure glass effect
button->base.style.effect.enabled = true;
button->base.style.effect.intensity = 100;
button->base.style.colors.primary = 0x07FF; // Cyan (glass effect)

// Configure pulse animation
button->base.style.animation.enabled = true;
button->base.style.animation.duration = 1000; // 1 second
button->base.style.animation.progress = 0.0f; // Start at 0%
```

---

## Rendering Pipeline

The recommended rendering order for widgets with combined styles:

```c
void draw_widget(Widget* widget) {
    // 1. Draw background effects (shadows that go behind)
    if (widget->style.draw_style & WIDGET_DRAW_STYLE_DROP_SHADOW) {
        draw_drop_shadow(widget,
            widget->style.effect.blur_radius,
            widget->style.effect.offset_x,
            widget->style.effect.offset_y,
            widget->style.effect.color);
    }

    // 2. Draw fill (based on fill style)
    uint64_t fill_style = widget->style.draw_style & WIDGET_DRAW_STYLE_FILL_MASK;
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
        // ... other fill styles
        default:
            if (!(widget->style.draw_style & WIDGET_DRAW_STYLE_TRANSPARENT_FILL)) {
                draw_solid_fill(widget);
            }
            break;
    }

    // 3. Draw shape/border (based on border and shape styles)
    uint64_t border_style = widget->style.draw_style & WIDGET_DRAW_STYLE_BORDER_MASK;
    uint64_t shape_style = widget->style.draw_style & WIDGET_DRAW_STYLE_SHAPE_MASK;
    
    // Handle special shapes first
    if (shape_style == WIDGET_DRAW_STYLE_CIRCLE) {
        draw_circle(widget);
    } else if (shape_style == WIDGET_DRAW_STYLE_POLYGON) {
        draw_polygon(widget);
    } else if (shape_style == WIDGET_DRAW_STYLE_BEZIER_PATH) {
        draw_bezier_path(widget);
    } else {
        // Default: draw rectangle with appropriate border
        draw_rectangle_border(widget, border_style);
    }

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

---

## ESP8266 Optimizations

### Precompute Gradients

Gradients can be expensive if recalculated every frame. Precompute them:

```c
// Array to store precomputed gradient colors
uint16_t gradient_cache[320]; // For 320px wide screen

void precompute_gradient(Widget* widget) {
    uint16_t width = widget->rect.width;
    uint16_t color1 = widget->style.colors.primary;
    uint16_t color2 = widget->style.colors.secondary;

    for (uint16_t i = 0; i < width; i++) {
        float ratio = (float)i / (width - 1);
        gradient_cache[i] = interpolate_color(color1, color2, ratio);
    }
}

void draw_horizontal_gradient(Widget* widget) {
    precompute_gradient(widget);
    for (uint16_t i = 0; i < widget->rect.width; i++) {
        tft.drawFastVLine(
            widget->rect.x + i,
            widget->rect.y,
            widget->rect.height,
            gradient_cache[i]
        );
    }
}
```

### Pool for Custom Shapes

Avoid dynamic allocation for polygon/Bezier points:

```c
#define MAX_CUSTOM_SHAPES 10
#define MAX_POINTS_PER_SHAPE 20

Point shape_pool[MAX_CUSTOM_SHAPES][MAX_POINTS_PER_SHAPE];
uint8_t shape_pool_index = 0;

Point* allocate_shape_points(uint8_t num_points) {
    if (shape_pool_index >= MAX_CUSTOM_SHAPES) return NULL;
    if (num_points > MAX_POINTS_PER_SHAPE) return NULL;
    return shape_pool[shape_pool_index++];
}
```

### Style Caching

Cache frequently used style combinations:

```c
#define MAX_CACHED_STYLES 20

typedef struct {
    uint64_t style;
    WidgetStyle cached_style;
} CachedStyle;

CachedStyle style_cache[MAX_CACHED_STYLES];
uint8_t style_cache_count = 0;

WidgetStyle* get_cached_style(uint64_t style) {
    for (uint8_t i = 0; i < style_cache_count; i++) {
        if (style_cache[i].style == style) {
            return &style_cache[i].cached_style;
        }
    }
    return NULL;  // Not found
}

void cache_style(uint64_t style, WidgetStyle* src_style) {
    if (style_cache_count >= MAX_CACHED_STYLES) return;
    style_cache[style_cache_count].style = style;
    style_cache[style_cache_count].cached_style = *src_style;
    style_cache_count++;
}
```

---

## Summary

| Version | Approach | Combinable | Type-Safe | ESP8266 Friendly | Lines of Code |
|---------|----------|------------|-----------|-------------------|---------------|
| 1 | Basic enum | ❌ No | ✅ Yes | ✅ Yes | ~15 |
| 2 | Extended enum | ❌ No | ✅ Yes | ✅ Yes | ~50 |
| 3 | **Bitmask flags** | ✅ **Yes** | ✅ Yes | ✅ **Yes** | ~100 | **Recommended** |

The bitmask flag approach (Version 3) is the recommended solution because:
- ✅ Allows combining any number of styles
- ✅ Type-safe at compile time
- ✅ Zero runtime overhead (bit operations are fast)
- ✅ Memory efficient (64 styles in one uint64_t)
- ✅ Easy to check/add/remove styles
- ✅ Category masks for filtering

---

*Source: Extracted from discussion_guikit.txt, lines 1720-3270*
*Documentation organized by Mistral Vibe*
