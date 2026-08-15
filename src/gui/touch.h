/**
 * @file touch.h
 * @brief Touch handling interface for GUIKit
 * @author GUIKit for ESP8266
 * @date 2026
 * 
 * Interface for XPT2046 touchscreen controller
 * ESP8266-friendly: Optimized for embedded touch input
 */

#ifndef TOUCH_H
#define TOUCH_H

#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// TOUCH STATE
// =============================================================================

/** Touch event callback type */
typedef void (*TouchCallback)(uint16_t x, uint16_t y, bool pressed);

/**
 * @brief Initialize touch system
 */
void touch_init(void);

/**
 * @brief Update touch state (call regularly)
 * 
 * @return true if touch state changed
 */
bool touch_update(void);

/**
 * @brief Get current touch X coordinate
 * 
 * @return X coordinate (0-319 for 320px screen)
 */
uint16_t touch_get_x(void);

/**
 * @brief Get current touch Y coordinate
 * 
 * @return Y coordinate (0-239 for 240px screen)
 */
uint16_t touch_get_y(void);

/**
 * @brief Check if touch is currently pressed
 * 
 * @return true if touched
 */
bool touch_is_pressed(void);

/**
 * @brief Register touch callback
 * 
 * @param callback Function to call on touch events
 */
void touch_set_callback(TouchCallback callback);

/**
 * @brief Set touch calibration parameters
 * 
 * @param x_min Minimum X raw value
 * @param x_max Maximum X raw value
 * @param y_min Minimum Y raw value
 * @param y_max Maximum Y raw value
 */
void touch_set_calibration(uint16_t x_min, uint16_t x_max, uint16_t y_min, uint16_t y_max);

#endif // TOUCH_H
