# Memory Management

> This document references the main memory management documentation in `docs/MEMORY_MANAGEMENT.md`

## Overview

This document provides a reference to the complete memory management concepts for GUIKit. For detailed information, please see the main memory management documentation.

**IMPORTANT**: As per project constraints for ESP8266 (80KB RAM limit, no atomic ops):
- **DO NOT** use ARC (Automatic Reference Counting)
- **DO NOT** use reference counting
- **USE** manual memory management with accessors/macros
- **USE** object pooling where appropriate
- **USE** explicit `free()` calls or pool release macros

---

## Key Concepts

See `docs/MEMORY_MANAGEMENT.md` for complete details on:

### Memory Management Philosophy
- Why NOT to use ARC/reference counting on ESP8266
- Memory constraints (80KB RAM total)
- No atomic operations support
- Manual memory management principles

### Object Pooling
- Widget pool implementation
- Text field pool implementation
- File info pool implementation
- Pool-based constructors
- Pool release functions

### Accessor/Macro Pattern
- RELEASE() macro
- Widget-specific release macros (RELEASE_WIDGET, RELEASE_BUTTON, etc.)
- General memory management patterns

### Static Buffers
- Pre-allocated buffers for text, clipboard, file operations
- Avoiding dynamic allocation in critical paths
- Memory-efficient data structures

### Scope Guards
- RAII-style memory management for C
- Automatic cleanup on scope exit

---

## Quick Reference

### Memory Management Macros

```c
// General release
#define RELEASE(obj) \
    if(obj) { free(obj); obj = NULL; }

// Widget-specific
#define RELEASE_WIDGET(w) \
    if(w) { free_widget(w); w = NULL; }

#define RELEASE_BUTTON(b) \
    if(b) { \
        if((b)->on_click) (b)->on_click = NULL; \
        if((b)->on_release) (b)->on_release = NULL; \
        free(b); \
        b = NULL; \
    }

#define RELEASE_LABEL(l) \
    if(l) { free_widget(&(l)->base); l = NULL; }

#define RELEASE_TEXTFIELD(tf) \
    if(tf) { \
        if((tf)->buffer) free((tf)->buffer); \
        if((tf)->suggestions) free((tf)->suggestions); \
        free(tf); \
        tf = NULL; \
    }
```

### Pool-Based Allocation

```c
// Widget pool
Widget* widget_pool_alloc(WIDGET_TYPE type);
void widget_pool_release(Widget* widget);

// Text field pool
WidgetTextField* textfield_pool_alloc(TEXTFIELD_STYLE style);
void textfield_pool_release(WidgetTextField* textfield);

// Pool-based constructors
WidgetButton* new_button_pooled(void);
WidgetLabel* new_label_pooled(const char* text);
```

---

## Cross-References

- **Main Memory Management**: See `docs/MEMORY_MANAGEMENT.md`
- **Widget Implementations**: See `09_WIDGET_IMPLEMENTATIONS.md`
- **Optimizations**: See `12_OPTIMIZATIONS.md` (Memory Optimization section)
- **Advanced Features**: See `14_ADVANCED_FEATURES.md`

---

## File Locations

- `docs/MEMORY_MANAGEMENT.md` - Main memory management documentation
- `src/gui/widget_pool.h` - Widget pool declarations
- `src/gui/widget_pool.c` - Widget pool implementation
- `src/gui/widget_macros.h` - Memory management macros
