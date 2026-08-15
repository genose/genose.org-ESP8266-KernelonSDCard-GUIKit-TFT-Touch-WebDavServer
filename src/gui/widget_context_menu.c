/**
 * @file widget_context_menu.c
 * @brief Contextual menu widget implementation for GUIKit
 * @author GUIKit for ESP8266
 * @date 2026
 */

#include "widget_context_menu.h"
#include "renderer.h"
#include "touch.h"
#include <string.h>
#include <stdlib.h>

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================

/** Current active context menu (singleton - only one at a time) */
static ContextMenu* current_menu = NULL;

/** Widget type configurations */
static ContextMenuWidgetConfig widget_configs[CONTEXT_MENU_MAX_WIDGET_TYPES];

/** Number of registered widget configurations */
static uint8_t widget_config_count = 0;

/** Default action callback */
static void (*default_on_action)(Widget*, ContextActionType, uint16_t) = NULL;

// =============================================================================
// STATIC FUNCTIONS
// =============================================================================

/**
 * @brief Find widget configuration by widget type
 * 
 * @param widget_type The widget type to find
 * @return Index of configuration, or -1 if not found
 */
static int8_t find_widget_config_index(WIDGET_TYPE widget_type) {
    for (uint8_t i = 0; i < widget_config_count; i++) {
        if (widget_configs[i].widget_type == widget_type) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Calculate text width in pixels
 * 
 * @param text The text to measure
 * @param font_width Character width in pixels
 * @return Width in pixels
 */
static uint16_t calculate_text_width(const char* text, uint8_t font_width) {
    if (!text) return 0;
    return (uint16_t)(strlen(text) * font_width);
}

/**
 * @brief Draw a menu item
 * 
 * @param menu Pointer to the context menu
 * @param index Item index
 * @param x Item X position
 * @param y Item Y position
 * @param width Item width
 * @param height Item height
 */
static void draw_menu_item(const ContextMenu* menu, uint8_t index, 
                           uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    const ContextMenuItem* item = &menu->items[index];
    bool selected = (index == menu->selected_index);
    
    // Draw background
    if (selected && item->text != NULL) {
        // Selected item background
        renderer_fill_rect(x, y, width, height, menu->select_bg_color);
    } else {
        // Normal item background
        renderer_fill_rect(x, y, width, height, menu->bg_color);
    }
    
    // Draw separator if this is a separator item
    if (item->text == NULL) {
        // Draw horizontal line for separator
        uint16_t line_y = y + height / 2;
        renderer_draw_hline(x, line_y, x + width - 1, menu->border_color);
        return;
    }
    
    // Draw item text
    uint16_t text_x = x + 8;  // 8px padding
    uint16_t text_y = y + (height - CONTEXT_MENU_FONT_HEIGHT) / 2;
    Color text_color = item->enabled ? (item->text_color != 0 ? item->text_color : menu->text_color) : 0x8410; // Gray if disabled
    
    renderer_draw_text(text_x, text_y, item->text, text_color, menu->bg_color, 1);
}

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

// --- Initialization ---

void context_menu_init(void) {
    current_menu = NULL;
    widget_config_count = 0;
    default_on_action = NULL;
    memset(widget_configs, 0, sizeof(widget_configs));
}

// --- Menu Management ---

ContextMenu* context_menu_create(uint16_t x, uint16_t y, Widget* target_widget) {
    // Allocate menu (using pool or static for ESP8266)
    // For now, use static allocation (single menu at a time)
    if (current_menu) {
        context_menu_destroy(current_menu);
    }
    
    ContextMenu* menu = (ContextMenu*)malloc(sizeof(ContextMenu));
    if (!menu) {
        return NULL;
    }
    
    memset(menu, 0, sizeof(ContextMenu));
    
    // Initialize base widget
    menu->base.type = WIDGET_TYPE_MENU;
    menu->base.visible = true;
    menu->base.enabled = true;
    
    // Initialize position
    menu->x = x;
    menu->y = y;
    menu->target_widget = target_widget;
    
    // Default styling
    menu->bg_color = CONTEXT_MENU_BG_COLOR;
    menu->border_color = CONTEXT_MENU_BORDER_COLOR;
    menu->text_color = CONTEXT_MENU_TEXT_COLOR;
    menu->select_bg_color = CONTEXT_MENU_SELECT_BG;
    menu->border_width = CONTEXT_MENU_BORDER_WIDTH;
    menu->item_height = CONTEXT_MENU_ITEM_HEIGHT;
    
    // Default state
    menu->visible = false;
    menu->selected_index = 0;
    menu->item_count = 0;
    menu->on_action = NULL;
    
    return menu;
}

void context_menu_destroy(ContextMenu* menu) {
    if (!menu) return;
    
    if (menu == current_menu) {
        current_menu = NULL;
    }
    
    free(menu);
}

ContextMenu* context_menu_show_at(uint16_t x, uint16_t y, Widget* target_widget) {
    // Hide current menu if any
    context_menu_hide();
    
    // Create new menu
    ContextMenu* menu = context_menu_create(x, y, target_widget);
    if (!menu) {
        return NULL;
    }
    
    // Find widget type configuration
    ContextMenuWidgetConfig* config = context_menu_get_config(target_widget->type);
    
    if (config) {
        // Copy items from configuration
        for (uint8_t i = 0; i < config->item_count && i < CONTEXT_MENU_MAX_ITEMS; i++) {
            menu->items[i] = config->items[i];
        }
        menu->item_count = config->item_count;
        menu->on_action = config->on_action;
    } else {
        // Default items for unknown widget types
        ContextMenuItem default_items[] = {
            { "Properties", CONTEXT_ACTION_PROPERTIES, 0, true, 0 }
        };
        context_menu_set_items(menu, default_items, 1);
    }
    
    // Calculate menu dimensions
    menu->width = context_menu_calculate_width(menu);
    menu->height = menu->item_count * menu->item_height;
    
    // Adjust position to fit on screen
    context_menu_adjust_position(menu, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // Make visible
    menu->visible = true;
    current_menu = menu;
    
    return menu;
}

void context_menu_hide(void) {
    if (current_menu) {
        current_menu->visible = false;
        context_menu_destroy(current_menu);
        current_menu = NULL;
    }
}

void context_menu_toggle(void) {
    if (context_menu_is_visible()) {
        context_menu_hide();
    } else {
        // Can't show without position - need to call show_at
    }
}

bool context_menu_is_visible(void) {
    return current_menu && current_menu->visible;
}

// --- Menu Items ---

void context_menu_add_item(ContextMenu* menu, const char* text, 
                          ContextActionType action, uint16_t custom_id, 
                          bool enabled) {
    if (!menu || menu->item_count >= CONTEXT_MENU_MAX_ITEMS) {
        return;
    }
    
    ContextMenuItem* item = &menu->items[menu->item_count++];
    item->text = text;
    item->action = action;
    item->custom_id = custom_id;
    item->enabled = enabled;
    item->text_color = 0;  // Use default
    
    // Recalculate dimensions
    if (menu->visible) {
        menu->width = context_menu_calculate_width(menu);
        menu->height = menu->item_count * menu->item_height;
    }
}

void context_menu_clear_items(ContextMenu* menu) {
    if (!menu) return;
    
    menu->item_count = 0;
    menu->selected_index = 0;
    
    if (menu->visible) {
        menu->width = CONTEXT_MENU_MIN_WIDTH;
        menu->height = 0;
    }
}

void context_menu_set_items(ContextMenu* menu, ContextMenuItem* items, uint8_t count) {
    if (!menu) return;
    
    context_menu_clear_items(menu);
    
    if (count == 0) {
        // Auto-detect count from NULL terminator
        if (items) {
            while (items[count].text != NULL && count < CONTEXT_MENU_MAX_ITEMS) {
                menu->items[count] = items[count];
                count++;
            }
        }
    } else {
        for (uint8_t i = 0; i < count && i < CONTEXT_MENU_MAX_ITEMS; i++) {
            menu->items[i] = items[i];
        }
    }
    
    menu->item_count = count;
    menu->selected_index = 0;
    
    if (menu->visible) {
        menu->width = context_menu_calculate_width(menu);
        menu->height = menu->item_count * menu->item_height;
    }
}

// --- Widget Type Registration ---

bool context_menu_register_widget_type(WIDGET_TYPE widget_type, 
                                       ContextMenuItem* items, 
                                       uint8_t count,
                                       void (*on_action)(Widget*, ContextActionType, uint16_t)) {
    // Check if already registered
    int8_t index = find_widget_config_index(widget_type);
    
    if (index >= 0) {
        // Update existing
        widget_configs[index].items = items;
        widget_configs[index].item_count = count;
        widget_configs[index].on_action = on_action;
        return true;
    }
    
    // Add new configuration
    if (widget_config_count >= CONTEXT_MENU_MAX_WIDGET_TYPES) {
        return false;
    }
    
    ContextMenuWidgetConfig* config = &widget_configs[widget_config_count++];
    config->widget_type = widget_type;
    config->items = items;
    config->item_count = count;
    config->on_action = on_action;
    
    return true;
}

void context_menu_unregister_widget_type(WIDGET_TYPE widget_type) {
    int8_t index = find_widget_config_index(widget_type);
    
    if (index >= 0) {
        // Remove by shifting
        for (uint8_t i = index; i < widget_config_count - 1; i++) {
            widget_configs[i] = widget_configs[i + 1];
        }
        widget_config_count--;
    }
}

ContextMenuWidgetConfig* context_menu_get_config(WIDGET_TYPE widget_type) {
    int8_t index = find_widget_config_index(widget_type);
    
    if (index >= 0) {
        return &widget_configs[index];
    }
    
    return NULL;
}

// --- Styling ---

void context_menu_set_bg_color(ContextMenu* menu, Color color) {
    if (menu) {
        menu->bg_color = color;
    }
}

void context_menu_set_border_color(ContextMenu* menu, Color color) {
    if (menu) {
        menu->border_color = color;
    }
}

void context_menu_set_text_color(ContextMenu* menu, Color color) {
    if (menu) {
        menu->text_color = color;
    }
}

void context_menu_set_select_bg_color(ContextMenu* menu, Color color) {
    if (menu) {
        menu->select_bg_color = color;
    }
}

void context_menu_set_border_width(ContextMenu* menu, uint8_t width) {
    if (menu) {
        menu->border_width = width;
    }
}

void context_menu_set_item_height(ContextMenu* menu, uint8_t height) {
    if (menu) {
        menu->item_height = height;
        if (menu->visible) {
            menu->height = menu->item_count * height;
        }
    }
}

// --- Navigation ---

void context_menu_next_item(ContextMenu* menu) {
    if (!menu || menu->item_count == 0) return;
    
    do {
        menu->selected_index = (menu->selected_index + 1) % menu->item_count;
    } while (menu->items[menu->selected_index].text == NULL && 
             menu->selected_index < menu->item_count);
}

void context_menu_prev_item(ContextMenu* menu) {
    if (!menu || menu->item_count == 0) return;
    
    do {
        menu->selected_index = (menu->selected_index - 1 + menu->item_count) % menu->item_count;
    } while (menu->items[menu->selected_index].text == NULL &&
             menu->selected_index < menu->item_count);
}

void context_menu_select_item(ContextMenu* menu, uint8_t index) {
    if (!menu || index >= menu->item_count) return;
    
    if (menu->items[index].text == NULL) {
        // Skip separators
        return;
    }
    
    menu->selected_index = index;
}

int8_t context_menu_get_selected_index(const ContextMenu* menu) {
    if (!menu || menu->item_count == 0 || menu->selected_index >= menu->item_count) {
        return -1;
    }
    return menu->selected_index;
}

// --- Action Handling ---

void context_menu_execute_selected_action(ContextMenu* menu) {
    if (!menu || !menu->visible) return;
    
    int8_t index = context_menu_get_selected_index(menu);
    if (index < 0) return;
    
    ContextMenuItem* item = &menu->items[index];
    
    // Check if enabled
    if (!item->enabled || item->text == NULL) return;
    
    // Execute callback
    void (*callback)(Widget*, ContextActionType, uint16_t) = menu->on_action;
    if (!callback && default_on_action) {
        callback = default_on_action;
    }
    
    if (callback && menu->target_widget) {
        callback(menu->target_widget, item->action, item->custom_id);
    }
    
    // Hide menu after action
    context_menu_hide();
}

void context_menu_set_on_action(ContextMenu* menu, 
                                void (*on_action)(Widget*, ContextActionType, uint16_t)) {
    if (menu) {
        menu->on_action = on_action;
    }
}

void context_menu_set_default_on_action(void (*on_action)(Widget*, ContextActionType, uint16_t)) {
    default_on_action = on_action;
}

// --- Rendering ---

void context_menu_render(const ContextMenu* menu) {
    if (!menu || !menu->visible) return;
    
    // Draw border/background
    renderer_fill_rect(menu->x, menu->y, menu->width, menu->height, menu->bg_color);
    
    // Draw border
    if (menu->border_width > 0) {
        renderer_draw_rect(menu->x, menu->y, menu->width, menu->height, menu->border_color);
    }
    
    // Draw each item
    for (uint8_t i = 0; i < menu->item_count; i++) {
        uint16_t item_y = menu->y + i * menu->item_height;
        draw_menu_item(menu, i, menu->x, item_y, menu->width, menu->item_height);
        
        // Draw separator line between items (except last)
        if (i < menu->item_count - 1 && menu->items[i+1].text == NULL) {
            uint16_t separator_y = item_y + menu->item_height - 1;
            renderer_draw_hline(menu->x, separator_y, menu->x + menu->width - 1, menu->border_color);
        }
    }
}

void context_menu_render_current(void) {
    if (current_menu) {
        context_menu_render(current_menu);
    }
}

// --- Touch Handling ---

bool context_menu_handle_touch(uint16_t x, uint16_t y, bool pressed) {
    if (!current_menu || !current_menu->visible) {
        return false;
    }
    
    // Check if touch is inside menu
    bool inside = context_menu_contains_point(current_menu, x, y);
    
    if (pressed) {
        if (inside) {
            // Find which item was touched
            int8_t item_index = context_menu_get_item_at_point(current_menu, x, y);
            if (item_index >= 0) {
                context_menu_select_item(current_menu, item_index);
                // Don't execute yet - wait for release
            }
            return true;  // Event handled
        } else {
            // Touch outside menu - hide it
            context_menu_hide();
            return true;  // Event handled
        }
    } else {
        // Touch released
        if (inside) {
            int8_t item_index = context_menu_get_item_at_point(current_menu, x, y);
            if (item_index >= 0 && current_menu->items[item_index].text != NULL) {
                context_menu_execute_selected_action(current_menu);
            }
            return true;  // Event handled
        }
    }
    
    return false;  // Event not handled
}

bool context_menu_handle_gesture(GestureType gesture, uint16_t x, uint16_t y) {
    if (gesture == GESTURE_LONG_PRESS) {
        // Long press can trigger context menu
        // But only if we don't already have one showing
        if (!context_menu_is_visible()) {
            // Find widget at position (this would need widget hit testing)
            Widget* widget = widget_find_at_position(x, y);
            if (widget) {
                context_menu_show_at(x, y, widget);
                return true;
            }
        }
        return false;
    }
    
    if (context_menu_is_visible()) {
        // Handle gesture while menu is visible
        switch (gesture) {
            case GESTURE_SWIPE_UP:
                context_menu_prev_item(current_menu);
                return true;
            case GESTURE_SWIPE_DOWN:
                context_menu_next_item(current_menu);
                return true;
            case GESTURE_TAP:
                // Tap on menu - this is handled by touch handling
                break;
            default:
                // Other gestures hide the menu
                context_menu_hide();
                return true;
        }
    }
    
    return false;
}

// --- Hit Testing ---

bool context_menu_contains_point(const ContextMenu* menu, uint16_t x, uint16_t y) {
    if (!menu || !menu->visible) return false;
    
    return (x >= menu->x && x < menu->x + menu->width &&
            y >= menu->y && y < menu->y + menu->height);
}

int8_t context_menu_get_item_at_point(const ContextMenu* menu, uint16_t x, uint16_t y) {
    if (!menu || !menu->visible) return -1;
    
    // Check if point is inside menu
    if (!context_menu_contains_point(menu, x, y)) {
        return -1;
    }
    
    // Calculate relative Y
    uint16_t relative_y = y - menu->y;
    uint8_t item_index = relative_y / menu->item_height;
    
    if (item_index < menu->item_count) {
        return item_index;
    }
    
    return -1;
}

// --- Getters ---

ContextMenu* context_menu_get_current(void) {
    return current_menu;
}

Widget* context_menu_get_target_widget(void) {
    if (current_menu) {
        return current_menu->target_widget;
    }
    return NULL;
}

// --- Utility ---

uint16_t context_menu_calculate_width(const ContextMenu* menu) {
    if (!menu || menu->item_count == 0) {
        return CONTEXT_MENU_MIN_WIDTH;
    }
    
    uint16_t max_width = 0;
    uint8_t font_width = menu->base.font_width > 0 ? menu->base.font_width : CONTEXT_MENU_FONT_WIDTH;
    
    for (uint8_t i = 0; i < menu->item_count; i++) {
        if (menu->items[i].text != NULL) {
            uint16_t width = calculate_text_width(menu->items[i].text, font_width);
            // Add padding (16px total: 8px left + 8px right)
            width += 16;
            if (width > max_width) {
                max_width = width;
            }
        }
    }
    
    // Ensure minimum width
    if (max_width < CONTEXT_MENU_MIN_WIDTH) {
        max_width = CONTEXT_MENU_MIN_WIDTH;
    }
    
    return max_width;
}

void context_menu_adjust_position(ContextMenu* menu, uint16_t screen_width, uint16_t screen_height) {
    if (!menu) return;
    
    // Adjust X to fit on screen
    if (menu->x + menu->width > screen_width) {
        menu->x = screen_width - menu->width;
        if (menu->x < 0) menu->x = 0;
    }
    
    // Adjust Y to fit on screen
    if (menu->y + menu->height > screen_height) {
        menu->y = screen_height - menu->height;
        if (menu->y < 0) menu->y = 0;
    }
}

// =============================================================================
// DEFAULT TEXT EDITOR MENU ITEMS
// =============================================================================

// Default context menu items for text editor
ContextMenuItem text_editor_context_menu_items[] = {
    { "Copy",    CONTEXT_ACTION_COPY,     0, true, 0 },
    { "Paste",   CONTEXT_ACTION_PASTE,    0, true, 0 },
    { "Cut",     CONTEXT_ACTION_CUT,      0, true, 0 },
    { NULL,      CONTEXT_ACTION_NONE,     0, false, 0 },  // Separator
    { "Select All", CONTEXT_ACTION_SELECT_ALL, 0, true, 0 },
    { NULL,      CONTEXT_ACTION_NONE,     0, false, 0 }   // Separator
    { "Undo",    CONTEXT_ACTION_UNDO,     0, true, 0 },
    { "Redo",    CONTEXT_ACTION_REDO,     0, true, 0 }
};

// Default context menu items for textfield (TEXT_INPUT)
ContextMenuItem textfield_context_menu_items[] = {
    { "Copy",    CONTEXT_ACTION_COPY,     0, true, 0 },
    { "Paste",   CONTEXT_ACTION_PASTE,    0, true, 0 },
    { "Cut",     CONTEXT_ACTION_CUT,      0, true, 0 },
    { NULL,      CONTEXT_ACTION_NONE,     0, false, 0 },  // Separator
    { "Select All", CONTEXT_ACTION_SELECT_ALL, 0, true, 0 }
};

// =============================================================================
// INITIALIZATION WITH DEFAULT MENU
// =============================================================================

/**
 * @brief Initialize context menu system with default configurations
 * 
 * Registers default context menus for built-in widget types.
 * 
 * Note: TextEditor extends TextField, so both have context menus.
 * TextField (TEXT_INPUT) has basic editing, TextEditor has full editing with undo/redo.
 */
void context_menu_init_with_defaults(void) {
    context_menu_init();
    
    // Register textfield context menu (base text input)
    context_menu_register_widget_type(
        WIDGET_TYPE_TEXT_INPUT,
        textfield_context_menu_items,
        sizeof(textfield_context_menu_items) / sizeof(textfield_context_menu_items[0]),
        NULL  // Use default callback
    );
    
    // Register text editor context menu (extends textfield with undo/redo)
    context_menu_register_widget_type(
        WIDGET_TYPE_TEXT_EDITOR,
        text_editor_context_menu_items,
        sizeof(text_editor_context_menu_items) / sizeof(text_editor_context_menu_items[0]),
        NULL  // Use default callback
    );
}
