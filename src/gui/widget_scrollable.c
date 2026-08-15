/**
 * @file widget_scrollable.c
 * @brief Scrollable widget support implementation with union-based memory optimization
 * @author GUIKit for ESP8266
 * @date 2026
 */

#include "widget_scrollable.h"
#include "widget.h"
#include <string.h>

// =============================================================================
// INLINE FUNCTION IMPLEMENTATIONS
// =============================================================================

// Get current horizontal scroll position
static inline int16_t widget_getScrollX(const Widget* widget) {
    if (!IS_SCROLLABLE_X(widget)) return 0;
    return IS_SCROLLABLE_Y(widget) ?
        widget->scroll.scroll_data.both_data.x :
        widget->scroll.scroll_data.x_data.x;
}

// Get current vertical scroll position
static inline int16_t widget_getScrollY(const Widget* widget) {
    if (!IS_SCROLLABLE_Y(widget)) return 0;
    return IS_SCROLLABLE_X(widget) ?
        widget->scroll.scroll_data.both_data.y :
        widget->scroll.scroll_data.y_data.y;
}

// Get scrollable content width
static inline uint16_t widget_getContentWidth(const Widget* widget) {
    if (!IS_SCROLLABLE_X(widget)) return widget->size.width;
    return IS_SCROLLABLE_Y(widget) ?
        widget->scroll.scroll_data.both_data.content_width :
        widget->scroll.scroll_data.x_data.content_width;
}

// Get scrollable content height
static inline uint16_t widget_getContentHeight(const Widget* widget) {
    if (!IS_SCROLLABLE_Y(widget)) return widget->size.height;
    return IS_SCROLLABLE_X(widget) ?
        widget->scroll.scroll_data.both_data.content_height :
        widget->scroll.scroll_data.y_data.content_height;
}

// Get maximum horizontal scroll position
static inline int16_t widget_getMaxScrollX(const Widget* widget) {
    if (!IS_SCROLLABLE_X(widget)) return 0;
    return IS_SCROLLABLE_Y(widget) ?
        widget->scroll.scroll_data.both_data.max_x :
        widget->scroll.scroll_data.x_data.max_x;
}

// Get maximum vertical scroll position
static inline int16_t widget_getMaxScrollY(const Widget* widget) {
    if (!IS_SCROLLABLE_Y(widget)) return 0;
    return IS_SCROLLABLE_X(widget) ?
        widget->scroll.scroll_data.both_data.max_y :
        widget->scroll.scroll_data.y_data.max_y;
}

// Check if at scroll top
static inline bool widget_isAtScrollTop(const Widget* widget) {
    return widget_getScrollY(widget) <= 0;
}

// Check if at scroll bottom
static inline bool widget_isAtScrollBottom(const Widget* widget) {
    return widget_getScrollY(widget) >= widget_getMaxScrollY(widget);
}

// Check if at scroll left
static inline bool widget_isAtScrollLeft(const Widget* widget) {
    return widget_getScrollX(widget) <= 0;
}

// Check if at scroll right
static inline bool widget_isAtScrollRight(const Widget* widget) {
    return widget_getScrollX(widget) >= widget_getMaxScrollX(widget);
}

// Enable/disable horizontal scrolling
static inline void widget_enableScrollX(Widget* widget, bool enable) {
    uint8_t new_flags = widget->scroll.scrollable_flags;
    if (enable) {
        new_flags |= SCROLLABLE_X;
    } else {
        new_flags &= ~SCROLLABLE_X;
    }
    widget_setScrollable(widget, new_flags);
}

// Enable/disable vertical scrolling
static inline void widget_enableScrollY(Widget* widget, bool enable) {
    uint8_t new_flags = widget->scroll.scrollable_flags;
    if (enable) {
        new_flags |= SCROLLABLE_Y;
    } else {
        new_flags &= ~SCROLLABLE_Y;
    }
    widget_setScrollable(widget, new_flags);
}

// Scroll to top-left corner
static inline void widget_scrollToTopLeft(Widget* widget) {
    widget_setScroll(widget, 0, 0);
}

// =============================================================================
// FUNCTION IMPLEMENTATIONS
// =============================================================================

// Set scrollable flags for a widget
void widget_setScrollable(Widget* widget, uint8_t flags) {
    uint8_t old_flags = widget->scroll.scrollable_flags;
    
    // Only update if flags actually change
    flags = flags & (SCROLLABLE_X | SCROLLABLE_Y);  // Mask to valid flags
    if (flags == old_flags) {
        return;
    }

    widget->scroll.scrollable_flags = flags;

    // Clear existing data
    memset(&widget->scroll.scroll_data, 0, sizeof(widget->scroll.scroll_data));

    // Initialize based on new flags
    switch (widget->scroll.scrollable_flags) {
        case SCROLLABLE_NONE:
            widget->scroll.scroll_data.none._padding = 0;
            break;

        case SCROLLABLE_X:
            widget->scroll.scroll_data.x_data.x = 0;
            widget->scroll.scroll_data.x_data.content_width = widget->size.width;
            widget->scroll.scroll_data.x_data.max_x = 0;
            break;

        case SCROLLABLE_Y:
            widget->scroll.scroll_data.y_data.y = 0;
            widget->scroll.scroll_data.y_data.content_height = widget->size.height;
            widget->scroll.scroll_data.y_data.max_y = 0;
            break;

        case SCROLLABLE_BOTH:
            widget->scroll.scroll_data.both_data.x = 0;
            widget->scroll.scroll_data.both_data.y = 0;
            widget->scroll.scroll_data.both_data.content_width = widget->size.width;
            widget->scroll.scroll_data.both_data.content_height = widget->size.height;
            widget->scroll.scroll_data.both_data.max_x = 0;
            widget->scroll.scroll_data.both_data.max_y = 0;
            break;
    }

    // Update bounds
    widget_updateScrollBounds(widget);
}

// Set scroll position (absolute, clamped to bounds)
void widget_setScroll(Widget* widget, int16_t x, int16_t y) {
    bool changed = false;

    if (IS_SCROLLABLE_X(widget)) {
        int16_t max_x = widget_getMaxScrollX(widget);
        int16_t new_x = x;
        
        // Clamp to bounds
        if (new_x < 0) new_x = 0;
        if (new_x > max_x) new_x = max_x;

        if (IS_SCROLLABLE_Y(widget)) {
            if (widget->scroll.scroll_data.both_data.x != new_x) {
                widget->scroll.scroll_data.both_data.x = new_x;
                changed = true;
            }
        } else {
            if (widget->scroll.scroll_data.x_data.x != new_x) {
                widget->scroll.scroll_data.x_data.x = new_x;
                changed = true;
            }
        }
    }

    if (IS_SCROLLABLE_Y(widget)) {
        int16_t max_y = widget_getMaxScrollY(widget);
        int16_t new_y = y;
        
        // Clamp to bounds
        if (new_y < 0) new_y = 0;
        if (new_y > max_y) new_y = max_y;

        if (IS_SCROLLABLE_X(widget)) {
            if (widget->scroll.scroll_data.both_data.y != new_y) {
                widget->scroll.scroll_data.both_data.y = new_y;
                changed = true;
            }
        } else {
            if (widget->scroll.scroll_data.y_data.y != new_y) {
                widget->scroll.scroll_data.y_data.y = new_y;
                changed = true;
            }
        }
    }

    if (changed) {
        widget->dirty = true;
    }
}

// Scroll by relative amount
void widget_scrollBy(Widget* widget, int16_t dx, int16_t dy) {
    int16_t x = widget_getScrollX(widget);
    int16_t y = widget_getScrollY(widget);
    widget_setScroll(widget, x + dx, y + dy);
}

// Set scrollable content size
void widget_setContentSize(Widget* widget, uint16_t width, uint16_t height) {
    bool changed = false;

    if (IS_SCROLLABLE_X(widget)) {
        if (IS_SCROLLABLE_Y(widget)) {
            if (widget->scroll.scroll_data.both_data.content_width != width) {
                widget->scroll.scroll_data.both_data.content_width = width;
                changed = true;
            }
        } else {
            if (widget->scroll.scroll_data.x_data.content_width != width) {
                widget->scroll.scroll_data.x_data.content_width = width;
                changed = true;
            }
        }
    }

    if (IS_SCROLLABLE_Y(widget)) {
        if (IS_SCROLLABLE_X(widget)) {
            if (widget->scroll.scroll_data.both_data.content_height != height) {
                widget->scroll.scroll_data.both_data.content_height = height;
                changed = true;
            }
        } else {
            if (widget->scroll.scroll_data.y_data.content_height != height) {
                widget->scroll.scroll_data.y_data.content_height = height;
                changed = true;
            }
        }
    }

    if (changed) {
        widget_updateScrollBounds(widget);
        widget->dirty = true;
    }
}

// Update scroll bounds based on widget size and content size
void widget_updateScrollBounds(Widget* widget) {
    if (IS_SCROLLABLE_X(widget)) {
        int16_t content_w = widget_getContentWidth(widget);
        int16_t max_x = content_w - (int16_t)widget->size.width;
        
        if (max_x < 0) max_x = 0;  // Can't scroll if content smaller than widget

        if (IS_SCROLLABLE_Y(widget)) {
            widget->scroll.scroll_data.both_data.max_x = max_x;
        } else {
            widget->scroll.scroll_data.x_data.max_x = max_x;
        }

        // Clamp current x
        int16_t current_x = widget_getScrollX(widget);
        if (current_x > max_x) {
            widget_setScroll(widget, max_x, widget_getScrollY(widget));
        }
    }

    if (IS_SCROLLABLE_Y(widget)) {
        int16_t content_h = widget_getContentHeight(widget);
        int16_t max_y = content_h - (int16_t)widget->size.height;
        
        if (max_y < 0) max_y = 0;  // Can't scroll if content smaller than widget

        if (IS_SCROLLABLE_X(widget)) {
            widget->scroll.scroll_data.both_data.max_y = max_y;
        } else {
            widget->scroll.scroll_data.y_data.max_y = max_y;
        }

        // Clamp current y
        int16_t current_y = widget_getScrollY(widget);
        if (current_y > max_y) {
            widget_setScroll(widget, widget_getScrollX(widget), max_y);
        }
    }
}

// Scroll to make a specific point visible
void widget_scrollToMakeVisible(Widget* widget, uint16_t point_x, uint16_t point_y) {
    if (!IS_SCROLLABLE(widget)) return;

    int16_t new_x = widget_getScrollX(widget);
    int16_t new_y = widget_getScrollY(widget);

    if (IS_SCROLLABLE_X(widget)) {
        uint16_t widget_right = widget->position.x + widget->size.width;
        uint16_t content_right = widget->position.x + widget_getContentWidth(widget);
        
        if (point_x < widget->position.x + new_x) {
            // Point is to the left of visible area
            new_x = point_x - widget->position.x;
        } else if (point_x > widget->position.x + new_x + widget->size.width) {
            // Point is to the right of visible area
            new_x = point_x - widget->position.x - widget->size.width;
        }
        
        // Clamp
        int16_t max_x = widget_getMaxScrollX(widget);
        if (new_x > max_x) new_x = max_x;
        if (new_x < 0) new_x = 0;
    }

    if (IS_SCROLLABLE_Y(widget)) {
        uint16_t widget_bottom = widget->position.y + widget->size.height;
        uint16_t content_bottom = widget->position.y + widget_getContentHeight(widget);
        
        if (point_y < widget->position.y + new_y) {
            // Point is above visible area
            new_y = point_y - widget->position.y;
        } else if (point_y > widget->position.y + new_y + widget->size.height) {
            // Point is below visible area
            new_y = point_y - widget->position.y - widget->size.height;
        }
        
        // Clamp
        int16_t max_y = widget_getMaxScrollY(widget);
        if (new_y > max_y) new_y = max_y;
        if (new_y < 0) new_y = 0;
    }

    widget_setScroll(widget, new_x, new_y);
}
