/**
 * @file widget_scrollable.h
 * @brief Scrollable widget support with union-based memory optimization
 * @author GUIKit for ESP8266
 * @date 2026
 * 
 * This module provides scrollable functionality for widgets using a union-based
 * memory optimization. Only the data needed for the active scroll directions
 * is stored, saving memory on ESP8266 (80KB RAM limit).
 * 
 * Memory usage:
 * - SCROLLABLE_NONE: 1 byte
 * - SCROLLABLE_X: 8 bytes
 * - SCROLLABLE_Y: 8 bytes
 * - SCROLLABLE_BOTH: 14 bytes
 * 
 * Usage:
 * @code
 * Widget* view = new_widget(WIDGET_TYPE_VIEW);
 * widget_setScrollable(view, SCROLLABLE_BOTH);
 * widget_setContentSize(view, 500, 800);
 * widget_setScroll(view, 0, 100);
 * @endcode
 */

#ifndef WIDGET_SCROLLABLE_H
#define WIDGET_SCROLLABLE_H

#include <stdint.h>
#include <stdbool.h>

// Forward declaration
typedef struct Widget Widget;

/**
 * @brief Scrollable direction flags for widgets
 * @note Bitmask flags - can be combined with | (bitwise OR)
 */
typedef enum {
    SCROLLABLE_NONE    = 0,        ///< Not scrollable (default)
    SCROLLABLE_X       = 1 << 0,  ///< Scrollable horizontally
    SCROLLABLE_Y       = 1 << 1,  ///< Scrollable vertically
    SCROLLABLE_BOTH    = SCROLLABLE_X | SCROLLABLE_Y  ///< Scrollable in both directions
} WIDGET_SCROLLABLE_FLAGS;

/**
 * @brief Optimized scrollable data using tagged union
 * @note Memory usage varies by scrollable_flags:
 *       - SCROLLABLE_NONE: 1 byte
 *       - SCROLLABLE_X: 8 bytes
 *       - SCROLLABLE_Y: 8 bytes
 *       - SCROLLABLE_BOTH: 14 bytes
 */
typedef struct Scrollable {
    uint8_t scrollable_flags;  ///< Bitmask of SCROLLABLE_* flags

    /**
     * @brief Scroll data union - shares memory based on scrollable_flags
     * @note DO NOT access directly. Use accessor functions/macros instead.
     */
    union {
        // Not scrollable - minimal storage (1 byte total with flags)
        struct {
            uint8_t _padding;  ///< Padding to avoid zero-size struct
        } none;

        // Horizontal scroll only (8 bytes)
        struct {
            int16_t x;               ///< Current horizontal scroll offset (pixels)
            int16_t content_width;   ///< Total content width (can exceed widget width)
            int16_t max_x;           ///< Maximum X scroll (content_width - widget_width)
        } x_data;

        // Vertical scroll only (8 bytes)
        struct {
            int16_t y;               ///< Current vertical scroll offset (pixels)
            int16_t content_height;  ///< Total content height (can exceed widget height)
            int16_t max_y;           ///< Maximum Y scroll (content_height - widget_height)
        } y_data;

        // Both directions (14 bytes)
        struct {
            int16_t x;               ///< Current horizontal scroll offset (pixels)
            int16_t y;               ///< Current vertical scroll offset (pixels)
            uint16_t content_width;   ///< Total content width (can exceed widget width)
            uint16_t content_height;  ///< Total content height (can exceed widget height)
            int16_t max_x;           ///< Maximum X scroll (content_width - widget_width)
            int16_t max_y;           ///< Maximum Y scroll (content_height - widget_height)
        } both_data;
    } scroll_data;
} Scrollable;

// =============================================================================
// ACCESSOR MACROS (Type-safe, check flags before accessing union members)
// =============================================================================

/**
 * @brief Get scrollable flags
 * @param w Widget pointer
 * @return WIDGET_SCROLLABLE_FLAGS bitmask
 */
#define SCROLLABLE_FLAGS(w)       ((w)->scroll.scrollable_flags)

/**
 * @brief Check if widget is scrollable in any direction
 * @param w Widget pointer
 * @return true if scrollable
 */
#define IS_SCROLLABLE(w)          (SCROLLABLE_FLAGS(w) != SCROLLABLE_NONE)

/**
 * @brief Check if widget is scrollable horizontally
 * @param w Widget pointer
 * @return true if scrollable in X direction
 */
#define IS_SCROLLABLE_X(w)        ((SCROLLABLE_FLAGS(w) & SCROLLABLE_X) != 0)

/**
 * @brief Check if widget is scrollable vertically
 * @param w Widget pointer
 * @return true if scrollable in Y direction
 */
#define IS_SCROLLABLE_Y(w)        ((SCROLLABLE_FLAGS(w) & SCROLLABLE_Y) != 0)

// =============================================================================
// INLINE ACCESSOR FUNCTIONS (Optimized for performance)
// =============================================================================

/**
 * @brief Get current horizontal scroll position
 * @param widget Widget pointer
 * @return Current X scroll offset (0 if not scrollable in X)
 */
static inline int16_t widget_getScrollX(const Widget* widget);

/**
 * @brief Get current vertical scroll position
 * @param widget Widget pointer
 * @return Current Y scroll offset (0 if not scrollable in Y)
 */
static inline int16_t widget_getScrollY(const Widget* widget);

/**
 * @brief Get scrollable content width
 * @param widget Widget pointer
 * @return Content width (widget width if not scrollable in X)
 */
static inline uint16_t widget_getContentWidth(const Widget* widget);

/**
 * @brief Get scrollable content height
 * @param widget Widget pointer
 * @return Content height (widget height if not scrollable in Y)
 */
static inline uint16_t widget_getContentHeight(const Widget* widget);

/**
 * @brief Get maximum horizontal scroll position
 * @param widget Widget pointer
 * @return Maximum X scroll (0 if not scrollable in X)
 */
static inline int16_t widget_getMaxScrollX(const Widget* widget);

/**
 * @brief Get maximum vertical scroll position
 * @param widget Widget pointer
 * @return Maximum Y scroll (0 if not scrollable in Y)
 */
static inline int16_t widget_getMaxScrollY(const Widget* widget);

/**
 * @brief Check if widget is scrolled to the top
 * @param widget Widget pointer
 * @return true if at top (scroll Y = 0)
 */
static inline bool widget_isAtScrollTop(const Widget* widget);

/**
 * @brief Check if widget is scrolled to the bottom
 * @param widget Widget pointer
 * @return true if at bottom (scroll Y = max Y)
 */
static inline bool widget_isAtScrollBottom(const Widget* widget);

/**
 * @brief Check if widget is scrolled to the left
 * @param widget Widget pointer
 * @return true if at left (scroll X = 0)
 */
static inline bool widget_isAtScrollLeft(const Widget* widget);

/**
 * @brief Check if widget is scrolled to the right
 * @param widget Widget pointer
 * @return true if at right (scroll X = max X)
 */
static inline bool widget_isAtScrollRight(const Widget* widget);

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

/**
 * @brief Set scrollable flags for a widget
 * @param widget Widget pointer
 * @param flags SCROLLABLE_* bitmask flags
 * @note This initializes the appropriate union member based on flags
 */
void widget_setScrollable(Widget* widget, uint8_t flags);

/**
 * @brief Enable/disable horizontal scrolling
 * @param widget Widget pointer
 * @param enable true to enable, false to disable
 */
static inline void widget_enableScrollX(Widget* widget, bool enable);

/**
 * @brief Enable/disable vertical scrolling
 * @param widget Widget pointer
 * @param enable true to enable, false to disable
 */
static inline void widget_enableScrollY(Widget* widget, bool enable);

/**
 * @brief Set scroll position (absolute, clamped to bounds)
 * @param widget Widget pointer
 * @param x Horizontal scroll offset
 * @param y Vertical scroll offset
 */
void widget_setScroll(Widget* widget, int16_t x, int16_t y);

/**
 * @brief Scroll by relative amount
 * @param widget Widget pointer
 * @param dx Horizontal delta (positive = scroll right)
 * @param dy Vertical delta (positive = scroll down)
 */
void widget_scrollBy(Widget* widget, int16_t dx, int16_t dy);

/**
 * @brief Set scrollable content size
 * @param widget Widget pointer
 * @param width Total content width
 * @param height Total content height
 */
void widget_setContentSize(Widget* widget, uint16_t width, uint16_t height);

/**
 * @brief Update scroll bounds based on widget size and content size
 * @param widget Widget pointer
 * @note Called automatically when scrollability or content size changes
 */
void widget_updateScrollBounds(Widget* widget);

/**
 * @brief Scroll to make a specific point visible
 * @param widget Widget pointer
 * @param point_x X coordinate of point to make visible
 * @param point_y Y coordinate of point to make visible
 */
void widget_scrollToMakeVisible(Widget* widget, uint16_t point_x, uint16_t point_y);

/**
 * @brief Scroll to top-left corner
 * @param widget Widget pointer
 */
static inline void widget_scrollToTopLeft(Widget* widget);

// =============================================================================
// MACRO-BASED SYNTAX (Objective-C style, for convenience)
// =============================================================================

// Flag macros
#define SET_SCROLLABLE(w, f)      widget_setScrollable(w, f)
#define ENABLE_SCROLL_X(w)         widget_enableScrollX(w, true)
#define DISABLE_SCROLL_X(w)        widget_enableScrollX(w, false)
#define ENABLE_SCROLL_Y(w)         widget_enableScrollY(w, true)
#define DISABLE_SCROLL_Y(w)        widget_enableScrollY(w, false)

// Position macros
#define GET_SCROLL_X(w)           widget_getScrollX(w)
#define GET_SCROLL_Y(w)           widget_getScrollY(w)
#define SET_SCROLL(w, x, y)       widget_setScroll(w, x, y)
#define SCROLL_BY(w, dx, dy)      widget_scrollBy(w, dx, dy)

// Content size macros
#define GET_CONTENT_W(w)          widget_getContentWidth(w)
#define GET_CONTENT_H(w)          widget_getContentHeight(w)
#define SET_CONTENT_SIZE(w, wd, ht) widget_setContentSize(w, wd, ht)

// Boundary check macros
#define IS_AT_SCROLL_TOP(w)       widget_isAtScrollTop(w)
#define IS_AT_SCROLL_BOTTOM(w)    widget_isAtScrollBottom(w)
#define IS_AT_SCROLL_LEFT(w)      widget_isAtScrollLeft(w)
#define IS_AT_SCROLL_RIGHT(w)     widget_isAtScrollRight(w)

// Navigation macros
#define SCROLL_TO_TOP_LEFT(w)     widget_scrollToTopLeft(w)
#define SCROLL_TO_MAKE_VISIBLE(w, x, y) widget_scrollToMakeVisible(w, x, y)

#endif // WIDGET_SCROLLABLE_H
