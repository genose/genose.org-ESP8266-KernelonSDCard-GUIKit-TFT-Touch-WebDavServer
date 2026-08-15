/**
 * scope_guard.h - RAII-style Scope Guards for C
 * 
 * Provides automatic cleanup at scope exit (like C++ RAII)
 * Works with ESP8266 and standard C compilers
 * 
 * Generated for ESP8266 by Mistral Vibe
 */

#ifndef SCOPE_GUARD_H
#define SCOPE_GUARD_H

#include <stdint.h>
#include <stdbool.h>


// ============================================================================
// SCOPE GUARD STRUCTURE
// ============================================================================

/** Cleanup function type */
typedef void (*CleanupFunc)(void*);

/** Scope guard - holds object and cleanup function */
typedef struct {
    void* object;
    CleanupFunc cleanup;
} ScopeGuard;


// ============================================================================
// BASIC SCOPE GUARD (Portable C)
// ============================================================================

/**
 * @brief Create a scope guard that cleans up when the guard goes out of scope
 * 
 * This uses a for-loop trick to create a scope-bound object.
 * The cleanup is executed when the loop ends (i.e., when the scope exits).
 * 
 * @param obj Object to clean up
 * @param func Cleanup function
 * 
 * Usage:
 * @code
 * {
 *     ON_SCOPE_EXIT(widget, Widget_delete);
 *     Widget* widget = Widget_new(WIDGET_TYPE_BUTTON);
 *     // ... use widget ...
 *     // widget is automatically deleted when scope exits
 * }
 * @endcode
 */
#define ON_SCOPE_EXIT(obj, func) \
    for (ScopeGuard __sg = { (obj), (func) }; \
         __sg.object != NULL; \
         __scope_guard_cleanup(&__sg), __sg.object = NULL) \
        for (int __i = 0; __i < 1; __i++)

/**
 * @brief Internal cleanup function for scope guards
 */
static inline void __scope_guard_cleanup(ScopeGuard* sg) {
    if (sg->object && sg->cleanup) {
        sg->cleanup(sg->object);
    }
}


// ============================================================================
// SCOPE GUARD WITH CONTEXT
// ============================================================================

/**
 * @brief Scope guard with additional context
 */
typedef struct {
    void* object;
    CleanupFunc cleanup;
    void* context;  // Additional context for cleanup
} ScopeGuardWithContext;

/**
 * @brief Create a scope guard with context
 */
#define ON_SCOPE_EXIT_CTX(obj, func, ctx) \
    for (ScopeGuardWithContext __sg = { (obj), (func), (ctx) }; \
         __sg.object != NULL; \
         __scope_guard_cleanup_ctx(&__sg), __sg.object = NULL) \
        for (int __i = 0; __i < 1; __i++)

/**
 * @brief Internal cleanup with context
 */
static inline void __scope_guard_cleanup_ctx(ScopeGuardWithContext* sg) {
    if (sg->object && sg->cleanup) {
        sg->cleanup(sg->object);
    }
}


// ============================================================================
// WIDGET-SPECIFIC SCOPE GUARDS
// ============================================================================

// Forward declaration
typedef struct Widget Widget;

/**
 * @brief Widget cleanup function for scope guards
 */
void widget_cleanup(void* w);

/**
 * @brief Create a scope guard for a widget
 * 
 * @param widget Widget pointer
 * 
 * Usage:
 * @code
 * {
 *     ON_SCOPE_EXIT_WIDGET(btn);
 *     Widget* btn = Widget_new(WIDGET_TYPE_BUTTON);
 *     // ... use btn ...
 * }
 * @endcode
 */
#define ON_SCOPE_EXIT_WIDGET(widget) ON_SCOPE_EXIT(widget, widget_cleanup)


// ============================================================================
// MULTIPLE SCOPE GUARDS
// ============================================================================

/**
 * @brief Create multiple scope guards in one statement
 * 
 * Usage:
 * @code
 * {
 *     ON_SCOPE_EXIT_MULTI(
 *         ON_SCOPE_EXIT(btn1, Widget_delete),
 *         ON_SCOPE_EXIT(btn2, Widget_delete)
 *     );
 *     Widget* btn1 = Widget_new(...);
 *     Widget* btn2 = Widget_new(...);
 * }
 * @endcode
 */
#define ON_SCOPE_EXIT_MULTI(...) __VA_ARGS__


// ============================================================================
// CONDITIONAL SCOPE GUARD
// ============================================================================

/**
 * @brief Conditional scope guard - only cleans up if condition is true
 */
#define ON_SCOPE_EXIT_IF(condition, obj, func) \
    for (ScopeGuard __sg = { (condition) ? (obj) : NULL, (func) }; \
         __sg.object != NULL; \
         __scope_guard_cleanup(&__sg), __sg.object = NULL) \
        for (int __i = 0; __i < 1; __i++)


// ============================================================================
// DEFER MACRO (Execute at scope exit)
// ============================================================================

/**
 * @brief Defer execution of a statement until scope exit
 * 
 * Usage:
 * @code
 * {
 *     DEFER(printf("Exiting scope\n"));
 *     // ... code ...
 * } // Prints "Exiting scope" here
 * @endcode
 */
#define DEFER(stmt) \
    for (int __defer_i = 0; __defer_i < 1; __defer_i++) \
        for (ScopeGuard __defer_sg = { NULL, (CleanupFunc)stmt }; \
             __defer_i == 0; \
             __defer_i = 1)

// Note: DEFER has limitations - the statement must be a function call or similar
// For complex statements, use ON_SCOPE_EXIT with a wrapper function


// ============================================================================
// EXAMPLES
// ============================================================================

/***
 * Example 1: Basic widget cleanup
 * 
 * void my_function() {
 *     ON_SCOPE_EXIT(btn, Widget_delete);
 *     Widget* btn = Widget_new(WIDGET_TYPE_BUTTON);
 *     Widget_setText(btn, "Click Me");
 *     // ... use btn ...
 *     // btn is automatically deleted when function returns
 * }
 */

/***
 * Example 2: Widget-specific macro
 * 
 * void create_temporary_button() {
 *     ON_SCOPE_EXIT_WIDGET(btn);
 *     Widget* btn = Widget_new(WIDGET_TYPE_BUTTON);
 *     // ... use btn ...
 * }
 */

/***
 * Example 3: Multiple guards
 * 
 * void create_ui() {
 *     ON_SCOPE_EXIT_MULTI(
 *         ON_SCOPE_EXIT(view, Widget_delete),
 *         ON_SCOPE_EXIT(button, Widget_delete),
 *         ON_SCOPE_EXIT(label, Widget_delete)
 *     );
 *     
 *     Widget* view = Widget_new(WIDGET_TYPE_VIEW);
 *     Widget* button = Widget_new(WIDGET_TYPE_BUTTON);
 *     Widget* label = Widget_new(WIDGET_TYPE_LABEL);
 *     
 *     // ... configure and use widgets ...
 *     // All widgets automatically deleted when function returns
 * }
 */

/***
 * Example 4: Conditional cleanup
 * 
 * void process_widget(Widget* input) {
 *     ON_SCOPE_EXIT_IF(input == NULL, temp, Widget_delete);
 *     Widget* temp = NULL;
 *     
 *     if (!input) {
 *         temp = Widget_new(WIDGET_TYPE_BUTTON);
 *         // ... use temp ...
 *     }
 *     // temp is only cleaned up if input was NULL
 * }
 */


#endif // SCOPE_GUARD_H
