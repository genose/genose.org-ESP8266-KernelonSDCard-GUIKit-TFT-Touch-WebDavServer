# Touch Handling

> Extracted from discussion_guikit.txt - XPT2046 touchscreen integration

## Overview

Touch handling in GUIKit uses the XPT2046 touchscreen controller library to detect user input and dispatch events to widgets. This document covers touch initialization, calibration, event handling, and widget interaction.

---

## Hardware Setup

### Pin Configuration

```c
// XPT2046 touchscreen connections
#define TS_CS_PIN    D2    // Chip select
#define TS_IRQ_PIN   D1    // Interrupt (optional)

// TFT display connections (for reference)
#define TFT_CS_PIN   D8
#define TFT_DC_PIN   D3
#define TFT_RST_PIN  D4
```

### Library Installation

Add to `platformio.ini`:

```ini
lib_deps =
    https://github.com/Bodmer/TFT_eSPI.git
    https://github.com/PaulStoffregen/XPT2046_Touchscreen.git
```

---

## Touchscreen Initialization

### Basic Setup

```c
#include <XPT2046_Touchscreen.h>

// Create touchscreen instance
XPT2046_Touchscreen ts(TS_CS_PIN);

void setup() {
    // Initialize TFT
    tft.init();
    
    // Initialize touchscreen
    ts.begin();
    
    // Optional: set rotation to match display
    ts.setRotation(3); // Adjust based on your setup
}
```

### With Interrupt Pin

```c
#include <XPT2046_Touchscreen.h>

// Create touchscreen with interrupt
XPT2046_Touchscreen ts(TS_CS_PIN, TS_IRQ_PIN);

void setup() {
    ts.begin();
    
    // Enable interrupt (optional - reduces CPU usage)
    // This requires external interrupt setup
}
```

---

## Touch Coordinate Mapping

### Understanding Raw vs Display Coordinates

The XPT2046 returns raw ADC values (0-4095) that need to be mapped to display coordinates.

```c
// Screen dimensions
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// Map raw touch coordinates to display coordinates
uint16_t map_touch_x(uint16_t raw_x) {
    return map(raw_x, 0, 4095, 0, SCREEN_WIDTH);
}

uint16_t map_touch_y(uint16_t raw_y) {
    return map(raw_y, 0, 4095, 0, SCREEN_HEIGHT);
}
```

### Calibration

For accurate touch detection, calibration is required. The XPT2046 library provides built-in calibration support.

```c
// Calibration points (adjust based on your display)
#define CAL_X_MIN 300
#define CAL_X_MAX 3700
#define CAL_Y_MIN 200
#define CAL_Y_MAX 3800

// Calibrated mapping
uint16_t calibrated_x(uint16_t raw_x) {
    return map(raw_x, CAL_X_MIN, CAL_X_MAX, 0, SCREEN_WIDTH);
}

uint16_t calibrated_y(uint16_t raw_y) {
    return map(raw_y, CAL_Y_MIN, CAL_Y_MAX, 0, SCREEN_HEIGHT);
}

// Or use the library's calibration
// The library can store calibration data in EEPROM
void calibrate_touchscreen() {
    ts.calibrate();
}
```

### Calibration Test Sketch

```c
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(TS_CS_PIN);

void setup() {
    tft.init();
    ts.begin();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.println("Touch calibration test");
    tft.println("Touch the corners...");
}

void loop() {
    if (ts.touched()) {
        TS_Point p = ts.getPoint();
        
        uint16_t x = map(p.x, 0, 4095, 0, tft.width());
        uint16_t y = map(p.y, 0, 4095, 0, tft.height());
        
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0);
        tft.print("Raw: x="); tft.print(p.x); tft.print(" y="); tft.println(p.y);
        tft.print("Mapped: x="); tft.print(x); tft.print(" y="); tft.println(y);
        tft.fillCircle(x, y, 10, TFT_RED);
    }
}
```

---

## Touch Event Handling

### Basic Touch Detection

```c
void loop() {
    if (ts.touched()) {
        TS_Point p = ts.getPoint();
        
        // Map to display coordinates
        uint16_t touch_x = map(p.x, 0, 4095, 0, tft.width());
        uint16_t touch_y = map(p.y, 0, 4095, 0, tft.height());
        
        // Handle touch for all widgets
        handle_touch_for_all_widgets(root_widget, touch_x, touch_y);
    }
}
```

### Touch State Management

```c
// Track touch state for press/release detection
typedef enum {
    TOUCH_STATE_NONE,
    TOUCH_STATE_PRESSED,
    TOUCH_STATE_HELD,
    TOUCH_STATE_RELEASED
} TouchState;

TouchState current_touch_state = TOUCH_STATE_NONE;
uint16_t last_touch_x = 0;
uint16_t last_touch_y = 0;

void loop() {
    bool touched = ts.touched();
    
    if (touched) {
        TS_Point p = ts.getPoint();
        uint16_t touch_x = map(p.x, 0, 4095, 0, tft.width());
        uint16_t touch_y = map(p.y, 0, 4095, 0, tft.height());
        
        if (current_touch_state == TOUCH_STATE_NONE) {
            // New touch
            current_touch_state = TOUCH_STATE_PRESSED;
            handle_touch_press(root_widget, touch_x, touch_y);
        } else if (current_touch_state == TOUCH_STATE_PRESSED) {
            // Touch held
            current_touch_state = TOUCH_STATE_HELD;
            handle_touch_move(root_widget, touch_x, touch_y);
        } else if (current_touch_state == TOUCH_STATE_HELD) {
            // Touch moving
            handle_touch_move(root_widget, touch_x, touch_y);
        }
        
        last_touch_x = touch_x;
        last_touch_y = touch_y;
    } else {
        if (current_touch_state == TOUCH_STATE_PRESSED || 
            current_touch_state == TOUCH_STATE_HELD) {
            // Touch released
            current_touch_state = TOUCH_STATE_RELEASED;
            handle_touch_release(root_widget, last_touch_x, last_touch_y);
        } else {
            current_touch_state = TOUCH_STATE_NONE;
        }
    }
}
```

---

## Widget Touch Handling

### Hit Testing

```c
// Check if a point is within a widget's bounds
bool widget_contains_point(Widget* widget, uint16_t x, uint16_t y) {
    return (x >= widget->rect.x &&
            x <= widget->rect.x + widget->rect.width &&
            y >= widget->rect.y &&
            y <= widget->rect.y + widget->rect.height);
}

// Find the topmost widget at a given point
Widget* find_widget_at_point(Widget* root, uint16_t x, uint16_t y) {
    // Check children first (topmost first for proper z-order)
    for (int i = root->children_count - 1; i >= 0; i--) {
        Widget* child = root->children[i];
        Widget* result = find_widget_at_point(child, x, y);
        if (result != NULL) {
            return result;
        }
    }
    
    // Check this widget
    if (widget_contains_point(root, x, y)) {
        return root;
    }
    
    return NULL;
}
```

### Touch Event Dispatch

```c
// Touch press handler
void handle_touch_press(Widget* widget, uint16_t x, uint16_t y) {
    switch (widget->type) {
        case WIDGET_TYPE_BUTTON: {
            WidgetButton* button = (WidgetButton*)widget;
            if (button->on_click) {
                button->pressed = true;
                draw_widget((Widget*)button); // Redraw pressed state
            }
            break;
        }
        case WIDGET_TYPE_SLIDER: {
            WidgetSlider* slider = (WidgetSlider*)widget;
            handle_slider_touch_press(slider, x, y);
            break;
        }
        case WIDGET_TYPE_CHECKBOX: {
            WidgetCheckbox* checkbox = (WidgetCheckbox*)widget;
            checkbox->checked = !checkbox->checked;
            draw_widget((Widget*)checkbox);
            if (checkbox->on_toggle) {
                checkbox->on_toggle(checkbox->checked);
            }
            break;
        }
        default:
            // Generic widget touch handling
            break;
    }
}

// Touch release handler
void handle_touch_release(Widget* widget, uint16_t x, uint16_t y) {
    switch (widget->type) {
        case WIDGET_TYPE_BUTTON: {
            WidgetButton* button = (WidgetButton*)widget;
            if (button->pressed && button->on_click) {
                button->on_click();
            }
            button->pressed = false;
            draw_widget((Widget*)button); // Redraw normal state
            break;
        }
        case WIDGET_TYPE_SLIDER: {
            WidgetSlider* slider = (WidgetSlider*)widget;
            handle_slider_touch_release(slider, x, y);
            break;
        }
        default:
            break;
    }
}

// Touch move handler (for draggable widgets)
void handle_touch_move(Widget* widget, uint16_t x, uint16_t y) {
    switch (widget->type) {
        case WIDGET_TYPE_SLIDER: {
            WidgetSlider* slider = (WidgetSlider*)widget;
            handle_slider_touch_move(slider, x, y);
            break;
        }
        default:
            break;
    }
}
```

---

## Slider Touch Handling

```c
void handle_slider_touch_press(WidgetSlider* slider, uint16_t x, uint16_t y) {
    // Calculate value based on touch position
    float ratio;
    
    if (slider->base.rect.width > slider->base.rect.height) {
        // Horizontal slider
        int16_t relative_x = x - slider->base.rect.x;
        ratio = (float)relative_x / slider->base.rect.width;
    } else {
        // Vertical slider
        int16_t relative_y = y - slider->base.rect.y;
        ratio = (float)relative_y / slider->base.rect.height;
    }
    
    ratio = max(0.0f, min(1.0f, ratio));
    slider->current_value = slider->min_value + 
                           ratio * (slider->max_value - slider->min_value);
    
    if (slider->on_change) {
        slider->on_change(slider->current_value);
    }
    
    draw_widget((Widget*)slider);
}

void handle_slider_touch_move(WidgetSlider* slider, uint16_t x, uint16_t y) {
    // Same as press - update value based on current position
    handle_slider_touch_press(slider, x, y);
}

void handle_slider_touch_release(WidgetSlider* slider, uint16_t x, uint16_t y) {
    // Final value update
    handle_slider_touch_press(slider, x, y);
}
```

---

## Gesture Recognition

### Basic Gestures

```c
typedef enum {
    GESTURE_NONE,
    GESTURE_TAP,
    GESTURE_DOUBLE_TAP,
    GESTURE_LONG_PRESS,
    GESTURE_SWIPE_LEFT,
    GESTURE_SWIPE_RIGHT,
    GESTURE_SWIPE_UP,
    GESTURE_SWIPE_DOWN
} GestureType;

// Gesture detection state
typedef struct {
    uint16_t start_x, start_y;
    uint32_t start_time;
    uint8_t tap_count;
    bool long_press_detected;
} GestureState;

GestureState gesture_state = {0, 0, 0, 0, false};

GestureType detect_gesture(uint16_t x, uint16_t y, bool touched) {
    const uint32_t DOUBLE_TAP_TIME = 300;  // ms
    const uint32_t LONG_PRESS_TIME = 1000; // ms
    const uint16_t SWIPE_THRESHOLD = 50;   // pixels
    
    if (touched) {
        if (gesture_state.tap_count > 0 && 
            millis() - gesture_state.start_time > DOUBLE_TAP_TIME) {
            gesture_state.tap_count = 0;
        }
        
        if (gesture_state.tap_count == 0) {
            // First tap
            gesture_state.start_x = x;
            gesture_state.start_y = y;
            gesture_state.start_time = millis();
            gesture_state.tap_count = 1;
            gesture_state.long_press_detected = false;
            return GESTURE_NONE; // Wait for release
        } else if (gesture_state.tap_count == 1) {
            // Second tap - check for double tap
            uint32_t elapsed = millis() - gesture_state.start_time;
            uint16_t dx = abs((int16_t)x - (int16_t)gesture_state.start_x);
            uint16_t dy = abs((int16_t)y - (int16_t)gesture_state.start_y);
            uint16_t distance = sqrt(dx * dx + dy * dy);
            
            if (elapsed < DOUBLE_TAP_TIME && distance < 30) {
                gesture_state.tap_count = 0;
                return GESTURE_DOUBLE_TAP;
            }
        }
        
        // Check for long press
        if (!gesture_state.long_press_detected && 
            millis() - gesture_state.start_time > LONG_PRESS_TIME) {
            gesture_state.long_press_detected = true;
            return GESTURE_LONG_PRESS;
        }
        
        return GESTURE_NONE;
    } else {
        // Touch released
        if (gesture_state.tap_count == 1) {
            // Check for swipe
            uint16_t dx = abs((int16_t)x - (int16_t)gesture_state.start_x);
            uint16_t dy = abs((int16_t)y - (int16_t)gesture_state.start_y);
            
            if (dx > SWIPE_THRESHOLD || dy > SWIPE_THRESHOLD) {
                if (dx > dy) {
                    if (x > gesture_state.start_x) {
                        gesture_state.tap_count = 0;
                        return GESTURE_SWIPE_RIGHT;
                    } else {
                        gesture_state.tap_count = 0;
                        return GESTURE_SWIPE_LEFT;
                    }
                } else {
                    if (y > gesture_state.start_y) {
                        gesture_state.tap_count = 0;
                        return GESTURE_SWIPE_DOWN;
                    } else {
                        gesture_state.tap_count = 0;
                        return GESTURE_SWIPE_UP;
                    }
                }
            } else if (!gesture_state.long_press_detected) {
                gesture_state.tap_count = 0;
                return GESTURE_TAP;
            }
        }
        
        gesture_state.tap_count = 0;
        return GESTURE_NONE;
    }
}
```

### Using Gestures

```c
void loop() {
    if (ts.touched()) {
        TS_Point p = ts.getPoint();
        uint16_t x = map(p.x, 0, 4095, 0, tft.width());
        uint16_t y = map(p.y, 0, 4095, 0, tft.height());
        
        GestureType gesture = detect_gesture(x, y, true);
        
        switch (gesture) {
            case GESTURE_TAP:
                handle_tap(x, y);
                break;
            case GESTURE_DOUBLE_TAP:
                handle_double_tap(x, y);
                break;
            case GESTURE_LONG_PRESS:
                handle_long_press(x, y);
                break;
            case GESTURE_SWIPE_LEFT:
                handle_swipe_left();
                break;
            case GESTURE_SWIPE_RIGHT:
                handle_swipe_right();
                break;
            case GESTURE_SWIPE_UP:
                handle_swipe_up();
                break;
            case GESTURE_SWIPE_DOWN:
                handle_swipe_down();
                break;
            default:
                break;
        }
    } else {
        detect_gesture(0, 0, false);
    }
}
```

---

## Complete Touch Integration Example

```c
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(TS_CS_PIN);

// Touch state
bool last_touched = false;

void setup() {
    tft.init();
    ts.begin();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    
    // Create UI
    setup_ui();
}

void loop() {
    bool touched = ts.touched();
    
    if (touched) {
        TS_Point p = ts.getPoint();
        uint16_t x = map(p.x, 0, 4095, 0, tft.width());
        uint16_t y = map(p.y, 0, 4095, 0, tft.height());
        
        if (!last_touched) {
            // New touch
            handle_touch_press(root_widget, x, y);
        } else {
            // Touch moving
            handle_touch_move(root_widget, x, y);
        }
    } else {
        if (last_touched) {
            // Touch released
            handle_touch_release(root_widget, 0, 0);
        }
    }
    
    last_touched = touched;
}
```

---

## Touch Optimization

### Reducing CPU Usage

```c
// Use interrupt to detect touch (reduces polling)
volatile bool touch_detected = false;

void IRAM_ATTR touch_isr() {
    touch_detected = true;
}

void setup() {
    // ... initialization ...
    
    // Set up interrupt
    pinMode(TS_IRQ_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TS_IRQ_PIN), touch_isr, FALLING);
}

void loop() {
    if (touch_detected) {
        touch_detected = false;
        
        if (ts.touched()) {
            TS_Point p = ts.getPoint();
            uint16_t x = map(p.x, 0, 4095, 0, tft.width());
            uint16_t y = map(p.y, 0, 4095, 0, tft.height());
            handle_touch(x, y);
        }
    }
    
    // Other processing can happen here without polling touch
}
```

### Debouncing

```c
// Simple debounce
uint32_t last_touch_time = 0;
const uint32_t DEBOUNCE_TIME = 50; // ms

void loop() {
    if (ts.touched()) {
        uint32_t now = millis();
        if (now - last_touch_time > DEBOUNCE_TIME) {
            last_touch_time = now;
            TS_Point p = ts.getPoint();
            uint16_t x = map(p.x, 0, 4095, 0, tft.width());
            uint16_t y = map(p.y, 0, 4095, 0, tft.height());
            handle_touch(x, y);
        }
    }
}
```

---

## Summary

Touch handling provides:
- ✅ XPT2046 library integration
- ✅ Coordinate mapping and calibration
- ✅ Press, release, and move event handling
- ✅ Widget-specific touch processing
- ✅ Gesture recognition (tap, double-tap, long-press, swipe)
- ✅ Interrupt-based detection for reduced CPU usage
- ✅ Debouncing support
- ✅ ESP8266-optimized

---

*Source: Extracted from discussion_guikit.txt, lines 5930-7620*
*Documentation organized by Mistral Vibe*
