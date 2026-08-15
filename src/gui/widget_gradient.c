/**
 * @file widget_gradient.c
 * @brief Gradient color support implementation with union-based memory optimization
 * @author GUIKit for ESP8266
 * @date 2026
 */

#include "widget_gradient.h"
#include <string.h>

// =============================================================================
// INLINE FUNCTION IMPLEMENTATIONS
// =============================================================================

// Get primary color (works for all gradient types)
static inline Color gradient_getColor(const Gradient* g) {
    return g->color;
}

// Set primary color (works for all gradient types)
static inline void gradient_setColor(Gradient* g, Color color) {
    g->color = color;
}

// Get end color (for gradient types)
static inline Color gradient_getEndColor(const Gradient* g) {
    if (!IS_GRADIENT(g)) {
        return g->color;  // Return primary color if not a gradient
    }
    
    switch (g->type) {
        case GRADIENT_LINEAR:
            return g->data.linear.end_color;
        case GRADIENT_HORIZONTAL:
            return g->data.horizontal.end_color;
        case GRADIENT_VERTICAL:
            return g->data.vertical.end_color;
        case GRADIENT_DIAGONAL:
            return g->data.diagonal.end_color;
        case GRADIENT_RADIAL:
            return g->data.radial.end_color;
        case GRADIENT_CONIC:
            return g->data.conic.end_color;
        default:
            return g->color;
    }
}

// Set end color (for gradient types)
static inline void gradient_setEndColor(Gradient* g, Color color) {
    if (!IS_GRADIENT(g)) {
        return;  // Can't set end color for non-gradients
    }
    
    switch (g->type) {
        case GRADIENT_LINEAR:
            g->data.linear.end_color = color;
            break;
        case GRADIENT_HORIZONTAL:
            g->data.horizontal.end_color = color;
            break;
        case GRADIENT_VERTICAL:
            g->data.vertical.end_color = color;
            break;
        case GRADIENT_DIAGONAL:
            g->data.diagonal.end_color = color;
            break;
        case GRADIENT_RADIAL:
            g->data.radial.end_color = color;
            break;
        case GRADIENT_CONIC:
            g->data.conic.end_color = color;
            break;
        default:
            break;
    }
}

// Get gradient angle (for linear/conic gradients)
static inline int16_t gradient_getAngle(const Gradient* g) {
    switch (g->type) {
        case GRADIENT_LINEAR:
            return g->data.linear.angle;
        case GRADIENT_HORIZONTAL:
            return 0;  // Horizontal = 0 degrees
        case GRADIENT_VERTICAL:
            return 90;  // Vertical = 90 degrees
        case GRADIENT_DIAGONAL:
            return 45;  // Diagonal = 45 degrees
        case GRADIENT_CONIC:
            return g->data.conic.start_angle;
        default:
            return 0;
    }
}

// Set gradient angle (for linear/conic gradients)
static inline void gradient_setAngle(Gradient* g, int16_t angle) {
    switch (g->type) {
        case GRADIENT_LINEAR:
            g->data.linear.angle = angle;
            break;
        case GRADIENT_CONIC:
            // For conic, angle typically means start angle
            g->data.conic.start_angle = angle;
            break;
        default:
            // For simplified gradients, angle is fixed
            break;
    }
}

// Helper to get center X
static inline uint16_t gradient_getCenterX(const Gradient* g) {
    switch (g->type) {
        case GRADIENT_RADIAL:
            return g->data.radial.center.x;
        case GRADIENT_CONIC:
            return g->data.conic.center.x;
        default:
            return 0;
    }
}

// Helper to get center Y
static inline uint16_t gradient_getCenterY(const Gradient* g) {
    switch (g->type) {
        case GRADIENT_RADIAL:
            return g->data.radial.center.y;
        case GRADIENT_CONIC:
            return g->data.conic.center.y;
        default:
            return 0;
    }
}

// Get center point (for radial/conic gradients)
static inline void gradient_getCenter(const Gradient* g, uint16_t* x, uint16_t* y) {
    if (x) *x = gradient_getCenterX(g);
    if (y) *y = gradient_getCenterY(g);
}

// Set center point (for radial/conic gradients)
static inline void gradient_setCenter(Gradient* g, uint16_t x, uint16_t y) {
    switch (g->type) {
        case GRADIENT_RADIAL:
            g->data.radial.center.x = x;
            g->data.radial.center.y = y;
            break;
        case GRADIENT_CONIC:
            g->data.conic.center.x = x;
            g->data.conic.center.y = y;
            break;
        default:
            break;
    }
}

// Get radius (for radial gradients)
static inline uint16_t gradient_getRadius(const Gradient* g) {
    if (g->type == GRADIENT_RADIAL) {
        return g->data.radial.radius;
    }
    return 0;
}

// Set radius (for radial gradients)
static inline void gradient_setRadius(Gradient* g, uint16_t radius) {
    if (g->type == GRADIENT_RADIAL) {
        g->data.radial.radius = radius;
    }
}

// Get start angle (for conic gradients)
static inline int16_t gradient_getStartAngle(const Gradient* g) {
    if (g->type == GRADIENT_CONIC) {
        return g->data.conic.start_angle;
    }
    return 0;
}

// Set start angle (for conic gradients)
static inline void gradient_setStartAngle(Gradient* g, int16_t angle) {
    if (g->type == GRADIENT_CONIC) {
        g->data.conic.start_angle = angle;
    }
}

// Get end angle (for conic gradients)
static inline int16_t gradient_getEndAngle(const Gradient* g) {
    if (g->type == GRADIENT_CONIC) {
        return g->data.conic.end_angle;
    }
    return 0;
}

// Set end angle (for conic gradients)
static inline void gradient_setEndAngle(Gradient* g, int16_t angle) {
    if (g->type == GRADIENT_CONIC) {
        g->data.conic.end_angle = angle;
    }
}

// =============================================================================
// FUNCTION IMPLEMENTATIONS
// =============================================================================

// Set gradient type and initialize data
void gradient_setType(Gradient* g, GRADIENT_TYPE type) {
    if (g->type == type) {
        return;  // No change needed
    }
    
    g->type = type;
    
    // Clear existing data
    memset(&g->data, 0, sizeof(g->data));
    
    // Initialize based on type
    switch (type) {
        case GRADIENT_NONE:
        case GRADIENT_SOLID:
            // No additional data needed
            g->data.none._padding[0] = 0;
            g->data.none._padding[1] = 0;
            break;
            
        case GRADIENT_HORIZONTAL:
            g->data.horizontal.end_color = g->color;  // Default end color = primary
            g->data.horizontal._padding[0] = 0;
            g->data.horizontal._padding[1] = 0;
            break;
            
        case GRADIENT_VERTICAL:
            g->data.vertical.end_color = g->color;
            g->data.vertical._padding[0] = 0;
            g->data.vertical._padding[1] = 0;
            break;
            
        case GRADIENT_DIAGONAL:
            g->data.diagonal.end_color = g->color;
            g->data.diagonal._padding[0] = 0;
            g->data.diagonal._padding[1] = 0;
            break;
            
        case GRADIENT_LINEAR:
            g->data.linear.end_color = g->color;
            g->data.linear.angle = 0;  // Default: left to right
            g->data.linear._padding[0] = 0;
            g->data.linear._padding[1] = 0;
            break;
            
        case GRADIENT_RADIAL:
            g->data.radial.end_color = g->color;
            g->data.radial.center.x = 0;
            g->data.radial.center.y = 0;
            g->data.radial.radius = 0;
            break;
            
        case GRADIENT_CONIC:
            g->data.conic.end_color = g->color;
            g->data.conic.center.x = 0;
            g->data.conic.center.y = 0;
            g->data.conic.start_angle = 0;
            g->data.conic.end_angle = 360;
            break;
    }
}

// Set gradient to solid color (no gradient)
void gradient_setSolid(Gradient* g, Color color) {
    g->color = color;
    gradient_setType(g, GRADIENT_SOLID);
}

// Set gradient to horizontal linear gradient
void gradient_setHorizontal(Gradient* g, Color start_color, Color end_color) {
    g->color = start_color;
    gradient_setType(g, GRADIENT_HORIZONTAL);
    g->data.horizontal.end_color = end_color;
}

// Set gradient to vertical linear gradient
void gradient_setVertical(Gradient* g, Color start_color, Color end_color) {
    g->color = start_color;
    gradient_setType(g, GRADIENT_VERTICAL);
    g->data.vertical.end_color = end_color;
}

// Set gradient to diagonal linear gradient (45 degrees)
void gradient_setDiagonal(Gradient* g, Color start_color, Color end_color) {
    g->color = start_color;
    gradient_setType(g, GRADIENT_DIAGONAL);
    g->data.diagonal.end_color = end_color;
}

// Set gradient to linear with custom angle
void gradient_setLinear(Gradient* g, Color start_color, Color end_color, int16_t angle) {
    g->color = start_color;
    gradient_setType(g, GRADIENT_LINEAR);
    g->data.linear.end_color = end_color;
    g->data.linear.angle = angle;
}

// Set gradient to radial
void gradient_setRadial(Gradient* g, Color start_color, Color end_color,
                       uint16_t center_x, uint16_t center_y, uint16_t radius) {
    g->color = start_color;
    gradient_setType(g, GRADIENT_RADIAL);
    g->data.radial.end_color = end_color;
    g->data.radial.center.x = center_x;
    g->data.radial.center.y = center_y;
    g->data.radial.radius = radius;
}

// Set gradient to conic
void gradient_setConic(Gradient* g, Color start_color, Color end_color,
                      uint16_t center_x, uint16_t center_y,
                      int16_t start_angle, int16_t end_angle) {
    g->color = start_color;
    gradient_setType(g, GRADIENT_CONIC);
    g->data.conic.end_color = end_color;
    g->data.conic.center.x = center_x;
    g->data.conic.center.y = center_y;
    g->data.conic.start_angle = start_angle;
    g->data.conic.end_angle = end_angle;
}

// Copy gradient from one to another
void gradient_copy(Gradient* dest, const Gradient* src) {
    if (dest == src) return;
    
    // Copy the entire struct (including union)
    memcpy(dest, src, sizeof(Gradient));
}

// Reset gradient to solid color (no gradient)
void gradient_reset(Gradient* g, Color color) {
    g->color = color;
    gradient_setType(g, GRADIENT_SOLID);
}

// =============================================================================
// RENDERING HELPER FUNCTIONS
// =============================================================================

/**
 * @brief Interpolate between two colors based on ratio
 * @param color1 First color (RGB565)
 * @param color2 Second color (RGB565)
 * @param ratio Interpolation ratio (0.0 to 1.0)
 * @return Interpolated color (RGB565)
 * @note Uses fixed-point arithmetic for ESP8266 (no float)
 */
Color gradient_interpolate(Color color1, Color color2, uint16_t ratio) {
    // ratio is 0-256 (fixed point 8.8)
    // Extract RGB components from color1
    uint8_t r1 = (color1 >> 11) & 0x1F;
    uint8_t g1 = (color1 >> 5) & 0x3F;
    uint8_t b1 = color1 & 0x1F;
    
    // Extract RGB components from color2
    uint8_t r2 = (color2 >> 11) & 0x1F;
    uint8_t g2 = (color2 >> 5) & 0x3F;
    uint8_t b2 = color2 & 0x1F;
    
    // Interpolate each component
    uint8_t r = r1 + (((int16_t)(r2 - r1) * ratio) >> 8);
    uint8_t g = g1 + (((int16_t)(g2 - g1) * ratio) >> 8);
    uint8_t b = b1 + (((int16_t)(b2 - b1) * ratio) >> 8);
    
    // Clamp and return as RGB565
    r = (r > 31) ? 31 : r;
    g = (g > 63) ? 63 : g;
    b = (b > 31) ? 31 : b;
    
    return (r << 11) | (g << 5) | b;
}

/**
 * @brief Get color at a specific position for a gradient
 * @param g Gradient pointer
 * @param x X position (0-255 for normalized, or absolute for some types)
 * @param y Y position (0-255 for normalized, or absolute for some types)
 * @param width Total width of area
 * @param height Total height of area
 * @return Color at position (RGB565)
 */
Color gradient_getColorAt(const Gradient* g, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height) {
    if (!IS_GRADIENT(g)) {
        return g->color;  // Solid color
    }
    
    uint16_t ratio = 0;
    
    switch (g->type) {
        case GRADIENT_HORIZONTAL: {
            // Ratio based on x position (0-255)
            ratio = (x * 256) / width;
            break;
        }
        
        case GRADIENT_VERTICAL: {
            // Ratio based on y position (0-255)
            ratio = (y * 256) / height;
            break;
        }
        
        case GRADIENT_DIAGONAL: {
            // Ratio based on distance along diagonal
            // Simple approximation: average of x and y
            uint32_t dist = (uint32_t)x * height + (uint32_t)y * width;
            uint32_t max_dist = (uint32_t)width * height + (uint32_t)height * width;
            ratio = (uint16_t)((dist * 256) / max_dist);
            break;
        }
        
        case GRADIENT_LINEAR: {
            // Ratio based on angle
            int16_t angle = g->data.linear.angle;
            // Convert angle to radians approximation (fixed point)
            // Simple: use cos/sin approximation
            int16_t cos_a = 0, sin_a = 0;
            
            // Approximate cos and sin for common angles
            switch (angle % 360) {
                case 0:   cos_a = 256; sin_a = 0; break;    // 0 degrees: right
                case 90:  cos_a = 0; sin_a = 256; break;     // 90 degrees: down
                case 180: cos_a = -256; sin_a = 0; break;   // 180 degrees: left
                case 270: cos_a = 0; sin_a = -256; break;    // 270 degrees: up
                case 45:  cos_a = 181; sin_a = 181; break;   // 45 degrees
                case 135: cos_a = -181; sin_a = 181; break;  // 135 degrees
                case 225: cos_a = -181; sin_a = -181; break; // 225 degrees
                case 315: cos_a = 181; sin_a = -181; break;  // 315 degrees
                default: 
                    // Use angle directly (simplified)
                    cos_a = 256; sin_a = 0;
                    break;
            }
            
            // Project position onto gradient direction
            int32_t dot = (int32_t)x * cos_a + (int32_t)y * sin_a;
            int32_t max_dot = (int32_t)width * cos_a + (int32_t)height * sin_a;
            
            if (max_dot <= 0) max_dot = 1;
            
            int32_t dot_normalized = dot * 256 / max_dot;
            if (dot_normalized < 0) dot_normalized = 0;
            if (dot_normalized > 256) dot_normalized = 256;
            
            ratio = (uint16_t)dot_normalized;
            break;
        }
        
        case GRADIENT_RADIAL: {
            // Ratio based on distance from center
            Point center = g->data.radial.center;
            uint16_t radius = g->data.radial.radius;
            
            if (radius == 0) {
                return g->color;  // Avoid division by zero
            }
            
            // Calculate distance from center (squared to avoid sqrt)
            int32_t dx = (int32_t)x - center.x;
            int32_t dy = (int32_t)y - center.y;
            uint32_t dist_sq = (uint32_t)(dx * dx) + (uint32_t)(dy * dy);
            uint32_t radius_sq = (uint32_t)radius * radius;
            
            if (dist_sq >= radius_sq) {
                return g->data.radial.end_color;  // Outside radius = end color
            }
            
            // ratio = distance / radius (0-255)
            ratio = (uint16_t)((sqrt_approx(dist_sq) * 256) / radius);
            break;
        }
        
        case GRADIENT_CONIC: {
            // Ratio based on angle from center
            Point center = g->data.conic.center;
            int16_t start_angle = g->data.conic.start_angle;
            int16_t end_angle = g->data.conic.end_angle;
            int16_t angle_range = end_angle - start_angle;
            
            if (angle_range <= 0) angle_range = 360;
            
            // Calculate angle of point from center
            int32_t dx = (int32_t)x - center.x;
            int32_t dy = (int32_t)y - center.y;
            int16_t point_angle = atan2_approx(dy, dx);
            
            // Normalize angles
            point_angle = (point_angle + 360) % 360;
            start_angle = (start_angle + 360) % 360;
            
            // Calculate position in range
            int16_t pos = point_angle - start_angle;
            if (pos < 0) pos += 360;
            
            // ratio = position / range (0-255)
            ratio = (uint16_t)((pos * 256) / angle_range);
            break;
        }
        
        default:
            return g->color;
    }
    
    // Clamp ratio and interpolate
    if (ratio > 256) ratio = 256;
    return gradient_interpolate(g->color, gradient_getEndColor(g), ratio);
}

/**
 * @brief Approximate square root (for radial gradient)
 * @param x Value to take square root of
 * @return Approximate square root
 */
static inline uint16_t sqrt_approx(uint32_t x) {
    if (x == 0) return 0;
    
    uint16_t res = 0;
    uint16_t bit = 1 << 14; // Start with highest possible bit
    
    while (bit > x) bit >>= 2;
    
    while (bit != 0) {
        if (x >= res + bit) {
            x -= res + bit;
            res = (res >> 1) + bit;
        } else {
            res >>= 1;
        }
        bit >>= 2;
    }
    return res;
}

/**
 * @brief Approximate atan2 (for conic gradient)
 * @param y Y component
 * @param x X component
 * @return Angle in degrees (0-360)
 */
static inline int16_t atan2_approx(int32_t y, int32_t x) {
    // Simple approximation of atan2
    if (x == 0 && y == 0) return 0;
    
    if (x >= 0) {
        if (y >= 0) {
            // First quadrant
            if (x >= y) return (int16_t)((atan_approx((uint32_t)y * 256 / x) * 180) / 256);
            else return (int16_t)(90 - (atan_approx((uint32_t)x * 256 / y) * 180) / 256);
        } else {
            // Fourth quadrant
            if (x >= -y) return (int16_t)(360 - (atan_approx((uint32_t)(-y) * 256 / x) * 180) / 256);
            else return (int16_t)(270 + (atan_approx((uint32_t)x * 256 / (-y)) * 180) / 256);
        }
    } else {
        if (y >= 0) {
            // Second quadrant
            if (-x >= y) return (int16_t)(180 - (atan_approx((uint32_t)y * 256 / (-x)) * 180) / 256);
            else return (int16_t)(90 + (atan_approx((uint32_t)(-x) * 256 / y) * 180) / 256);
        } else {
            // Third quadrant
            if (-x >= -y) return (int16_t)(180 + (atan_approx((uint32_t)(-y) * 256 / (-x)) * 180) / 256);
            else return (int16_t)(270 - (atan_approx((uint32_t)(-x) * 256 / (-y)) * 180) / 256);
        }
    }
}

/**
 * @brief Approximate atan (for angle calculation)
 * @param x Ratio (y/x * 256)
 * @return Angle in radians * 256 (fixed point)
 */
static inline uint16_t atan_approx(uint32_t x) {
    // Polynomial approximation of atan
    // atan(z) ≈ z - z^3/3 + z^5/5 - z^7/7 for |z| < 1
    // We use a simpler lookup-based approach
    
    if (x > 256) x = 256;
    
    // Simple linear approximation (not very accurate but fast)
    // Better: use a small lookup table
    static const uint8_t atan_table[32] = {
        0, 19, 38, 56, 73, 89, 104, 117, 129, 140, 150, 159, 167, 174, 181, 187,
        192, 197, 201, 205, 208, 211, 214, 217, 219, 221, 223, 225, 227, 228, 229, 230
    };
    
    uint8_t idx = (x + 16) >> 5; // x / 32
    if (idx >= 32) idx = 31;
    return atan_table[idx] * 256 / 230; // Scale to 0-256
}
