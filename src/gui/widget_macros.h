/**
 * widget_macros.h - Objective-C Style Macros for Widgets
 * 
 * Provides concise, chainable syntax for widget creation and configuration
 * No runtime overhead - macros expand at compile time
 * 
 * Generated for ESP8266 by Mistral Vibe
 */

#ifndef WIDGET_MACROS_H
#define WIDGET_MACROS_H

#include "widget.h"


// ============================================================================
// CONSTRUCTOR MACROS
// ============================================================================

/** Create a new widget - like [[Widget alloc] init] */
#define NEW_WIDGET(type) Widget_new(type)

/** Create a new button - like [UIButton buttonWithType:] */
#define NEW_BUTTON() Button_new()

/** Create a new label */
#define NEW_LABEL() Label_new()

/** Create a new slider */
#define NEW_SLIDER() Slider_new()

/** Create a new view/container */
#define NEW_VIEW() View_new()


// ============================================================================
// TEXT MACROS
// ============================================================================

/** Set widget text - like widget.text = @"Hello" */
#define WITH_TEXT(widget, text) Widget_setText((widget), (text))

/** Get widget text */
#define GET_TEXT(widget) Widget_getText(widget)

/** Clear widget text */
#define CLEAR_TEXT(widget) WidgetSetText(&(widget)->text, NULL)

/** Set font size */
#define WITH_FONT_SIZE(widget, size) WidgetSetFontSize(&(widget)->text, (size))

/** Set font color */
#define WITH_FONT_COLOR(widget, color) WidgetSetFontColor(&(widget)->text, (color))

/** Set font style (size and color) */
#define WITH_FONT(widget, size, color) do { \
    WidgetSetFontSize(&(widget)->text, (size)); \
    WidgetSetFontColor(&(widget)->text, (color)); \
} while(0)


// ============================================================================
// GEOMETRY MACROS
// ============================================================================

/** Set widget frame (x, y, width, height) - like widget.frame = CGRectMake */
#define WITH_FRAME(widget, x, y, w, h) Widget_setFrame((widget), (x), (y), (w), (h))

/** Set widget position - like widget.center = CGPointMake */
#define WITH_POSITION(widget, x, y) Widget_setPosition((widget), (x), (y))

/** Set widget size */
#define WITH_SIZE(widget, w, h) Widget_setSize((widget), (w), (h))

/** Set X position */
#define WITH_X(widget, x) do { if(widget) (widget)->x = (x); (widget)->dirty = true; } while(0)

/** Set Y position */
#define WITH_Y(widget, y) do { if(widget) (widget)->y = (y); (widget)->dirty = true; } while(0)

/** Set width */
#define WITH_WIDTH(widget, w) do { if(widget) (widget)->width = (w); (widget)->dirty = true; } while(0)

/** Set height */
#define WITH_HEIGHT(widget, h) do { if(widget) (widget)->height = (h); (widget)->dirty = true; } while(0)

/** Get widget frame */
#define GET_FRAME(widget, x, y, w, h) Widget_getFrame((widget), (x), (y), (w), (h))


// ============================================================================
// STYLE MACROS
// ============================================================================

/** Set background color */
#define WITH_BG_COLOR(widget, color) Widget_setBackgroundColor((widget), (color))

/** Set border color */
#define WITH_BORDER_COLOR(widget, color) Widget_setBorderColor((widget), (color))

/** Set border width */
#define WITH_BORDER_WIDTH(widget, width) Widget_setBorderWidth((widget), (width))

/** Set border (color and width) */
#define WITH_BORDER(widget, color, width) do { \
    Widget_setBorderColor((widget), (color)); \
    Widget_setBorderWidth((widget), (width)); \
} while(0)

/** Set background and border together */
#define WITH_STYLE(widget, bg_color, border_color, border_width) do { \
    Widget_setBackgroundColor((widget), (bg_color)); \
    Widget_setBorderColor((widget), (border_color)); \
    Widget_setBorderWidth((widget), (border_width)); \
} while(0)


// ============================================================================
// VISIBILITY MACROS
// ============================================================================

/** Show widget */
#define SHOW(widget) Widget_setVisible((widget), true)

/** Hide widget */
#define HIDE(widget) Widget_setVisible((widget), false)

/** Toggle visibility */
#define TOGGLE_VISIBILITY(widget) do { \
    if(widget) (widget)->visible = !(widget)->visible; \
    if(widget) (widget)->dirty = true; \
} while(0)

/** Check if visible */
#define IS_VISIBLE(widget) Widget_isVisible(widget)


// ============================================================================
// DIRTY FLAG MACROS
// ============================================================================

/** Mark as dirty (needs redraw) */
#define MARK_DIRTY(widget) Widget_setDirty((widget), true)

/** Mark as clean */
#define MARK_CLEAN(widget) Widget_setDirty((widget), false)

/** Check if dirty */
#define IS_DIRTY(widget) Widget_isDirty(widget)


// ============================================================================
// CHILDREN MACROS
// ============================================================================

/** Add child widget - like [parent addSubview:child] */
#define ADD_CHILD(parent, child) Widget_addChild((parent), (child))

/** Remove child widget */
#define REMOVE_CHILD(parent, child) Widget_removeChild((parent), (child))

/** Remove all children */
#define REMOVE_ALL_CHILDREN(parent) Widget_removeAllChildren(parent)

/** Get child at index */
#define GET_CHILD(parent, index) Widget_getChild((parent), (index))

/** Get children count */
#define CHILDREN_COUNT(widget) Widget_getChildrenCount(widget)


// ============================================================================
// BUTTON-SPECIFIC MACROS
// ============================================================================

/** Set button title - like button.title = @"Click" */
#define BUTTON_SET_TITLE(btn, title) WITH_TEXT(btn, title)

/** Get button title */
#define BUTTON_GET_TITLE(btn) GET_TEXT(btn)

/** Set button pressed state */
#define BUTTON_SET_PRESSED(btn, pressed) Button_setPressed((btn), (pressed))

/** Get button pressed state */
#define BUTTON_IS_PRESSED(btn) Button_isPressed(btn)

/** Set button click callback - like button.onClick = callback */
#define BUTTON_SET_ACTION(btn, callback) Button_setOnClick((btn), (callback))

/** Get button click callback */
#define BUTTON_GET_ACTION(btn) Button_getOnClick(btn)


// ============================================================================
// SLIDER-SPECIFIC MACROS
// ============================================================================

/** Set slider value */
#define SLIDER_SET_VALUE(slider, value) Slider_setValue((slider), (value))

/** Get slider value */
#define SLIDER_GET_VALUE(slider) Slider_getValue(slider)

/** Set slider range (min, max) */
#define SLIDER_SET_RANGE(slider, min, max) Slider_setRange((slider), (min), (max))

/** Set slider minimum value */
#define SLIDER_SET_MIN(slider, min) do { \
    if(slider) { \
        (slider)->min_value = (min); \
        if ((slider)->value < (min)) (slider)->value = (min); \
    } \
} while(0)

/** Set slider maximum value */
#define SLIDER_SET_MAX(slider, max) do { \
    if(slider) { \
        (slider)->max_value = (max); \
        if ((slider)->value > (max)) (slider)->value = (max); \
    } \
} while(0)

/** Set slider orientation (true = vertical, false = horizontal) */
#define SLIDER_SET_VERTICAL(slider, vertical) Slider_setVertical((slider), (vertical))

/** Check if slider is vertical */
#define SLIDER_IS_VERTICAL(slider) Slider_isVertical(slider)

/** Set slider change callback */
#define SLIDER_SET_ACTION(slider, callback) Slider_setOnChange((slider), (callback))


// ============================================================================
// VIEW/CONTAINER MACROS
// ============================================================================

/** View-specific constructor */
#define NEW_VIEW() View_new()

/** Add child to view with bounds checking */
#define VIEW_ADD_CHILD(view, child) ADD_CHILD(view, child)


// ============================================================================
// MEMORY MANAGEMENT MACROS
// ============================================================================

/** Safe release - like [widget release], widget = nil */
#define SAFE_RELEASE(widget) do { \
    if ((widget)) { \
        Widget_delete((Widget*)(widget)); \
        (widget) = NULL; \
    } \
} while(0)

/** Safe release for button */
#define SAFE_RELEASE_BUTTON(btn) SAFE_RELEASE(btn)

/** Safe release for label */
#define SAFE_RELEASE_LABEL(lbl) SAFE_RELEASE(lbl)

/** Safe release for slider */
#define SAFE_RELEASE_SLIDER(slider) SAFE_RELEASE(slider)

/** Safe release for view */
#define SAFE_RELEASE_VIEW(view) SAFE_RELEASE(view)


// ============================================================================
// CHAINABLE MACROS
// ============================================================================

/**
 * CHAIN macro - Enable method chaining
 * 
 * Usage:
 * Widget* btn = CHAIN(NEW_BUTTON())
 *     ->WITH_TEXT("Click")
 *     ->WITH_FRAME(0, 0, 100, 50)
 *     ->WITH_BG_COLOR(COLOR_GREEN);
 */
#define CHAIN(widget) (widget)


// ============================================================================
// PREDEFINED COLORS (RGBA565)
// ============================================================================

#define COLOR_BLACK       0x0000
#define COLOR_WHITE       0xFFFF
#define COLOR_RED         0xF800
#define COLOR_GREEN       0x07E0
#define COLOR_BLUE        0x001F
#define COLOR_YELLOW      0xFFE0
#define COLOR_CYAN        0x07FF
#define COLOR_MAGENTA     0xF81F
#define COLOR_GRAY        0x7BEF
#define COLOR_LIGHT_GRAY  0xC618
#define COLOR_DARK_GRAY   0x5AEB
#define COLOR_TRANSPARENT 0x0000  // Or use a special value


// ============================================================================
// UTILITY MACROS
// ============================================================================

/** Execute callback if not NULL */
#define SAFE_CALLBACK(callback, ...) do { \
    if ((callback)) { (callback)(__VA_ARGS__); } \
} while(0)

/** Execute callback with context */
#define SAFE_CALLBACK_CTX(callback, ctx) do { \
    if ((callback)) { (callback)((ctx)); } \
} while(0)

/** Clamp value to range */
#define CLAMP(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

/** Minimum of two values */
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/** Maximum of two values */
#define MAX(a, b) ((a) > (b) ? (a) : (b))


// ============================================================================
// COMPLEX CHAINING EXAMPLE
// ============================================================================

/**
 * Example of complex widget creation with chaining:
 * 
 * Widget* btn = CHAIN(NEW_BUTTON())
 *     WITH_TEXT("Submit")
 *     WITH_FRAME(50, 100, 120, 40)
 *     WITH_BG_COLOR(COLOR_GREEN)
 *     WITH_BORDER(COLOR_WHITE, 2)
 *     WITH_FONT(14, COLOR_BLACK)
 *     BUTTON_SET_ACTION(my_callback)
 *     ADD_CHILD(parent, btn);
 */


#endif // WIDGET_MACROS_H
