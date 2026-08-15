/**
 * @file widget_gradient_group.c
 * @brief Gradient group implementation for multiple independent gradients per widget
 * @author GUIKit for ESP8266
 * @date 2026
 */

#include "widget_gradient_group.h"
#include <string.h>

// =============================================================================
// INITIALIZATION
// =============================================================================

void gradientGroup_init(GradientGroup* group) {
    if (!group) return;
    memset(group, 0, sizeof(GradientGroup));
    group->count = 0;
}

void gradientGroup_reset(GradientGroup* group) {
    gradientGroup_init(group);
}

// =============================================================================
// GRADIENT MANAGEMENT
// =============================================================================

GradientEntry* gradientGroup_addGradient(GradientGroup* group) {
    if (!group || group->count >= MAX_GRADIENTS_PER_GROUP) {
        return NULL;
    }
    
    GradientEntry* entry = &group->gradients[group->count];
    memset(entry, 0, sizeof(GradientEntry));
    
    // Initialize gradient to solid black (will be configured later)
    gradient_setSolid(&entry->gradient, COLOR_BLACK);
    
    // Default values
    entry->bounds.x = 0;
    entry->bounds.y = 0;
    entry->bounds.width = 0;  // 0 means full widget width
    entry->bounds.height = 0; // 0 means full widget height
    entry->blend_mode = BLEND_NORMAL;
    entry->opacity = 255;  // Fully opaque
    entry->enabled = true;
    
    group->count++;
    return entry;
}

GradientEntry* gradientGroup_addGradientAt(GradientGroup* group, uint8_t index) {
    if (!group || index >= MAX_GRADIENTS_PER_GROUP) {
        return NULL;
    }
    
    // If index is beyond current count, fill in between
    if (index >= group->count) {
        return gradientGroup_addGradient(group);
    }
    
    // If index already has a gradient, just return it
    // (caller can modify it directly)
    return &group->gradients[index];
}

void gradientGroup_removeGradient(GradientGroup* group, uint8_t index) {
    if (!group || index >= group->count) {
        return;
    }
    
    // Shift all gradients after index down by one
    if (index < group->count - 1) {
        memmove(&group->gradients[index], 
                &group->gradients[index + 1],
                (group->count - index - 1) * sizeof(GradientEntry));
    }
    
    group->count--;
    
    // Clear the last entry
    if (group->count <= MAX_GRADIENTS_PER_GROUP) {
        memset(&group->gradients[group->count], 0, sizeof(GradientEntry));
    }
}

void gradientGroup_removeLast(GradientGroup* group) {
    if (!group || group->count == 0) {
        return;
    }
    
    group->count--;
    memset(&group->gradients[group->count], 0, sizeof(GradientEntry));
}

void gradientGroup_clear(GradientGroup* group) {
    if (!group) return;
    group->count = 0;
    memset(group->gradients, 0, sizeof(group->gradients));
}

// =============================================================================
// GRADIENT CONFIGURATION HELPERS
// =============================================================================

GradientEntry* gradientGroup_addHorizontal(GradientGroup* group,
                                          Color start_color, Color end_color,
                                          uint16_t x, uint16_t y,
                                          uint16_t width, uint16_t height,
                                          uint8_t opacity, BLEND_MODE blend_mode) {
    GradientEntry* entry = gradientGroup_addGradient(group);
    if (!entry) return NULL;
    
    gradient_setHorizontal(&entry->gradient, start_color, end_color);
    entry->bounds.x = x;
    entry->bounds.y = y;
    entry->bounds.width = width;
    entry->bounds.height = height;
    entry->opacity = opacity;
    entry->blend_mode = blend_mode;
    entry->enabled = true;
    
    return entry;
}

GradientEntry* gradientGroup_addVertical(GradientGroup* group,
                                        Color start_color, Color end_color,
                                        uint16_t x, uint16_t y,
                                        uint16_t width, uint16_t height,
                                        uint8_t opacity, BLEND_MODE blend_mode) {
    GradientEntry* entry = gradientGroup_addGradient(group);
    if (!entry) return NULL;
    
    gradient_setVertical(&entry->gradient, start_color, end_color);
    entry->bounds.x = x;
    entry->bounds.y = y;
    entry->bounds.width = width;
    entry->bounds.height = height;
    entry->opacity = opacity;
    entry->blend_mode = blend_mode;
    entry->enabled = true;
    
    return entry;
}

GradientEntry* gradientGroup_addDiagonal(GradientGroup* group,
                                        Color start_color, Color end_color,
                                        uint16_t x, uint16_t y,
                                        uint16_t width, uint16_t height,
                                        uint8_t opacity, BLEND_MODE blend_mode) {
    GradientEntry* entry = gradientGroup_addGradient(group);
    if (!entry) return NULL;
    
    gradient_setDiagonal(&entry->gradient, start_color, end_color);
    entry->bounds.x = x;
    entry->bounds.y = y;
    entry->bounds.width = width;
    entry->bounds.height = height;
    entry->opacity = opacity;
    entry->blend_mode = blend_mode;
    entry->enabled = true;
    
    return entry;
}

GradientEntry* gradientGroup_addLinear(GradientGroup* group,
                                       Color start_color, Color end_color,
                                       int16_t angle,
                                       uint16_t x, uint16_t y,
                                       uint16_t width, uint16_t height,
                                       uint8_t opacity, BLEND_MODE blend_mode) {
    GradientEntry* entry = gradientGroup_addGradient(group);
    if (!entry) return NULL;
    
    gradient_setLinear(&entry->gradient, start_color, end_color, angle);
    entry->bounds.x = x;
    entry->bounds.y = y;
    entry->bounds.width = width;
    entry->bounds.height = height;
    entry->opacity = opacity;
    entry->blend_mode = blend_mode;
    entry->enabled = true;
    
    return entry;
}

GradientEntry* gradientGroup_addRadial(GradientGroup* group,
                                       Color start_color, Color end_color,
                                       uint16_t center_x, uint16_t center_y,
                                       uint16_t radius,
                                       uint16_t x, uint16_t y,
                                       uint16_t width, uint16_t height,
                                       uint8_t opacity, BLEND_MODE blend_mode) {
    GradientEntry* entry = gradientGroup_addGradient(group);
    if (!entry) return NULL;
    
    gradient_setRadial(&entry->gradient, start_color, end_color,
                      center_x, center_y, radius);
    entry->bounds.x = x;
    entry->bounds.y = y;
    entry->bounds.width = width;
    entry->bounds.height = height;
    entry->opacity = opacity;
    entry->blend_mode = blend_mode;
    entry->enabled = true;
    
    return entry;
}

// =============================================================================
// COPY AND COMPARE
// =============================================================================

void gradientGroup_copy(GradientGroup* dest, const GradientGroup* src) {
    if (!dest || !src) return;
    if (dest == src) return;
    
    memcpy(dest, src, sizeof(GradientGroup));
}

bool gradientGroup_equal(const GradientGroup* g1, const GradientGroup* g2) {
    if (!g1 || !g2) return false;
    if (g1 == g2) return true;
    if (g1->count != g2->count) return false;
    
    for (uint8_t i = 0; i < g1->count; i++) {
        const GradientEntry* e1 = &g1->gradients[i];
        const GradientEntry* e2 = &g2->gradients[i];
        
        // Compare gradient
        if (memcmp(&e1->gradient, &e2->gradient, sizeof(Gradient)) != 0) {
            return false;
        }
        
        // Compare bounds
        if (e1->bounds.x != e2->bounds.x ||
            e1->bounds.y != e2->bounds.y ||
            e1->bounds.width != e2->bounds.width ||
            e1->bounds.height != e2->bounds.height) {
            return false;
        }
        
        // Compare other properties
        if (e1->blend_mode != e2->blend_mode ||
            e1->opacity != e2->opacity ||
            e1->enabled != e2->enabled) {
            return false;
        }
    }
    
    return true;
}

// =============================================================================
// BLEND FUNCTIONS
// =============================================================================

// Extract RGB components from RGB565
static inline void rgb565_to_rgb(Color c, uint8_t* r, uint8_t* g, uint8_t* b) {
    *r = (c >> 11) & 0x1F;  // 0-31
    *g = (c >> 5) & 0x3F;   // 0-63
    *b = c & 0x1F;          // 0-31
}

// Create RGB565 from RGB components (0-31, 0-63, 0-31)
static inline Color rgb_to_rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 11) | (g << 5) | b;
}

// Clamp value to range
static inline uint8_t clamp8(int16_t v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (uint8_t)v;
}

// Blend two values with opacity (fixed-point)
static inline uint8_t blend_value(uint8_t base, uint8_t top, uint8_t opacity) {
    // base + (top - base) * opacity / 255
    int16_t result = base + (((int16_t)(top - base) * opacity) >> 8);
    return (uint8_t)clamp8(result);
}

Color blend_colors(Color base, Color top, uint8_t opacity) {
    if (opacity == 0) return base;
    if (opacity == 255) return top;
    
    uint8_t r1, g1, b1;
    uint8_t r2, g2, b2;
    
    rgb565_to_rgb(base, &r1, &g1, &b1);
    rgb565_to_rgb(top, &r2, &g2, &b2);
    
    // Scale RGB565 values to 0-255 for blending
    uint8_t r1_8 = (r1 * 255) / 31;
    uint8_t g1_8 = (g1 * 255) / 63;
    uint8_t b1_8 = (b1 * 255) / 31;
    
    uint8_t r2_8 = (r2 * 255) / 31;
    uint8_t g2_8 = (g2 * 255) / 63;
    uint8_t b2_8 = (b2 * 255) / 31;
    
    // Blend
    uint8_t r = blend_value(r1_8, r2_8, opacity);
    uint8_t g = blend_value(g1_8, g2_8, opacity);
    uint8_t b = blend_value(b1_8, b2_8, opacity);
    
    // Scale back to RGB565 ranges and create color
    r = (r * 31) / 255;
    g = (g * 63) / 255;
    b = (b * 31) / 255;
    
    return rgb_to_rgb565(r, g, b);
}

Color blend_colors_with_mode(Color base, Color top, BLEND_MODE mode) {
    uint8_t r1, g1, b1;
    uint8_t r2, g2, b2;
    
    rgb565_to_rgb(base, &r1, &g1, &b1);
    rgb565_to_rgb(top, &r2, &g2, &b2);
    
    // Scale to 0-255
    uint8_t r1_8 = (r1 * 255) / 31;
    uint8_t g1_8 = (g1 * 255) / 63;
    uint8_t b1_8 = (b1 * 255) / 31;
    
    uint8_t r2_8 = (r2 * 255) / 31;
    uint8_t g2_8 = (g2 * 255) / 63;
    uint8_t b2_8 = (b2 * 255) / 31;
    
    uint8_t r, g, b;
    
    switch (mode) {
        case BLEND_NORMAL:
            r = r2_8;
            g = g2_8;
            b = b2_8;
            break;
            
        case BLEND_OVERLAY:
            r = (r1_8 < 128) ? (r1_8 * r2_8 / 128) : (255 - ((255 - r1_8) * (255 - r2_8) / 128));
            g = (g1_8 < 128) ? (g1_8 * g2_8 / 128) : (255 - ((255 - g1_8) * (255 - g2_8) / 128));
            b = (b1_8 < 128) ? (b1_8 * b2_8 / 128) : (255 - ((255 - b1_8) * (255 - b2_8) / 128));
            break;
            
        case BLEND_MULTIPLY:
            r = (r1_8 * r2_8) / 255;
            g = (g1_8 * g2_8) / 255;
            b = (b1_8 * b2_8) / 255;
            break;
            
        case BLEND_SCREEN:
            r = 255 - ((255 - r1_8) * (255 - r2_8)) / 255;
            g = 255 - ((255 - g1_8) * (255 - g2_8)) / 255;
            b = 255 - ((255 - b1_8) * (255 - b2_8)) / 255;
            break;
            
        case BLEND_ADD:
            r = clamp8(r1_8 + r2_8);
            g = clamp8(g1_8 + g2_8);
            b = clamp8(b1_8 + b2_8);
            break;
            
        case BLEND_DARKEN:
            r = (r1_8 < r2_8) ? r1_8 : r2_8;
            g = (g1_8 < g2_8) ? g1_8 : g2_8;
            b = (b1_8 < b2_8) ? b1_8 : b2_8;
            break;
            
        case BLEND_LIGHTEN:
            r = (r1_8 > r2_8) ? r1_8 : r2_8;
            g = (g1_8 > g2_8) ? g1_8 : g2_8;
            b = (b1_8 > b2_8) ? b1_8 : b2_8;
            break;
            
        case BLEND_LINEAR_DODGE:
            r = clamp8(r1_8 + r2_8);
            g = clamp8(g1_8 + g2_8);
            b = clamp8(b1_8 + b2_8);
            break;
            
        case BLEND_LINEAR_BURN:
            r = clamp8(r1_8 + r2_8 - 255);
            g = clamp8(g1_8 + g2_8 - 255);
            b = clamp8(b1_8 + b2_8 - 255);
            break;
            
        default:
            r = r2_8;
            g = g2_8;
            b = b2_8;
            break;
    }
    
    // Scale back to RGB565 ranges
    r = (r * 31) / 255;
    g = (g * 63) / 255;
    b = (b * 31) / 255;
    
    return rgb_to_rgb565(r, g, b);
}

Color blend_colors_ex(Color base, Color top, uint8_t opacity, BLEND_MODE mode) {
    if (opacity == 255) {
        return blend_colors_with_mode(base, top, mode);
    }
    if (opacity == 0) {
        return base;
    }
    
    // First apply blend mode, then apply opacity
    Color blended = blend_colors_with_mode(base, top, mode);
    return blend_colors(base, blended, opacity);
}

// =============================================================================
// RENDERING HELPERS
// =============================================================================

Color gradientEntry_getColorAt(const GradientEntry* entry,
                                uint16_t x, uint16_t y,
                                uint16_t widget_width, uint16_t widget_height) {
    if (!entry || !entry->enabled) {
        return 0;  // Transparent/black
    }
    
    // Calculate the position within this gradient's bounds
    uint16_t grad_x = x - entry->bounds.x;
    uint16_t grad_y = y - entry->bounds.y;
    
    // Get the width and height of the gradient area
    uint16_t grad_width = entry->bounds.width;
    uint16_t grad_height = entry->bounds.height;
    
    // If width or height is 0, use widget dimensions
    if (grad_width == 0) grad_width = widget_width - entry->bounds.x;
    if (grad_height == 0) grad_height = widget_height - entry->bounds.y;
    
    // Check if position is within this gradient's bounds
    if (x < entry->bounds.x || y < entry->bounds.y ||
        grad_x >= grad_width || grad_y >= grad_height) {
        // Position is outside this gradient's area
        // Return transparent (which won't affect blending)
        return 0;
    }
    
    // Get the color from the gradient at this position
    Color grad_color = gradient_getColorAt(&entry->gradient,
                                           grad_x, grad_y,
                                           grad_width, grad_height);
    
    return grad_color;
}

Color gradientGroup_getColorAt(const GradientGroup* group,
                                uint16_t x, uint16_t y,
                                uint16_t widget_width, uint16_t widget_height) {
    if (!group || group->count == 0) {
        return 0;  // No gradients
    }
    
    Color result = 0;  // Start with transparent/black
    bool has_color = false;
    
    // Process gradients in order (first to last)
    // Later gradients are blended on top of earlier ones
    for (uint8_t i = 0; i < group->count; i++) {
        const GradientEntry* entry = &group->gradients[i];
        
        if (!entry->enabled) continue;
        
        // Get the color from this gradient at (x, y)
        Color grad_color = gradientEntry_getColorAt(entry, x, y, widget_width, widget_height);
        
        if (!has_color) {
            // First gradient with color at this position
            result = grad_color;
            has_color = true;
        } else {
            // Blend this gradient's color with the accumulated result
            result = blend_colors_ex(result, grad_color, entry->opacity, entry->blend_mode);
        }
    }
    
    return result;
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * @brief Get the base color (first gradient's primary color) from a group
 * @param group GradientGroup pointer
 * @return Base color (RGB565), or 0 if no gradients
 */
Color gradientGroup_getBaseColor(const GradientGroup* group) {
    if (!group || group->count == 0) {
        return 0;
    }
    return gradient_getColor(&group->gradients[0].gradient);
}

/**
 * @brief Set the base color for all gradients in the group
 * @param group GradientGroup pointer
 * @param color Base color (RGB565)
 */
void gradientGroup_setBaseColor(GradientGroup* group, Color color) {
    if (!group) return;
    for (uint8_t i = 0; i < group->count; i++) {
        gradient_setColor(&group->gradients[i].gradient, color);
    }
}

/**
 * @brief Enable all gradients in the group
 * @param group GradientGroup pointer
 */
void gradientGroup_enableAll(GradientGroup* group) {
    if (!group) return;
    for (uint8_t i = 0; i < group->count; i++) {
        group->gradients[i].enabled = true;
    }
}

/**
 * @brief Disable all gradients in the group
 * @param group GradientGroup pointer
 */
void gradientGroup_disableAll(GradientGroup* group) {
    if (!group) return;
    for (uint8_t i = 0; i < group->count; i++) {
        group->gradients[i].enabled = false;
    }
}

/**
 * @brief Set opacity for all gradients in the group
 * @param group GradientGroup pointer
 * @param opacity Opacity (0-255)
 */
void gradientGroup_setOpacityAll(GradientGroup* group, uint8_t opacity) {
    if (!group) return;
    for (uint8_t i = 0; i < group->count; i++) {
        group->gradients[i].opacity = opacity;
    }
}

/**
 * @brief Set blend mode for all gradients in the group
 * @param group GradientGroup pointer
 * @param mode Blend mode
 */
void gradientGroup_setBlendModeAll(GradientGroup* group, BLEND_MODE mode) {
    if (!group) return;
    for (uint8_t i = 0; i < group->count; i++) {
        group->gradients[i].blend_mode = mode;
    }
}
