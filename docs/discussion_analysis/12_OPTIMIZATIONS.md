# ESP8266-Specific Optimizations

> Extracted from discussion_guikit.txt - Optimization techniques for ESP8266 constraints

## Overview

This document covers all optimization strategies for GUIKit running on ESP8266 with its strict resource constraints:
- **RAM**: 80KB total (approximately 40KB available for application)
- **Flash**: 4MB (code + data)
- **No FPU**: No floating-point unit
- **No atomic ops**: No hardware atomic operation support

All optimizations follow Objective-C style memory management (NOT ARC/reference counting).

---

## Optimization Summary Table

| Feature | Optimization Approach | Estimated Memory |
|---------|---------------------|-----------------|
| Levenshtein Corrector | Simplified algorithm + PROGMEM dictionary | ~2 KB (Flash) |
| System Clipboard | Static buffer (64 characters) | ~64 bytes |
| Multi-touch Gestures | Single-point simulation (XPT2046 limitation) | ~0 bytes |
| Clipping | Limit rendering to visible areas | ~0 bytes |
| Undo/Redo History | Static stack (10 states, 32 chars each) | ~320 bytes |

---

## Memory Optimization

### Object Pooling

**Principle**: Pre-allocate objects at startup instead of dynamic allocation during runtime.

#### Widget Pool

```c
// Widget pool definition
#define MAX_WIDGET_POOL 50
#define WIDGET_POOL_SIZE (MAX_WIDGET_POOL * sizeof(Widget))

Widget widget_pool[MAX_WIDGET_POOL];
bool widget_pool_used[MAX_WIDGET_POOL] = {false};

// Allocate widget from pool
Widget* widget_pool_alloc(WIDGET_TYPE type) {
    for (int i = 0; i < MAX_WIDGET_POOL; i++) {
        if (!widget_pool_used[i]) {
            widget_pool_used[i] = true;
            memset(&widget_pool[i], 0, sizeof(Widget));
            widget_pool[i].type = type;
            // Initialize default values
            widget_pool[i].rect = (Rect){{0, 0}, {100, 50}};
            widget_pool[i].style.draw_style = STYLE_DEFAULT;
            widget_pool[i].parent = NULL;
            widget_pool[i].children = NULL;
            widget_pool[i].children_count = 0;
            widget_pool[i].data = NULL;
            return &widget_pool[i];
        }
    }
    return NULL;  // Pool exhausted
}

// Release widget to pool
void widget_pool_release(Widget* widget) {
    if (widget >= widget_pool && widget < widget_pool + MAX_WIDGET_POOL) {
        int index = (widget - widget_pool);
        widget_pool_used[index] = false;
        // Clear widget
        memset(&widget_pool[index], 0, sizeof(Widget));
    }
}

// Pool-based constructors
WidgetButton* new_button_pooled(void) {
    WidgetButton* button = (WidgetButton*)widget_pool_alloc(WIDGET_TYPE_BUTTON);
    if (!button) return NULL;
    button->pressed = false;
    button->on_click = NULL;
    button->on_release = NULL;
    return button;
}

WidgetLabel* new_label_pooled(const char* text) {
    WidgetLabel* label = (WidgetLabel*)widget_pool_alloc(WIDGET_TYPE_LABEL);
    if (!label) return NULL;
    label->auto_resize = true;
    if (text) {
        strncpy(label->base.text.text, text, MAX_TEXT_LENGTH - 1);
        label->base.text.text[MAX_TEXT_LENGTH - 1] = '\0';
    }
    return label;
}
```

#### Text Field Pool

```c
#define MAX_TEXTFIELD_POOL 10
#define TEXTFIELD_BUFFER_SIZE 256

WidgetTextField textfield_pool[MAX_TEXTFIELD_POOL];
bool textfield_pool_used[MAX_TEXTFIELD_POOL] = {false};
char textfield_buffers[MAX_TEXTFIELD_POOL][TEXTFIELD_BUFFER_SIZE + 1];

WidgetTextField* textfield_pool_alloc(TEXTFIELD_STYLE style) {
    for (int i = 0; i < MAX_TEXTFIELD_POOL; i++) {
        if (!textfield_pool_used[i]) {
            textfield_pool_used[i] = true;
            WidgetTextField* tf = &textfield_pool[i];
            memset(tf, 0, sizeof(WidgetTextField));
            tf->base.type = WIDGET_TYPE_TEXTFIELD;
            tf->buffer = textfield_buffers[i];
            tf->buffer[0] = '\0';
            tf->buffer_size = TEXTFIELD_BUFFER_SIZE;
            tf->cursor_pos = 0;
            tf->selection.active = false;
            tf->style = style;
            tf->has_focus = false;
            return tf;
        }
    }
    return NULL;
}
```

#### File Info Pool

```c
#define MAX_FILEINFO_POOL 20
FileInfo fileinfo_pool[MAX_FILEINFO_POOL];

bool sd_list_files_pooled(const char* path, FileInfo** files, uint8_t* count) {
    if (!is_sd_card_available() || !files || !count) return false;
    
    *count = 0;
    SdFile dir;
    if (!dir.open(path)) return false;

    SdFile entry;
    while (entry.openNext(&dir, O_READ) && *count < MAX_FILEINFO_POOL) {
        entry.getName(fileinfo_pool[*count].name, 32);
        fileinfo_pool[*count].size = entry.fileSize();
        fileinfo_pool[*count].is_dir = entry.isDir();
        fileinfo_pool[*count].shared = false;
        (*count)++;
        entry.close();
    }
    dir.close();
    
    *files = fileinfo_pool;
    return true;
}
```

### Static Buffers

**Principle**: Use pre-allocated static buffers instead of dynamic allocation.

```c
// Text rendering buffer
#define TEXT_RENDER_BUFFER_SIZE (320 * 240)  // Full screen
uint16_t text_render_buffer[TEXT_RENDER_BUFFER_SIZE];

// File read buffer
#define MAX_FILE_SIZE 1024
char sd_read_buffer[MAX_FILE_SIZE + 1];

// Clipboard buffer
#define CLIPBOARD_BUFFER_SIZE 64
char clipboard_buffer[CLIPBOARD_BUFFER_SIZE + 1];

// String formatting buffer
#define FORMAT_BUFFER_SIZE 128
char format_buffer[FORMAT_BUFFER_SIZE];
```

### Avoiding Dynamic Allocation in Critical Paths

```c
// BAD - allocates in touch handler (can cause memory fragmentation)
void bad_touch_handler(Widget* widget) {
    char* temp = (char*)malloc(256);  // Avoid!
    if (temp) {
        // Process touch
        free(temp);
    }
}

// GOOD - uses static buffer
void good_touch_handler(Widget* widget) {
    static char temp_buffer[256];  // Pre-allocated, reused
    // Process touch
}

// BAD - allocates in render loop
void bad_render_widget(Widget* widget) {
    char* text = (char*)malloc(strlen(widget->text.text) + 1);
    strcpy(text, widget->text.text);
    // Render
    free(text);
}

// GOOD - uses existing buffer
void good_render_widget(Widget* widget) {
    const char* text = widget->text.text;  // Already allocated
    // Render
}
```

### Memory-Efficient Data Structures

```c
// Use bitmask flags instead of multiple booleans
// BAD - 5 booleans = 5 bytes (with padding)
struct BadFlags {
    bool visible;
    bool enabled;
    bool focused;
    bool dirty;
    bool selected;
};  // May use 8+ bytes due to padding

// GOOD - bitmask = 1 byte
struct GoodFlags {
    uint8_t flags;
};
#define FLAG_VISIBLE   (1 << 0)
#define FLAG_ENABLED   (1 << 1)
#define FLAG_FOCUSED   (1 << 2)
#define FLAG_DIRTY     (1 << 3)
#define FLAG_SELECTED  (1 << 4)

// Use union for mutually exclusive data
union WidgetData {
    ButtonData button;
    SliderData slider;
    TextFieldData textfield;
};  // Uses max size of all types, not sum
```

---

## Performance Optimization

### Rendering Optimizations

#### Clipping System

**Principle**: Only draw widgets that are visible on screen.

```c
typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
} ClipRect;

// Global clipping rectangle
ClipRect current_clip_rect = {0, 0, 320, 240};

// Set clipping rectangle
void set_clip_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    current_clip_rect.x = x;
    current_clip_rect.y = y;
    current_clip_rect.width = w;
    current_clip_rect.height = h;
}

// Check if rectangle is visible
bool is_visible(Rect rect) {
    if (rect.position.x >= current_clip_rect.x + current_clip_rect.width) return false;
    if (rect.position.y >= current_clip_rect.y + current_clip_rect.height) return false;
    if (rect.position.x + rect.size.width <= current_clip_rect.x) return false;
    if (rect.position.y + rect.size.height <= current_clip_rect.y) return false;
    return true;
}

// Clipped rectangle
Rect get_clipped_rect(Rect rect) {
    Rect clipped = rect;
    
    if (clipped.position.x < current_clip_rect.x) {
        clipped.size.width -= (current_clip_rect.x - clipped.position.x);
        clipped.position.x = current_clip_rect.x;
    }
    if (clipped.position.y < current_clip_rect.y) {
        clipped.size.height -= (current_clip_rect.y - clipped.position.y);
        clipped.position.y = current_clip_rect.y;
    }
    
    if (clipped.position.x + clipped.size.width > current_clip_rect.x + current_clip_rect.width) {
        clipped.size.width = current_clip_rect.x + current_clip_rect.width - clipped.position.x;
    }
    if (clipped.position.y + clipped.size.height > current_clip_rect.y + current_clip_rect.height) {
        clipped.size.height = current_clip_rect.y + current_clip_rect.height - clipped.position.y;
    }
    
    return clipped;
}

// Draw widget with clipping
void draw_widget_clipped(Widget* widget) {
    if (!widget) return;
    
    if (!is_visible(widget->rect)) return;
    
    Rect clipped = get_clipped_rect(widget->rect);
    
    // Save current clip
    ClipRect old_clip = current_clip_rect;
    set_clip_rect(clipped.position.x, clipped.position.y, clipped.size.width, clipped.size.height);
    
    // Draw widget
    draw_widget(widget);
    
    // Restore clip
    current_clip_rect = old_clip;
}
```

#### Dirty Flag System

**Principle**: Only redraw widgets that have changed.

```c
typedef struct {
    bool dirty;              // Needs redraw
    Rect dirty_rect;        // Region to redraw
    bool children_dirty;    // Children need redraw
} DirtyState;

// Mark widget as dirty
void widget_mark_dirty(Widget* widget) {
    if (!widget) return;
    widget->dirty = true;
    widget->dirty_rect = widget->rect;
}

// Mark widget and all parents as dirty
void widget_mark_dirty_recursive(Widget* widget) {
    if (!widget) return;
    widget_mark_dirty(widget);
    if (widget->parent) {
        widget_mark_dirty_recursive(widget->parent);
    }
}

// Check if widget needs redraw
bool widget_needs_redraw(Widget* widget) {
    if (!widget) return false;
    if (widget->dirty) return true;
    // Check children
    for (uint8_t i = 0; i < widget->children_count; i++) {
        if (widget_needs_redraw(widget->children[i])) {
            return true;
        }
    }
    return false;
}

// Clear dirty flag
void widget_clear_dirty(Widget* widget) {
    if (!widget) return;
    widget->dirty = false;
    for (uint8_t i = 0; i < widget->children_count; i++) {
        widget_clear_dirty(widget->children[i]);
    }
}

// Optimized draw function
void draw_widget_tree_optimized(Widget* root) {
    if (!root || !widget_needs_redraw(root)) return;
    
    draw_widget(root);
    widget_clear_dirty(root);
    
    for (uint8_t i = 0; i < root->children_count; i++) {
        draw_widget_tree_optimized(root->children[i]);
    }
}
```

#### Double Buffering

**Principle**: Render to buffer first, then copy to screen to prevent flickering.

```c
// Double buffer for rendering
#define DOUBLE_BUFFER_WIDTH 320
#define DOUBLE_BUFFER_HEIGHT 240
uint16_t double_buffer[DOUBLE_BUFFER_WIDTH * DOUBLE_BUFFER_HEIGHT];

// Initialize double buffer
void init_double_buffer(void) {
    memset(double_buffer, 0, sizeof(double_buffer));
}

// Clear double buffer
void clear_double_buffer(Color color) {
    for (uint32_t i = 0; i < DOUBLE_BUFFER_WIDTH * DOUBLE_BUFFER_HEIGHT; i++) {
        double_buffer[i] = color;
    }
}

// Draw to double buffer
void draw_to_buffer(uint16_t x, uint16_t y, Color color) {
    if (x < DOUBLE_BUFFER_WIDTH && y < DOUBLE_BUFFER_HEIGHT) {
        double_buffer[y * DOUBLE_BUFFER_WIDTH + x] = color;
    }
}

// Flush buffer to TFT
void flush_double_buffer(void) {
    tft.startWrite();
    for (uint16_t y = 0; y < DOUBLE_BUFFER_HEIGHT; y++) {
        tft.writeData(double_buffer + y * DOUBLE_BUFFER_WIDTH, DOUBLE_BUFFER_WIDTH);
    }
    tft.endWrite();
}
```

### Precomputation and Caching

#### Gradient Cache

```c
#define MAX_GRADIENT_CACHE 10
#define GRADIENT_CACHE_SIZE 256

typedef struct {
    Color start_color;
    Color end_color;
    uint16_t length;
    Color colors[GRADIENT_CACHE_SIZE];
    bool valid;
} GradientCacheEntry;

GradientCacheEntry gradient_cache[MAX_GRADIENT_CACHE];

// Precompute gradient
Color* get_cached_gradient(Color start, Color end, uint16_t length) {
    for (int i = 0; i < MAX_GRADIENT_CACHE; i++) {
        if (gradient_cache[i].valid &&
            gradient_cache[i].start_color == start &&
            gradient_cache[i].end_color == end &&
            gradient_cache[i].length == length) {
            return gradient_cache[i].colors;
        }
    }
    
    // Find empty slot
    for (int i = 0; i < MAX_GRADIENT_CACHE; i++) {
        if (!gradient_cache[i].valid) {
            gradient_cache[i].start_color = start;
            gradient_cache[i].end_color = end;
            gradient_cache[i].length = length;
            gradient_cache[i].valid = true;
            
            // Compute gradient
            for (uint16_t j = 0; j < length; j++) {
                float ratio = (float)j / (length - 1);
                gradient_cache[i].colors[j] = interpolate_color(start, end, ratio);
            }
            
            return gradient_cache[i].colors;
        }
    }
    
    return NULL;  // Cache full
}
```

#### Text Width Cache

```c
#define MAX_TEXT_CACHE 50
typedef struct {
    char text[32];
    uint16_t width;
    uint8_t font_size;
    bool valid;
} TextWidthCacheEntry;

TextWidthCacheEntry text_width_cache[MAX_TEXT_CACHE];

// Get cached text width
uint16_t get_cached_text_width(const char* text, uint8_t font_size) {
    for (int i = 0; i < MAX_TEXT_CACHE; i++) {
        if (text_width_cache[i].valid &&
            text_width_cache[i].font_size == font_size &&
            strcmp(text_width_cache[i].text, text) == 0) {
            return text_width_cache[i].width;
        }
    }
    
    // Compute and cache
    for (int i = 0; i < MAX_TEXT_CACHE; i++) {
        if (!text_width_cache[i].valid) {
            strncpy(text_width_cache[i].text, text, 31);
            text_width_cache[i].text[31] = '\0';
            text_width_cache[i].font_size = font_size;
            text_width_cache[i].width = tft.textWidth(text, font_size);
            text_width_cache[i].valid = true;
            return text_width_cache[i].width;
        }
    }
    
    // Cache full, compute directly
    return tft.textWidth(text, font_size);
}
```

---

## Algorithm Optimizations

### Levenshtein Distance (Simplified)

**Principle**: Use single-row matrix to reduce memory usage from O(n*m) to O(m).

```c
// Standard Levenshtein (O(n*m) memory) - AVOID
// Uses full matrix, wastes RAM

// Optimized Levenshtein (O(m) memory)
uint8_t levenshtein_distance(const char* s1, const char* s2) {
    uint8_t len1 = strlen(s1);
    uint8_t len2 = strlen(s2);

    // If either string is empty, distance is length of the other
    if (len1 == 0) return len2;
    if (len2 == 0) return len1;

    // Use single row of matrix (memory optimization)
    uint8_t prev_row[MAX_WORD_LENGTH + 1];
    uint8_t curr_row[MAX_WORD_LENGTH + 1];

    for (uint8_t i = 0; i <= len2; i++) {
        prev_row[i] = i;
    }

    for (uint8_t i = 1; i <= len1; i++) {
        curr_row[0] = i;
        for (uint8_t j = 1; j <= len2; j++) {
            uint8_t cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            curr_row[j] = min(
                min(
                    prev_row[j] + 1,          // Deletion
                    curr_row[j - 1] + 1       // Insertion
                ),
                prev_row[j - 1] + cost      // Substitution
            );
        }
        // Copy curr_row to prev_row for next iteration
        for (uint8_t j = 0; j <= len2; j++) {
            prev_row[j] = curr_row[j];
        }
    }

    return curr_row[len2];
}
```

### String Operations (Avoid strcpy/strcat)

```c
// BAD - uses strcpy (slow, copies byte by byte)
void bad_copy_string(char* dest, const char* src) {
    strcpy(dest, src);
}

// GOOD - direct assignment when possible
void good_copy_string(char* dest, const char* src) {
    // Use memcpy for known lengths
    memcpy(dest, src, strlen(src) + 1);
}

// BAD - uses strcat (slow, searches for null terminator each time)
void bad_concat(char* dest, const char* src) {
    strcat(dest, src);
}

// GOOD - manual concatenation
void good_concat(char* dest, const char* src) {
    char* ptr = dest + strlen(dest);
    while (*src) {
        *ptr++ = *src++;
    }
    *ptr = '\0';
}
```

### Integer Math (Avoid Division)

```c
// BAD - uses division (slow on ESP8266)
uint16_t bad_divide(uint16_t value) {
    return value / 2;
}

// GOOD - uses bit shift
uint16_t good_divide(uint16_t value) {
    return value >> 1;
}

// BAD - uses modulo (slow)
uint8_t bad_modulo(uint16_t value) {
    return value % 10;
}

// GOOD - uses multiplication trick (for known divisors)
uint8_t good_modulo(uint16_t value) {
    // For modulo 10: multiply by 0.1 and get fractional part
    // Or use lookup table
    static const uint8_t mod10_table[256] = {0,1,2,3,4,5,6,7,8,9,0,1,...};
    return mod10_table[value & 0xFF] + mod10_table[(value >> 8) & 0xFF];
    // Simplified version:
    return value - ((value / 10) * 10);
}
```

---

## Power Optimization

### Sleep Modes

```c
// Put ESP8266 to sleep when idle
void enter_deep_sleep(uint32_t seconds) {
    Serial.println("Entering deep sleep...");
    ESP.deepSleep(seconds * 1000000);  // Microseconds
}

// Light sleep (wakes on touch interrupt)
void enter_light_sleep(void) {
    Serial.println("Entering light sleep...");
    WiFi.mode(WIFI_OFF);  // Turn off WiFi
    // Configure wakeup sources
    ESP.lightSleep();
}
```

### Peripheral Power Management

```c
// Turn off WiFi when not needed
void disable_wifi(void) {
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
}

void enable_wifi(void) {
    WiFi.forceSleepWake();
    WiFi.mode(WIFI_STA);
}

// Turn off SD card when not in use
void disable_sd_card(void) {
    digitalWrite(SD_CS, HIGH);
    SPI.end();
}

void enable_sd_card(void) {
    SPI.begin();
    digitalWrite(SD_CS, LOW);
}

// Dim display when idle
void dim_display(bool dim) {
    if (dim) {
        tft.setBrightness(50);  // If supported
    } else {
        tft.setBrightness(255);
    }
}
```

---

## SPI Bus Optimization

### SPI Sharing Between SD and TFT

```c
// CS pins
#define TFT_CS D8
#define TOUCH_CS D2
#define SD_CS D5

// Active device tracking
typedef enum {
    SPI_DEVICE_NONE,
    SPI_DEVICE_TFT,
    SPI_DEVICE_TOUCH,
    SPI_DEVICE_SD
} SPIDevice;

SPIDevice current_spi_device = SPI_DEVICE_NONE;

// Select device
void select_spi_device(SPIDevice device) {
    if (current_spi_device == device) return;
    
    // Deselect current
    switch (current_spi_device) {
        case SPI_DEVICE_TFT:
            digitalWrite(TFT_CS, HIGH);
            break;
        case SPI_DEVICE_TOUCH:
            digitalWrite(TOUCH_CS, HIGH);
            break;
        case SPI_DEVICE_SD:
            digitalWrite(SD_CS, HIGH);
            break;
    }
    
    // Select new
    switch (device) {
        case SPI_DEVICE_TFT:
            digitalWrite(TFT_CS, LOW);
            break;
        case SPI_DEVICE_TOUCH:
            digitalWrite(TOUCH_CS, LOW);
            break;
        case SPI_DEVICE_SD:
            digitalWrite(SD_CS, LOW);
            break;
    }
    
    current_spi_device = device;
}

// Optimized TFT write
void tft_write_pixel(uint16_t x, uint16_t y, Color color) {
    select_spi_device(SPI_DEVICE_TFT);
    tft.drawPixel(x, y, color);
}

// Optimized SD read
bool sd_read_optimized(const char* path, char* buffer, uint16_t size) {
    select_spi_device(SPI_DEVICE_SD);
    return sd_read_file(path, buffer, size);
}
```

---

## Touch Handling Optimization

### Single-Point Multi-Touch Simulation

Since XPT2046 doesn't support multi-touch, simulate gestures with single point:

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

typedef struct {
    uint16_t x;
    uint16_t y;
    uint32_t timestamp;
    bool pressed;
    uint32_t press_duration;
} TouchState;

// Gesture detection
GestureType detect_gesture(TouchState* state) {
    static uint16_t last_x = 0, last_y = 0;
    static uint32_t last_tap_time = 0;
    static uint32_t press_start = 0;
    
    if (state->pressed) {
        // Press started
        press_start = millis();
        return GESTURE_NONE;
    } else {
        // Press ended
        uint32_t duration = millis() - press_start;
        
        if (duration > 1000) {
            return GESTURE_LONG_PRESS;
        }
        
        // Check for double tap
        if (millis() - last_tap_time < 300) {
            last_tap_time = 0;
            return GESTURE_DOUBLE_TAP;
        }
        last_tap_time = millis();
        
        // Check for swipe
        int16_t dx = state->x - last_x;
        int16_t dy = state->y - last_y;
        
        if (abs(dx) > 50 && abs(dy) < 20) {
            if (dx > 0) return GESTURE_SWIPE_RIGHT;
            else return GESTURE_SWIPE_LEFT;
        }
        if (abs(dy) > 50 && abs(dx) < 20) {
            if (dy > 0) return GESTURE_SWIPE_DOWN;
            else return GESTURE_SWIPE_UP;
        }
        
        return GESTURE_TAP;
    }
    
    return GESTURE_NONE;
}
```

---

## Memory Management Patterns

### Accessor/Macro Pattern

```c
// Preferred pattern for memory management

// General release macro
#define RELEASE(obj) \
    if(obj) { free(obj); obj = NULL; }

// Widget-specific release
#define RELEASE_WIDGET(w) \
    if(w) { free_widget(w); w = NULL; }

// Button-specific release
#define RELEASE_BUTTON(b) \
    if(b) { \
        if((b)->on_click) (b)->on_click = NULL; \
        if((b)->on_release) (b)->on_release = NULL; \
        free(b); \
        b = NULL; \
    }

// Text field release
#define RELEASE_TEXTFIELD(tf) \
    if(tf) { \
        if((tf)->buffer) free((tf)->buffer); \
        if((tf)->suggestions) free((tf)->suggestions); \
        free(tf); \
        tf = NULL; \
    }

// Keyboard release
#define RELEASE_KEYBOARD(kb) \
    if(kb) { \
        if((kb)->keys) { \
            for(uint8_t i = 0; i < (kb)->keys_count; i++) { \
                RELEASE_BUTTON((kb)->keys[i]); \
            } \
            free((kb)->keys); \
        } \
        free(kb); \
        kb = NULL; \
    }
```

### Scope Guard Pattern

```c
// RAII-style scope guard for C
#define SCOPE_GUARD(name, cleanup) \
    __attribute__((cleanup(cleanup))) name

// Example usage
void function_that_needs_cleanup(void) {
    Widget* widget = new_widget(WIDGET_TYPE_BUTTON);
    SCOPE_GUARD(widget, widget_cleanup);
    
    // If we return early or throw, widget is automatically freed
    if (error_condition) return;
    
    // Use widget...
}

// Simple scope guard implementation
void widget_cleanup(Widget** widget_ptr) {
    if (widget_ptr && *widget_ptr) {
        free_widget(*widget_ptr);
    }
}
```

---

## Best Practices Summary

### Memory Management

1. **Pre-allocate** all buffers at startup
2. **Use pools** for frequently created/destroyed objects
3. **Avoid dynamic allocation** in touch handlers and render loops
4. **Free resources** immediately when no longer needed
5. **Use stack allocation** for small, short-lived objects

### Performance

1. **Use clipping** to avoid rendering off-screen widgets
2. **Use dirty flags** to avoid redrawing unchanged widgets
3. **Cache computations** (gradients, text widths)
4. **Batch draw operations** to minimize TFT_eSPI calls
5. **Avoid floating-point** math when possible (use fixed-point)

### Power

1. **Turn off peripherals** when not in use (WiFi, SD card)
2. **Use sleep modes** when idle
3. **Dim display** when inactive
4. **Minimize SPI bus switching**

### Code Structure

1. **Keep functions small** (easier for compiler to optimize)
2. **Use const** for read-only parameters
3. **Use static** for local buffers
4. **Avoid recursion** (stack space is limited)
5. **Use bit operations** instead of arithmetic when possible

---

## Cross-References

- **Memory Management**: See `06_MEMORY_MANAGEMENT.md` and `docs/MEMORY_MANAGEMENT.md`
- **Widget Implementations**: See `09_WIDGET_IMPLEMENTATIONS.md`
- **Rendering System**: See `07_RENDERING_SYSTEM.md`
- **SD Card/WebDAV**: See `11_SD_CARD_WEBDAV.md`

---

## File Locations

Optimization implementations:
- `src/gui/widget_pool.h` - Object pool declarations
- `src/gui/widget_pool.c` - Object pool implementation
- `src/gui/renderer_optimized.h` - Optimized renderer with clipping
- `src/gui/renderer_optimized.cpp` - Optimized renderer implementation
