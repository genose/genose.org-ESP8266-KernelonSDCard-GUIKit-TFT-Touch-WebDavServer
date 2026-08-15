/**
 * @file widget_gradient_group.h
 * @brief Gradient group support for multiple independent gradients per widget
 * @author GUIKit for ESP8266
 * @date 2026
 * 
 * This module provides gradient GROUP support, allowing widgets to have up to 6
 * independent gradient definitions. Each gradient in the group can have its own
 * colors, coordinates, and direction. This enables complex shading effects with
 * multiple gradient lines/areas on a single widget.
 * 
 * Memory optimization: Uses union-based Gradient as base, grouped in array.
 * Each gradient in group is independent with its own type, colors, coords, direction.
 * 
 * Usage:
 * @code
 * Widget* view = new_widget(WIDGET_TYPE_VIEW);
 * gradientGroup_init(&view->background.gradient_group);
 * 
 * // Add first gradient (horizontal, red to blue)
 * Gradient* g1 = gradientGroup_addGradient(&view->background.gradient_group);
 * gradient_setHorizontal(g1, COLOR_RED, COLOR_BLUE);
 * 
 * // Add second gradient (vertical, white to black at 50% opacity)
 * Gradient* g2 = gradientGroup_addGradient(&view->background.gradient_group);
 * gradient_setVertical(g2, COLOR_WHITE, COLOR_BLACK);
 * gradient_setBlendMode(g2, BLEND_OVERLAY);
 * 
 * // Render with all gradients
 * render_widget_with_gradient_group(view);
 * @endcode
 */

#ifndef WIDGET_GRADIENT_GROUP_H
#define WIDGET_GRADIENT_GROUP_H

#include "widget_gradient.h"
#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// CONSTANTS
// =============================================================================

/**
 * @brief Maximum number of gradients in a group
 * 
 * ESP8266 memory constraint: 6 gradients × 15 bytes = 90 bytes per widget
 * This is acceptable as widgets typically have 200+ bytes each
 */
#define MAX_GRADIENTS_PER_GROUP 6

// =============================================================================
// BLEND MODES
// =============================================================================

/**
 * @brief Blend modes for combining multiple gradients
 */
typedef enum {
    BLEND_NORMAL = 0,      ///< Normal alpha blending (default)
    BLEND_OVERLAY,        ///< Overlay blend mode
    BLEND_MULTIPLY,       ///< Multiply blend mode
    BLEND_SCREEN,         ///< Screen blend mode
    BLEND_ADD,            ///< Additive blending
    BLEND_DARKEN,         ///< Darken (min of each channel)
    BLEND_LIGHTEN,        ///< Lighten (max of each channel)
    BLEND_LINEAR_DODGE,   ///< Linear dodge (add with clamping)
    BLEND_LINEAR_BURN,    ///< Linear burn
} BLEND_MODE;

// =============================================================================
// GRADIENT GROUP STRUCTURE
// =============================================================================

/**
 * @brief Individual gradient entry within a group
 * 
 * Each gradient has its own properties and can be positioned independently.
 */
typedef struct {
    Gradient gradient;           ///< The gradient definition (union-based)
    
    // Position and size for this gradient within the widget
    struct {
        uint16_t x;              ///< X offset within widget (0 = left edge)
        uint16_t y;              ///< Y offset within widget (0 = top edge)
        uint16_t width;          ///< Width of gradient area (0 = full widget width)
        uint16_t height;         ///< Height of gradient area (0 = full widget height)
    } bounds;
    
    BLEND_MODE blend_mode;      ///< How this gradient blends with previous ones
    uint8_t opacity;           ///< Opacity (0-255, 255 = fully opaque)
    bool enabled;              ///< Whether this gradient is active
    uint8_t _padding[1];       ///< Padding for alignment
} GradientEntry;

/**
 * @brief Gradient group - collection of multiple gradients for a widget
 * 
 * Memory: 6 × (sizeof(Gradient) + bounds + blend + opacity + enabled)
 *        = 6 × (15 + 8 + 1 + 1 + 1 + 1) = 6 × 27 = 162 bytes max
 */
typedef struct {
    GradientEntry gradients[MAX_GRADIENTS_PER_GROUP];  ///< Array of gradient entries
    uint8_t count;                                   ///< Number of active gradients (0-6)
    uint8_t _padding[3];                            ///< Padding for alignment
} GradientGroup;

// =============================================================================
// ACCESSOR MACROS (Objective-C style)
// =============================================================================

// Gradient count
#define GRADIENT_GROUP_COUNT(g)          ((g)->count)
#define GRADIENT_GROUP_EMPTY(g)          ((g)->count == 0)
#define GRADIENT_GROUP_FULL(g)           ((g)->count >= MAX_GRADIENTS_PER_GROUP)

// Gradient access
#define GRADIENT_GROUP_GET(g, i)        (&(g)->gradients[i])
#define GRADIENT_GROUP_FIRST(g)         GRADIENT_GROUP_GET(g, 0)
#define GRADIENT_GROUP_LAST(g)          GRADIENT_GROUP_GET(g, (g)->count - 1)

// Enable/disable
#define GRADIENT_ENTRY_ENABLE(e)        ((e)->enabled = true)
#define GRADIENT_ENTRY_DISABLE(e)       ((e)->enabled = false)
#define GRADIENT_ENTRY_IS_ENABLED(e)    ((e)->enabled)

// Opacity
#define GRADIENT_ENTRY_SET_OPACITY(e, o) ((e)->opacity = (o))
#define GRADIENT_ENTRY_GET_OPACITY(e)   ((e)->opacity)

// Blend mode
#define GRADIENT_ENTRY_SET_BLEND(e, m) ((e)->blend_mode = (m))
#define GRADIENT_ENTRY_GET_BLEND(e)    ((e)->blend_mode)

// Bounds
#define GRADIENT_ENTRY_SET_BOUNDS(e, x, y, w, h) \
    do { (e)->bounds.x = (x); (e)->bounds.y = (y); (e)->bounds.width = (w); (e)->bounds.height = (h); } while(0)

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// --- Initialization ---

/**
 * @brief Initialize a gradient group (clear all gradients)
 * @param group GradientGroup pointer
 */
void gradientGroup_init(GradientGroup* group);

/**
 * @brief Reset gradient group to empty state
 * @param group GradientGroup pointer
 */
void gradientGroup_reset(GradientGroup* group);

// --- Gradient Management ---

/**
 * @brief Add a new gradient to the group
 * @param group GradientGroup pointer
 * @return Pointer to the new GradientEntry, or NULL if group is full
 */
GradientEntry* gradientGroup_addGradient(GradientGroup* group);

/**
 * @brief Add a gradient at a specific index
 * @param group GradientGroup pointer
 * @param index Index to insert at (0-5)
 * @return Pointer to the GradientEntry at index, or NULL if invalid
 */
GradientEntry* gradientGroup_addGradientAt(GradientGroup* group, uint8_t index);

/**
 * @brief Remove a gradient from the group
 * @param group GradientGroup pointer
 * @param index Index of gradient to remove (0-5)
 */
void gradientGroup_removeGradient(GradientGroup* group, uint8_t index);

/**
 * @brief Remove the last gradient from the group
 * @param group GradientGroup pointer
 */
void gradientGroup_removeLast(GradientGroup* group);

/**
 * @brief Clear all gradients from the group
 * @param group GradientGroup pointer
 */
void gradientGroup_clear(GradientGroup* group);

// --- Gradient Configuration Helpers ---

/**
 * @brief Add a horizontal gradient to the group
 * @param group GradientGroup pointer
 * @param start_color Start color (RGB565)
 * @param end_color End color (RGB565)
 * @param x X offset (0 = left edge)
 * @param y Y offset (0 = top edge)
 * @param width Width of gradient area (0 = full width)
 * @param height Height of gradient area (0 = full height)
 * @param opacity Opacity (0-255)
 * @param blend_mode Blend mode
 * @return Pointer to the new GradientEntry, or NULL if group is full
 */
GradientEntry* gradientGroup_addHorizontal(GradientGroup* group,
                                          Color start_color, Color end_color,
                                          uint16_t x, uint16_t y,
                                          uint16_t width, uint16_t height,
                                          uint8_t opacity, BLEND_MODE blend_mode);

/**
 * @brief Add a vertical gradient to the group
 * @param group GradientGroup pointer
 * @param start_color Start color (RGB565)
 * @param end_color End color (RGB565)
 * @param x X offset
 * @param y Y offset
 * @param width Width of gradient area
 * @param height Height of gradient area
 * @param opacity Opacity (0-255)
 * @param blend_mode Blend mode
 * @return Pointer to the new GradientEntry, or NULL if group is full
 */
GradientEntry* gradientGroup_addVertical(GradientGroup* group,
                                        Color start_color, Color end_color,
                                        uint16_t x, uint16_t y,
                                        uint16_t width, uint16_t height,
                                        uint8_t opacity, BLEND_MODE blend_mode);

/**
 * @brief Add a diagonal gradient to the group
 * @param group GradientGroup pointer
 * @param start_color Start color (RGB565)
 * @param end_color End color (RGB565)
 * @param x X offset
 * @param y Y offset
 * @param width Width of gradient area
 * @param height Height of gradient area
 * @param opacity Opacity (0-255)
 * @param blend_mode Blend mode
 * @return Pointer to the new GradientEntry, or NULL if group is full
 */
GradientEntry* gradientGroup_addDiagonal(GradientGroup* group,
                                        Color start_color, Color end_color,
                                        uint16_t x, uint16_t y,
                                        uint16_t width, uint16_t height,
                                        uint8_t opacity, BLEND_MODE blend_mode);

/**
 * @brief Add a linear gradient with custom angle to the group
 * @param group GradientGroup pointer
 * @param start_color Start color (RGB565)
 * @param end_color End color (RGB565)
 * @param angle Angle in degrees (0-360)
 * @param x X offset
 * @param y Y offset
 * @param width Width of gradient area
 * @param height Height of gradient area
 * @param opacity Opacity (0-255)
 * @param blend_mode Blend mode
 * @return Pointer to the new GradientEntry, or NULL if group is full
 */
GradientEntry* gradientGroup_addLinear(GradientGroup* group,
                                       Color start_color, Color end_color,
                                       int16_t angle,
                                       uint16_t x, uint16_t y,
                                       uint16_t width, uint16_t height,
                                       uint8_t opacity, BLEND_MODE blend_mode);

/**
 * @brief Add a radial gradient to the group
 * @param group GradientGroup pointer
 * @param start_color Center color (RGB565)
 * @param end_color Outer color (RGB565)
 * @param center_x Center X within gradient area
 * @param center_y Center Y within gradient area
 * @param radius Radius of gradient
 * @param x X offset within widget
 * @param y Y offset within widget
 * @param width Width of gradient area
 * @param height Height of gradient area
 * @param opacity Opacity (0-255)
 * @param blend_mode Blend mode
 * @return Pointer to the new GradientEntry, or NULL if group is full
 */
GradientEntry* gradientGroup_addRadial(GradientGroup* group,
                                       Color start_color, Color end_color,
                                       uint16_t center_x, uint16_t center_y,
                                       uint16_t radius,
                                       uint16_t x, uint16_t y,
                                       uint16_t width, uint16_t height,
                                       uint8_t opacity, BLEND_MODE blend_mode);

// --- Copy and Compare ---

/**
 * @brief Copy a gradient group from one to another
 * @param dest Destination GradientGroup pointer
 * @param src Source GradientGroup pointer
 */
void gradientGroup_copy(GradientGroup* dest, const GradientGroup* src);

/**
 * @brief Check if two gradient groups are equal
 * @param g1 First GradientGroup pointer
 * @param g2 Second GradientGroup pointer
 * @return true if equal
 */
bool gradientGroup_equal(const GradientGroup* g1, const GradientGroup* g2);

// --- Rendering Helpers ---

/**
 * @brief Get the blended color at a specific position from all gradients in group
 * @param group GradientGroup pointer
 * @param x X position within widget (0 to width-1)
 * @param y Y position within widget (0 to height-1)
 * @param widget_width Total widget width
 * @param widget_height Total widget height
 * @return Blended color (RGB565)
 */
Color gradientGroup_getColorAt(const GradientGroup* group,
                                uint16_t x, uint16_t y,
                                uint16_t widget_width, uint16_t widget_height);

/**
 * @brief Get the blended color from a single gradient entry
 * @param entry GradientEntry pointer
 * @param x X position within widget
 * @param y Y position within widget
 * @param widget_width Total widget width
 * @param widget_height Total widget height
 * @return Blended color (RGB565)
 */
Color gradientEntry_getColorAt(const GradientEntry* entry,
                                uint16_t x, uint16_t y,
                                uint16_t widget_width, uint16_t widget_height);

// =============================================================================
// BLEND FUNCTIONS
// =============================================================================

/**
 * @brief Blend two RGB565 colors with opacity
 * @param base Base color (RGB565)
 * @param top Top color (RGB565)
 * @param opacity Opacity of top color (0-255)
 * @return Blended color (RGB565)
 */
Color blend_colors(Color base, Color top, uint8_t opacity);

/**
 * @brief Blend two colors using a specific blend mode
 * @param base Base color (RGB565)
 * @param top Top color (RGB565)
 * @param mode Blend mode
 * @return Blended color (RGB565)
 */
Color blend_colors_with_mode(Color base, Color top, BLEND_MODE mode);

/**
 * @brief Blend with opacity and mode
 * @param base Base color (RGB565)
 * @param top Top color (RGB565)
 * @param opacity Opacity (0-255)
 * @param mode Blend mode
 * @return Blended color (RGB565)
 */
Color blend_colors_ex(Color base, Color top, uint8_t opacity, BLEND_MODE mode);

// =============================================================================
// MACRO-BASED SYNTAX (Objective-C style)
// =============================================================================

// Initialization
#define INIT_GRADIENT_GROUP(g)        gradientGroup_init(g)
#define RESET_GRADIENT_GROUP(g)       gradientGroup_reset(g)
#define CLEAR_GRADIENT_GROUP(g)       gradientGroup_clear(g)

// Adding gradients
#define ADD_GRADIENT(g)               gradientGroup_addGradient(g)
#define ADD_HGRADIENT(g, s, e, x, y, w, h, o, m) gradientGroup_addHorizontal(g, s, e, x, y, w, h, o, m)
#define ADD_VGRADIENT(g, s, e, x, y, w, h, o, m) gradientGroup_addVertical(g, s, e, x, y, w, h, o, m)
#define ADD_DGRADIENT(g, s, e, x, y, w, h, o, m) gradientGroup_addDiagonal(g, s, e, x, y, w, h, o, m)
#define ADD_LGRADIENT(g, s, e, a, x, y, w, h, o, m) gradientGroup_addLinear(g, s, e, a, x, y, w, h, o, m)
#define ADD_RGRADIENT(g, s, e, cx, cy, r, x, y, w, h, o, m) gradientGroup_addRadial(g, s, e, cx, cy, r, x, y, w, h, o, m)

// Gradient entry access
#define GET_GRADIENT_ENTRY(g, i)      GRADIENT_GROUP_GET(g, i)
#define GET_FIRST_GRADIENT(g)        GRADIENT_GROUP_FIRST(g)
#define GET_LAST_GRADIENT(g)         GRADIENT_GROUP_LAST(g)

// Remove
#define REMOVE_GRADIENT(g, i)        gradientGroup_removeGradient(g, i)
#define REMOVE_LAST_GRADIENT(g)       gradientGroup_removeLast(g)

// Copy
#define COPY_GRADIENT_GROUP(d, s)    gradientGroup_copy(d, s)

// Get color
#define GET_GROUP_COLOR(g, x, y, w, h) gradientGroup_getColorAt(g, x, y, w, h)

// =============================================================================
// INLINE FUNCTIONS
// =============================================================================

/**
 * @brief Get number of gradients in group
 * @param group GradientGroup pointer
 * @return Number of gradients
 */
static inline uint8_t gradientGroup_getCount(const GradientGroup* group) {
    return group->count;
}

/**
 * @brief Check if gradient group has any gradients
 * @param group GradientGroup pointer
 * @return true if group has gradients
 */
static inline bool gradientGroup_hasGradients(const GradientGroup* group) {
    return group->count > 0;
}

/**
 * @brief Check if gradient group is at maximum capacity
 * @param group GradientGroup pointer
 * @return true if group is full
 */
static inline bool gradientGroup_isFull(const GradientGroup* group) {
    return group->count >= MAX_GRADIENTS_PER_GROUP;
}

/**
 * @brief Get the gradient at a specific index
 * @param group GradientGroup pointer
 * @param index Index (0-5)
 * @return Pointer to GradientEntry, or NULL if invalid
 */
static inline GradientEntry* gradientGroup_getEntry(GradientGroup* group, uint8_t index) {
    if (index >= group->count) return NULL;
    return &group->gradients[index];
}

/**
 * @brief Get the gradient at a specific index (const version)
 * @param group GradientGroup pointer
 * @param index Index (0-5)
 * @return Pointer to GradientEntry, or NULL if invalid
 */
static inline const GradientEntry* gradientGroup_getEntryConst(const GradientGroup* group, uint8_t index) {
    if (index >= group->count) return NULL;
    return &group->gradients[index];
}

/**
 * @brief Enable a gradient entry
 * @param entry GradientEntry pointer
 */
static inline void gradientEntry_enable(GradientEntry* entry) {
    entry->enabled = true;
}

/**
 * @brief Disable a gradient entry
 * @param entry GradientEntry pointer
 */
static inline void gradientEntry_disable(GradientEntry* entry) {
    entry->enabled = false;
}

/**
 * @brief Check if a gradient entry is enabled
 * @param entry GradientEntry pointer
 * @return true if enabled
 */
static inline bool gradientEntry_isEnabled(const GradientEntry* entry) {
    return entry->enabled;
}

/**
 * @brief Set opacity for a gradient entry
 * @param entry GradientEntry pointer
 * @param opacity Opacity (0-255)
 */
static inline void gradientEntry_setOpacity(GradientEntry* entry, uint8_t opacity) {
    entry->opacity = opacity;
}

/**
 * @brief Get opacity for a gradient entry
 * @param entry GradientEntry pointer
 * @return Opacity (0-255)
 */
static inline uint8_t gradientEntry_getOpacity(const GradientEntry* entry) {
    return entry->opacity;
}

/**
 * @brief Set blend mode for a gradient entry
 * @param entry GradientEntry pointer
 * @param mode Blend mode
 */
static inline void gradientEntry_setBlendMode(GradientEntry* entry, BLEND_MODE mode) {
    entry->blend_mode = mode;
}

/**
 * @brief Get blend mode for a gradient entry
 * @param entry GradientEntry pointer
 * @return Blend mode
 */
static inline BLEND_MODE gradientEntry_getBlendMode(const GradientEntry* entry) {
    return entry->blend_mode;
}

/**
 * @brief Set bounds for a gradient entry
 * @param entry GradientEntry pointer
 * @param x X offset
 * @param y Y offset
 * @param width Width
 * @param height Height
 */
static inline void gradientEntry_setBounds(GradientEntry* entry,
                                          uint16_t x, uint16_t y,
                                          uint16_t width, uint16_t height) {
    entry->bounds.x = x;
    entry->bounds.y = y;
    entry->bounds.width = width;
    entry->bounds.height = height;
}

#endif // WIDGET_GRADIENT_GROUP_H
