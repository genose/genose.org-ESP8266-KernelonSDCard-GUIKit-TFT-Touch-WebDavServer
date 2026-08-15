/**
 * widget.h - Widget System with Objective-C Style Accessors
 * 
 * Memory-safe widget system for ESP8266 GUIKit
 * Uses object pooling and static buffers for efficiency
 * 
 * Generated for ESP8266 by Mistral Vibe
 */

#ifndef WIDGET_H
#define WIDGET_H

#include <stdint.h>
#include <stdbool.h>
#include "widget_text.h"
#include "widget_pool.h"


// ============================================================================
// CONSTRUCTORS (Objective-C style: [[Widget alloc] init])
// ============================================================================

/**
 * @brief Create a new widget of the specified type
 * 
 * Allocates from the appropriate pool. Returns NULL if pool is exhausted.
 * 
 * @param type Widget type
 * @return Pointer to new widget, or NULL on failure
 */
Widget* Widget_new(WIDGET_TYPE type);

/**
 * @brief Create a new button widget
 * 
 * @return Pointer to new button, or NULL on failure
 */
WidgetButton* Button_new(void);

/**
 * @brief Create a new label widget
 * 
 * @return Pointer to new label, or NULL on failure
 */
WidgetLabel* Label_new(void);

/**
 * @brief Create a new slider widget
 * 
 * @return Pointer to new slider, or NULL on failure
 */
WidgetSlider* Slider_new(void);

/**
 * @brief Create a new view widget
 * 
 * @return Pointer to new view, or NULL on failure
 */
WidgetView* View_new(void);


// ============================================================================
// DESTRUCTORS (Objective-C style: [widget release])
// ============================================================================

/**
 * @brief Delete a widget and return it to its pool
 * 
 * If widget is from a pool, marks it as free.
 * If widget was dynamically allocated (shouldn't happen), frees it.
 * 
 * @param self Widget to delete
 */
void Widget_delete(Widget* self);


// ============================================================================
// TEXT ACCESSORS
// ============================================================================

/**
 * @brief Set widget text (safe, bounds-checked)
 * 
 * @param self Widget pointer
 * @param text Text to set (can be NULL to clear)
 */
static inline void Widget_setText(Widget* self, const char* text) {
    if (self) {
        WidgetSetText(&self->text, text);
        self->dirty = true;  // Mark for redraw
    }
}

/**
 * @brief Get widget text
 * 
 * @param self Widget pointer
 * @return Text string (empty if NULL or widget has no text)
 */
static inline const char* Widget_getText(const Widget* self) {
    return self ? WidgetGetText(&self->text) : "";
}


// ============================================================================
// GEOMETRY ACCESSORS
// ============================================================================

/**
 * @brief Set widget frame (position and size)
 * 
 * @param self Widget pointer
 * @param x X position
 * @param y Y position
 * @param width Width
 * @param height Height
 */
static inline void Widget_setFrame(Widget* self, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    if (self) {
        self->x = x;
        self->y = y;
        self->width = width;
        self->height = height;
        self->dirty = true;
    }
}

/**
 * @brief Set widget position
 * 
 * @param self Widget pointer
 * @param x X position
 * @param y Y position
 */
static inline void Widget_setPosition(Widget* self, uint16_t x, uint16_t y) {
    if (self) {
        self->x = x;
        self->y = y;
        self->dirty = true;
    }
}

/**
 * @brief Set widget size
 * 
 * @param self Widget pointer
 * @param width Width
 * @param height Height
 */
static inline void Widget_setSize(Widget* self, uint16_t width, uint16_t height) {
    if (self) {
        self->width = width;
        self->height = height;
        self->dirty = true;
    }
}

/**
 * @brief Get widget frame
 * 
 * @param self Widget pointer
 * @param x Output: X position
 * @param y Output: Y position
 * @param width Output: Width
 * @param height Output: Height
 */
static inline void Widget_getFrame(const Widget* self, uint16_t* x, uint16_t* y, uint16_t* width, uint16_t* height) {
    if (self && x && y && width && height) {
        *x = self->x;
        *y = self->y;
        *width = self->width;
        *height = self->height;
    }
}

/**
 * @brief Get X position
 */
static inline uint16_t Widget_getX(const Widget* self) {
    return self ? self->x : 0;
}

/**
 * @brief Get Y position
 */
static inline uint16_t Widget_getY(const Widget* self) {
    return self ? self->y : 0;
}

/**
 * @brief Get width
 */
static inline uint16_t Widget_getWidth(const Widget* self) {
    return self ? self->width : 0;
}

/**
 * @brief Get height
 */
static inline uint16_t Widget_getHeight(const Widget* self) {
    return self ? self->height : 0;
}


// ============================================================================
// STYLE ACCESSORS
// ============================================================================

/**
 * @brief Set background color
 * 
 * @param self Widget pointer
 * @param color Color in RGBA565 format
 */
static inline void Widget_setBackgroundColor(Widget* self, uint16_t color) {
    if (self) {
        self->bg_color = color;
        self->dirty = true;
    }
}

/**
 * @brief Get background color
 * 
 * @param self Widget pointer
 * @return Background color
 */
static inline uint16_t Widget_getBackgroundColor(const Widget* self) {
    return self ? self->bg_color : 0;
}

/**
 * @brief Set border color
 * 
 * @param self Widget pointer
 * @param color Border color in RGBA565 format
 */
static inline void Widget_setBorderColor(Widget* self, uint16_t color) {
    if (self) {
        self->border_color = color;
        self->dirty = true;
    }
}

/**
 * @brief Get border color
 * 
 * @param self Widget pointer
 * @return Border color
 */
static inline uint16_t Widget_getBorderColor(const Widget* self) {
    return self ? self->border_color : 0;
}

/**
 * @brief Set border width
 * 
 * @param self Widget pointer
 * @param width Border width in pixels
 */
static inline void Widget_setBorderWidth(Widget* self, uint8_t width) {
    if (self) {
        self->border_width = width;
        self->dirty = true;
    }
}

/**
 * @brief Get border width
 * 
 * @param self Widget pointer
 * @return Border width
 */
static inline uint8_t Widget_getBorderWidth(const Widget* self) {
    return self ? self->border_width : 0;
}


// ============================================================================
// VISIBILITY ACCESSORS
// ============================================================================

/**
 * @brief Set widget visibility
 * 
 * @param self Widget pointer
 * @param visible true to show, false to hide
 */
static inline void Widget_setVisible(Widget* self, bool visible) {
    if (self) {
        self->visible = visible;
        self->dirty = true;
    }
}

/**
 * @brief Get widget visibility
 * 
 * @param self Widget pointer
 * @return true if visible
 */
static inline bool Widget_isVisible(const Widget* self) {
    return self ? self->visible : false;
}


// ============================================================================
// DIRTY FLAG ACCESSORS
// ============================================================================

/**
 * @brief Mark widget as needing redraw
 * 
 * @param self Widget pointer
 * @param dirty true to mark as dirty
 */
static inline void Widget_setDirty(Widget* self, bool dirty) {
    if (self) {
        self->dirty = dirty;
    }
}

/**
 * @brief Check if widget needs redraw
 * 
 * @param self Widget pointer
 * @return true if dirty
 */
static inline bool Widget_isDirty(const Widget* self) {
    return self ? self->dirty : false;
}


// ============================================================================
// CHILDREN ACCESSORS
// ============================================================================

/**
 * @brief Add a child widget
 * 
 * @param self Parent widget
 * @param child Child widget to add
 */
void Widget_addChild(Widget* self, Widget* child);

/**
 * @brief Remove a child widget
 * 
 * @param self Parent widget
 * @param child Child widget to remove
 */
void Widget_removeChild(Widget* self, Widget* child);

/**
 * @brief Get child at index
 * 
 * @param self Parent widget
 * @param index Child index
 * @return Child widget, or NULL if index out of bounds
 */
static inline Widget* Widget_getChild(const Widget* self, uint8_t index) {
    return (self && index < self->children_count) ? self->children[index] : NULL;
}

/**
 * @brief Get number of children
 * 
 * @param self Widget pointer
 * @return Number of children
 */
static inline uint8_t Widget_getChildrenCount(const Widget* self) {
    return self ? self->children_count : 0;
}

/**
 * @brief Remove all children
 * 
 * @param self Parent widget
 */
void Widget_removeAllChildren(Widget* self);


// ============================================================================
// BUTTON-SPECIFIC ACCESSORS
// ============================================================================

/**
 * @brief Set button pressed state
 * 
 * @param self Button widget
 * @param pressed true if pressed
 */
static inline void Button_setPressed(WidgetButton* self, bool pressed) {
    if (self) {
        self->pressed = pressed;
        self->base.base.dirty = true;
    }
}

/**
 * @brief Get button pressed state
 * 
 * @param self Button widget
 * @return true if pressed
 */
static inline bool Button_isPressed(const WidgetButton* self) {
    return self ? self->pressed : false;
}

/**
 * @brief Set button click callback
 * 
 * @param self Button widget
 * @param callback Function to call when clicked (can be NULL)
 */
static inline void Button_setOnClick(WidgetButton* self, void (*callback)(void*)) {
    if (self) {
        self->onClick = callback;
    }
}

/**
 * @brief Get button click callback
 * 
 * @param self Button widget
 * @return Click callback function
 */
static inline void (*Button_getOnClick(const WidgetButton* self))(void*) {
    return self ? self->onClick : NULL;
}


// ============================================================================
// SLIDER-SPECIFIC ACCESSORS
// ============================================================================

/**
 * @brief Set slider value
 * 
 * @param self Slider widget
 * @param value New value (will be clamped to min/max)
 */
void Slider_setValue(WidgetSlider* self, uint16_t value);

/**
 * @brief Get slider value
 * 
 * @param self Slider widget
 * @return Current value
 */
static inline uint16_t Slider_getValue(const WidgetSlider* self) {
    return self ? self->value : 0;
}

/**
 * @brief Set slider range
 * 
 * @param self Slider widget
 * @param min Minimum value
 * @param max Maximum value
 */
static inline void Slider_setRange(WidgetSlider* self, uint16_t min, uint16_t max) {
    if (self) {
        self->min_value = min;
        self->max_value = max;
        // Clamp current value
        if (self->value < min) self->value = min;
        if (self->value > max) self->value = max;
    }
}

/**
 * @brief Set slider orientation
 * 
 * @param self Slider widget
 * @param vertical true for vertical, false for horizontal
 */
static inline void Slider_setVertical(WidgetSlider* self, bool vertical) {
    if (self) {
        self->vertical = vertical;
        self->base.base.dirty = true;
    }
}

/**
 * @brief Get slider orientation
 * 
 * @param self Slider widget
 * @return true if vertical
 */
static inline bool Slider_isVertical(const WidgetSlider* self) {
    return self ? self->vertical : false;
}

/**
 * @brief Set slider change callback
 * 
 * @param self Slider widget
 * @param callback Function to call when value changes
 */
static inline void Slider_setOnChange(WidgetSlider* self, void (*callback)(uint16_t)) {
    if (self) {
        self->onChange = callback;
    }
}


// ============================================================================
// RENDERING
// ============================================================================

/**
 * @brief Render widget to display
 * 
 * @param self Widget to render
 */
void Widget_render(const Widget* self);

/**
 * @brief Mark widget and all ancestors as dirty
 * 
 * @param self Widget to mark
 */
void Widget_markDirty(Widget* self);


// ============================================================================
// CONVENIENCE MACROS (Objective-C style)
// ============================================================================

// Constructor macros
#define NEW_WIDGET(type) Widget_new(type)
#define NEW_BUTTON() Button_new()
#define NEW_LABEL() Label_new()
#define NEW_SLIDER() Slider_new()
#define NEW_VIEW() View_new()

// Text macros
#define WITH_TEXT(widget, text) Widget_setText((widget), (text))
#define GET_TEXT(widget) Widget_getText(widget)

// Frame macros
#define WITH_FRAME(widget, x, y, w, h) Widget_setFrame((widget), (x), (y), (w), (h))
#define WITH_POSITION(widget, x, y) Widget_setPosition((widget), (x), (y))
#define WITH_SIZE(widget, w, h) Widget_setSize((widget), (w), (h))

// Style macros
#define WITH_BG_COLOR(widget, color) Widget_setBackgroundColor((widget), (color))
#define WITH_BORDER_COLOR(widget, color) Widget_setBorderColor((widget), (color))
#define WITH_BORDER_WIDTH(widget, width) Widget_setBorderWidth((widget), (width))

// Children macros
#define ADD_CHILD(parent, child) Widget_addChild((parent), (child))
#define REMOVE_CHILD(parent, child) Widget_removeChild((parent), (child))

// Visibility macros
#define SHOW(widget) Widget_setVisible((widget), true)
#define HIDE(widget) Widget_setVisible((widget), false)

// Button-specific macros
#define BUTTON_SET_TITLE(btn, title) WITH_TEXT(btn, title)
#define BUTTON_SET_PRESSED(btn, pressed) Button_setPressed((btn), (pressed))
#define BUTTON_SET_ACTION(btn, callback) Button_setOnClick((btn), (callback))

// Slider-specific macros
#define SLIDER_SET_VALUE(slider, value) Slider_setValue((slider), (value))
#define SLIDER_SET_RANGE(slider, min, max) Slider_setRange((slider), (min), (max))
#define SLIDER_SET_ACTION(slider, callback) Slider_setOnChange((slider), (callback))


// ============================================================================
// SAFE CLEANUP
// ============================================================================

// Already defined in widget_pool.h
// #define SAFE_RELEASE(w) do { if(w) { Widget_delete(w); w = NULL; } } while(0)


#endif // WIDGET_H
