# SPI Port Expanders for ESP8266 GUIKit

> *Comprehensive guide to adding GPIO via SPI for ESP8266 systems with TFT, Touch, and SD card*

---

## 📋 Table of Contents

1. [The GPIO Pin Problem](#the-gpio-pin-problem)
2. [SPI Expander Types](#spi-expander-types)
3. [MCP23S17 - The Gold Standard](#mcp23s17---the-gold-standard)
4. [Wiring & Connection](#wiring--connection)
5. [Library Support](#library-support)
6. [Performance Characteristics](#performance-characteristics)
7. [GUIKit Integration](#guikit-integration)
8. [Practical Examples](#practical-examples)
9. [SPI Bus Multiplexing](#spi-bus-multiplexing)
10. [Comparison: SPI vs I2C](#comparison-spi-vs-i2c)
11. [Recommended Configuration](#recommended-configuration)
12. [Shopping List](#shopping-list)
13. [FAQ](#faq)

---

## 🎯 The GPIO Pin Problem

ESP8266 modules (like NodeMCU v2) have **limited GPIO pins**, and many are already used by the GUIKit system components:

### Current Pin Usage

| Component | Pins Required | Typical Assignment |
|-----------|---------------|-------------------|
| **TFT Display (ST7789)** | 3-4 | CS=D8, DC=D3, RST=D4 |
| **Touchscreen (XPT2046)** | 2-3 | CS=D2, IRQ=D1 (+ optional MOSI/MISO) |
| **SD Card** | 1-6 | CS=D5 (+ MOSI, MISO, SCK if not shared) |
| **Serial Debug** | 2 | RX, TX (optional) |
| **Total Used** | **6-13** | Leaves very few free |

### ESP8266 GPIO Map (NodeMCU v2)

| GPIO | Label | Available? | Notes |
|------|-------|------------|-------|
| 0 | D3 | ⚠️ Conditional | Used by TFT DC if not expander |
| 1 | TX | ⚠️ Conditional | Serial TX, avoid for outputs |
| 2 | D4 | ✅ Yes | Built-in LED, can be used |
| 3 | RX | ⚠️ Conditional | Serial RX, avoid for inputs |
| 4 | D2 | ⚠️ Used | Touch CS |
| 5 | D1 | ⚠️ Used | Touch IRQ |
| 9 | D8/SD2 | ⚠️ Used | TFT CS |
| 10 | D3/SD3 | ⚠️ Used | TFT DC |
| 12 | D6 | ✅ Yes | MISO |
| 13 | D7 | ✅ Yes | MOSI |
| 14 | D5 | ⚠️ Used | SD CS |
| 15 | D8 | ⚠️ Used | TFT CS (conflict) |
| 16 | D0 | ✅ Yes | Available for expander |

**Free Pins:** D0 (GPIO16), D6 (GPIO12), D7 (GPIO13) = **3 pins**

**Problem:** Not enough for additional buttons, LEDs, sensors, etc.

---

## 🔌 SPI Expander Types

### GPIO Expanders (Digital I/O)

| Chip | I/O Pins | Interface | Max Speed | Key Features | Best For |
|------|----------|-----------|-----------|--------------|----------|
| **MCP23S17** | 16 | SPI | 10 MHz | ✅ 16 GPIO, ✅ Hardware addressable, ✅ Interrupts, ✅ Pull-ups | **Best choice** |
| **MCP23S08** | 8 | SPI | 10 MHz | 8 GPIO, Same features as 23S17 | Small projects |
| **MCP23S18** | 16 | SPI | 10 MHz | Same as 23S17, different package | Alternative |
| **MCP23017** | 16 | I2C | 1.7 MHz | Same as MCP23S17 but I2C | I2C projects |
| **MCP23008** | 8 | I2C | 1.7 MHz | I2C version of MCP23S08 | I2C, 8 pins |
| **74HC595** | 8 | SPI-like | 20+ MHz | Output only, shift register | Output expansion |
| **74HC165** | 8 | SPI-like | 20+ MHz | Input only, shift register | Input expansion |

### ADC Expanders (Analog Input)

| Chip | Channels | Resolution | Interface | Speed |
|------|-----------|------------|-----------|-------|
| **MCP3008** | 8 | 10-bit | SPI | 200 ksps |
| **MCP3208** | 8 | 12-bit | SPI | 100 ksps |
| **ADS1115** | 4 | 16-bit | I2C | 860 sps |
| **PCA9685** | 16 | 12-bit PWM | I2C | - |

### Specialized Expanders

| Chip | Function | Interface |
|------|----------|-----------|
| **MAX7219** | 8-digit 7-segment | SPI |
| **HT16K33** | 16x8 LED matrix | I2C |
| **LC75880** | Resistive touch | SPI |

---

## ⭐ MCP23S17 - The Gold Standard

### Why MCP23S17 is Perfect for ESP8266 GUIKit

| Feature | Benefit |
|---------|---------|
| **16 GPIO pins** (GPA0-GPA7, GPB0-GPB7) | Massive expansion from 3 pins |
| **SPI interface** | Fast (up to 10 MHz), uses only 4 wires total |
| **Hardware addressable** | Up to 8 devices on same SPI bus (A0-A2 pins) |
| **Interrupt support** | Can trigger ESP8266 on pin change |
| **Internal pull-up resistors** | 100kΩ, no external resistors needed |
| **3.3V compatible** | Works directly with ESP8266 |
| **Low cost** | ~$1-2 per chip |
| **Widely available** | DIP, SOIC, TSSOP packages |
| **Adafruit library** | Well-supported in Arduino/PlatformIO |

### Pinout Diagram

```
MCP23S17 (28-pin DIP)
     ┌─────────────────┐
CS  ─┤1              28┤─ VDD (3.3V)
     ├─────────────────┤
SCK ─┤2              27┤─ VSS (GND)
     ├─────────────────┤
SI  ─┤3  MCP23S17     26┤─ RESET
     ├─────────────────┤
SO  ─┤4              25┤─ A0 (Address)
     ├─────────────────┤
     ├─────────────────┤
A1  ─┤5              24┤─ A2 (Address)
     ├─────────────────┤
A2  ─┤6              23┤─ B0 (GPIO)
     ├─────────────────┤
GPB0─┤7              22┤─ B1 (GPIO)
     ├─────────────────┤
GPB1─┤8              21┤─ B2 (GPIO)
     ├─────────────────┤
GPB2─┤9              20┤─ B3 (GPIO)
     ├─────────────────┤
GPB3─┤10             19┤─ B4 (GPIO)
     ├─────────────────┤
GPB4─┤11             18┤─ B5 (GPIO)
     ├─────────────────┤
GPB5─┤12             17┤─ B6 (GPIO)
     ├─────────────────┤
GPB6─┤13             16┤─ B7 (GPIO)
     ├─────────────────┤
GPB7─┤14             15┤─ A7 (GPIO)
     ├─────────────────┤
GPA0─┤15             14┤─ A6 (GPIO)
     ├─────────────────┤
GPA1─┤16             13┤─ A5 (GPIO)
     ├─────────────────┤
GPA2─┤17             12┤─ A4 (GPIO)
     ├─────────────────┤
GPA3─┤18             11┤─ A3 (GPIO)
     ├─────────────────┤
GPA4─┤19             10┤─ A2 (Address)
     ├─────────────────┤
GPA5─┤20              9┤─ A1 (Address)
     ├─────────────────┤
GPA6─┤21              8┤─ GPB2 (GPIO)
     ├─────────────────┤
GPA7─┤22              7┤─ GPB1 (GPIO)
     └─────────────────┘
```

### Wiring to ESP8266 (NodeMCU v2)

| MCP23S17 Pin | ESP8266 Pin | NodeMCU Label | Notes |
|---------------|--------------|----------------|-------|
| VDD | 3.3V | 3V3 | Power |
| VSS | GND | G | Ground |
| SCK | GPIO14 | D5 | SPI Clock (shared) |
| SI (MOSI) | GPIO13 | D7 | Master Out (shared) |
| SO (MISO) | GPIO12 | D6 | Master In (shared) |
| CS | GPIO16 | D0 | Chip Select (unique per device) |
| RESET | 3.3V | 3V3 | Connect to VDD (no reset needed) |
| A0 | GND | G | Hardware address bit 0 |
| A1 | GND | G | Hardware address bit 1 |
| A2 | GND | G | Hardware address bit 2 |

**Note:** For multiple MCP23S17 devices, connect A0-A2 to different combinations of GND/3.3V to set unique addresses (0-7).

---

## 📐 Wiring & Connection

### Basic Connection (Single MCP23S17)

```
ESP8266                            MCP23S17
   ┌─────────┐                         ┌─────────┐
   │ 3V3    │─────────────────────────│ VDD     │
   │ GND    │─────────────────────────│ VSS     │
   │ D7/MOSI│─────────────────────────│ SI      │
   │ D6/MISO│─────────────────────────│ SO      │
   │ D5/SCK │─────────────────────────│ SCK     │
   │ D0     │─────────────────────────│ CS      │
   │ 3V3    │─────────────────────────│ RESET   │
   └─────────┘                         └─────────┘
```

### Multiple Devices (Shared SPI Bus)

```
ESP8266
   ┌─────────┐
   │ MOSI    │─────┬──────────────────┐
   │ MISO    │─────┤                  │
   │ SCK     │─────┤   SPI Bus         │
   │         │     │                  │
   │ D8      │─────┴────────┬─────────┤
   │ D2      │──────────────┤         │
   │ D5      │──────────────┤         │
   │ D0      │──────────────┤         │
   │         │              │         │
   │         │              ▼         ▼
   │         │       ┌──────────┐  ┌──────────┐
   │         │       │ MCP23S17 │  │ MCP23S17 │
   │         │       │  (Addr 0)│  │  (Addr 1)│
   │         │       └──────────┘  └──────────┘
   │         │              │         │
   │         │              ▼         ▼
   │         │       TFT     Touch    SD Card
   └─────────┘
```

**SPI Bus Sharing:** All devices share MOSI, MISO, SCK. Each has its own CS line.

---

## 💻 Library Support

### PlatformIO Dependency

```ini
[env]
lib_deps =
    adafruit/Adafruit MCP23017 @ ^2.0.0
```

**Note:** The Adafruit MCP23017 library supports **both** MCP23017 (I2C) and MCP23S17 (SPI).

### Arduino Library Manager

1. Open Arduino IDE
2. Sketch → Include Library → Manage Libraries
3. Search for "Adafruit MCP23017"
4. Install

### Basic Usage Code

```cpp
#include <SPI.h>
#include <Adafruit_MCP23X17.h>

// Define CS pin
#define MCP23S17_CS D0

// Create instance
Adafruit_MCP23X17 mcp;

void setup() {
    SPI.begin();
    
    // Initialize MCP23S17
    // MCP23S17 address: A0-A2 all GND = 0x20
    if (!mcp.begin_SPI(MCP23S17_CS, 0x20)) {
        Serial.println("MCP23S17 not found!");
        while (1);
    }
    
    // Configure pins
    // Port A (GPA0-GPA7)
    for (int i = 0; i < 8; i++) {
        mcp.pinMode(i, INPUT_PULLUP);  // Buttons with pull-up
    }
    
    // Port B (GPB0-GPB7)
    for (int i = 8; i < 16; i++) {
        mcp.pinMode(i, OUTPUT);  // LEDs
    }
}

void loop() {
    // Read button on GPA0
    bool buttonState = mcp.digitalRead(0);
    
    // If button pressed (assuming pull-up, so LOW = pressed)
    if (buttonState == LOW) {
        // Toggle LED on GPB0
        static bool ledState = LOW;
        ledState = !ledState;
        mcp.digitalWrite(8, ledState);
        delay(200);  // Debounce
    }
}
```

### Multiple Devices

```cpp
Adafruit_MCP23X17 mcp1;  // Address 0 (A0-A2 = GND)
Adafruit_MCP23X17 mcp2;  // Address 1 (A0=VCC, A1-A2=GND)
Adafruit_MCP23X17 mcp3;  // Address 2 (A1=VCC, A0/A2=GND)

void setup() {
    SPI.begin();
    
    mcp1.begin_SPI(D0, 0x20);  // CS=D0, Address 0
    mcp2.begin_SPI(D1, 0x21);  // CS=D1, Address 1
    mcp3.begin_SPI(D3, 0x22);  // CS=D3, Address 2
}
```

### Register-Level Access (Advanced)

```cpp
// Read entire port A (8 bits at once)
uint8_t portAState = mcp.readGPIOAB();  // Reads both ports
uint8_t portA = portAState & 0xFF;      // Lower 8 bits = port A

// Write entire port B
mcp.writeGPIOAB(0x00FF);  // Lower 8 = port A, upper 8 = port B
```

---

## ⚡ Performance Characteristics

### Timing Specifications

| Operation | Time (typical) | Notes |
|-----------|----------------|-------|
| **Digital Read (single pin)** | ~1-2 µs | Via SPI transfer |
| **Digital Write (single pin)** | ~1-2 µs | Via SPI transfer |
| **Port Read (8 pins)** | ~10-20 µs | One SPI transaction |
| **Port Write (8 pins)** | ~10-20 µs | One SPI transaction |
| **Register Read/Write** | ~15-25 µs | Includes SPI overhead |

### Throughput

| SPI Speed | Bytes/sec | Pins/sec (read) |
|-----------|-----------|-----------------|
| 1 MHz | 125,000 | ~62,500 |
| 4 MHz | 500,000 | ~250,000 |
| 8 MHz | 1,000,000 | ~500,000 |
| 10 MHz | 1,250,000 | ~625,000 |

**Reality Check:** With SPI overhead, actual throughput is ~50-70% of theoretical.

### Comparison with Native GPIO

| Operation | Native GPIO | MCP23S17 | Ratio |
|-----------|-------------|-----------|-------|
| Single read | ~0.1 µs | ~2 µs | 20x slower |
| Single write | ~0.1 µs | ~2 µs | 20x slower |
| Port read (8 pins) | ~0.1 µs | ~15 µs | 150x slower |
| Port write (8 pins) | ~0.1 µs | ~15 µs | 150x slower |

**Conclusion:** MCP23S17 is **slower** than native GPIO, but the trade-off for **16 extra pins from 1 CS line** is worth it.

---

## 🎨 GUIKit Integration

### 1. Adding Expander Support to Widget System

#### Widget Type: `expander_button`

**JSON Definition:**
```json
{
  "id": "button_1",
  "type": "expander_button",
  "x": 10,
  "y": 10,
  "width": 80,
  "height": 40,
  "text": "Button 1",
  "text_size": 14,
  "text_color": "#FFFFFF",
  "background": "#1177BB",
  "expander": 0,
  "pin": 0,
  "action": "button_pressed",
  "invert": true,
  "debounce_ms": 50
}
```

**Properties:**

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `expander` | integer | 0 | Expander index (0-7) |
| `pin` | integer | 0 | Pin number (0-15) |
| `action` | string | - | JavaScript action to trigger |
| `invert` | boolean | true | Invert logic (pull-up = button LOW when pressed) |
| `debounce_ms` | integer | 50 | Debounce time in milliseconds |
| `pullup` | boolean | true | Use internal pull-up resistor |

**C++ Structure:**
```cpp
struct t_widget_expander_button {
    t_widget_base base;
    uint8_t expander_index;
    uint8_t pin;
    char action[64];
    bool invert;
    uint16_t debounce_ms;
    uint32_t last_read_time;
    bool last_state;
};
```

**Rendering:**
```cpp
void render_expander_button(t_widget_expander_button* widget) {
    // Draw button background
    render_button_background(&widget->base);
    
    // Draw button text
    render_button_text(&widget->base);
    
    // Read expander pin
    bool current_state = readExpanderPin(widget->expander_index, widget->pin);
    if (widget->invert) current_state = !current_state;
    
    // Handle state change
    if (current_state != widget->last_state && 
        millis() - widget->last_read_time > widget->debounce_ms) {
        
        widget->last_state = current_state;
        widget->last_read_time = millis();
        
        if (current_state) {
            // Button pressed
            executeAction(widget->action);
        }
    }
}
```

---

### 2. Adding Expander Support to LED Widget

#### Widget Type: `expander_led`

**JSON Definition:**
```json
{
  "id": "led_1",
  "type": "expander_led",
  "x": 100,
  "y": 10,
  "width": 20,
  "height": 20,
  "color_on": "#FF0000",
  "color_off": "#400000",
  "border_color": "#808080",
  "border_width": 1,
  "expander": 0,
  "pin": 0,
  "active_high": true,
  "initial_state": false
}
```

**Properties:**

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `expander` | integer | 0 | Expander index (0-7) |
| `pin` | integer | 0 | Pin number (0-15) |
| `color_on` | string | "#FFFFFF" | Color when LED is on |
| `color_off` | string | "#404040" | Color when LED is off |
| `active_high` | boolean | true | LED on when pin HIGH |
| `initial_state` | boolean | false | Initial state on startup |

**C++ Structure:**
```cpp
struct t_widget_expander_led {
    t_widget_base base;
    uint16_t color_on;
    uint16_t color_off;
    uint8_t expander_index;
    uint8_t pin;
    bool active_high;
    bool current_state;
};
```

**Rendering:**
```cpp
void render_expander_led(t_widget_expander_led* widget) {
    // Determine current color
    bool pin_state = readExpanderPin(widget->expander_index, widget->pin);
    if (widget->active_high) {
        widget->current_state = pin_state;
    } else {
        widget->current_state = !pin_state;
    }
    
    uint16_t color = widget->current_state ? widget->color_on : widget->color_off;
    
    // Draw LED
    tft.fillRect(widget->base.position.x, widget->base.position.y,
                 widget->base.size.width, widget->base.size.height,
                 color);
    
    // Draw border
    tft.drawRect(widget->base.position.x, widget->base.position.y,
                 widget->base.size.width, widget->base.size.height,
                 widget->base.border.color);
}

void setExpanderLEDState(t_widget_expander_led* widget, bool state) {
    bool output_state = widget->active_high ? state : !state;
    writeExpanderPin(widget->expander_index, widget->pin, output_state);
}
```

---

### 3. Adding Expander Support to Input Widget

#### Widget Type: `expander_input`

**JSON Definition:**
```json
{
  "id": "sensor_1",
  "type": "expander_input",
  "x": 10,
  "y": 60,
  "width": 60,
  "height": 20,
  "text_on": "ON",
  "text_off": "OFF",
  "text_size": 12,
  "text_color": "#FFFFFF",
  "background": "#2D2D2D",
  "expander": 0,
  "pin": 0,
  "mode": "digital",
  "poll_interval": 100,
  "invert": false,
  "pullup": true
}
```

**Properties:**

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `expander` | integer | 0 | Expander index |
| `pin` | integer | 0 | Pin number |
| `mode` | string | "digital" | "digital" or "analog" (if ADC expander) |
| `poll_interval` | integer | 100 | Polling interval in ms |
| `invert` | boolean | false | Invert input logic |
| `pullup` | boolean | true | Enable internal pull-up |
| `text_on` | string | "ON" | Text when active |
| `text_off` | string | "OFF" | Text when inactive |

---

### 4. Adding Expander Support to Output Widget

#### Widget Type: `expander_output`

**JSON Definition:**
```json
{
  "id": "relay_1",
  "type": "expander_output",
  "x": 80,
  "y": 60,
  "width": 80,
  "height": 30,
  "text_on": "RELAY ON",
  "text_off": "RELAY OFF",
  "text_size": 12,
  "text_color": "#FFFFFF",
  "background": "#0E639C",
  "expander": 0,
  "pin": 0,
  "active_high": true,
  "initial_state": false,
  "toggle_on_click": true
}
```

**Properties:**

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `expander` | integer | 0 | Expander index |
| `pin` | integer | 0 | Pin number |
| `active_high` | boolean | true | Output active when HIGH |
| `initial_state` | boolean | false | Initial output state |
| `toggle_on_click` | boolean | true | Toggle on touch |
| `text_on` | string | "ON" | Text when output on |
| `text_off` | string | "OFF" | Text when output off |

**C++ Structure:**
```cpp
struct t_widget_expander_output {
    t_widget_base base;
    uint8_t expander_index;
    uint8_t pin;
    bool active_high;
    bool current_state;
    bool initial_state;
    bool toggle_on_click;
    char text_on[32];
    char text_off[32];
};
```

**Touch Handler:**
```cpp
void handle_expander_output_touch(t_widget_expander_output* widget) {
    if (widget->toggle_on_click) {
        widget->current_state = !widget->current_state;
    } else {
        widget->current_state = true;  // Momentary
    }
    
    // Update physical pin
    bool output_state = widget->active_high ? widget->current_state : !widget->current_state;
    writeExpanderPin(widget->expander_index, widget->pin, output_state);
    
    // Redraw
    widget->dirty = true;
}

void render_expander_output(t_widget_expander_output* widget) {
    uint16_t color = widget->current_state ? 0x1177 : 0x0E63;
    
    // Draw button
    tft.fillRect(widget->base.position.x, widget->base.position.y,
                 widget->base.size.width, widget->base.size.height,
                 color);
    
    // Draw text
    const char* text = widget->current_state ? widget->text_on : widget->text_off;
    drawText(text, widget->base.position.x, widget->base.position.y,
             widget->base.size.width, widget->base.size.height);
}
```

---

## 🎯 Practical Examples

### Example 1: 8-Button Keypad

**Hardware:** MCP23S17 with 8 buttons on Port A (GPA0-GPA7)

**GUIKit JSON:**
```json
{
  "version": "1.0",
  "name": "Keypad",
  "size": { "width": 320, "height": 240 },
  "background": "#1E1E1E",
  "widgets": [
    {
      "id": "btn1", "type": "expander_button",
      "x": 10, "y": 10, "width": 70, "height": 70,
      "text": "1", "text_size": 24,
      "expander": 0, "pin": 0, "action": "key_1_pressed"
    },
    {
      "id": "btn2", "type": "expander_button",
      "x": 90, "y": 10, "width": 70, "height": 70,
      "text": "2", "text_size": 24,
      "expander": 0, "pin": 1, "action": "key_2_pressed"
    },
    {
      "id": "btn3", "type": "expander_button",
      "x": 170, "y": 10, "width": 70, "height": 70,
      "text": "3", "text_size": 24,
      "expander": 0, "pin": 2, "action": "key_3_pressed"
    },
    {
      "id": "btn4", "type": "expander_button",
      "x": 250, "y": 10, "width": 70, "height": 70,
      "text": "A", "text_size": 24,
      "expander": 0, "pin": 3, "action": "key_A_pressed"
    },
    {
      "id": "btn5", "type": "expander_button",
      "x": 10, "y": 90, "width": 70, "height": 70,
      "text": "4", "text_size": 24,
      "expander": 0, "pin": 4, "action": "key_4_pressed"
    },
    {
      "id": "btn6", "type": "expander_button",
      "x": 90, "y": 90, "width": 70, "height": 70,
      "text": "5", "text_size": 24,
      "expander": 0, "pin": 5, "action": "key_5_pressed"
    },
    {
      "id": "btn7", "type": "expander_button",
      "x": 170, "y": 90, "width": 70, "height": 70,
      "text": "6", "text_size": 24,
      "expander": 0, "pin": 6, "action": "key_6_pressed"
    },
    {
      "id": "btn8", "type": "expander_button",
      "x": 250, "y": 90, "width": 70, "height": 70,
      "text": "B", "text_size": 24,
      "expander": 0, "pin": 7, "action": "key_B_pressed"
    },
    {
      "id": "display", "type": "label",
      "x": 20, "y": 180, "width": 280, "height": 40,
      "text": "Press a key...", "text_size": 16,
      "text_color": "#D4D4D4", "text_align": "center",
      "id": "display"
    }
  ]
}
```

**JavaScript Actions:**
```javascript
function key_1_pressed(widget, event) {
    GUI.getWidgetById('display').text = 'Key 1 pressed';
    GUI.redraw();
}
// ... similar for other keys
```

---

### Example 2: Status LED Bar (8 LEDs)

**Hardware:** MCP23S17 with 8 LEDs on Port B (GPB0-GPB7)

**GUIKit JSON:**
```json
{
  "version": "1.0",
  "name": "LED Status",
  "size": { "width": 320, "height": 240 },
  "background": "#000000",
  "widgets": [
    {
      "id": "led0", "type": "expander_led",
      "x": 10, "y": 10, "width": 30, "height": 30,
      "expander": 0, "pin": 8, "active_high": true,
      "color_on": "#FF0000", "color_off": "#400000"
    },
    {
      "id": "led1", "type": "expander_led",
      "x": 50, "y": 10, "width": 30, "height": 30,
      "expander": 0, "pin": 9, "active_high": true,
      "color_on": "#00FF00", "color_off": "#004000"
    },
    {
      "id": "led2", "type": "expander_led",
      "x": 90, "y": 10, "width": 30, "height": 30,
      "expander": 0, "pin": 10, "active_high": true,
      "color_on": "#0000FF", "color_off": "#000040"
    },
    {
      "id": "led3", "type": "expander_led",
      "x": 130, "y": 10, "width": 30, "height": 30,
      "expander": 0, "pin": 11, "active_high": true,
      "color_on": "#FFFF00", "color_off": "#404000"
    },
    {
      "id": "led4", "type": "expander_led",
      "x": 170, "y": 10, "width": 30, "height": 30,
      "expander": 0, "pin": 12, "active_high": true,
      "color_on": "#FF00FF", "color_off": "#400040"
    },
    {
      "id": "led5", "type": "expander_led",
      "x": 210, "y": 10, "width": 30, "height": 30,
      "expander": 0, "pin": 13, "active_high": true,
      "color_on": "#00FFFF", "color_off": "#004040"
    },
    {
      "id": "led6", "type": "expander_led",
      "x": 250, "y": 10, "width": 30, "height": 30,
      "expander": 0, "pin": 14, "active_high": true,
      "color_on": "#FFFFFF", "color_off": "#404040"
    },
    {
      "id": "led7", "type": "expander_led",
      "x": 290, "y": 10, "width": 30, "height": 30,
      "expander": 0, "pin": 15, "active_high": true,
      "color_on": "#FFA500", "color_off": "#402600"
    }
  ]
}
```

**JavaScript to Control LEDs:**
```javascript
// Turn on all LEDs
function all_leds_on(widget, event) {
    for (let i = 0; i < 8; i++) {
        GUIKitEditor.setExpanderLED(i, true);
    }
}

// Binary count on LEDs
function led_binary_count(widget, event) {
    let count = 0;
    setInterval(function() {
        count = (count + 1) % 256;
        for (let i = 0; i < 8; i++) {
            GUIKitEditor.setExpanderLED(i, (count >> i) & 1);
        }
    }, 500);
}
```

---

### Example 3: Combined Input/Output Panel

**Hardware:**
- MCP23S17 #1: Port A = 8 buttons, Port B = 8 LEDs
- MCP23S17 #2: Port A = 8 sensors, Port B = 8 relays

**GUIKit JSON:**
```json
{
  "version": "1.0",
  "name": "Control Panel",
  "size": { "width": 320, "height": 240 },
  "background": "#1E1E1E",
  "widgets": [
    // Buttons (Expander 0, Port A)
    {"id": "btn1", "type": "expander_button", "x": 10, "y": 10, "width": 50, "height": 30,
     "text": "B1", "expander": 0, "pin": 0, "action": "btn1_action"},
    {"id": "btn2", "type": "expander_button", "x": 70, "y": 10, "width": 50, "height": 30,
     "text": "B2", "expander": 0, "pin": 1, "action": "btn2_action"},
    
    // LEDs (Expander 0, Port B)
    {"id": "led1", "type": "expander_led", "x": 10, "y": 50, "width": 20, "height": 20,
     "expander": 0, "pin": 8, "active_high": true},
    {"id": "led2", "type": "expander_led", "x": 40, "y": 50, "width": 20, "height": 20,
     "expander": 0, "pin": 9, "active_high": true},
    
    // Relays (Expander 1, Port B)
    {"id": "relay1", "type": "expander_output", "x": 10, "y": 100, "width": 80, "height": 30,
     "text_on": "RELAY 1 ON", "text_off": "RELAY 1 OFF",
     "expander": 1, "pin": 8, "toggle_on_click": true},
    {"id": "relay2", "type": "expander_output", "x": 100, "y": 100, "width": 80, "height": 30,
     "text_on": "RELAY 2 ON", "text_off": "RELAY 2 OFF",
     "expander": 1, "pin": 9, "toggle_on_click": true},
    
    // Sensor Inputs (Expander 1, Port A)
    {"id": "sensor1", "type": "expander_input", "x": 10, "y": 150, "width": 60, "height": 20,
     "expander": 1, "pin": 0, "text_on": "OPEN", "text_off": "CLOSED"},
    {"id": "sensor2", "type": "expander_input", "x": 80, "y": 150, "width": 60, "height": 20,
     "expander": 1, "pin": 1, "text_on": "ACTIVE", "text_off": "INACTIVE"}
  ]
}
```

---

## 🔄 SPI Bus Multiplexing

### The Challenge

TFT, Touch, SD Card, and Expanders all want to use the **SPI bus**. The solution is **CS (Chip Select) multiplexing**.

### How It Works

All SPI devices share:
- **MOSI** (Master Out Slave In)
- **MISO** (Master In Slave Out)  
- **SCK** (Serial Clock)

Each device has its own **CS** line:
- **TFT:** CS = D8
- **Touch:** CS = D2
- **SD Card:** CS = D5
- **MCP23S17 #1:** CS = D0
- **MCP23S17 #2:** CS = D1
- **MCP23S17 #3:** CS = D3

### Wiring Diagram

```
ESP8266
   ┌─────────────────────┐
   │     SPI Bus         │
   │  ┌────────┐         │
   │  │ MOSI   │─────────┼─────────┐
   │  │ MISO   │─────────┼─────────┤
   │  │ SCK    │─────────┼─────────┤
   │  └────────┘         │         │
   │         │            │         │
   │    ┌────┴────┬──────┴────┬────┴─────┐
   │    ▼         ▼            ▼         ▼
   │ ┌────────┐ ┌─────┐ ┌────────┐ ┌─────┐
   │ │ TFT    │ │Touch│ │ SD Card │ │MCP  │
   │ │ ST7789 │ │XPT2046│ │        │ │#1   │
   │ └────┬───┘ └───┬──┘ └────┬──┘ └───┬──┘
   │      │         │         │       │
   └──────┼─────────┼─────────┼───────┼──
          │         │         │       │
     D8 ━─┘         D2 ━─┘       D5 ━─┘     D0 ━┘
          │         │         │       │
     (CS)   │    (CS)   │    (CS)    │  (CS)
```

### Dynamic SPI Speed Configuration

Different devices have different maximum SPI speeds:

| Device | Max SPI Speed | Recommended Speed |
|--------|----------------|-------------------|
| MCP23S17 | 10 MHz | 4-8 MHz |
| TFT ST7789 | 80 MHz | 40 MHz |
| Touch XPT2046 | 2 MHz | 1-2 MHz |
| SD Card | 25 MHz | 4-8 MHz |

**Solution:** Change SPI speed before accessing each device:

```cpp
// For MCP23S17
SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
mcp.digitalRead(0);
SPI.endTransaction();

// For TFT
SPI.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));
tft.drawPixel(0, 0, 0xFFFF);
SPI.endTransaction();

// For SD Card
SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
file.read(buffer, size);
SPI.endTransaction();
```

### Optimized SPI Access Class

```cpp
class SPIDevice {
private:
    uint8_t csPin;
    uint32_t speed;
public:
    SPIDevice(uint8_t cs, uint32_t spd) : csPin(cs), speed(spd) {
        pinMode(csPin, OUTPUT);
        digitalWrite(csPin, HIGH);
    }
    
    void begin() {
        digitalWrite(csPin, LOW);
        SPI.beginTransaction(SPISettings(speed, MSBFIRST, SPI_MODE0));
    }
    
    void end() {
        SPI.endTransaction();
        digitalWrite(csPin, HIGH);
    }
    
    uint8_t transfer(uint8_t data) {
        begin();
        uint8_t result = SPI.transfer(data);
        end();
        return result;
    }
    
    void transfer(void* buf, size_t count) {
        begin();
        SPI.transfer(buf, count);
        end();
    }
};

// Usage
SPIDevice tftDevice(D8, 40000000);
SPIDevice mcpDevice(D0, 8000000);
SPIDevice sdDevice(D5, 4000000);

// Write to TFT
tftDevice.begin();
tft.writeCommand(0x2C);  // Memory write
tft.writeData(color);
tftDevice.end();
```

---

## ⚖️ Comparison: SPI vs I2C Expanders

| Feature | SPI Expander | I2C Expander | Winner |
|---------|--------------|---------------|--------|
| **Speed** | ⚡⚡⚡⚡⚡ (10+ MHz) | ⚡⚡ (400 kHz - 1 MHz) | **SPI** |
| **Wires** | 4 (MOSI, MISO, SCK, CS) | 2 (SDA, SCL) | **I2C** |
| **Devices per bus** | Unlimited (with CS) | ~127 (address limited) | **SPI** |
| **Max distance** | Good (cm to meters) | Limited (typically < 1m) | **SPI** |
| **Noise immunity** | Good (differential possible) | Medium | **SPI** |
| **ESP8266 support** | ✅ Excellent | ✅ Good | Tie |
| **Hardware complexity** | Simple (CS lines) | Simple (pull-ups) | **I2C** |
| **Address conflicts** | None (CS lines) | Possible (address collisions) | **SPI** |
| **Best for GUIKit** | ✅ **Recommended** | ⚠️ Alternative | **SPI** |

### Why SPI Wins for GUIKit

1. **TFT already uses SPI** - Can share MOSI/MISO/SCK
2. **Only need CS lines** - Each device gets its own CS
3. **Faster** - Better for GUI updates
4. **No address conflicts** - Each CS line is unique
5. **Better for many devices** - Scale to 8+ expanders easily

### When to Use I2C

- Very few extra pins needed (< 16)
- Already using SPI for other devices
- Need very long wiring runs (I2C can be extended)
- Using MCP23017 (I2C version of MCP23S17)

---

## 🎯 Recommended Configuration for GUIKit

### Optimal Setup with 2 Expanders

| Component | CS Pin | SPI Speed | Pins Used |
|-----------|--------|-----------|-----------|
| TFT Display | D8 | 40 MHz | 3 (CS, DC, RST) |
| Touchscreen | D2 | 2 MHz | 2 (CS, IRQ) |
| SD Card | D5 | 8 MHz | 1 (CS) |
| MCP23S17 #1 | D0 | 8 MHz | 1 (CS) |
| MCP23S17 #2 | D1 | 8 MHz | 1 (CS) |
| **Total** | - | - | **8 pins** |

**Result:** 32 extra GPIO (2 × 16) from just 2 additional pins!

### With 3 Expanders

| Component | CS Pin | SPI Speed | Pins Used |
|-----------|--------|-----------|-----------|
| TFT Display | D8 | 40 MHz | 3 |
| Touchscreen | D2 | 2 MHz | 2 |
| SD Card | D5 | 8 MHz | 1 |
| MCP23S17 #1 | D0 | 8 MHz | 1 |
| MCP23S17 #2 | D1 | 8 MHz | 1 |
| MCP23S17 #3 | D3 | 8 MHz | 1 |
| **Total** | - | - | **9 pins** |

**Result:** 48 extra GPIO (3 × 16) from just 3 additional pins!

### Pin Map Summary

```
ESP8266 GPIO:
  D0 (GPIO16)  -> MCP23S17 #1 CS
  D1 (GPIO5)   -> MCP23S17 #2 CS
  D2 (GPIO4)   -> Touch CS
  D3 (GPIO0)   -> MCP23S17 #3 CS (or TFT DC)
  D4 (GPIO2)   -> Built-in LED (optional use)
  D5 (GPIO14)  -> SD Card CS
  D6 (GPIO12)  -> MISO (shared)
  D7 (GPIO13)  -> MOSI (shared)
  D8 (GPIO15)  -> TFT CS

SPI Bus (shared):
  MOSI: D7
  MISO: D6  
  SCK:  D5 (shared with SD CS - need careful timing)

Note: D5 is used for both SCK and SD CS. This requires careful
      SPI transaction management to avoid conflicts.
```

### Corrected SPI Pin Assignment

To avoid conflicts, use:
- **SCK:** D5 (GPIO14) - Dedicated
- **MOSI:** D7 (GPIO13) - Dedicated  
- **MISO:** D6 (GPIO12) - Dedicated
- **TFT CS:** D8 (GPIO15)
- **Touch CS:** D2 (GPIO4)
- **SD CS:** D5 is SCK! ❌ **Conflict!**

**Solution:** Use different pin for SD CS:
- **SD CS:** D4 (GPIO2)
- **SCK:** D5 (GPIO14)
- **TFT CS:** D8 (GPIO15)
- **Touch CS:** D2 (GPIO4)
- **Expander #1 CS:** D0 (GPIO16)
- **Expander #2 CS:** D1 (GPIO5)
- **Expander #3 CS:** D3 (GPIO0)

---

## 🛒 Shopping List

| Item | Quantity | Purpose | Estimated Cost | Notes |
|------|----------|---------|---------------|-------|
| **MCP23S17** | 1-3 | GPIO expansion | $1-2 each | DIP-28 package |
| **MCP23S17 Breakout** | 1-3 | Easy prototyping | $2-5 each | Optional, but recommended |
| **0.1" Header Pins** | 10-20 | Connection | $1-2 | Male and/or female |
| **Jumper Wires** | 20-30 | Wiring | $2-5 | Male-male, male-female |
| **Breadboard** | 1-2 | Prototyping | $5-10 | Optional for testing |
| **Total** | | | **$10-30** | Depending on quantity |

### Where to Buy

- **AliExpress:** Cheapest, slow shipping
- **eBay:** Good prices, variable quality
- **Amazon:** Fast, reliable, higher cost
- **Adafruit:** High quality, good documentation, premium price
- **SparkFun:** Good quality, premium price
- **Local Electronics Stores:** Instant, but limited selection

### Recommended Suppliers

- **MCP23S17 DIP:** LCSC, JLCPCB, AliExpress
- **Breakout Boards:** Adafruit, SparkFun, Amazon
- **Kits:** ESP8266 + MCP23S17 kits available on AliExpress

---

## ❓ FAQ

### Q: Can I use I2C expanders instead?

**A:** Yes, MCP23017 is the I2C version of MCP23S17. However:
- I2C is slower (400 kHz vs 10 MHz)
- I2C uses only 2 wires (SDA, SCL)
- I2C has address limitations (only ~127 devices)
- For GUIKit, SPI is usually better because TFT already uses SPI

### Q: How many MCP23S17 can I connect?

**A:** Up to **8** on the same SPI bus (using A0-A2 address pins). Each needs its own CS line from ESP8266. With ESP8266's limited GPIO, you can typically connect **2-3** expanders.

### Q: Can I share CS lines?

**A:** No. Each device must have its own unique CS line. CS is active-low, and only one device should be selected at a time.

### Q: What's the maximum SPI cable length?

**A:** For reliable operation:
- **< 30cm:** No issues
- **30cm - 1m:** Use twisted pair, keep away from noise
- **> 1m:** Consider SPI repeaters or different interface

### Q: Can I use MCP23S17 for analog inputs?

**A:** No, MCP23S17 is **digital only**. For analog:
- Use **MCP3008** (8-channel, 10-bit ADC, SPI)
- Use **ADS1115** (4-channel, 16-bit ADC, I2C)
- Or use ESP8266's built-in ADC (only 1 channel, 10-bit)

### Q: Can MCP23S17 do PWM?

**A:** No, MCP23S17 is **digital I/O only**. For PWM:
- Use ESP8266's hardware PWM pins
- Use **PCA9685** (16-channel, 12-bit PWM, I2C)
- Use software PWM (limited, CPU-intensive)

### Q: Can I use MCP23S17 interrupts with ESP8266?

**A:** Yes! MCP23S17 has interrupt pins (INTA, INTB). Connect to any GPIO and use:

```cpp
// Configure interrupt
mcp.setupInterrupts(true, false, LOW);

// Attach interrupt handler
void IRAM_ATTR mcpInterrupt() {
    interrupt_occurred = true;
}

void setup() {
    attachInterrupt(digitalPinToInterrupt(INT_PIN), mcpInterrupt, FALLING);
}

void loop() {
    if (interrupt_occurred) {
        uint8_t intCapA = mcp.getLastInterruptPinValueA();
        uint8_t intCapB = mcp.getLastInterruptPinValueB();
        // Handle interrupt
        interrupt_occurred = false;
    }
}
```

### Q: How do I power MCP23S17?

**A:** Connect VDD to **3.3V** and VSS to **GND**. MCP23S17 is **not** 5V tolerant!

### Q: Can I use MCP23S17 with 5V logic?

**A:** No. MCP23S17 is **3.3V only**. Connecting to 5V will damage the chip and ESP8266.

### Q: What's the current consumption?

**A:** ~1-2 µA in standby, ~1 mA active. Very low power, perfect for battery applications.

### Q: Can I use MCP23S17 with PlatformIO?

**A:** Yes! Add to `platformio.ini`:
```ini
lib_deps =
    adafruit/Adafruit MCP23017 @ ^2.0.0
```

### Q: Can I use MCP23S17 with Arduino IDE?

**A:** Yes! Install via Library Manager or download from GitHub.

---

## 📚 References

### Datasheets
- [MCP23S17 Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/20001952C.pdf)
- [MCP23017 Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/20001919D.pdf)
- [TFT_eSPI Library](https://github.com/Bodmer/TFT_eSPI)
- [Adafruit MCP23017 Library](https://github.com/adafruit/Adafruit_MCP23017)

### Tutorials
- [Adafruit MCP23017 Guide](https://learn.adafruit.com/mcp23017-i2c-gpio-expander)
- [SPI Communication Guide](https://learn.sparkfun.com/tutorials/serial-peripheral-interface-spi)
- [ESP8266 GPIO Reference](https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/)

### Related Libraries
- [PNGdec](https://github.com/bitbank2/PNGdec) - PNG decoder (ESP32 only)
- [SPI Library](https://www.arduino.cc/en/Reference/SPI) - Arduino SPI
- [Wire Library](https://www.arduino.cc/en/Reference/Wire) - Arduino I2C

---

## 🏁 Summary

### ✅ Key Takeaways

1. **MCP23S17 is the best choice** for GPIO expansion on ESP8266 GUIKit
2. **Adds 16 GPIO per chip** using only **1 CS pin**
3. **SPI bus sharing** works perfectly with TFT, Touch, SD Card
4. **Up to 8 chips** on same bus (48+ GPIO total, limited by CS pins)
5. **Easy to use** with Adafruit library
6. **Fast enough** for buttons, LEDs, sensors

### 🎯 Recommended Setup

```
ESP8266 + MCP23S17 × 2:
├── TFT: D8 (CS), D3 (DC), D4 (RST)
├── Touch: D2 (CS), D1 (IRQ)
├── SD: D4 (CS)
├── MCP23S17 #1: D0 (CS)
└── MCP23S17 #2: D1 (CS) - Wait, D1 is Touch IRQ!

Corrected:
├── TFT: D8 (CS), D3 (DC), D4 (RST)
├── Touch: D2 (CS), D1 (IRQ)
├── SD: D0 (CS) - Use D0 for SD
├── MCP23S17 #1: D1 (CS) - Use D1 for expander
└── MCP23S17 #2: D3 (CS)

Final:
- SCK: D5
- MOSI: D7
- MISO: D6
- TFT CS: D8
- Touch CS: D2
- Touch IRQ: D1
- SD CS: D0
- Expander #1 CS: D3
- Expander #2 CS: D4 (if not used for RST)
```

**Result:** 32 GPIO from 2 expanders using only 2 CS pins!

---

*Document generated by Mistral Vibe*  
*Topic: SPI Port Expanders for ESP8266 GUIKit*  
*Date: 2026-08-15*
