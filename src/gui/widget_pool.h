/**
 * widget_pool.h - Object Pooling for Widgets
 * 
 * Objective-C-style memory management for ESP8266 GUIKit
 * Avoids malloc/free overhead and fragmentation
 * 
 * Generated for ESP8266 by Mistral Vibe
 */

#ifndef WIDGET_POOL_H
#define WIDGET_POOL_H

#include <stdint.h>
#include <stdbool.h>
#include "widget_text.h"


// ============================================================================
// WIDGET TYPE DEFINITIONS
// ============================================================================

typedef enum {
    WIDGET_TYPE_VIEW,          // Container widget
    WIDGET_TYPE_BUTTON,        // Clickable button
    WIDGET_TYPE_LABEL,         // Text display
    WIDGET_TYPE_SLIDER,        // Slider control
    WIDGET_TYPE_TEXT_EDITOR,   // Full-featured text editor
    WIDGET_TYPE_MENU,          // Context menu
    WIDGET_TYPE_COUNT          // Number of widget types
} WIDGET_TYPE;


// Forward declarations
typedef struct Widget Widget;
typedef struct WidgetButton WidgetButton;
typedef struct WidgetLabel WidgetLabel;
typedef struct WidgetSlider WidgetSlider;
typedef struct WidgetView WidgetView;
typedef struct ContextMenu ContextMenu;


// ============================================================================
// BASE WIDGET STRUCTURE
// ============================================================================

/**
 * Base widget structure - common to all widget types
 */
typedef struct Widget {
    WIDGET_TYPE type;                    // Widget type
    uint16_t x, y;                      // Position
    uint16_t width, height;             // Dimensions
    WidgetText text;                    // Text content (static buffer)
    struct Widget** children;           // Child widgets
    uint8_t children_count;             // Number of children
    uint8_t children_capacity;          // Allocated capacity
    uint16_t bg_color;                  // Background color (RGBA565)
    uint16_t border_color;               // Border color
    uint8_t border_width;                // Border width
    bool dirty;                         // Needs redrawing
    bool visible;                       // Visibility flag
} Widget;


// ============================================================================
// SPECIFIC WIDGET TYPES
// ============================================================================

/** Button widget */
typedef struct WidgetButton {
    Widget base;                       // Base widget
    bool pressed;                     // Pressed state
    void (*onClick)(void*);            // Click callback
} WidgetButton;

/** Label widget */
typedef struct WidgetLabel {
    Widget base;                       // Base widget
} WidgetLabel;

/** Slider widget */
typedef struct WidgetSlider {
    Widget base;                       // Base widget
    uint16_t min_value;                // Minimum value
    uint16_t max_value;                // Maximum value
    uint16_t value;                    // Current value
    bool vertical;                     // Orientation
    void (*onChange)(uint16_t);        // Value change callback
} WidgetSlider;

/** View/Container widget */
typedef struct WidgetView {
    Widget base;                       // Base widget
} WidgetView;


// ============================================================================
// POOL CONFIGURATION
// ============================================================================

// Adjust these based on your application needs
#define MAX_WIDGETS     30      // Total generic widgets
#define MAX_BUTTONS     20      // Button widgets
#define MAX_LABELS      20      // Label widgets
#define MAX_SLIDERS     10      // Slider widgets
#define MAX_VIEWS       10      // View widgets


// ============================================================================
// POOLED WIDGET STRUCTURES
// ============================================================================

/** Pooled widget with usage tracking */
typedef struct {
    Widget base;
    bool in_use;
} PooledWidget;

/** Pooled button with usage tracking */
typedef struct {
    WidgetButton base;
    bool in_use;
} PooledButton;

/** Pooled label with usage tracking */
typedef struct {
    WidgetLabel base;
    bool in_use;
} PooledLabel;

/** Pooled slider with usage tracking */
typedef struct {
    WidgetSlider base;
    bool in_use;
} PooledSlider;

/** Pooled view with usage tracking */
typedef struct {
    WidgetView base;
    bool in_use;
} PooledView;


// ============================================================================
// POOL DECLARATIONS (defined in widget_pool.c)
// ============================================================================

// Global pools
extern PooledWidget widget_pool[MAX_WIDGETS];
extern PooledButton button_pool[MAX_BUTTONS];
extern PooledLabel label_pool[MAX_LABELS];
extern PooledSlider slider_pool[MAX_SLIDERS];
extern PooledView view_pool[MAX_VIEWS];


// ============================================================================
// POOL INITIALIZATION
// ============================================================================

/**
 * @brief Initialize all widget pools
 * 
 * Call this once at application startup
 */
void pools_init(void);


// ============================================================================
// POOL ALLOCATION
// ============================================================================

/**
 * @brief Allocate a widget from the appropriate pool
 * 
 * @param type Widget type to allocate
 * @return Pointer to widget, or NULL if pool exhausted
 */
Widget* widget_alloc(WIDGET_TYPE type);


// Type-specific allocation (for when you need the specific type)
WidgetButton* button_alloc(void);
WidgetLabel* label_alloc(void);
WidgetSlider* slider_alloc(void);
WidgetView* view_alloc(void);


// ============================================================================
// POOL RELEASE
// ============================================================================

/**
 * @brief Release a widget back to its pool
 * 
 * @param w Pointer to widget to release
 */
void widget_free(Widget* w);

// Type-specific release
void button_free(WidgetButton* btn);
void label_free(WidgetLabel* lbl);
void slider_free(WidgetSlider* slider);
void view_free(WidgetView* view);


// ============================================================================
// SAFE RELEASE MACRO (Objective-C style)
// ============================================================================

/**
 * @brief Safely release a widget and set pointer to NULL
 * 
 * Usage: SAFE_RELEASE(my_widget);
 */
#define SAFE_RELEASE(w) do { \
    if ((w)) { \
        widget_free((Widget*)(w)); \
        (w) = NULL; \
    } \
} while(0)


// ============================================================================
// POOL STATISTICS
// ============================================================================

/**
 * @brief Get number of allocated widgets of a specific type
 * 
 * @param type Widget type
 * @return Number of widgets currently allocated
 */
uint8_t pool_get_allocated_count(WIDGET_TYPE type);

/**
 * @brief Get total capacity for a widget type
 * 
 * @param type Widget type
 * @return Total capacity for that widget type
 */
uint8_t pool_get_capacity(WIDGET_TYPE type);

/**
 * @brief Check if a widget is from a pool
 * 
 * @param w Widget pointer
 * @return true if widget is from a pool
 */
bool widget_is_pooled(const Widget* w);

// ============================================================================
// WIDGET FINDING
// ============================================================================

/**
 * @brief Find widget at a specific screen position
 * 
 * @param x X coordinate
 * @param y Y coordinate
 * @return Pointer to widget at position, or NULL if none
 */
Widget* widget_find_at_position(uint16_t x, uint16_t y);


// ============================================================================
// UTILITY MACROS
// ============================================================================

/** Check if pool is exhausted */
#define POOL_IS_FULL(type) (pool_get_allocated_count(type) >= pool_get_capacity(type))

/** Get remaining capacity */
#define POOL_REMAINING(type) (pool_get_capacity(type) - pool_get_allocated_count(type))


#endif // WIDGET_POOL_H
