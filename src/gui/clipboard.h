/**
 * @file clipboard.h
 * @brief Clipboard system for GUIKit
 * @author GUIKit for ESP8266
 * @date 2026
 * 
 * Clipboard system for copy/paste operations across text widgets.
 * Uses static buffer to avoid dynamic memory allocation.
 * 
 * ESP8266-friendly: No malloc/free, fixed-size buffer
 */

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <stdint.h>
#include <stdbool.h>

// Maximum clipboard content size
#define CLIPBOARD_BUFFER_SIZE 4096  // 4K to match textfield capacity

/**
 * @brief Initialize clipboard system
 */
void clipboard_init(void);

/**
 * @brief Set clipboard content
 * 
 * @param text Text to copy to clipboard (NULL to clear)
 */
void clipboard_set(const char* text);

/**
 * @brief Get clipboard content
 * 
 * @return Pointer to clipboard text, or NULL if empty
 */
const char* clipboard_get(void);

/**
 * @brief Check if clipboard has content
 * 
 * @return true if clipboard has content
 */
bool clipboard_has_content(void);

/**
 * @brief Clear clipboard
 */
void clipboard_clear(void);

/**
 * @brief Copy text to clipboard
 * 
 * @param text Text to copy
 */
void clipboard_copy(const char* text);

/**
 * @brief Cut text to clipboard (copy + clear source)
 * 
 * @param text Text to cut
 */
void clipboard_cut(const char* text);

/**
 * @brief Paste from clipboard
 * 
 * @return Pointer to clipboard text, or NULL if empty
 */
const char* clipboard_paste(void);

/**
 * @brief Get clipboard text length
 * 
 * @return Length of clipboard content
 */
size_t clipboard_get_length(void);

#endif // CLIPBOARD_H
