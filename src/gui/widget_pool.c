/**
 * widget_pool.c - Object Pooling Implementation
 * 
 * Memory-safe widget allocation using pre-allocated pools
 * Avoids malloc/free overhead and fragmentation on ESP8266
 * 
 * Generated for ESP8266 by Mistral Vibe
 */

#include "widget_pool.h"
#include <string.h>


// ============================================================================
// GLOBAL POOLS
// ============================================================================

// Define the widget pools
PooledWidget widget_pool[MAX_WIDGETS];
PooledButton button_pool[MAX_BUTTONS];
PooledLabel label_pool[MAX_LABELS];
PooledSlider slider_pool[MAX_SLIDERS];
PooledView view_pool[MAX_VIEWS];


// ============================================================================
// POOL INITIALIZATION
// ============================================================================

/**
 * Initialize all widget pools
 * 
 * Call this once at application startup before creating any widgets
 */
void pools_init(void) {
    // Initialize generic widget pool
    for (int i = 0; i < MAX_WIDGETS; i++) {
        widget_pool[i].in_use = false;
    }

    // Initialize button pool
    for (int i = 0; i < MAX_BUTTONS; i++) {
        button_pool[i].in_use = false;
    }

    // Initialize label pool
    for (int i = 0; i < MAX_LABELS; i++) {
        label_pool[i].in_use = false;
    }

    // Initialize slider pool
    for (int i = 0; i < MAX_SLIDERS; i++) {
        slider_pool[i].in_use = false;
    }

    // Initialize view pool
    for (int i = 0; i < MAX_VIEWS; i++) {
        view_pool[i].in_use = false;
    }
}


// ============================================================================
// POOL ALLOCATION
// ============================================================================

/**
 * Allocate a widget from the appropriate pool based on type
 * 
 * @param type Widget type to allocate
 * @return Pointer to widget, or NULL if pool exhausted
 */
Widget* widget_alloc(WIDGET_TYPE type) {
    switch (type) {
        case WIDGET_TYPE_VIEW:
            return (Widget*)view_alloc();

        case WIDGET_TYPE_BUTTON:
            return (Widget*)button_alloc();

        case WIDGET_TYPE_LABEL:
            return (Widget*)label_alloc();

        case WIDGET_TYPE_SLIDER:
            return (Widget*)slider_alloc();

        default:
            // Try generic pool for unknown types
            for (int i = 0; i < MAX_WIDGETS; i++) {
                if (!widget_pool[i].in_use) {
                    widget_pool[i].in_use = true;
                    memset(&widget_pool[i].base, 0, sizeof(Widget));
                    widget_pool[i].base.type = type;
                    return &widget_pool[i].base;
                }
            }
            return NULL;
    }
}

/**
 * Allocate a button from the button pool
 * 
 * @return Pointer to button, or NULL if pool exhausted
 */
WidgetButton* button_alloc(void) {
    for (int i = 0; i < MAX_BUTTONS; i++) {
        if (!button_pool[i].in_use) {
            button_pool[i].in_use = true;
            memset(&button_pool[i].base, 0, sizeof(WidgetButton));
            button_pool[i].base.base.type = WIDGET_TYPE_BUTTON;
            return &button_pool[i].base;
        }
    }
    return NULL;
}

/**
 * Allocate a label from the label pool
 * 
 * @return Pointer to label, or NULL if pool exhausted
 */
WidgetLabel* label_alloc(void) {
    for (int i = 0; i < MAX_LABELS; i++) {
        if (!label_pool[i].in_use) {
            label_pool[i].in_use = true;
            memset(&label_pool[i].base, 0, sizeof(WidgetLabel));
            label_pool[i].base.base.type = WIDGET_TYPE_LABEL;
            return &label_pool[i].base;
        }
    }
    return NULL;
}

/**
 * Allocate a slider from the slider pool
 * 
 * @return Pointer to slider, or NULL if pool exhausted
 */
WidgetSlider* slider_alloc(void) {
    for (int i = 0; i < MAX_SLIDERS; i++) {
        if (!slider_pool[i].in_use) {
            slider_pool[i].in_use = true;
            memset(&slider_pool[i].base, 0, sizeof(WidgetSlider));
            slider_pool[i].base.base.type = WIDGET_TYPE_SLIDER;
            // Initialize default slider values
            slider_pool[i].base.min_value = 0;
            slider_pool[i].base.max_value = 100;
            slider_pool[i].base.value = 50;
            slider_pool[i].base.vertical = false;
            return &slider_pool[i].base;
        }
    }
    return NULL;
}

/**
 * Allocate a view from the view pool
 * 
 * @return Pointer to view, or NULL if pool exhausted
 */
WidgetView* view_alloc(void) {
    for (int i = 0; i < MAX_VIEWS; i++) {
        if (!view_pool[i].in_use) {
            view_pool[i].in_use = true;
            memset(&view_pool[i].base, 0, sizeof(WidgetView));
            view_pool[i].base.base.type = WIDGET_TYPE_VIEW;
            return &view_pool[i].base;
        }
    }
    return NULL;
}


// ============================================================================
// POOL RELEASE
// ============================================================================

/**
 * Release a widget back to its pool
 * 
 * @param w Pointer to widget to release
 */
void widget_free(Widget* w) {
    if (!w) return;

    switch (w->type) {
        case WIDGET_TYPE_VIEW:
            view_free((WidgetView*)w);
            break;

        case WIDGET_TYPE_BUTTON:
            button_free((WidgetButton*)w);
            break;

        case WIDGET_TYPE_LABEL:
            label_free((WidgetLabel*)w);
            break;

        case WIDGET_TYPE_SLIDER:
            slider_free((WidgetSlider*)w);
            break;

        default:
            // Try to find in generic pool
            for (int i = 0; i < MAX_WIDGETS; i++) {
                if (&widget_pool[i].base == w) {
                    widget_pool[i].in_use = false;
                    return;
                }
            }
            break;
    }
}

/**
 * Release a button back to the button pool
 * 
 * @param btn Pointer to button to release
 */
void button_free(WidgetButton* btn) {
    if (!btn) return;

    // Check if it's from the button pool
    for (int i = 0; i < MAX_BUTTONS; i++) {
        if (&button_pool[i].base.base == &btn->base) {
            button_pool[i].in_use = false;
            return;
        }
    }

    // Not from pool, try generic widget pool
    widget_free(&btn->base);
}

/**
 * Release a label back to the label pool
 * 
 * @param lbl Pointer to label to release
 */
void label_free(WidgetLabel* lbl) {
    if (!lbl) return;

    for (int i = 0; i < MAX_LABELS; i++) {
        if (&label_pool[i].base.base == &lbl->base) {
            label_pool[i].in_use = false;
            return;
        }
    }

    widget_free(&lbl->base);
}

/**
 * Release a slider back to the slider pool
 * 
 * @param slider Pointer to slider to release
 */
void slider_free(WidgetSlider* slider) {
    if (!slider) return;

    for (int i = 0; i < MAX_SLIDERS; i++) {
        if (&slider_pool[i].base.base == &slider->base) {
            slider_pool[i].in_use = false;
            return;
        }
    }

    widget_free(&slider->base);
}

/**
 * Release a view back to the view pool
 * 
 * @param view Pointer to view to release
 */
void view_free(WidgetView* view) {
    if (!view) return;

    for (int i = 0; i < MAX_VIEWS; i++) {
        if (&view_pool[i].base.base == &view->base) {
            view_pool[i].in_use = false;
            return;
        }
    }

    widget_free(&view->base);
}


// ============================================================================
// POOL STATISTICS
// ============================================================================

/**
 * Get number of allocated widgets of a specific type
 * 
 * @param type Widget type
 * @return Number of widgets currently allocated
 */
uint8_t pool_get_allocated_count(WIDGET_TYPE type) {
    switch (type) {
        case WIDGET_TYPE_VIEW:
            {
                uint8_t count = 0;
                for (int i = 0; i < MAX_VIEWS; i++) {
                    if (view_pool[i].in_use) count++;
                }
                return count;
            }

        case WIDGET_TYPE_BUTTON:
            {
                uint8_t count = 0;
                for (int i = 0; i < MAX_BUTTONS; i++) {
                    if (button_pool[i].in_use) count++;
                }
                return count;
            }

        case WIDGET_TYPE_LABEL:
            {
                uint8_t count = 0;
                for (int i = 0; i < MAX_LABELS; i++) {
                    if (label_pool[i].in_use) count++;
                }
                return count;
            }

        case WIDGET_TYPE_SLIDER:
            {
                uint8_t count = 0;
                for (int i = 0; i < MAX_SLIDERS; i++) {
                    if (slider_pool[i].in_use) count++;
                }
                return count;
            }

        default:
            return 0;
    }
}

/**
 * Get total capacity for a widget type
 * 
 * @param type Widget type
 * @return Total capacity for that widget type
 */
uint8_t pool_get_capacity(WIDGET_TYPE type) {
    switch (type) {
        case WIDGET_TYPE_VIEW:
            return MAX_VIEWS;

        case WIDGET_TYPE_BUTTON:
            return MAX_BUTTONS;

        case WIDGET_TYPE_LABEL:
            return MAX_LABELS;

        case WIDGET_TYPE_SLIDER:
            return MAX_SLIDERS;

        default:
            return MAX_WIDGETS;
    }
}

/**
 * Check if a widget is from a pool
 * 
 * @param w Widget pointer
 * @return true if widget is from a pool
 */
bool widget_is_pooled(const Widget* w) {
    if (!w) return false;

    // Check all pools
    for (int i = 0; i < MAX_WIDGETS; i++) {
        if (&widget_pool[i].base == w) return true;
    }

    for (int i = 0; i < MAX_BUTTONS; i++) {
        if ((Widget*)&button_pool[i].base.base == w) return true;
    }

    for (int i = 0; i < MAX_LABELS; i++) {
        if ((Widget*)&label_pool[i].base.base == w) return true;
    }

    for (int i = 0; i < MAX_SLIDERS; i++) {
        if ((Widget*)&slider_pool[i].base.base == w) return true;
    }

    for (int i = 0; i < MAX_VIEWS; i++) {
        if ((Widget*)&view_pool[i].base.base == w) return true;
    }

    return false;
}


// ============================================================================
// POOL DEBUG FUNCTIONS
// ============================================================================

#ifdef DEBUG_POOLS

#include <stdio.h>

/**
 * Print pool statistics for debugging
 */
void pool_print_stats(void) {
    printf("Pool Statistics:\n");
    printf("  Views: %d/%d\n", pool_get_allocated_count(WIDGET_TYPE_VIEW), pool_get_capacity(WIDGET_TYPE_VIEW));
    printf("  Buttons: %d/%d\n", pool_get_allocated_count(WIDGET_TYPE_BUTTON), pool_get_capacity(WIDGET_TYPE_BUTTON));
    printf("  Labels: %d/%d\n", pool_get_allocated_count(WIDGET_TYPE_LABEL), pool_get_capacity(WIDGET_TYPE_LABEL));
    printf("  Sliders: %d/%d\n", pool_get_allocated_count(WIDGET_TYPE_SLIDER), pool_get_capacity(WIDGET_TYPE_SLIDER));
    printf("  Generic: %d/%d\n", pool_get_allocated_count(WIDGET_TYPE_COUNT), MAX_WIDGETS);
}

/**
 * Check if any widgets are leaked (not returned to pool)
 * 
 * @return true if all widgets are properly released
 */
bool pool_check_for_leaks(void) {
    if (pool_get_allocated_count(WIDGET_TYPE_VIEW) > 0) return false;
    if (pool_get_allocated_count(WIDGET_TYPE_BUTTON) > 0) return false;
    if (pool_get_allocated_count(WIDGET_TYPE_LABEL) > 0) return false;
    if (pool_get_allocated_count(WIDGET_TYPE_SLIDER) > 0) return false;
    return true;
}

#endif // DEBUG_POOLS
