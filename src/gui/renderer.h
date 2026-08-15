/**
 * @file renderer.h
 * @brief Rendering functions for GUIKit
 * @author GUIKit for ESP8266
 * @date 2026
 * 
 * Minimal rendering interface for TFT_eSPI
 * ESP8266-friendly: Optimized for embedded displays
 */

#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>
#include <stdbool.h>

// Color type (RGBA565 format)
#ifndef COLOR_DEFINED
#define COLOR_DEFINED
typedef uint16_t Color;
#endif

// =============================================================================
// BASIC DRAWING FUNCTIONS
// =============================================================================

/**
 * @brief Draw a filled rectangle
 * 
 * @param x X position
 * @param y Y position
 * @param width Width
 * @param height Height
 * @param color Color (RGBA565)
 */
void renderer_fill_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, Color color);

/**
 * @brief Draw a rectangle outline
 * 
 * @param x X position
 * @param y Y position
 * @param width Width
 * @param height Height
 * @param color Color (RGBA565)
 */
void renderer_draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, Color color);

/**
 * @brief Draw a horizontal line
 * 
 * @param x1 Start X
 * @param y Y position
 * @param x2 End X
 * @param color Color (RGBA565)
 */
void renderer_draw_hline(uint16_t x1, uint16_t y, uint16_t x2, Color color);

/**
 * @brief Draw text
 * 
 * @param x X position
 * @param y Y position
 * @param text Text string
 * @param fg_color Foreground color
 * @param bg_color Background color
 * @param scale Text scale factor
 */
void renderer_draw_text(uint16_t x, uint16_t y, const char* text, Color fg_color, Color bg_color, uint8_t scale);

// =============================================================================
// SCREEN DIMENSIONS
// =============================================================================

/** Screen width in pixels */
#define SCREEN_WIDTH 320

/** Screen height in pixels */
#define SCREEN_HEIGHT 240

#endif // RENDERER_H
