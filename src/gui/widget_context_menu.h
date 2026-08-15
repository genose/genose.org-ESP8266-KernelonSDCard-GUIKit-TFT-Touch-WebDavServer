/**
 * @file widget_context_menu.h
 * @brief Contextual menu widget for GUIKit
 * @author GUIKit for ESP8266
 * @date 2026
 * 
 * Contextual menu appears on long touch (~2 seconds) and provides
 * context-sensitive actions for the widget under the touch.
 * 
 * Memory optimization: Uses fixed-size buffers, pooled memory
 * ESP8266-friendly: No dynamic allocation, static buffers
 * 
 * Features:
 * - Automatic positioning at touch location
 * - Configurable menu items per widget type
 * - Callback-based action handling
 * - Auto-hide on selection or outside touch
 * - Visual styling (background, border, text colors)
 * 
 * Usage:
 * @code
 * // Initialize context menu system
 * context_menu_init();
 * 
 * // Set callback for text editor
 * ContextMenuItem editor_items[] = {
 *     { "Copy",   ACTION_COPY },
 *     { "Paste",  ACTION_PASTE },
 *     { "Cut",    ACTION_CUT },
 *     { NULL,     0 }
 * };
 * context_menu_register_widget_type(WIDGET_TYPE_TEXT_EDITOR, editor_items);
 * 
 * // In touch handling:
 * if (gesture == GESTURE_LONG_PRESS) {
 *     context_menu_show_at(x, y, widget);
 * }
 * @endcode
 */

#ifndef WIDGET_CONTEXT_MENU_H
#define WIDGET_CONTEXT_MENU_H

#include "widget_pool.h"
#include <stdint.h>
#include <stdbool.h>

// Color type (RGBA565 format)
#ifndef COLOR_DEFINED
#define COLOR_DEFINED
typedef uint16_t Color;
#endif

// Gesture types (forward declaration - see gestures.h for full implementation)
#ifndef GESTURE_TYPE_DEFINED
#define GESTURE_TYPE_DEFINED
typedef enum {
    GESTURE_NONE,           // No gesture detected
    GESTURE_TAP,            // Single tap
    GESTURE_DOUBLE_TAP,     // Double tap
    GESTURE_LONG_PRESS,     // Long press (hold > CONTEXT_MENU_LONG_PRESS_DURATION)
    GESTURE_SWIPE_LEFT,     // Swipe left
    GESTURE_SWIPE_RIGHT,    // Swipe right
    GESTURE_SWIPE_UP,       // Swipe up
    GESTURE_SWIPE_DOWN,     // Swipe down
    GESTURE_PINCH_IN,       // Pinch in
    GESTURE_PINCH_OUT       // Pinch out
} GestureType;
#endif

// Default font dimensions (if not specified in widget)
#define CONTEXT_MENU_FONT_WIDTH 8
#define CONTEXT_MENU_FONT_HEIGHT 16

// =============================================================================
// CONSTANTS
// =============================================================================

/** Maximum number of menu items per context menu */
#define CONTEXT_MENU_MAX_ITEMS 8

/** Maximum length of menu item text */
#define CONTEXT_MENU_ITEM_TEXT_LEN 32

/** Context menu long press duration in milliseconds */
#define CONTEXT_MENU_LONG_PRESS_DURATION 2000

/** Menu item height in pixels */
#define CONTEXT_MENU_ITEM_HEIGHT 24

/** Menu minimum width in pixels */
#define CONTEXT_MENU_MIN_WIDTH 120

/** Menu background color (RGBA565) */
#define CONTEXT_MENU_BG_COLOR 0xFFFF  // White

/** Menu border color (RGBA565) */
#define CONTEXT_MENU_BORDER_COLOR 0x8410  // Dark gray

/** Menu text color (RGBA565) */
#define CONTEXT_MENU_TEXT_COLOR 0x0000  // Black

/** Menu selected item background color (RGBA565) */
#define CONTEXT_MENU_SELECT_BG 0xA514  // Light blue

/** Menu border width in pixels */
#define CONTEXT_MENU_BORDER_WIDTH 1

// =============================================================================
// ACTION TYPES
// =============================================================================

/** Context menu action types (extendable by user) */
typedef enum {
    CONTEXT_ACTION_NONE = 0,
    CONTEXT_ACTION_COPY,
    CONTEXT_ACTION_PASTE,
    CONTEXT_ACTION_CUT,
    CONTEXT_ACTION_DELETE,
    CONTEXT_ACTION_SELECT_ALL,
    CONTEXT_ACTION_UNDO,
    CONTEXT_ACTION_REDO,
    CONTEXT_ACTION_SAVE,
    CONTEXT_ACTION_LOAD,
    CONTEXT_ACTION_NEW,
    CONTEXT_ACTION_OPEN,
    CONTEXT_ACTION_PROPERTIES,
    CONTEXT_ACTION_CUSTOM  // User-defined actions start here
} ContextActionType;

// =============================================================================
// STRUCTURES
// =============================================================================

/** Context menu item structure */
typedef struct {
    const char* text;                   ///< Display text (NULL = separator)
    ContextActionType action;          ///< Action to perform
    uint16_t custom_id;               ///< Custom ID for user actions
    bool enabled;                     ///< Whether item is enabled
    Color text_color;                 ///< Text color (optional override)
} ContextMenuItem;

/** Context menu widget structure */
typedef struct {
    Widget base;                       ///< Base widget
    
    // Position and size
    uint16_t x;                       ///< X position (touch location)
    uint16_t y;                       ///< Y position (touch location)
    uint16_t width;                   ///< Menu width
    uint16_t height;                  ///< Menu height
    
    // Menu items
    ContextMenuItem items[CONTEXT_MENU_MAX_ITEMS];
    uint8_t item_count;               ///< Number of items
    
    // State
    uint8_t selected_index;           ///< Currently selected item (0 = none)
    bool visible;                     ///< Whether menu is visible
    Widget* target_widget;            ///< Widget that triggered the menu
    
    // Styling
    Color bg_color;                   ///< Background color
    Color border_color;               ///< Border color
    Color text_color;                 ///< Text color
    Color select_bg_color;            ///< Selected item background
    uint8_t border_width;             ///< Border width
    uint8_t item_height;              ///< Item height
    
    // Callback
    void (*on_action)(Widget* widget, ContextActionType action, uint16_t custom_id);
    
} ContextMenu;

/** Context menu configuration for widget types */
typedef struct {
    WIDGET_TYPE widget_type;          ///< Widget type this config applies to
    ContextMenuItem* items;           ///< Menu items for this type
    uint8_t item_count;               ///< Number of items
    void (*on_action)(Widget* widget, ContextActionType action, uint16_t custom_id);
} ContextMenuWidgetConfig;

// =============================================================================
// GLOBAL CONFIGURATION
// =============================================================================

/** Maximum number of widget type configurations */
#define CONTEXT_MENU_MAX_WIDGET_TYPES 16

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// --- Initialization ---

/**
 * @brief Initialize the context menu system
 * 
 * Call this once during setup before using any context menu functionality.
 */
void context_menu_init(void);

// --- Menu Management ---

/**
 * @brief Create a new context menu
 * 
 * @param x X position (touch location)
 * @param y Y position (touch location)
 * @param target_widget The widget that triggered the menu
 * @return Pointer to the created context menu, or NULL on failure
 */
ContextMenu* context_menu_create(uint16_t x, uint16_t y, Widget* target_widget);

/**
 * @brief Destroy a context menu
 * 
 * @param menu Pointer to the context menu to destroy
 */
void context_menu_destroy(ContextMenu* menu);

/**
 * @brief Show context menu at position
 * 
 * @param x X position (touch location)
 * @param y Y position (touch location)
 * @param target_widget The widget under the touch
 * @return Pointer to the shown context menu, or NULL on failure
 */
ContextMenu* context_menu_show_at(uint16_t x, uint16_t y, Widget* target_widget);

/**
 * @brief Hide the current context menu
 */
void context_menu_hide(void);

/**
 * @brief Toggle context menu visibility
 */
void context_menu_toggle(void);

/**
 * @brief Check if context menu is visible
 * 
 * @return true if context menu is visible
 */
bool context_menu_is_visible(void);

// --- Menu Items ---

/**
 * @brief Add an item to a context menu
 * 
 * @param menu Pointer to the context menu
 * @param text Display text (NULL for separator)
 * @param action Action type
 * @param custom_id Custom ID for user actions
 * @param enabled Whether item is enabled
 */
void context_menu_add_item(ContextMenu* menu, const char* text, 
                          ContextActionType action, uint16_t custom_id, 
                          bool enabled);

/**
 * @brief Clear all items from a context menu
 * 
 * @param menu Pointer to the context menu
 */
void context_menu_clear_items(ContextMenu* menu);

/**
 * @brief Set menu items from an array
 * 
 * @param menu Pointer to the context menu
 * @param items Array of menu items (NULL-terminated or with count)
 * @param count Number of items (0 = auto-detect from NULL terminator)
 */
void context_menu_set_items(ContextMenu* menu, ContextMenuItem* items, uint8_t count);

// --- Widget Type Registration ---

/**
 * @brief Register context menu configuration for a widget type
 * 
 * @param widget_type The widget type
 * @param items Array of menu items
 * @param count Number of items
 * @param on_action Callback for actions (can be NULL for default)
 * @return true on success
 */
bool context_menu_register_widget_type(WIDGET_TYPE widget_type, 
                                       ContextMenuItem* items, 
                                       uint8_t count,
                                       void (*on_action)(Widget*, ContextActionType, uint16_t));

/**
 * @brief Unregister context menu configuration for a widget type
 * 
 * @param widget_type The widget type
 */
void context_menu_unregister_widget_type(WIDGET_TYPE widget_type);

/**
 * @brief Get context menu configuration for a widget type
 * 
 * @param widget_type The widget type
 * @return Pointer to configuration, or NULL if not registered
 */
ContextMenuWidgetConfig* context_menu_get_config(WIDGET_TYPE widget_type);

// --- Styling ---

/**
 * @brief Set context menu background color
 * 
 * @param menu Pointer to the context menu (NULL for default)
 * @param color Background color
 */
void context_menu_set_bg_color(ContextMenu* menu, Color color);

/**
 * @brief Set context menu border color
 * 
 * @param menu Pointer to the context menu (NULL for default)
 * @param color Border color
 */
void context_menu_set_border_color(ContextMenu* menu, Color color);

/**
 * @brief Set context menu text color
 * 
 * @param menu Pointer to the context menu (NULL for default)
 * @param color Text color
 */
void context_menu_set_text_color(ContextMenu* menu, Color color);

/**
 * @brief Set context menu selected item background color
 * 
 * @param menu Pointer to the context menu (NULL for default)
 * @param color Selected item background color
 */
void context_menu_set_select_bg_color(ContextMenu* menu, Color color);

/**
 * @brief Set context menu border width
 * 
 * @param menu Pointer to the context menu (NULL for default)
 * @param width Border width in pixels
 */
void context_menu_set_border_width(ContextMenu* menu, uint8_t width);

/**
 * @brief Set context menu item height
 * 
 * @param menu Pointer to the context menu (NULL for default)
 * @param height Item height in pixels
 */
void context_menu_set_item_height(ContextMenu* menu, uint8_t height);

// --- Navigation ---

/**
 * @brief Select next menu item
 * 
 * @param menu Pointer to the context menu
 */
void context_menu_next_item(ContextMenu* menu);

/**
 * @brief Select previous menu item
 * 
 * @param menu Pointer to the context menu
 */
void context_menu_prev_item(ContextMenu* menu);

/**
 * @brief Select menu item at index
 * 
 * @param menu Pointer to the context menu
 * @param index Item index
 */
void context_menu_select_item(ContextMenu* menu, uint8_t index);

/**
 * @brief Get currently selected item index
 * 
 * @param menu Pointer to the context menu
 * @return Selected item index, or -1 if none
 */
int8_t context_menu_get_selected_index(const ContextMenu* menu);

// --- Action Handling ---

/**
 * @brief Execute the action for the currently selected item
 * 
 * @param menu Pointer to the context menu
 */
void context_menu_execute_selected_action(ContextMenu* menu);

/**
 * @brief Set action callback for a context menu
 * 
 * @param menu Pointer to the context menu
 * @param on_action Callback function
 */
void context_menu_set_on_action(ContextMenu* menu, 
                                void (*on_action)(Widget*, ContextActionType, uint16_t));

/**
 * @brief Set default action callback
 * 
 * @param on_action Callback function
 */
void context_menu_set_default_on_action(void (*on_action)(Widget*, ContextActionType, uint16_t));

// --- Rendering ---

/**
 * @brief Render a context menu
 * 
 * @param menu Pointer to the context menu
 */
void context_menu_render(const ContextMenu* menu);

/**
 * @brief Render the current (active) context menu
 */
void context_menu_render_current(void);

// --- Touch Handling ---

/**
 * @brief Handle touch event for context menu
 * 
 * @param x Touch X coordinate
 * @param y Touch Y coordinate
 * @param pressed Whether touch is pressed
 * @return true if event was handled by context menu
 */
bool context_menu_handle_touch(uint16_t x, uint16_t y, bool pressed);

/**
 * @brief Handle gesture for context menu
 * 
 * @param gesture Gesture type
 * @param x Touch X coordinate
 * @param y Touch Y coordinate
 * @return true if gesture was handled by context menu
 */
bool context_menu_handle_gesture(GestureType gesture, uint16_t x, uint16_t y);

// --- Hit Testing ---

/**
 * @brief Check if a point is inside the context menu
 * 
 * @param menu Pointer to the context menu
 * @param x X coordinate
 * @param y Y coordinate
 * @return true if point is inside the menu
 */
bool context_menu_contains_point(const ContextMenu* menu, uint16_t x, uint16_t y);

/**
 * @brief Get menu item at a point
 * 
 * @param menu Pointer to the context menu
 * @param x X coordinate
 * @param y Y coordinate
 * @return Item index, or -1 if not on any item
 */
int8_t context_menu_get_item_at_point(const ContextMenu* menu, uint16_t x, uint16_t y);

// --- Getters ---

/**
 * @brief Get the current (active) context menu
 * 
 * @return Pointer to the current context menu, or NULL if none
 */
ContextMenu* context_menu_get_current(void);

/**
 * @brief Get the target widget of the current context menu
 * 
 * @return Pointer to the target widget, or NULL if no menu
 */
Widget* context_menu_get_target_widget(void);

// --- Utility ---

/**
 * @brief Calculate menu width based on items
 * 
 * @param menu Pointer to the context menu
 * @return Calculated width
 */
uint16_t context_menu_calculate_width(const ContextMenu* menu);

/**
 * @brief Adjust menu position to fit on screen
 * 
 * @param menu Pointer to the context menu
 * @param screen_width Screen width
 * @param screen_height Screen height
 */
void context_menu_adjust_position(ContextMenu* menu, uint16_t screen_width, uint16_t screen_height);

#endif // WIDGET_CONTEXT_MENU_H
