/**
 * @file widget_gradient.h
 * @brief Gradient color support with union-based memory optimization
 * @author GUIKit for ESP8266
 * @date 2026
 * 
 * This module provides a memory-optimized gradient system for widgets using a union-based
 * approach. Only the data needed for the active gradient type is stored, saving memory on
 * ESP8266 (80KB RAM limit). Defaults to RGB565 solid color.
 * 
 * Memory usage:
 * - GRADIENT_NONE: 2 bytes (just color)
 * - GRADIENT_SOLID: 2 bytes (RGB565 color)
 * - GRADIENT_LINEAR: 10 bytes (color + direction)
 * - GRADIENT_RADIAL: 12 bytes (color + center + radius)
 * - GRADIENT_CONIC: 12 bytes (color + center + angle)
 * 
 * Usage:
 * @code
 * Widget* view = new_widget(WIDGET_TYPE_VIEW);
 * gradient_setType(&view->background, GRADIENT_LINEAR);
 * gradient_setStartColor(&view->background, 0xF800);  // Red
 * gradient_setEndColor(&view->background, 0x001F);    // Blue
 * gradient_setAngle(&view->background, 45);             // 45 degrees
 * @endcode
 */

#ifndef WIDGET_GRADIENT_H
#define WIDGET_GRADIENT_H

#include <stdint.h>
#include <stdbool.h>

// Forward declaration
typedef struct Widget Widget;

/**
 * @brief RGB565 color type (16-bit: 5 red, 6 green, 5 blue)
 */
typedef uint16_t Color;

/**
 * @brief Gradient types supported by GUIKit
 */
typedef enum {
    GRADIENT_NONE = 0,      ///< No gradient (solid color only)
    GRADIENT_SOLID,        ///< Solid color (same as NONE, for explicit clarity)
    GRADIENT_LINEAR,       ///< Linear gradient (two colors, direction)
    GRADIENT_RADIAL,       ///< Radial gradient (center, radius, two colors)
    GRADIENT_CONIC,        ///< Conic gradient (center, start/end angles, two colors)
    GRADIENT_HORIZONTAL,   ///< Horizontal linear gradient (simplified)
    GRADIENT_VERTICAL,     ///< Vertical linear gradient (simplified)
    GRADIENT_DIAGONAL,     ///< Diagonal linear gradient (45 degrees)
} GRADIENT_TYPE;

/**
 * @brief Point structure for gradient centers
 */
typedef struct {
    uint16_t x;  ///< X coordinate
    uint16_t y;  ///< Y coordinate
} Point;

/**
 * @brief Size structure for gradient dimensions
 */
typedef struct {
    uint16_t width;   ///< Width
    uint16_t height;  ///< Height
} Size;

/**
 * @brief Union-based gradient data structure
 * @note Memory usage varies by gradient_type:
 *       - GRADIENT_NONE/SOLID: 2 bytes (just color)
 *       - GRADIENT_LINEAR: 10 bytes
 *       - GRADIENT_RADIAL: 12 bytes
 *       - GRADIENT_CONIC: 12 bytes
 *       - GRADIENT_HORIZONTAL/VERTICAL/DIAGONAL: 6 bytes
 */
typedef struct Gradient {
    GRADIENT_TYPE type;  ///< Type of gradient
    Color color;         ///< Primary/fallback color (RGB565)
    
    /**
     * @brief Gradient data union - shares memory based on type
     * @note DO NOT access directly. Use accessor functions/macros instead.
     */
    union {
        // No gradient / solid color - uses primary color only
        struct {
            uint8_t _padding[2];  ///< Padding to match size expectations
        } none;

        // Solid color only (same as none, but explicit)
        struct {
            uint8_t _padding[2];
        } solid;

        // Simplified gradients (horizontal, vertical, diagonal)
        struct {
            Color end_color;       ///< End color (RGB565)
        } simple;

        // Horizontal gradient
        struct {
            Color end_color;       ///< End color (RGB565)
            uint8_t _padding[2];
        } horizontal;

        // Vertical gradient
        struct {
            Color end_color;       ///< End color (RGB565)
            uint8_t _padding[2];
        } vertical;

        // Diagonal gradient
        struct {
            Color end_color;       ///< End color (RGB565)
            uint8_t _padding[2];
        } diagonal;

        // Linear gradient with direction
        struct {
            Color end_color;       ///< End color (RGB565)
            int16_t angle;         ///< Angle in degrees (0-360)
            uint8_t _padding[2];
        } linear;

        // Radial gradient
        struct {
            Color end_color;       ///< End color (RGB565)
            Point center;         ///< Center point of gradient
            uint16_t radius;      ///< Radius of gradient
        } radial;

        // Conic gradient
        struct {
            Color end_color;       ///< End color (RGB565)
            Point center;         ///< Center point of gradient
            int16_t start_angle;   ///< Start angle in degrees
            int16_t end_angle;     ///< End angle in degrees
        } conic;

        // Multi-color gradient (for advanced effects)
        struct {
            Color colors[4];      ///< Up to 4 colors
            uint8_t color_count;  ///< Number of colors (1-4)
            int16_t angle;         ///< Angle for linear, start angle for conic
            uint8_t _padding[1];
        } multi;
    } data;
} Gradient;

// =============================================================================
// ACCESSOR MACROS (Type-safe, check type before accessing union members)
// =============================================================================

/**
 * @brief Get gradient type
 * @param g Gradient pointer
 * @return GRADIENT_TYPE
 */
#define GRADIENT_TYPE(g)          ((g)->type)

/**
 * @brief Check if gradient is enabled (not NONE/SOLID)
 * @param g Gradient pointer
 * @return true if gradient is enabled
 */
#define IS_GRADIENT(g)            (GRADIENT_TYPE(g) != GRADIENT_NONE && GRADIENT_TYPE(g) != GRADIENT_SOLID)

/**
 * @brief Check if gradient is solid color only
 * @param g Gradient pointer
 * @return true if solid color (no gradient)
 */
#define IS_SOLID(g)               (GRADIENT_TYPE(g) == GRADIENT_NONE || GRADIENT_TYPE(g) == GRADIENT_SOLID)

/**
 * @brief Check if gradient is linear
 * @param g Gradient pointer
 * @return true if linear gradient
 */
#define IS_LINEAR(g)              (GRADIENT_TYPE(g) == GRADIENT_LINEAR || \
                                   GRADIENT_TYPE(g) == GRADIENT_HORIZONTAL || \
                                   GRADIENT_TYPE(g) == GRADIENT_VERTICAL || \
                                   GRADIENT_TYPE(g) == GRADIENT_DIAGONAL)

/**
 * @brief Check if gradient is radial
 * @param g Gradient pointer
 * @return true if radial gradient
 */
#define IS_RADIAL(g)              (GRADIENT_TYPE(g) == GRADIENT_RADIAL)

/**
 * @brief Check if gradient is conic
 * @param g Gradient pointer
 * @return true if conic gradient
 */
#define IS_CONIC(g)               (GRADIENT_TYPE(g) == GRADIENT_CONIC)

// =============================================================================
// INLINE ACCESSOR FUNCTIONS (Optimized for performance)
// =============================================================================

/**
 * @brief Get primary color (works for all gradient types)
 * @param g Gradient pointer
 * @return Primary color (RGB565)
 */
static inline Color gradient_getColor(const Gradient* g);

/**
 * @brief Set primary color (works for all gradient types)
 * @param g Gradient pointer
 * @param color RGB565 color
 */
static inline void gradient_setColor(Gradient* g, Color color);

/**
 * @brief Get end color (for gradient types)
 * @param g Gradient pointer
 * @return End color (RGB565), or primary color if not a gradient
 */
static inline Color gradient_getEndColor(const Gradient* g);

/**
 * @brief Set end color (for gradient types)
 * @param g Gradient pointer
 * @param color RGB565 color
 */
static inline void gradient_setEndColor(Gradient* g, Color color);

/**
 * @brief Get gradient angle (for linear/conic gradients)
 * @param g Gradient pointer
 * @return Angle in degrees, or 0 if not applicable
 */
static inline int16_t gradient_getAngle(const Gradient* g);

/**
 * @brief Set gradient angle (for linear/conic gradients)
 * @param g Gradient pointer
 * @param angle Angle in degrees
 */
static inline void gradient_setAngle(Gradient* g, int16_t angle);

/**
 * @brief Get center point (for radial/conic gradients)
 * @param g Gradient pointer
 * @param x Output X coordinate (can be NULL)
 * @param y Output Y coordinate (can be NULL)
 */
static inline void gradient_getCenter(const Gradient* g, uint16_t* x, uint16_t* y);

/**
 * @brief Set center point (for radial/conic gradients)
 * @param g Gradient pointer
 * @param x X coordinate
 * @param y Y coordinate
 */
static inline void gradient_setCenter(Gradient* g, uint16_t x, uint16_t y);

/**
 * @brief Get radius (for radial gradients)
 * @param g Gradient pointer
 * @return Radius, or 0 if not a radial gradient
 */
static inline uint16_t gradient_getRadius(const Gradient* g);

/**
 * @brief Set radius (for radial gradients)
 * @param g Gradient pointer
 * @param radius Radius value
 */
static inline void gradient_setRadius(Gradient* g, uint16_t radius);

/**
 * @brief Get start angle (for conic gradients)
 * @param g Gradient pointer
 * @return Start angle in degrees, or 0 if not a conic gradient
 */
static inline int16_t gradient_getStartAngle(const Gradient* g);

/**
 * @brief Set start angle (for conic gradients)
 * @param g Gradient pointer
 * @param angle Start angle in degrees
 */
static inline void gradient_setStartAngle(Gradient* g, int16_t angle);

/**
 * @brief Get end angle (for conic gradients)
 * @param g Gradient pointer
 * @return End angle in degrees, or 0 if not a conic gradient
 */
static inline int16_t gradient_getEndAngle(const Gradient* g);

/**
 * @brief Set end angle (for conic gradients)
 * @param g Gradient pointer
 * @param angle End angle in degrees
 */
static inline void gradient_setEndAngle(Gradient* g, int16_t angle);

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

/**
 * @brief Set gradient type and initialize data
 * @param g Gradient pointer
 * @param type GRADIENT_TYPE
 */
void gradient_setType(Gradient* g, GRADIENT_TYPE type);

/**
 * @brief Set gradient to solid color (no gradient)
 * @param g Gradient pointer
 * @param color RGB565 color
 */
void gradient_setSolid(Gradient* g, Color color);

/**
 * @brief Set gradient to horizontal linear gradient
 * @param g Gradient pointer
 * @param start_color Start color (RGB565)
 * @param end_color End color (RGB565)
 */
void gradient_setHorizontal(Gradient* g, Color start_color, Color end_color);

/**
 * @brief Set gradient to vertical linear gradient
 * @param g Gradient pointer
 * @param start_color Start color (RGB565)
 * @param end_color End color (RGB565)
 */
void gradient_setVertical(Gradient* g, Color start_color, Color end_color);

/**
 * @brief Set gradient to diagonal linear gradient (45 degrees)
 * @param g Gradient pointer
 * @param start_color Start color (RGB565)
 * @param end_color End color (RGB565)
 */
void gradient_setDiagonal(Gradient* g, Color start_color, Color end_color);

/**
 * @brief Set gradient to linear with custom angle
 * @param g Gradient pointer
 * @param start_color Start color (RGB565)
 * @param end_color End color (RGB565)
 * @param angle Angle in degrees (0-360)
 */
void gradient_setLinear(Gradient* g, Color start_color, Color end_color, int16_t angle);

/**
 * @brief Set gradient to radial
 * @param g Gradient pointer
 * @param start_color Center color (RGB565)
 * @param end_color Outer color (RGB565)
 * @param center_x Center X coordinate
 * @param center_y Center Y coordinate
 * @param radius Radius of gradient
 */
void gradient_setRadial(Gradient* g, Color start_color, Color end_color, 
                       uint16_t center_x, uint16_t center_y, uint16_t radius);

/**
 * @brief Set gradient to conic
 * @param g Gradient pointer
 * @param start_color Start angle color (RGB565)
 * @param end_color End angle color (RGB565)
 * @param center_x Center X coordinate
 * @param center_y Center Y coordinate
 * @param start_angle Start angle in degrees
 * @param end_angle End angle in degrees
 */
void gradient_setConic(Gradient* g, Color start_color, Color end_color,
                      uint16_t center_x, uint16_t center_y,
                      int16_t start_angle, int16_t end_angle);

/**
 * @brief Copy gradient from one to another
 * @param dest Destination gradient
 * @param src Source gradient
 */
void gradient_copy(Gradient* dest, const Gradient* src);

/**
 * @brief Reset gradient to solid color (no gradient)
 * @param g Gradient pointer
 * @param color RGB565 color
 */
void gradient_reset(Gradient* g, Color color);

// =============================================================================
// MACRO-BASED SYNTAX (Objective-C style, for convenience)
// =============================================================================

// Type and check macros
#define SET_GRADIENT_TYPE(g, t)   gradient_setType(g, t)
#define GET_GRADIENT_TYPE(g)      GRADIENT_TYPE(g)
#define IS_GRADIENT_ENABLED(g)    IS_GRADIENT(g)
#define IS_SOLID_COLOR(g)        IS_SOLID(g)

// Color macros
#define SET_COLOR(g, c)          gradient_setColor(g, c)
#define GET_COLOR(g)             gradient_getColor(g)
#define SET_END_COLOR(g, c)      gradient_setEndColor(g, c)
#define GET_END_COLOR(g)         gradient_getEndColor(g)

// Gradient setup macros
#define SET_SOLID(g, c)          gradient_setSolid(g, c)
#define SET_HGRADIENT(g, s, e)   gradient_setHorizontal(g, s, e)
#define SET_VGRADIENT(g, s, e)   gradient_setVertical(g, s, e)
#define SET_DGRADIENT(g, s, e)   gradient_setDiagonal(g, s, e)
#define SET_LGRADIENT(g, s, e, a) gradient_setLinear(g, s, e, a)
#define SET_RGRADIENT(g, s, e, cx, cy, r) gradient_setRadial(g, s, e, cx, cy, r)
#define SET_CGRADIENT(g, s, e, cx, cy, sa, ea) gradient_setConic(g, s, e, cx, cy, sa, ea)

// Angle and position macros
#define SET_ANGLE(g, a)          gradient_setAngle(g, a)
#define GET_ANGLE(g)             gradient_getAngle(g)
#define SET_CENTER(g, x, y)      gradient_setCenter(g, x, y)
#define GET_CENTER_X(g)          (gradient_getCenterX(g))
#define GET_CENTER_Y(g)          (gradient_getCenterY(g))
#define SET_RADIUS(g, r)         gradient_setRadius(g, r)
#define GET_RADIUS(g)            gradient_getRadius(g)

// Reset macros
#define RESET_GRADIENT(g, c)     gradient_reset(g, c)
#define COPY_GRADIENT(d, s)      gradient_copy(d, s)

// =============================================================================
// COLOR UTILITY MACROS (RGB565)
// =============================================================================

// Create RGB565 color from R, G, B components (0-255)
#define RGB565(r, g, b)         ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

// Color constants
#define COLOR_BLACK             0x0000
#define COLOR_WHITE             0xFFFF
#define COLOR_RED               0xF800
#define COLOR_GREEN             0x07E0
#define COLOR_BLUE              0x001F
#define COLOR_YELLOW            0xFFE0
#define COLOR_CYAN              0x07FF
#define COLOR_MAGENTA           0xF81F
#define COLOR_GRAY              0x8410
#define COLOR_SILVER            0xC618
#define COLOR_ORANGE            0xFD20
#define COLOR_PURPLE            0x8010
#define COLOR_TRANSPARENT       0x0000  // Same as black for RGB565

#endif // WIDGET_GRADIENT_H
