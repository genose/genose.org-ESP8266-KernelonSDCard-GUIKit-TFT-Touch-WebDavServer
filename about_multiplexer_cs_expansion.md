# SPI CS Line Multiplexer Expansion - Complete Guide

> *Gain 16+ Chip Select lines from just 5 GPIO pins using digital multiplexers*  
> *For ESP8266/ESP32 GUIKit systems with limited GPIO but many SPI devices*

---

## 📋 Table of Contents

1. [The Problem: CS Line Exhaustion](#the-problem-cs-line-exhaustion)
2. [The Solution: Multiplexed CS Lines](#the-solution-multiplexed-cs-lines)
3. [Multiplexer Types](#multiplexer-types)
4. [74HC4067 - The 16-Channel Powerhouse](#74hc4067---the-16-channel-powerhouse)
5. [Alternative Multiplexers](#alternative-multiplexers)
6. [Wiring & Connection](#wiring--connection)
7. [Software Implementation](#software-implementation)
8. [Advanced Topics](#advanced-topics)
9. [Performance Analysis](#performance-analysis)
10. [GUIKit Integration](#guikit-integration)
11. [Shopping List](#shopping-list)
12. [FAQ](#faq)

---

## 🚨 The Problem: CS Line Exhaustion

### ESP8266 GPIO Reality

ESP8266 (NodeMCU v2/v3) has only **17 GPIO pins**, but many are pre-allocated:

| GPIO | Label | Default Use | Available? |
|------|-------|-------------|------------|
| 0 | D3 | TFT DC | ⚠️ Used |
| 1 | TX | Serial TX | ❌ Reserved |
| 2 | D4 | Built-in LED | ✅ Available |
| 3 | RX | Serial RX | ❌ Reserved |
| 4 | D2 | Touch CS | ⚠️ Used |
| 5 | D1 | Touch IRQ | ⚠️ Used |
| 9 | SD2 | TFT CS | ⚠️ Used |
| 10 | SD3 | TFT DC | ⚠️ Used |
| 12 | D6 | MISO | ✅ Available |
| 13 | D7 | MOSI | ✅ Available |
| 14 | D5 | SD Card CS | ⚠️ Used |
| 15 | D8 | TFT CS (alt) | ⚠️ Used |
| 16 | D0 | - | ✅ Available |

**Free GPIO (without expanders):** D0, D2, D6, D7 = **4 pins**

### The SPI Device Dilemma

Each SPI device requires:
- ✅ MOSI (shared)
- ✅ MISO (shared)
- ✅ SCK (shared)
- ❌ **CS (dedicated per device)**

**With 4 free GPIO, you can only control 4 SPI devices.**

### GUIKit SPI Device Requirements

| Device | Purpose | CS Needed |
|--------|---------|------------|
| TFT ST7789 | Display | 1 |
| XPT2046 Touch | Touch input | 1 |
| SD Card | Storage | 1 |
| SRAM (23LC1024) | External RAM | 1 |
| MCP23S17 | GPIO Expansion | 1 per chip |
| Additional SD Cards | More storage | 1 per card |
| Additional TFTs | Multi-display | 1 per display |

**Result:** Without CS expansion, you're limited to **~4 SPI devices total**.

---

## ✅ The Solution: Multiplexed CS Lines

### How It Works

A **digital multiplexer/demultiplexer** takes N control lines and outputs 2^N CS lines:

```
N Control Lines → 2^N CS Outputs
1 line   → 2 outputs
2 lines  → 4 outputs
3 lines  → 8 outputs
4 lines  → 16 outputs  ← 74HC4067
5 lines  → 32 outputs  ← Cascaded 74HC4067
```

### The Magic Formula

```
GPIO Used = Control Lines + Enable Lines
CS Lines  = 2 ^ Control Lines

Efficiency = CS Lines / GPIO Used

74HC4067: 5 GPIO → 16 CS = 3.2× expansion
2x 74HC4067: 10 GPIO → 32 CS = 3.2× expansion
```

### SPI Bus Architecture with Multiplexed CS

```
ESP8266
   ┌──────────────────────────────────────────────────────┐
   │                                                      │
   │   ┌─────────┐    ┌─────────┐    ┌─────────┐         │
   │   │  MOSI   │    │  MISO   │    │  SCK    │         │
   │   └────┬────┘    └────┬────┘    └────┬────┘         │
   │        │                │               │              │
   │        └────────────────┼───────────────┘              │
   │                         │                              │
   │                         ▼                              │
   │              ┌─────────────────────┐                  │
   │              │   74HC4067 MUX      │                  │
   │              │  16-Channel          │                  │
   │              │                     │                  │
   │   Control:   │ S0 S1 S2 S3  EN     │ ← 5 GPIO           │
   │   Outputs:   │ Y0 Y1 Y2 ... Y15   │ ← 16 CS lines      │
   │              └──────┬──────┬──────┬────┬              │
   │                     │      │      │    │              │
   │        ┌────────────┼────────────┼────────────┼────┐  │
   │        ▼            ▼            ▼            ▼       ▼  │
   │   ┌────────┐   ┌─────┐   ┌─────┐   ┌─────┐   ┌─────┐│
   │   │ SRAM   │   │SD   │   │TFT  │   │Touch│   │MCP  ││
   │   │ 23LC   │   │Card │   │ST7789│  │XPT2046││23S17││
   │   └────────┘   └─────┘   └─────┘   └─────┘   └─────┘│
   │                                            (CS)    │
   └──────────────────────────────────────────────────────┘
```

---

## 🔧 Multiplexer Types

### Comparison Table

| Chip | Type | Outputs | Control | Enable | Speed | Voltage | Price | Best For |
|------|------|---------|---------|--------|-------|---------|-------|----------|
| **74HC4067** | Analog MUX/DMUX | 16 | 4 (S0-S3) | 1 (EN) | 20+ MHz | 2-5.5V | ~$0.50 | **Best overall** |
| 74HCT4067 | Same, 5V tolerant | 16 | 4 | 1 | 20+ MHz | 4.5-5.5V | ~$0.75 | 5V systems |
| **74HC138** | 3-to-8 Decoder | 8 | 3 (A0-A2) | 3 (ENs) | 20+ MHz | 2-5.5V | ~$0.30 | 8 devices, active-low |
| 74HC139 | Dual 2-to-4 Decoder | 8 | 2 (A0-A1) | 1 | 20+ MHz | 2-5.5V | ~$0.40 | 2 groups of 4 |
| CD4051 | Analog MUX/DMUX | 8 | 3 (A0-A2) | 1 (INH) | 10 MHz | 3-15V | ~$0.60 | Analog signals |
| CD4052 | Dual 4-channel | 8 (4+4) | 2 (A0-A1) | 1 | 10 MHz | 3-15V | ~$0.70 | Dual bus |
| CD4053 | Triple 2-channel | 6 (2+2+2) | 1 (A0) | 1 | 10 MHz | 3-15V | ~$0.75 | Simple cases |

### Selection Guide

**Choose based on your needs:**

1. **74HC4067** - Need 16 CS lines, best price/performance
2. **74HC138** - Need only 8 CS lines, slightly simpler
3. **74HC139** - Need two separate groups of 4 CS lines each
4. **CD4051** - Working with analog signals or higher voltage

---

## ⭐ 74HC4067 - The 16-Channel Powerhouse

### Why 74HC4067 is Perfect for GUIKit

| Feature | Benefit |
|---------|---------|
| **16 outputs** | Control up to 16 SPI devices |
| **4 control lines** | Only 4 GPIO for addressing |
| **Bidirectional** | Can be used as MUX or DMUX |
| **High speed** | 20+ MHz switching (faster than SPI devices) |
| **Low cost** | ~$0.50-$1.50 per chip |
| **3.3V compatible** | Works directly with ESP8266/ESP32 |
| **Small package** | DIP-16 or SOIC-16 |
| **Widely available** | Stocked by all major suppliers |

### Pinout & Function

```
74HC4067 - 16-Channel Analog Multiplexer/Demultiplexer
┌───────────────────────────────────┐
│  IN/OUT    1 │ ▁        ▁ │ 24  VCC    │  
│    S0      2 │           │ 23  S3     │  ← Control input 3
│    S1      3 │           │ 22  S2     │  ← Control input 2
│    S2      4 │           │ 21  EN     │  ← Enable (active LOW)
│   Y0/Y16   5 │           │ 20  Y1     │  ← Output 1
│   Y1/Y15   6 │           │ 19  Y2     │  ← Output 2
│   Y2/Y14   7 │           │ 18  Y3     │  ← Output 3
│   Y3/Y13   8 │           │ 17  Y4     │  ← Output 4
│   Y4/Y12   9 │           │ 16  Y5     │  ← Output 5
│   Y5/Y11  10 │           │ 15  Y6     │  ← Output 6
│   Y6/Y10  11 │           │ 14  Y7     │  ← Output 7
│   Y7/Y9   12 │           │ 13  Y8     │  ← Output 8/9 (shared)
│    GND    13 │           │ 12  Y9/Y8   │  ← Output 8/9 (shared)
└───────────────────────────────────┘

Note: Y8 and Y9 share the same pin (pin 12)
```

**Truth Table:**

| EN | S3 | S2 | S1 | S0 | Selected Output |
|----|----|----|----|----|-----------------|
| H | X | X | X | X | None (all disabled) |
| L | L | L | L | L | Y0 |
| L | L | L | L | H | Y1 |
| L | L | L | H | L | Y2 |
| L | L | L | H | H | Y3 |
| L | L | H | L | L | Y4 |
| L | L | H | L | H | Y5 |
| L | L | H | H | L | Y6 |
| L | L | H | H | H | Y7 |
| L | H | L | L | L | Y8 |
| L | H | L | L | H | Y9 |
| L | H | L | H | L | Y10 |
| L | H | L | H | H | Y11 |
| L | H | H | L | L | Y12 |
| L | H | H | L | H | Y13 |
| L | H | H | H | L | Y14 |
| L | H | H | H | H | Y15 |

### Wiring Configuration

#### ESP8266 NodeMCU to 74HC4067

```
┌──────────────────────────────────────────────────────────┐
│                    WIRING DIAGRAM                          │
├──────────────────────────────────────────────────────────┤
│                                                              │
│  ESP8266 (NodeMCU)          74HC4067                        │
│  ───────────────────       ─────────                       │
│  D0  (GPIO16) ────► EN     (Pin 21)   Enable (active LOW)   │
│  D1  (GPIO5)   ────► S0     (Pin 2)   Address bit 0         │
│  D2  (GPIO4)   ────► S1     (Pin 3)   Address bit 1         │
│  D3  (GPIO0)   ────► S2     (Pin 4)   Address bit 2         │
│  D4  (GPIO2)   ────► S3     (Pin 23)  Address bit 3         │
│  3.3V        ────► VCC    (Pin 24)  Power                  │
│  GND         ────► GND    (Pin 13)  Ground                 │
│                                                              │
│  74HC4067 Outputs          SPI Device CS Lines             │
│  ───────────────────       ──────────────────            │
│  Y0  (Pin 5)   ────► SRAM Bank 0 CS                       │
│  Y1  (Pin 6)   ────► SRAM Bank 1 CS                       │
│  Y2  (Pin 7)   ────► SRAM Bank 2 CS                       │
│  Y3  (Pin 8)   ────► SRAM Bank 3 CS                       │
│  Y4  (Pin 9)   ────► SD Card CS                           │
│  Y5  (Pin 10)  ────► TFT Display CS                       │
│  Y6  (Pin 11)  ────► Touch Controller CS                  │
│  Y7  (Pin 12)  ────► GPIO Expander CS                    │
│  Y8  (Pin 15)  ────► (Available)                         │
│  Y9  (Pin 15)  ────► (Shared with Y8, use one)            │
│  Y10 (Pin 14)  ────► (Available)                         │
│  Y11 (Pin 15)  ────► (Available)                         │
│  Y12 (Pin 16)  ────► (Available)                         │
│  Y13 (Pin 17)  ────► (Available)                         │
│  Y14 (Pin 18)  ────► (Available)                         │
│  Y15 (Pin 19)  ────► (Available)                         │
│                                                              │
│  Shared SPI Bus:                                          │
│  D7  (GPIO13, MOSI) ────► All device MOSI pins            │
│  D6  (GPIO12, MISO) ────► All device MISO pins            │
│  D5  (GPIO14, SCK)  ────► All device SCK pins             │
│                                                              │
└──────────────────────────────────────────────────────────┘
```

**Total GPIO Used: 5 (D0-D4) + 3 (MOSI/MISO/SCK) = 8 pins**  
**CS Lines Available: 16**

---

## 🔄 Alternative Multiplexers

### 74HC138 - 3-to-8 Line Decoder

**When to use:** Need only 8 CS lines, want active-low outputs

```
74HC138 Pinout:
┌─────────────────────────────┐
│   A0    1 │                 │ 16  VCC   │
│   A1    2 │                 │ 15  Y0    │
│   A2    3 │                 │ 14  Y1    │
│   E1   4 │                 │ 13  Y2    │
│   E2A  5 │                 │ 12  Y3    │
│   E2B  6 │                 │ 11  Y4    │
│   GND  7 │                 │ 10  Y5    │
│          │                 │ 9   Y6    │
│          │                 │ 8   Y7    │
└──────────┴─────────────────┘

Truth Table (all E must be LOW):
A2 A1 A0 | Output
 0  0  0 → Y0 (active LOW)
 0  0  1 → Y1 (active LOW)
 ...
 1  1  1 → Y7 (active LOW)
```

**Pros:**
- Only 3 control lines needed
- Active-low outputs (common for SPI CS)
- Slightly cheaper

**Cons:**
- Only 8 outputs (vs 16 for 74HC4067)
- 3 enable lines instead of 1

### 74HC139 - Dual 2-to-4 Line Decoder

**When to use:** Need two separate groups of 4 devices each

```
Two independent 2-to-4 decoders in one chip:
- Decoder 1: A0, A1, E1 → Y0-Y3
- Decoder 2: A0, A1, E2 → Y4-Y7

Useful when you want to group devices (e.g., Group A: RAM, Group B: Peripherals)
```

### Cascading for More Outputs

#### 2x 74HC4067 = 32 CS Lines

```
Level 1 (Selector MUX):
- 1 chip selects which of 2 device MUXes to enable
- Uses 5 GPIO (EN1 + S0-S3)

Level 2 (Device MUXes):
- 2 chips, each with 16 outputs
- Each enabled by Level 1 outputs Y0 and Y1

Total:
- GPIO: 5 (Level 1) + 0 (Level 2 shares) = 5
- CS Lines: 16 + 16 = 32
- Expansion: 6.4×
```

#### 4x 74HC4067 = 64 CS Lines

```
Level 1 (Selector MUX):
- 1 chip (74HC4067) selects which of 4 device MUXes
- Uses 5 GPIO → 16 outputs, use first 4

Level 2 (Device MUXes):
- 4 chips, each with 16 outputs
- Each enabled by Level 1 outputs Y0-Y3

Total:
- GPIO: 5
- CS Lines: 16 × 4 = 64
- Expansion: 12.8×
```

---

## 🔌 Wiring & Connection

### Single 74HC4067 Wiring

```
┌─────────────────────────────────────────────────────────────┐
│                    DETAILED WIRING SCHEME                       │
├─────────────────────────────────────────────────────────────┤
│                                                                 │
│  ESP8266                                                          │
│  ┌─────────┐                                                     │
│  │ D0/16   │──┬───────────────────────────────────────► EN   │
│  │ D1/5    │──┼───────┐                                   │
│  │ D2/4    │──┼───────┼───────────────────────► S0    │
│  │ D3/0    │──┼───────┼───────┐               │           │
│  │ D4/2    │──┼───────┼───────┼───────┐       │           │
│  │ D5/14   │──┼───────┼───────┼───────┼───► SCK    │
│  │ D6/12   │──┼───────┼───────┼───────┼───► MISO   │
│  │ D7/13   │──┴───────┴───────┴───────┴───► MOSI   │
│  │ 3.3V    │───────────────────────────────────► VCC   │
│  │ GND     │───────────────────────────────────► GND   │
│  └─────────┘                                                     │
│         │                                                         │
│         ▼                                                         │
│  ┌─────────────────┐                                             │
│  │    74HC4067     │                                             │
│  │  16-Channel MUX │                                             │
│  │                 │                                             │
│  │   ┌─────────┐  │                                             │
│  │   │ S0-S3  │  │◄──── Control lines (D1-D4)                │
│  │   │ EN     │  │◄──── Enable (D0, active LOW)                │
│  │   └─────────┘  │                                             │
│  │                 │                                             │
│  │  Outputs:       │                                             │
│  │  Y0-Y15 ────────► 16 CS lines to SPI devices               │
│  └─────────────────┘                                             │
│         │                                                         │
│    ┌────┴────┐    ┌────┴────┐    ┌────┴────┐    ┌────┴────┐   │
│    ▼         ▼         ▼         ▼           ▼             │
│  ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐              │
│  │SRAM │  │SD   │  │TFT  │  │Touch│  │MCP  │              │
│  │23LC │  │Card │  │ST77 │  │XPT2 │  │23S17│              │
│  └──┬──┘  └──┬──┘  └──┬──┘  └──┬──┘  └──┬──┘              │
│     │       │        │        │        │                    │
│     └───────┼────────┼────────┼────────┴───────┐          │
│             │        │        │                   │          │
│        MOSI ◄─┴────────┴────────┴───────────────┴──┐      │
│        MISO ─┬────────┬────────┬───────────────────┬──┘      │
│             │        │        │                   │          │
│        SCK  ◄─┴────────┴────────┴───────────────┴──┘      │
│                                                                 │
└─────────────────────────────────────────────────────────────┘
```

### Multiple 74HC4067 Cascade Wiring

```
For 32 CS lines (2x 74HC4067):

Level 1 MUX (Selector):
  EN1  ← D0
  S0   ← D1  (selects MUX A or B)
  S1-S3 ← Not used (or cascade further)
  
Level 2 MUX A:
  EN2  ← Y0 from Level 1
  S0-S3 ← D2-D5
  Y0-Y15 → Devices 0-15
  
Level 2 MUX B:
  EN2  ← Y1 from Level 1
  S0-S3 ← D2-D5 (same as MUX A)
  Y0-Y15 → Devices 16-31

Total: 6 GPIO (D0-D5) → 32 CS lines
```

### Power & Decoupling

**Important:** Always add decoupling capacitors!

```
74HC4067 Power:
  VCC ────┬──── 3.3V
           │
          △ 0.1uF (ceramic)
           │
          △ 10uF (electrolytic)
           │
          GND
```

---

## 💻 Software Implementation

### Basic Arduino Implementation

```cpp
// Pin definitions
#define MUX_EN   D0    // Enable (active LOW)
#define MUX_S0   D1    // Address bit 0
#define MUX_S1   D2    // Address bit 1
#define MUX_S2   D3    // Address bit 2
#define MUX_S3   D4    // Address bit 3

// Device CS line assignments
#define SRAM_0_CS   0
#define SRAM_1_CS   1
#define SD_CARD_CS  4
#define TFT_CS      5
#define TOUCH_CS    6

afio
// Initialize multiplexer
void setupMux() {
    pinMode(MUX_EN, OUTPUT);
    pinMode(MUX_S0, OUTPUT);
    pinMode(MUX_S1, OUTPUT);
    pinMode(MUX_S2, OUTPUT);
    pinMode(MUX_S3, OUTPUT);
    
    // Start with all CS lines deselected
    digitalWrite(MUX_EN, HIGH);
}

// Select a specific CS line (0-15)
void selectCS(uint8_t deviceNum) {
    // Validate input
    if (deviceNum > 15) return;
    
    // Enable the multiplexer
    digitalWrite(MUX_EN, LOW);
    
    // Set address bits
    digitalWrite(MUX_S0, bitRead(deviceNum, 0));
    digitalWrite(MUX_S1, bitRead(deviceNum, 1));
    digitalWrite(MUX_S2, bitRead(deviceNum, 2));
    digitalWrite(MUX_S3, bitRead(deviceNum, 3));
    
    // Small delay for signal propagation
    delayMicroseconds(1);
}

// Deselect all CS lines
void deselectAll() {
    digitalWrite(MUX_EN, HIGH);
}

// Example: Read from SRAM Bank 0
uint8_t readSram0(uint16_t address) {
    selectCS(SRAM_0_CS);
    uint8_t data = sram.read(address);
    deselectAll();
    return data;
}

// Example: Write to SD Card
void writeSdCard(uint32_t address, uint8_t data) {
    selectCS(SD_CARD_CS);
    sdFile.write(data);
    deselectAll();
}
```

### Optimized Implementation with Registers

For maximum speed (ESP8266):

```cpp
// Direct register access for ESP8266
#define MUX_PINS_MASK ((1 << MUX_EN) | (1 << MUX_S0) | (1 << MUX_S1) | (1 << MUX_S2) | (1 << MUX_S3))

// Select CS line with minimal overhead
void selectCS_fast(uint8_t deviceNum) {
    // Clear all MUX pins first
    GPIO_OUTPUT_REG &= ~MUX_PINS_MASK;
    
    // Set EN to LOW (active) and address bits
    // Note: EN is active LOW, so we invert it
    uint32_t bits = (deviceNum & 0x0F) | (1 << MUX_EN);
    GPIO_OUTPUT_REG |= bits;
    
    // No delay needed for most applications
}

// Deselect all - set EN HIGH
void deselectAll_fast() {
    GPIO_OUTPUT_REG |= (1 << MUX_EN);
    GPIO_OUTPUT_REG &= ~((1 << MUX_S0) | (1 << MUX_S1) | (1 << MUX_S2) | (1 << MUX_S3));
}
```

### Class-Based Implementation

```cpp
class CSMultiplexer {
private:
    uint8_t enPin;
    uint8_t s0Pin, s1Pin, s2Pin, s3Pin;
    
public:
    CSMultiplexer(uint8_t en, uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3)
        : enPin(en), s0Pin(s0), s1Pin(s1), s2Pin(s2), s3Pin(s3) {}
    
    void begin() {
        pinMode(enPin, OUTPUT);
        pinMode(s0Pin, OUTPUT);
        pinMode(s1Pin, OUTPUT);
        pinMode(s2Pin, OUTPUT);
        pinMode(s3Pin, OUTPUT);
        deselect();
    }
    
    void select(uint8_t device) {
        if (device > 15) return;
        digitalWrite(enPin, LOW);
        digitalWrite(s0Pin, bitRead(device, 0));
        digitalWrite(s1Pin, bitRead(device, 1));
        digitalWrite(s2Pin, bitRead(device, 2));
        digitalWrite(s3Pin, bitRead(device, 3));
    }
    
    void deselect() {
        digitalWrite(enPin, HIGH);
    }
    
    // Transaction wrapper for SPI
    void transaction(uint8_t device, void (*callback)()) {
        select(device);
        callback();
        deselect();
    }
};

// Usage
CSMultiplexer mux(D0, D1, D2, D3, D4);

void setup() {
    mux.begin();
}

void loop() {
    mux.transaction(SRAM_0_CS, []() {
        sram.read(0x1234);
    });
}
```

### Integration with SPI Library

```cpp
// Wrapper class for multiplexed SPI devices
class MultiplexedSPI {
private:
    SPIClass* spi;
    CSMultiplexer* mux;
    uint8_t deviceNum;
    
public:
    MultiplexedSPI(SPIClass* s, CSMultiplexer* m, uint8_t dev)
        : spi(s), mux(m), deviceNum(dev) {}
    
    void begin() {
        spi->begin();
    }
    
    uint8_t transfer(uint8_t data) {
        mux->select(deviceNum);
        uint8_t result = spi->transfer(data);
        mux->deselect();
        return result;
    }
    
    void beginTransaction(uint32_t speed, uint8_t mode) {
        mux->select(deviceNum);
        spi->beginTransaction(SPISettings(speed, MSBFIRST, mode));
    }
    
    void endTransaction() {
        spi->endTransaction();
        mux->deselect();
    }
};
```

---

## 🚀 Advanced Topics

### Dynamic Speed Configuration

Different devices need different SPI speeds. With multiplexed CS, you can change speed per device:

```cpp
void accessDevice(uint8_t device, uint32_t speed, void (*callback)()) {
    // Select device
    mux.select(device);
    
    // Configure SPI for this device's speed
    SPI.beginTransaction(SPISettings(speed, MSBFIRST, SPI_MODE0));
    
    // Execute operation
    callback();
    
    // Clean up
    SPI.endTransaction();
    mux.deselect();
}

// Usage
accessDevice(SRAM_0_CS, 8000000, []() {
    sram.read(address, buffer, length);
});

accessDevice(TFT_CS, 40000000, []() {
    tft.drawPixel(x, y, color);
});
```

### Interrupt Handling

**Problem:** Multiplexer switching takes time. If an interrupt occurs during access, the CS line might be left active.

**Solution:** Use atomic operations or disable interrupts during critical sections:

```cpp
// Atomic CS selection
void selectCS_atomic(uint8_t device) {
    noInterrupts();
    selectCS(device);
    // Don't restore interrupts yet - keep CS stable during transaction
}

void deselectAll_atomic() {
    deselectAll();
    interrupts();  // Safe to restore now
}
```

### Multiple SPI Buses

ESP32 has multiple SPI buses (HSPI, VSPI). You can use separate multiplexers for each:

```cpp
// HSPI for fast devices (TFT, SRAM)
CSMultiplexer hspiMux(D0, D1, D2, D3, D4);

// VSPI for slower devices (SD Card, Touch)
CSMultiplexer vspiMux(D5, D6, D7, D8, D9);

// Each bus has its own multiplexer
// Total: 10 GPIO → 32 CS lines (16 per bus)
```

### Daisy-Chaining Multiplexers

For very large systems:

```cpp
// 4x 74HC4067 = 64 CS lines
class CascadeMux {
private:
    uint8_t enPin;
    uint8_t s0Pin, s1Pin, s2Pin, s3Pin;
    uint8_t subMuxCount = 4;  // Number of downstream multiplexers
    
public:
    CascadeMux(uint8_t en, uint8_t s0, uint8_t s1)
        : enPin(en), s0Pin(s0), s1Pin(s1) {}
    
    void select(uint8_t device) {
        // Upper 2 bits select which sub-mux
        uint8_t subMux = (device >> 4) & 0x03;
        uint8_t subDevice = device & 0x0F;
        
        // Select sub-mux (only uses S0, S1)
        digitalWrite(enPin, LOW);
        digitalWrite(s0Pin, bitRead(subMux, 0));
        digitalWrite(s1Pin, bitRead(subMux, 1));
        
        // Activate sub-mux and select device on it
        // (This would require additional connections to sub-mux EN lines)
    }
};
```

---

## ⚡ Performance Analysis

### Timing Breakdown

| Operation | Time | Notes |
|-----------|------|-------|
| GPIO write (digitalWrite) | ~1-5 µs | Depends on Arduino implementation |
| Direct register access | ~50-100 ns | Fastest method |
| Multiplexer propagation | ~20-50 ns | 74HC4067 datasheet |
| SPI transaction setup | ~1-2 µs | beginTransaction overhead |
| SPI byte transfer | ~1-10 µs | Depends on clock speed |

### Total Access Time

```
Method 1: digitalWrite()
  selectCS()   : 4× GPIO writes × 5µs = 20µs
  + multiplexer : 50ns
  + SPI setup  : 2µs
  = ~22µs overhead per transaction

Method 2: Direct Register Access
  selectCS()   : 1× register write = 100ns
  + multiplexer : 50ns
  + SPI setup  : 2µs
  = ~2.15µs overhead per transaction

Method 3: Optimized (pre-configured SPI)
  selectCS()   : 100ns
  + SPI transfer: 1µs (at 8MHz)
  + deselect   : 100ns
  = ~1.2µs per byte
```

### Comparison with Direct GPIO

| Method | Overhead | Max Throughput | Complexity |
|--------|----------|----------------|------------|
| Direct GPIO CS | ~500ns | ~2 MB/s | ⭐ |
| Multiplexer (digitalWrite) | ~22µs | ~45 KB/s | ⭐⭐ |
| Multiplexer (register) | ~1.2µs | ~800 KB/s | ⭐⭐⭐ |
| Direct GPIO (no mux) | ~500ns | ~2 MB/s | ⭐ |

**Conclusion:** Multiplexer overhead is negligible for most SPI devices (which typically run at 1-40 MHz). The 1-2µs overhead is acceptable for all GUIKit applications.

### Real-World Performance

```
TFT ST7789:
  Pixel write: ~50µs (includes multiplexer overhead)
  Screen fill: 320×240×50µs = 3.84 seconds
  Without mux: ~3.6 seconds
  Difference: 7% (negligible)

SD Card:
  Sector read: ~15ms (SPI at 4MHz)
  Multiplexer overhead: ~2µs
  Difference: 0.013% (negligible)

SRAM:
  Byte read: ~100ns
  Multiplexer overhead: ~1µs
  Difference: 0.1% (negligible)
```

---

## 🖥️ GUIKit Integration

### Recommended Configuration

```
For ESP8266 GUIKit with 16 devices:

CS Line Map:
  Y0  → SRAM Bank 0 (23LC1024, 128KB)
  Y1  → SRAM Bank 1 (23LC1024, 128KB)
  Y2  → SRAM Bank 2 (LY68L6400, 512KB)
  Y3  → SRAM Bank 3 (LY68L6400, 512KB)
  Y4  → SD Card Slot 0 (Primary)
  Y5  → SD Card Slot 1 (Backup/Expansion)
  Y6  → TFT Display (ST7789)
  Y7  → Touch Controller (XPT2046)
  Y8  → GPIO Expander 0 (MCP23S17)
  Y9  → GPIO Expander 1 (MCP23S17)
  Y10 → GPIO Expander 2 (MCP23S17)
  Y11 → GPIO Expander 3 (MCP23S17)
  Y12 → GPIO Expander 4 (MCP23S17)
  Y13 → Audio DAC (MAX98357)
  Y14 → RTC (DS3231 via SPI)
  Y15 → Reserved

Total Memory:
  SRAM: 128KB × 2 + 512KB × 2 = 1.25 MB
  SD Card: 2 × 32GB (theoretical)
  GPIO: 16 × 16 = 256 pins

All controlled with just 5 GPIO pins!
```

### Hardware Configuration Struct

```cpp
typedef struct {
    // Multiplexer control
    uint8_t mux_en;
    uint8_t mux_s0;
    uint8_t mux_s1;
    uint8_t mux_s2;
    uint8_t mux_s3;
    
    // Device assignments
    uint8_t sram_cs[4];      // CS lines for SRAM banks
    uint8_t sd_cs[2];       // CS lines for SD cards
    uint8_t tft_cs;
    uint8_t touch_cs;
    uint8_t expander_cs[8]; // CS lines for GPIO expanders
    
    // Device counts
    uint8_t sram_count;
    uint8_t sd_count;
    uint8_t expander_count;
    
} guikit_mux_config_t;
```

### Initialization Code

```cpp
// In guikit_hw_config.cpp
void guikit_hw_init_mux() {
    guikit_mux_config_t muxConfig = {
        .mux_en = D0,
        .mux_s0 = D1,
        .mux_s1 = D2,
        .mux_s2 = D3,
        .mux_s3 = D4,
        .sram_cs = {0, 1, 2, 3},
        .sd_cs = {4, 5},
        .tft_cs = 6,
        .touch_cs = 7,
        .expander_cs = {8, 9, 10, 11, 12, 13, 14, 15},
        .sram_count = 4,
        .sd_count = 2,
        .expander_count = 8
    };
    
    muxManager.begin(&muxConfig);
}
```

---

## 🛒 Shopping List

### Primary Recommendation

| Item | Quantity | Price | Supplier | Notes |
|------|----------|-------|----------|-------|
| **74HC4067** | 1-2 | ~$0.50-$1.50 | LCSC, DigiKey, Mouser | 16-channel MUX/DMUX |
| 0.1µF Ceramic Cap | 1 per chip | ~$0.05 | Any | Decoupling |
| 10µF Electrolytic Cap | 1 per chip | ~$0.10 | Any | Bulk decoupling |
| Jumper Wires | As needed | ~$5 | Any | For prototyping |

### Alternative Options

| Item | Quantity | Price | Use Case |
|------|----------|-------|----------|
| 74HCT4067 | 1 | ~$0.75 | 5V tolerant systems |
| 74HC138 | 1-2 | ~$0.30 | 8 CS lines, active-low |
| 74HC139 | 1 | ~$0.40 | Dual 2-to-4 groups |
| CD4051 | 2 | ~$0.60 | 8 CS lines, analog |

### Complete 16-Device System

| Component | Quantity | Price | Total |
|-----------|----------|-------|-------|
| ESP8266 NodeMCU | 1 | $5 | $5 |
| 74HC4067 | 1 | $1 | $1 |
| 23LC1024 SRAM | 2 | $3 each | $6 |
| LY68L6400 SRAM | 2 | $4 each | $8 |
| MicroSD Card | 1 | $2 | $2 |
| TFT ST7789 | 1 | $8 | $8 |
| XPT2046 Touch | 1 | $3 | $3 |
| MCP23S17 | 4 | $2 each | $8 |
| Capacitors | - | $2 | $2 |
| **Total** | | | **~$45** |

---

## ❓ FAQ

### Q: Can I use a multiplexer with I2C?
**A:** No, I2C uses address lines, not CS lines. Multiplexers are specifically for SPI CS expansion.

### Q: What's the maximum number of devices I can control?
**A:** Theoretically unlimited with cascading. Practically:
- 1x 74HC4067: 16 devices (5 GPIO)
- 2x 74HC4067: 32 devices (10 GPIO)
- 4x 74HC4067: 64 devices (10 GPIO)
- ESP8266 has ~17 GPIO, so ~64 devices is practical maximum

### Q: Does the multiplexer add latency?
**A:** Yes, but negligible. The 74HC4067 switches in ~20-50ns, which is much faster than SPI transaction setup time.

### Q: Can I use analog multiplexers like CD4051?
**A:** Yes, but they're slower (10MHz vs 20+MHz). For digital CS lines, 74HC4067 is better.

### Q: What about active-low vs active-high?
**A:** 74HC4067 outputs are active-high when enabled. Most SPI devices use active-low CS, so just invert in software or use a chip with active-low outputs (like 74HC138).

### Q: Can I share the multiplexer with other signals?
**A:** No, the multiplexer outputs are dedicated to CS lines. Keep it simple.

### Q: Do I need pull-up/down resistors?
**A:** Not usually. The 74HC4067 outputs are push-pull. However, ensure EN is held HIGH at boot to prevent glitches.

### Q: What if my device needs a different SPI mode?
**A:** Configure SPI mode in software before selecting the device. The multiplexer doesn't care about SPI mode.

### Q: Can I use this with ESP32?
**A:** Absolutely! ESP32 has more GPIO, but you can still benefit from multiplexing if you need many SPI devices.

### Q: How do I handle devices with different voltage requirements?
**A:** 74HC4067 is 3.3V-5V tolerant. For 5V-only devices, use level shifters on the CS lines.

---

## 🎯 Summary

### Key Points

1. **Problem:** ESP8266 has limited GPIO, restricting SPI device count
2. **Solution:** Use 74HC4067 multiplexer to expand 5 GPIO → 16 CS lines
3. **Benefit:** Control 16+ SPI devices with minimal GPIO overhead
4. **Performance:** Negligible overhead (~1-2µs per transaction)
5. **Cost:** Extremely low (~$0.50 per chip)

### Recommended Setup

For most GUIKit systems:
- **1x 74HC4067** for 16 CS lines
- **5 GPIO pins** (D0-D4)
- **Unlimited SPI devices** (within the 16 limit)

For very large systems:
- **2x 74HC4067** for 32 CS lines
- **10 GPIO pins** (D0-D9)
- **Cascaded configuration**

### Next Steps

1. Order 74HC4067 chips
2. Wire as shown in diagrams
3. Implement the software library
4. Test with 2-3 devices first
5. Scale up as needed

---

*Document generated by Mistral Vibe*  
*Topic: SPI CS Line Multiplexer Expansion for ESP8266/ESP32 GUIKit*  
*Date: 2026-08-17*  
*Related: [about_port_expander.md](about_port_expander.md)*
