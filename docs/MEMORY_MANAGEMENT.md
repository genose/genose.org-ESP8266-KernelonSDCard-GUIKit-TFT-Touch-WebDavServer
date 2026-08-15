# Memory Management Guide

This document describes the **Objective-C-style memory management** patterns used in GUIKit, optimized for ESP8266 constraints.

> **⚠️ Important**: Traditional reference counting (ARC-style) is **NOT recommended** for ESP8266 due to:
> - Atomic operations overhead (slow on limited hardware)
> - RAM overhead (each object needs refcount field)
> - Complexity in constrained environments
> - Only ~80KB RAM available

---

## 📋 Table of Contents

1. [Memory Management Philosophy](#memory-management-philosophy)
2. [Object Pooling](#1-object-pooling)
3. [Static Text Buffers](#2-static-text-buffers)
4. [Accessor Pattern](#3-accessor-pattern)
5. [Macro-Based Syntax](#4-macro-based-syntax)
6. [Scope Guards (RAII for C)](#5-scope-guards-rai-for-c)
7. [Builder Pattern](#6-builder-pattern)
8. [Comparison Table](#comparison-table)
9. [Best Practices](#best-practices)

---

## Memory Management Philosophy

### Why Not Reference Counting?

| Issue | Problem on ESP8266 |
|-------|-------------------|
| **Atomic Operations** | Slow on 80MHz processor, no hardware support |
| **RAM Overhead** | 2+ bytes per object for refcount (significant with many widgets) |
| **Complexity** | Requires careful tracking across all code paths |
| **Fragmentation** | Doesn't solve memory fragmentation issues |

### ESP8266 Constraints

- **RAM**: ~80KB total
- **Flash**: 512KB - 1MB (but kernel loaded from SD)
- **No MMU**: No virtual memory
- **No ARC**: No automatic reference counting

### Recommended Approach

**Use Object Pooling + Static Buffers**:
- ✅ **Predictable memory usage** - Known maximum at compile time
- ✅ **Zero fragmentation** - All allocations from pre-allocated pools
- ✅ **Deterministic** - No malloc/free overhead
- ✅ **Fast** - O(1) allocation with simple pointer arithmetic
- ✅ **Safe** - No leaks, no double-frees
- ✅ **Objective-C friendly** - Can use accessor/macro patterns

---

## 1. Object Pooling

**Best for**: Widgets, buttons, labels, and other UI elements

### Implementation

```c
// widget_pool.h
#pragma once
#include <stdint.h>
#include <stdbool.h>

#define MAX_WIDGETS 30
#define MAX_BUTTONS 20
#define MAX_LABELS 20

typedef struct Widget Widget;
typedef struct WidgetButton WidgetButton;

// Pooled widget types
typedef struct {
    Widget base;
    bool in_use;
} PooledWidget;

typedef struct {
    WidgetButton base;
    bool in_use;
} PooledButton;

typedef struct {
    WidgetLabel base;
    bool in_use;
} PooledLabel;

// Global pools - defined in widget_pool.c
extern PooledWidget widget_pool[MAX_WIDGETS];
extern PooledButton button_pool[MAX_BUTTONS];
extern PooledLabel label_pool[MAX_LABELS];

// Initialize all pools
void pools_init(void);

// Allocate from appropriate pool
Widget* widget_alloc(WIDGET_TYPE type);

// Release back to pool
void widget_free(Widget* w);

// Safe release macro (Objective-C style)
#define SAFE_RELEASE(w) do { if(w) { widget_free(w); w = NULL; } } while(0)
```

### Usage

```c
// Initialize pools at startup
pools_init();

// Allocate a button
Widget* btn = widget_alloc(WIDGET_TYPE_BUTTON);
if (!btn) {
    // Pool exhausted - handle error
    return;
}

// Use the button...

// Release when done
SAFE_RELEASE(btn);
```

### Benefits

- **Zero RAM overhead** beyond the pool itself
- **O(1) allocation** with linear scan (can be optimized with bitmap)
- **Deterministic cleanup** - no fragmentation
- **No atomic operations** - simple boolean flags
- **Predictable** - known maximum memory usage at compile time

---

## 2. Static Text Buffers

**Best for**: Widget text, labels, button titles

### Implementation

```c
// widget_text.h
#pragma once
#include <stdint.h>

#define MAX_TEXT_LENGTH 512  // Shared for all widgets

typedef struct {
    char text[MAX_TEXT_LENGTH];  // Fixed buffer - no malloc needed
    uint8_t font_size;
    uint16_t font_color;
} WidgetText;

// Safe text assignment macro
#define WidgetSetText(wt, str) do { \
    if ((str)) { \
        strncpy((wt)->text, (str), MAX_TEXT_LENGTH - 1); \
        (wt)->text[MAX_TEXT_LENGTH - 1] = '\0'; \
    } else { \
        (wt)->text[0] = '\0'; \
    } \
} while(0)

// Safe text access
const char* WidgetGetText(const WidgetText* wt);

// Check if text is empty
bool WidgetTextIsEmpty(const WidgetText* wt);
```

### Usage

```c
WidgetText text;

// Set text - automatically handles NULL and truncation
WidgetSetText(&text, "Hello World!");
WidgetSetText(&text, very_long_string);  // Truncated to MAX_TEXT_LENGTH-1
WidgetSetText(&text, NULL);  // Clears text

// Get text
const char* msg = WidgetGetText(&text);

// Check if empty
if (WidgetTextIsEmpty(&text)) {
    // Handle empty text
}
```

### Benefits

- **No heap allocation** - text stored in struct
- **No free() needed** - automatic cleanup
- **Bounds checked** - prevents buffer overflows
- **Fast** - just memcpy/memset operations
- **Thread-safe** - no dynamic memory operations

---

## 3. Accessor Pattern

**Best for**: Objective-C developers, clean API, encapsulation

### Implementation

```c
// widget.h - Public API with accessors
#pragma once
#include "widget_text.h"

typedef enum {
    WIDGET_TYPE_VIEW,
    WIDGET_TYPE_BUTTON,
    WIDGET_TYPE_LABEL,
    WIDGET_TYPE_SLIDER
} WIDGET_TYPE;

typedef struct Widget Widget;

// Constructor - like [[Widget alloc] init]
Widget* Widget_new(WIDGET_TYPE type);

// Destructor - like [widget release]
void Widget_delete(Widget* self);

// Text accessors
void Widget_setText(Widget* self, const char* text);
const char* Widget_getText(const Widget* self);

// Geometry accessors
void Widget_setFrame(Widget* self, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
void Widget_getFrame(const Widget* self, uint16_t* x, uint16_t* y, uint16_t* width, uint16_t* height);

// Style accessors
void Widget_setBackgroundColor(Widget* self, uint16_t color);
uint16_t Widget_getBackgroundColor(const Widget* self);
void Widget_setBorderColor(Widget* self, uint16_t color);
void Widget_setBorderWidth(Widget* self, uint8_t width);

// Children accessors
void Widget_addChild(Widget* self, Widget* child);
void Widget_removeChild(Widget* self, Widget* child);
Widget* Widget_getChild(const Widget* self, uint8_t index);
uint8_t Widget_getChildrenCount(const Widget* self);

// Rendering
void Widget_setDirty(Widget* self, bool dirty);
bool Widget_isDirty(const Widget* self);
void Widget_render(Widget* self);
```

### Usage

```c
// Create and configure widget using accessors
Widget* button = Widget_new(WIDGET_TYPE_BUTTON);
Widget_setText(button, "Click Me");
Widget_setFrame(button, 10, 20, 100, 40);
Widget_setBackgroundColor(button, 0x00FF00);

// Get values
const char* text = Widget_getText(button);
uint16_t x, y, w, h;
Widget_getFrame(button, &x, &y, &w, &h);

// Cleanup
Widget_delete(button);
```

### Benefits

- **Encapsulation** - Internal representation hidden
- **Type safety** - Compile-time checking
- **Familiar to Objective-C devs** - Similar method naming
- **Maintainable** - Clear API boundaries
- **Extensible** - Easy to add new properties

---

## 4. Macro-Based Syntax

**Best for**: Concise code, Objective-C-like chaining

### Implementation

```c
// widget_macros.h
#pragma once
#include "widget.h"

// NEW_WIDGET(type) - like [[Widget alloc] initWithType:type]
#define NEW_WIDGET(type) Widget_new(type)

// WITH_TEXT(widget, text) - like widget.text = @"text"
#define WITH_TEXT(widget, text) Widget_setText((widget), (text))

// WITH_FRAME(widget, x, y, w, h) - like widget.frame = CGRectMake(x,y,w,h)
#define WITH_FRAME(widget, x, y, w, h) Widget_setFrame((widget), (x), (y), (w), (h))

// WITH_COLOR(widget, color) - like widget.backgroundColor
#define WITH_COLOR(widget, color) Widget_setBackgroundColor((widget), (color))

// WITH_BORDER(widget, color, width)
#define WITH_BORDER(widget, color, width) do { \
    Widget_setBorderColor((widget), (color)); \
    Widget_setBorderWidth((widget), (width)); \
} while(0)

// ADD_TO(parent, child) - like [parent addSubview:child]
#define ADD_TO(parent, child) Widget_addChild((parent), (child))

// SAFE_RELEASE - like [widget release], widget = nil
#define SAFE_RELEASE(widget) do { if(widget) { Widget_delete(widget); widget = NULL; } } while(0)

// Button-specific macros
#define NEW_BUTTON NEW_WIDGET(WIDGET_TYPE_BUTTON)
#define BUTTON_SET_TITLE(btn, title) WITH_TEXT(btn, title)
#define BUTTON_SET_ACTION(btn, callback) ((WidgetButton*)(btn))->onClick = (callback)
```

### Usage

```c
// Chainable, concise syntax
Widget* view = NEW_WIDGET(WIDGET_TYPE_VIEW)
    WITH_FRAME(view, 0, 0, 240, 320);

Widget* btn = NEW_BUTTON
    WITH_FRAME(btn, 10, 20, 100, 40)
    WITH_TEXT(btn, "OK")
    WITH_COLOR(btn, 0x00FF00)
    ADD_TO(view, btn);

// Cleanup
SAFE_RELEASE(view);  // Also releases children

// Button with action
BUTTON_SET_ACTION(btn, my_callback);
```

### Benefits

- **Concise** - Less boilerplate
- **Readable** - Intent is clear
- **Familiar** - Similar to Objective-C method chaining
- **Type-safe** - Still has compile-time checking
- **Fast** - No runtime overhead (macros expand at compile time)

---

## 5. Scope Guards (RAII for C)

**Best for**: Temporary objects, automatic cleanup

### Implementation

```c
// scope_guard.h
#pragma once

typedef void (*CleanupFunc)(void*);

typedef struct {
    void* object;
    CleanupFunc cleanup;
} ScopeGuard;

// Create guard - cleanup when scope exits
// Use __attribute__((cleanup)) if available, otherwise manual

#if defined(__GNUC__) || defined(__clang__)
    // Compiler-supported cleanup (GCC/clang extension)
    #define ON_SCOPE_EXIT(obj, func) \
        __attribute__((cleanup(func))) obj
#else
    // Manual scope guard
    typedef struct {
        void* obj;
        CleanupFunc func;
    } ScopeGuard;
    
    static inline ScopeGuard __make_guard(void* obj, CleanupFunc func) {
        return (ScopeGuard){obj, func};
    }
    
    static inline void __guard_cleanup(ScopeGuard* sg) {
        if (sg->obj && sg->func) sg->func(sg->obj);
    }
    
    #define ON_SCOPE_EXIT(obj, func) \
        for (ScopeGuard __sg = __make_guard(obj, func); \
             __sg.obj; \
             __guard_cleanup(&__sg), __sg.obj = NULL) \
            for (int __i = 0; __i < 1; __i++)

#endif

// Widget-specific cleanup
void widget_cleanup(void* w);
```

### Usage

```c
// Automatic cleanup at scope exit
{
    ON_SCOPE_EXIT(btn, widget_cleanup);
    Widget* btn = Widget_new(WIDGET_TYPE_BUTTON);
    Widget_setText(btn, "Temporary");
    // ... use btn ...
    // btn automatically deleted when scope exits
}

// Manual cleanup (portable version)
{
    ON_SCOPE_EXIT(btn, Widget_delete);
    Widget* btn = Widget_new(WIDGET_TYPE_BUTTON);
    // ... use btn ...
}
```

### Benefits

- **Automatic cleanup** - No need to remember to free
- **Exception-safe** - Cleanup happens even if code throws/returns
- **RAII pattern** - Familiar to C++/Objective-C developers
- **Minimal overhead** - Just a function pointer call

---

## 6. Builder Pattern

**Best for**: Complex widget creation, fluent API

### Implementation

```c
// widget_builder.h
#pragma once
#include "widget.h"

typedef Widget* WidgetChain;

// Constructor
WidgetChain Widget_create(WIDGET_TYPE type);

// Chainable setters
WidgetChain Widget_chain_setText(WidgetChain self, const char* text);
WidgetChain Widget_chain_setFrame(WidgetChain self, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
WidgetChain Widget_chain_setColor(WidgetChain self, uint16_t color);
WidgetChain Widget_chain_setBorder(WidgetChain self, uint16_t color, uint8_t width);
WidgetChain Widget_chain_addToParent(WidgetChain self, Widget* parent);

// Macro for even cleaner syntax
#define CHAIN(widget) (widget)
```

### Usage

```c
// Method chaining style
Widget* button = Widget_create(WIDGET_TYPE_BUTTON)
    ->chain_setText("Click Me")
    ->chain_setFrame(10, 20, 100, 40)
    ->chain_setColor(0x00FF00)
    ->chain_setBorder(0xFFFFFF, 1)
    ->chain_addToParent(view);

// With CHAIN macro for clarity
Widget* label = CHAIN(Widget_create(WIDGET_TYPE_LABEL))
    ->chain_setText("Hello")
    ->chain_setFrame(50, 50, 200, 30);
```

### Benefits

- **Fluent API** - Very readable, chainable
- **Familiar** - Similar to Objective-C builder patterns
- **Flexible** - Easy to extend with new chainable methods
- **Type-safe** - Each method returns WidgetChain

---

## Comparison Table

| Approach | RAM Overhead | Speed | Safety | Code Style | ESP8266 Friendly | Recommended |
|----------|--------------|-------|--------|------------|------------------|-------------|
| malloc/free | ❌ Fragmentation | ⚡ Fast | ❌ Error-prone | ❌ C-style | ❌ No | ❌ No |
| Reference Counting | ❌ High (per-object) | ❌ Slow (atomic) | ✅ Safe | ✅ Obj-C | ❌ **No** | ❌ **No** |
| **Object Pooling** | ✅ **Predictable** | ⚡⚡ **Fastest** | ✅ **Safe** | ✅ **Obj-C-like** | ✅✅ **Best** | ✅✅ **YES** |
| Static Buffers | ✅ **Zero** | ⚡⚡ Fastest | ✅ Safe | ✅ Clean | ✅✅ Best | ✅✅ **YES** |
| Accessor Pattern | ✅ Minimal | ⚡ Fast | ✅ Safe | ✅✅ Very Obj-C | ✅ Yes | ✅ **YES** |
| Macro-Based | ✅ None | ⚡⚡ Fastest | ✅ Safe | ✅✅ Very Obj-C | ✅ Yes | ✅✅ **YES** |
| Scope Guards | ✅ Minimal | ⚡ Fast | ✅ Safe | ✅ RAII-like | ✅ Yes | ✅ Good |
| Builder Pattern | ✅ None | ⚡ Fast | ✅ Safe | ✅✅ Very Obj-C | ✅ Yes | ✅ Good |

---

## Best Practices

### Do This ✅

```c
// ✅ Use pooling for widgets
Widget* btn = Widget_new(WIDGET_TYPE_BUTTON);

// ✅ Use static buffers for text
WidgetText text;
WidgetSetText(&text, "Hello");

// ✅ Use accessor methods
Widget_setText(btn, "Click");
Widget_setFrame(btn, 0, 0, 100, 50);

// ✅ Use macros for concise code
Widget* view = NEW_WIDGET(WIDGET_TYPE_VIEW)
    WITH_FRAME(view, 0, 0, 240, 320);

// ✅ Use SAFE_RELEASE
SAFE_RELEASE(btn);

// ✅ Use scope guards for temporaries
{
    ON_SCOPE_EXIT(temp, Widget_delete);
    Widget* temp = Widget_new(WIDGET_TYPE_LABEL);
    // ...
} // Auto-cleanup
```

### Don't Do This ❌

```c
// ❌ malloc/free - fragmentation, error-prone
Widget* btn = malloc(sizeof(Widget));
free(btn);

// ❌ Reference counting - atomic ops, RAM overhead
btn->refcount++;
if (btn->refcount == 0) free(btn);

// ❌ Unbounded strdup - fragmentation, no bounds
char* text = strdup(user_input);

// ❌ Manual memcpy without bounds - buffer overflow
strcpy(widget->text, long_string);

// ❌ Forgetting to free - memory leak
Widget* btn = Widget_new(...);
// Forgot to free!
```

---

## Recommended Implementation Strategy

### For ESP8266 GUIKit, use this combination:

1. **Object Pooling** for all widgets
   - Fixed number of each widget type
   - Allocated at startup
   - No malloc/free during runtime

2. **Static Text Buffers** for all text
   - Fixed-size char arrays
   - Safe string copy macros
   - No dynamic allocation

3. **Accessor Pattern** for public API
   - Clean, encapsulated interface
   - Familiar to Objective-C developers
   - Type-safe

4. **Macros** for convenient syntax
   - Chainable operations
   - Concise code
   - No runtime overhead

5. **Scope Guards** for automatic cleanup
   - RAII pattern in C
   - Exception-safe
   - Automatic resource management

### File Structure

```
src/gui/
├── widget.h          # Public API with accessors
├── widget.c          # Implementation
├── widget_pool.h     # Pool declarations
├── widget_pool.c     # Pool implementation
├── widget_macros.h   # Macro-based syntax helpers
├── widget_builder.h  # Builder pattern
└── scope_guard.h     # RAII scope guards

docs/
└── MEMORY_MANAGEMENT.md  # This file
```

---

## Migration Guide

### From Manual Memory Management

**Before:**
```c
Widget* btn = malloc(sizeof(Widget));
strcpy(btn->text, "Click");
// ... use btn ...
free(btn);
```

**After:**
```c
Widget* btn = Widget_new(WIDGET_TYPE_BUTTON);
Widget_setText(btn, "Click");
// ... use btn ...
SAFE_RELEASE(btn);
```

### From strdup

**Before:**
```c
char* text = strdup(user_input);
// ... use text ...
free(text);
```

**After:**
```c
WidgetText text;
WidgetSetText(&text, user_input);
// ... use text.text ...
// No free needed!
```

---

## Summary

**For ESP8266, avoid reference counting (ARC) and instead use:**

1. ✅ **Object Pooling** - Predictable, fast, safe
2. ✅ **Static Buffers** - Zero overhead, bounds-checked
3. ✅ **Accessors** - Clean API, encapsulated
4. ✅ **Macros** - Concise, Objective-C-like
5. ✅ **Scope Guards** - Automatic cleanup

This gives you **Objective-C developer experience** with **C performance** and **ESP8266 constraints**. Memory management becomes **deterministic, safe, and efficient** without the overhead of reference counting.

---

*See [ARCHITECTURE.md](ARCHITECTURE.md) for overall system architecture*
*See [SOFTWARE.md](SOFTWARE.md) for widget system details*

---

*Generated for ESP8266 GUIKit by Mistral Vibe*
