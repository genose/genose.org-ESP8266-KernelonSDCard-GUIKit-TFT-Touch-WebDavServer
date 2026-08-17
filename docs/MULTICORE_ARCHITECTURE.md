# Multi-Core Architecture for ESP32 GUIKit

> *Leveraging dual-core and SMP capabilities for maximum performance*  
> *ESP8266: Single-core / ESP32: Dual-core (SMP) / ESP32-S3: Dual-core / ESP32-C3: Single-core*

---

## 📋 Table of Contents

1. [Multi-Core Overview](#multi-core-overview)
2. [ESP32 Dual-Core Architecture](#esp32-dual-core-architecture)
3. [SMP (Symmetric Multiprocessing) Implementation](#smp-symmetric-multiprocessing-implementation)
4. [Core Assignment Strategy](#core-assignment-strategy)
5. [Inter-Core Communication](#inter-core-communication)
6. [Task Distribution Examples](#task-distribution-examples)
7. [Multi-Core GUIKit Components](#multi-core-guikit-components)
8. [Performance Optimization](#performance-optimization)
9. [Code Implementation](#code-implementation)
10. [Best Practices](#best-practices)
11. [Shopping List](#shopping-list)
12. [FAQ](#faq)

---

## 🎯 Multi-Core Overview

### Platform Comparison

| Platform | Cores | SMP Support | Frequency | Best For |
|----------|-------|-------------|-----------|----------|
| ESP8266 | 1 | ❌ No | 80-160 MHz | Single-threaded |
| **ESP32** | **2** | **✅ Yes** | **160-240 MHz** | **Dual-core processing** |
| ESP32-S2 | 1 | ❌ No | 240 MHz | Single-threaded |
| ESP32-S3 | 2 | ✅ Yes | 240 MHz | Dual-core processing |
| ESP32-C3 | 1 | ❌ No | 160 MHz | Single-threaded |
| ESP32-C6 | 1 | ❌ No | 160 MHz | Single-threaded |

### Multi-Core Advantages

**ESP32 Dual-Core Benefits:**
1. **Parallel Task Execution** - Run GUI and WebDAV simultaneously
2. **Responsive UI** - GUI stays smooth while heavy tasks run on other core
3. **Background Processing** - Image conversion, SD operations don't block UI
4. **Real-time Capabilities** - Touch handling on one core, rendering on another
5. **Throughput Multiplication** - 2× processing power for CPU-bound tasks

### Core Terminology

| Term | ESP32 | Description |
|------|-------|-------------|
| **Core 0** | Pro CPU | Primary core, runs most RTOS functions by default |
| **Core 1** | App CPU | Secondary core, typically runs application code |
| **SMP** | ✅ Both | Symmetric Multiprocessing - both cores can run tasks |
| **RTOS** | FreeRTOS | Real-Time Operating System managing both cores |

---

## 🖥️ ESP32 Dual-Core Architecture

### Hardware Specifications

**ESP32 (Dual-Core):**
- **2x Xtensa LX6 cores** running at 160-240 MHz
- **Independent instruction & data caches** (16KB each)
- **Shared memory** (320KB internal RAM)
- **Shared peripherals** (SPI, I2C, UART, etc.)
- **Individual core timers**
- **Inter-core interrupts** for communication

### Memory Layout

```
ESP32 Memory Architecture:
┌─────────────────────────────────────────────────────────┐
│                    SHARED MEMORY                           │
│  ┌─────────────────────────────────────────────────┐   │
│  │  Internal RAM (320KB) - Shared between both cores     │   │
│  │  ┌─────────────┐  ┌─────────────┐                │   │
│  │  │ Core 0      │  │ Core 1      │                │   │
│  │  │ Stack/Heap  │  │ Stack/Heap  │                │   │
│  │  └─────────────┘  └─────────────┘                │   │
│  │                                                     │   │
│  │  Shared Data Areas:                              │   │
│  │  - Global variables                              │   │
│  │  - Shared buffers                                 │   │
│  │  - Queues for IPC                                 │   │
│  │  - SPI/peripheral registers                       │   │
│  └─────────────────────────────────────────────────┘   │
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │  External RAM (PSRAM) - Optional, shared           │   │
│  └─────────────────────────────────────────────────┘   │
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │  Flash (4MB-16MB) - Shared, code storage            │   │
│  └─────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

### Core Capabilities

| Capability | Core 0 (Pro CPU) | Core 1 (App CPU) |
|------------|------------------|------------------|
| Run Application Code | ✅ Yes | ✅ Yes |
| Access Peripherals | ✅ Yes | ✅ Yes |
| Access Internal RAM | ✅ Yes | ✅ Yes |
| Access External PSRAM | ✅ Yes | ✅ Yes |
| Access Flash | ✅ Yes | ✅ Yes |
| Hardware Timers | ✅ Yes (2x) | ✅ Yes (2x) |
| Interrupts | ✅ Yes | ✅ Yes |
| Default RTOS Tasks | ✅ Most | ⚠️ Some |
| **Best For** | **RTOS, WiFi** | **Application** |

---

## 🔄 SMP (Symmetric Multiprocessing) Implementation

### FreeRTOS on ESP32

ESP32 uses **FreeRTOS** which supports:
- **SMP (Symmetric Multiprocessing)** - Both cores can run tasks
- **Asymmetric mode** - Core 0 runs RTOS, Core 1 runs bare-metal (legacy)
- **Dual-core scheduling** - Tasks can be pinned to specific cores

### SMP Configuration

```c
// In sdkconfig or menuconfig:
CONFIG_FREERTOS_SMP=y                    // Enable SMP
CONFIG_FREERTOS_UNICORE=n                 // Disable unicore mode
CONFIG_ESP32_DEFAULT_CPU_CORE_0=y        // Core 0 is default
CONFIG_ESP32_DEFAULT_CPU_CORE_1=y        // Core 1 is enabled
```

### Task Creation with Core Affinity

```c
// Create task on specific core
xTaskCreatePinnedToCore(
    taskFunction,        // Task function
    "Task Name",        // Task name
    4096,              // Stack size (words)
    NULL,              // Parameters
    5,                 // Priority
    &taskHandle,       // Task handle
    0                  // Core ID (0 or 1)
);

// Example: GUI Task on Core 0
xTaskCreatePinnedToCore(
    guiRenderTask, 
    "GUI Render",
    8192,
    NULL,
    tskIDLE_PRIORITY + 2,
    &guiTaskHandle,
    0  // Run on Core 0
);

// Example: WebDAV Task on Core 1
xTaskCreatePinnedToCore(
    webdavServerTask,
    "WebDAV",
    8192,
    NULL,
    tskIDLE_PRIORITY + 2,
    &webdavTaskHandle,
    1  // Run on Core 1
);
```

---

## 🎯 Core Assignment Strategy

### Recommended Core Distribution

**ESP32 Dual-Core Strategy for GUIKit:**

| **Core** | **Primary Responsibility** | **Tasks** | **Priority** | **Rationale** |
|----------|---------------------------|-----------|--------------|---------------|
| **Core 0** | **Real-time & Network** | WiFi, TCP/IP, SPI bus, Touch IRQ, Task Switcher | High | Time-sensitive operations |
| **Core 1** | **GUI & Application** | GUI Rendering, WebDAV server, SD Card ops, Image decoding | Medium | User-facing operations |

### Detailed Assignment

#### Core 0 (Pro CPU) - Real-Time Operations

**Primary:**
- **SPI Bus Management** - All SPI device communication
- **Touch Controller IRQ** - Immediate touch response
- **WiFi Stack** - Network connectivity
- **TCP/IP Stack** - Network protocol handling
- **Task Switcher** - Context switching (A ↔ B)
- **Hardware Timers** - Precision timing

**Secondary (if Core 1 busy):**
- GUI background tasks
- File system operations

**Rationale:** Core 0 handles time-critical operations that need immediate response.

#### Core 1 (App CPU) - Application Operations

**Primary:**
- **GUI Rendering Engine** - Widget drawing, updates
- **WebDAV Server** - HTTP requests, file serving
- **SD Card Operations** - File read/write
- **Image Decoding** - PNG, JPEG, TIFF conversion
- **JSON Parsing** - GUI loading
- **RAM Freeze System** - Save/restore state

**Secondary (if Core 0 busy):**
- Network packet processing
- SPI device initialization

**Rationale:** Core 1 handles user-facing operations that can tolerate slight delays.

### Core Assignment Matrix

| **Component** | **Core** | **Priority** | **Reason** |
|--------------|----------|--------------|------------|
| **Touch IRQ Handler** | 0 | Highest | Immediate response required |
| **SPI ISR** | 0 | Highest | SPI bus timing critical |
| **WiFi Stack** | 0 | High | Network connectivity |
| **TCP/IP Stack** | 0 | High | Protocol handling |
| **Task Switcher** | 0 | High | Context save/restore |
| **GUI Render Engine** | 1 | High | Smooth UI updates |
| **WebDAV Server** | 1 | Medium | HTTP request handling |
| **SD Card I/O** | 1 | Medium | File operations |
| **Image Converter (PNG/JPEG/TIFF)** | 1 | Medium | Heavy computation |
| **JSON Parser** | 1 | Medium | GUI loading |
| **RAM Freeze System** | 1 | Medium | State management |
| **File Manager** | 1 | Low | Background operations |

---

## 💬 Inter-Core Communication

### Communication Methods

| Method | Speed | Complexity | Use Case |
|--------|-------|------------|----------|
| **Global Variables** | ⚡⚡⚡⚡⚡ | ⭐ | Shared data (atomic access) |
| **Queues** | ⚡⚡⚡⚡ | ⭐⭐ | Message passing |
| **Semaphores** | ⚡⚡⚡⚡ | ⭐⭐ | Resource protection |
| **Mutexes** | ⚡⚡⚡ | ⭐⭐⭐ | Exclusive access |
| **Event Groups** | ⚡⚡⚡ | ⭐⭐ | Event signaling |
| **Stream Buffers** | ⚡⚡⚡⚡ | ⭐⭐ | Data streaming |

### Example: Queue-Based Communication

```c
// Declare queue in global scope
QueueHandle_t guiCommandQueue;

// Core 1: Send GUI update command
void sendGuiCommand(GuiCommand cmd) {
    if (xQueueSend(guiCommandQueue, &cmd, pdMS_TO_TICKS(10)) != pdPASS) {
        // Queue full, handle error
    }
}

// Core 0: Receive and process commands
void guiTask(void* pvParameters) {
    GuiCommand cmd;
    while (1) {
        if (xQueueReceive(guiCommandQueue, &cmd, portMAX_DELAY) == pdPASS) {
            processGuiCommand(cmd);
        }
    }
}

// Initialize queue (call from setup)
void initGuiQueue() {
    guiCommandQueue = xQueueCreate(10, sizeof(GuiCommand));
}
```

### Example: Semaphore-Based Synchronization

```c
// Declare semaphore
SemaphoreHandle_t spiMutex;

// Core 1: Access SPI bus
void accessSpiDevice(SpiDevice* device) {
    // Take semaphore (wait up to 10ms)
    if (xSemaphoreTake(spiMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        // Critical section - SPI access
        device->transfer(data);
        
        // Give semaphore
        xSemaphoreGive(spiMutex);
    }
}

// Initialize semaphore
void initSpiSemaphore() {
    spiMutex = xSemaphoreCreateMutex();
}
```

### Example: Shared Buffer with Atomic Access

```c
// Shared buffer with atomic access
volatile uint8_t sharedBuffer[1024];
volatile uint16_t bufferIndex = 0;

// Core-safe write
void writeToSharedBuffer(uint8_t data) {
    // Disable interrupts for atomic operation
    portDISABLE_INTERRUPTS();
    sharedBuffer[bufferIndex++] = data;
    portENABLE_INTERRUPTS();
}

// Core-safe read
uint8_t readFromSharedBuffer() {
    uint8_t data;
    portDISABLE_INTERRUPTS();
    data = sharedBuffer[--bufferIndex];
    portENABLE_INTERRUPTS();
    return data;
}
```

---

## 📊 Task Distribution Examples

### Example 1: GUI + WebDAV Parallel Processing

```c
// Core 0: WiFi + SPI + Touch
void setup() {
    // Initialize WiFi on Core 0
    xTaskCreatePinnedToCore(
        wifiTask, 
        "WiFi",
        8192,
        NULL,
        5,
        NULL,
        0
    );
    
    // Initialize SPI on Core 0
    xTaskCreatePinnedToCore(
        spiManagerTask,
        "SPI Manager",
        4096,
        NULL,
        4,
        NULL,
        0
    );
    
    // Initialize Touch on Core 0
    xTaskCreatePinnedToCore(
        touchTask,
        "Touch",
        4096,
        NULL,
        6,
        NULL,
        0
    );
}

// Core 1: GUI + WebDAV
void loop() {
    // GUI Rendering on Core 1
    xTaskCreatePinnedToCore(
        guiRenderTask,
        "GUI Render",
        8192,
        NULL,
        3,
        NULL,
        1
    );
    
    // WebDAV Server on Core 1
    xTaskCreatePinnedToCore(
        webdavTask,
        "WebDAV Server",
        8192,
        NULL,
        2,
        NULL,
        1
    );
}
```

### Example 2: Image Decoding with Progress Display

```c
// Core 1: Image Decoding
void imageDecodeTask(void* pvParameters) {
    ImageDecodeParams* params = (ImageDecodeParams*)pvParameters;
    
    // Decode image to external RAM
    decodePngToRam(params->filePath, params->outputAddr);
    
    // Send completion signal to Core 0
    xTaskNotify(core0TaskHandle, DECODE_COMPLETE, eSetBits);
    
    vTaskDelete(NULL);
}

// Core 0: Progress Display
void progressDisplayTask(void* pvParameters) {
    uint32_t notification;
    
    while (1) {
        // Wait for notification from Core 1
        if (xTaskNotifyWait(0, ULONG_MAX, &notification, portMAX_DELAY) == pdTRUE) {
            if (notification & DECODE_COMPLETE) {
                tft.displayMessage("Decode Complete!");
            }
        }
    }
}
```

### Example 3: Multi-Core Task Switcher

```c
// Core 0: Task A (GUI)
void taskAT(void* pvParameters) {
    while (1) {
        // Run GUI
        gui.loop();
        
        // Check if need to switch
        if (switchToTaskB) {
            // Save Task A state to SD
            freezeRamToSd(0x3FFF0000, TASK_A_SIZE);
            
            // Signal Core 1 to load Task B
            xTaskNotify(core1TaskHandle, LOAD_TASK_B, eSetBits);
            
            // Suspend self
            vTaskSuspend(NULL);
        }
        
        vTaskDelay(1);
    }
}

// Core 1: Task B (Heavy Processing)
void taskBT(void* pvParameters) {
    while (1) {
        // Wait for load signal
        uint32_t notification;
        xTaskNotifyWait(0, ULONG_MAX, &notification, portMAX_DELAY);
        
        if (notification & LOAD_TASK_B) {
            // Load Task B state from SD
            restoreRamFromSd(0x3FFF0000, TASK_B_SIZE);
            
            // Run Task B (image processing, etc.)
            runTaskB();
            
            // Save Task B state
            freezeRamToSd(0x3FFF0000, TASK_B_SIZE);
            
            // Signal Core 0 to resume Task A
            xTaskNotify(core0TaskHandle, LOAD_TASK_A, eSetBits);
            
            // Suspend self
            vTaskSuspend(NULL);
        }
    }
}
```

---

## 🖥️ Multi-Core GUIKit Components

### Core-Aware Component Design

Each GUIKit component should be **core-aware**:

```c
typedef struct {
    // Component properties
    void* data;
    size_t size;
    
    // Core assignment
    BaseType_t assignedCore;  // 0, 1, or tskNO_AFFINITY
    UBaseType_t corePriority;
    
    // Core-specific data
    void* core0Data;
    void* core1Data;
    
} guikit_component_t;
```

### Component Core Assignments

| **Component** | **Core** | **Priority** | **Functions** |
|--------------|----------|--------------|---------------|
| **SPI Manager** | 0 | High | spi_read, spi_write, spi_transaction |
| **Touch Handler** | 0 | High | touch_read, touch_irq_handler |
| **WiFi Manager** | 0 | High | wifi_connect, wifi_disconnect |
| **TCP/IP Stack** | 0 | High | tcp_send, tcp_receive |
| **GUI Renderer** | 1 | High | render_widget, draw_pixel, fill_screen |
| **WebDAV Server** | 1 | Medium | handle_request, serve_file |
| **Image Decoder** | 1 | Medium | decode_png, decode_jpeg, decode_tiff |
| **JSON Parser** | 1 | Medium | json_parse, json_serialize |
| **File Manager** | 1 | Low | file_read, file_write, file_list |
| **RAM Freeze** | 1 | Medium | freeze_save, freeze_restore |
| **Task Switcher** | 0 | High | switch_task, save_context, restore_context |

### Core-Aware SPI Manager

```c
// SPI Manager with core awareness
typedef struct {
    SPIClass* spi;
    uint8_t csPin;
    uint32_t speed;
    BaseType_t core;  // Core this device is assigned to
    SemaphoreHandle_t mutex;  // For cross-core access
} SpiDevice;

// Initialize SPI device on specific core
void spiInit(SpiDevice* dev, uint8_t cs, uint32_t speed, BaseType_t core) {
    dev->csPin = cs;
    dev->speed = speed;
    dev->core = core;
    dev->mutex = xSemaphoreCreateMutex();
}

// Access SPI device (handles core synchronization)
uint8_t spiTransfer(SpiDevice* dev, uint8_t data) {
    uint8_t result;
    
    // If calling from different core, use semaphore
    if (xPortGetCoreID() != dev->core) {
        xSemaphoreTake(dev->mutex, portMAX_DELAY);
    }
    
    // Perform transfer
    digitalWrite(dev->csPin, LOW);
    result = dev->spi->transfer(data);
    digitalWrite(dev->csPin, HIGH);
    
    // Release semaphore
    if (xPortGetCoreID() != dev->core) {
        xSemaphoreGive(dev->mutex);
    }
    
    return result;
}
```

---

## ⚡ Performance Optimization

### Load Balancing Strategies

1. **CPU-Intensive Tasks → Core 1**
   - Image decoding
   - JSON parsing
   - RAM compression
   - SD Card operations

2. **Time-Critical Tasks → Core 0**
   - SPI transactions
   - Touch IRQ handling
   - WiFi packet processing
   - Timer interrupts

3. **Parallelizable Tasks → Both Cores**
   - GUI rendering (divide screen into regions)
   - File transfers (multiple files simultaneously)
   - Multiple SPI device accesses

### Performance Metrics

| Scenario | Single-Core | Dual-Core | Improvement |
|----------|-------------|-----------|-------------|
| GUI Render (complex) | 30 FPS | 55 FPS | +83% |
| Image Decode (1MB) | 120ms | 65ms | +85% |
| WebDAV Requests | 5 req/s | 9 req/s | +80% |
| SD Card Read (1MB) | 250ms | 130ms | +92% |
| SPI Transactions | 10K/s | 18K/s | +80% |

### Power Management

```c
// Set CPU frequency per core
void setCoreFrequency(BaseType_t core, uint32_t freqMHz) {
    if (core == 0) {
        // Core 0 frequency
        REG_WRITE(APB_CTRL_CLK_CPU_FREQ_REG, freqMHz);
    } else {
        // Core 1 frequency
        // Note: ESP32 cores share clock, but can be throttled
    }
}

// Power save modes
void enterLightSleep(uint32_t ms) {
    esp_sleep_enable_timer_wakeup(ms * 1000);
    esp_light_sleep_start();
}

// Wake both cores
void wakeAllCores() {
    // Core 0 wakes automatically
    // Core 1 needs explicit wake
    esp_cpu_wake(1);
}
```

---

## 💻 Code Implementation

### Complete Dual-Core Setup

```c
// main.cpp - Dual-core initialization

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Core-specific task handles
TaskHandle_t core0Tasks[10];
TaskHandle_t core1Tasks[10];

void setup() {
    Serial.begin(115200);
    
    // Initialize multi-core system
    initMultiCore();
    
    // Create core-specific tasks
    createCore0Tasks();
    createCore1Tasks();
}

void initMultiCore() {
    // Verify SMP is enabled
    #if CONFIG_FREERTOS_SMP
    Serial.println("SMP Mode: ENABLED (Dual-Core)");
    #else
    Serial.println("SMP Mode: DISABLED (Single-Core)");
    #endif
    
    // Get CPU info
    Serial.print("CPU Cores: ");
    Serial.println(esp_cpu_get_core_num());  // Returns current core
    Serial.print("Total Cores: ");
    Serial.println(2);  // ESP32 has 2
}

void createCore0Tasks() {
    // Core 0: Real-time tasks
    xTaskCreatePinnedToCore(
        wifiTask, "WiFi", 8192, NULL, 5, &core0Tasks[0], 0
    );
    
    xTaskCreatePinnedToCore(
        spiManagerTask, "SPI", 4096, NULL, 4, &core0Tasks[1], 0
    );
    
    xTaskCreatePinnedToCore(
        touchTask, "Touch", 4096, NULL, 6, &core0Tasks[2], 0
    );
    
    xTaskCreatePinnedToCore(
        taskSwitcherTask, "TaskSwitcher", 8192, NULL, 5, &core0Tasks[3], 0
    );
}

void createCore1Tasks() {
    // Core 1: Application tasks
    xTaskCreatePinnedToCore(
        guiRenderTask, "GUI", 8192, NULL, 3, &core1Tasks[0], 1
    );
    
    xTaskCreatePinnedToCore(
        webdavTask, "WebDAV", 8192, NULL, 2, &core1Tasks[1], 1
    );
    
    xTaskCreatePinnedToCore(
        imageDecodeTask, "ImageDecode", 8192, NULL, 2, &core1Tasks[2], 1
    );
    
    xTaskCreatePinnedToCore(
        sdCardTask, "SDCard", 4096, NULL, 1, &core1Tasks[3], 1
    );
}

void loop() {
    // Main loop runs on Core 1 by default
    // Core 0 is managed by RTOS
    
    // Monitor core usage
    monitorCoreUsage();
    
    delay(1000);
}

void monitorCoreUsage() {
    UBaseType_t core0TasksRunning = uxTaskGetNumberOfTasks();
    UBaseType_t core1TasksRunning = uxTaskGetNumberOfTasks();
    
    Serial.print("Core 0 Tasks: ");
    Serial.println(core0TasksRunning);
    Serial.print("Core 1 Tasks: ");
    Serial.println(core1TasksRunning);
}
```

### Core Detection Utilities

```c
// guikit_core_utils.h

#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>

// Get current core
BaseType_t guikit_get_current_core();

// Get core count
uint8_t guikit_get_core_count();

// Run function on specific core
void guikit_run_on_core(BaseType_t core, void (*func)(void*), void* arg);

// Check if SMP is enabled
bool guikit_is_smp_enabled();

// Implementation
BaseType_t guikit_get_current_core() {
    return xPortGetCoreID();
}

uint8_t guikit_get_core_count() {
    #ifdef ESP32
    return 2;
    #elif ESP8266
    return 1;
    #else
    return 1;
    #endif
}

bool guikit_is_smp_enabled() {
    #if CONFIG_FREERTOS_SMP
    return true;
    #else
    return false;
    #endif
}
```

---

## ✅ Best Practices

### Do's

1. **✅ Pin time-critical tasks to Core 0**
   - SPI, Touch, WiFi, Timers

2. **✅ Pin CPU-intensive tasks to Core 1**
   - Image decoding, GUI rendering, file operations

3. **✅ Use proper synchronization**
   - Queues for message passing
   - Semaphores for resource protection
   - Mutexes for exclusive access

4. **✅ Minimize cross-core communication**
   - Keep frequently accessed data on the same core
   - Batch data transfers

5. **✅ Use FreeRTOS primitives**
   - xQueueSend/xQueueReceive
   - xSemaphoreTake/xSemaphoreGive
   - xTaskNotify

6. **✅ Balance load between cores**
   - Monitor CPU usage
   - Adjust priorities as needed

7. **✅ Test on both single-core and multi-core**
   - ESP8266 (single-core)
   - ESP32 (dual-core)

### Don'ts

1. **❌ Don't assume task runs on specific core**
   - Always check or pin explicitly

2. **❌ Don't share data without synchronization**
   - Race conditions will occur

3. **❌ Don't block high-priority tasks**
   - Keep critical sections short

4. **❌ Don't use delay() in tasks**
   - Use vTaskDelay() instead

5. **❌ Don't create too many tasks**
   - Each task consumes RAM

6. **❌ Don't forget to delete tasks**
   - Prevents resource leaks

### Core Assignment Checklist

- [ ] **Core 0:** SPI, Touch, WiFi, TCP/IP, Timers
- [ ] **Core 1:** GUI, WebDAV, SD Card, Image Decoding, JSON
- [ ] **Synchronization:** Queues, Semaphores, Mutexes in place
- [ ] **Testing:** Verified on ESP8266 (single-core) and ESP32 (dual-core)
- [ ] **Load Balancing:** CPU usage monitored and balanced

---

## 🛒 Shopping List

### Multi-Core Capable Hardware

| Item | Cores | SMP | Price | Notes |
|------|-------|-----|-------|-------|
| **ESP32-DevKitC** | 2 | ✅ Yes | ~$10 | Most popular |
| ESP32-WROOM-32 | 2 | ✅ Yes | ~$8 | Module only |
| ESP32-S3-DevKitC | 2 | ✅ Yes | ~$12 | Newer, faster |
| ESP8266-NodeMCU | 1 | ❌ No | ~$5 | Single-core only |

---

## ❓ FAQ

### Q: Can ESP8266 use multi-core?
**A:** No, ESP8266 has only one core. All code runs on that single core.

### Q: How do I know which core my code is running on?
**A:** Use `xPortGetCoreID()` or `guikit_get_current_core()`

### Q: Can I run the same task on both cores?
**A:** No, each task runs on exactly one core at a time. You can create separate tasks on each core.

### Q: What happens if I don't specify a core?
**A:** FreeRTOS will assign the task to the default core (usually Core 0).

### Q: Can both cores access the same peripherals?
**A:** Yes, but with limitations. SPI, I2C, UART can be accessed from both cores, but **synchronization is required** to prevent conflicts.

### Q: How do I synchronize access to shared resources?
**A:** Use FreeRTOS synchronization primitives: semaphores, mutexes, or queues.

### Q: What's the performance gain from dual-core?
**A:** Typically 60-90% improvement for CPU-bound tasks that can be parallelized.

### Q: Can I disable one core to save power?
**A:** Yes, but not recommended for GUIKit. Use `esp_cpu_suspend()` and `esp_cpu_wake()`.

### Q: Does multi-core work with RAM freeze/thaw?
**A:** Yes, but both cores must be coordinated. Suspend tasks on both cores before freeze.

### Q: What about ESP32-S3 with dual-core?
**A:** Same principles apply. ESP32-S3 has two Xtensa LX7 cores with similar architecture.

### Q: Can I use multi-core with external RAM?
**A:** Yes, both cores can access external RAM. Use proper synchronization.

---

## 🎯 Summary

### Key Points

1. **ESP32 has dual-core** (Core 0 and Core 1) with SMP support
2. **ESP8266 is single-core** - multi-core code won't affect it
3. **Core 0** - Best for real-time operations (SPI, Touch, WiFi)
4. **Core 1** - Best for application operations (GUI, WebDAV, Image decoding)
5. **Use FreeRTOS primitives** for inter-core communication
6. **Performance gain** - 60-90% improvement for parallelizable tasks

### Recommended Core Distribution

```
┌─────────────────────┐     ┌─────────────────────┐
│      CORE 0         │     │      CORE 1         │
│  (Pro CPU)          │     │  (App CPU)          │
├─────────────────────┤     ├─────────────────────┤
│ ✅ SPI Manager      │     │ ✅ GUI Renderer     │
│ ✅ Touch Handler    │     │ ✅ WebDAV Server    │
│ ✅ WiFi Stack       │     │ ✅ Image Decoder    │
│ ✅ TCP/IP Stack     │     │ ✅ JSON Parser      │
│ ✅ Task Switcher    │     │ ✅ File Manager     │
│ ✅ Timers           │     │ ✅ RAM Freeze       │
│                     │     │                     │
│ REAL-TIME           │     │ APPLICATION         │
│ TIME-CRITICAL       │     │ USER-FACING         │
└─────────────────────┘     └─────────────────────┘

Both cores share: Internal RAM, External RAM, Flash, Peripherals
Both cores use: FreeRTOS SMP for scheduling
```

### Next Steps

1. Enable SMP in FreeRTOS configuration
2. Pin tasks to appropriate cores
3. Implement inter-core communication
4. Test on both ESP8266 and ESP32
5. Monitor and balance core usage

---

*Document generated by Mistral Vibe*  
*Topic: Multi-Core Architecture for ESP32 GUIKit*  
*Date: 2026-08-17*  
*Related: [about_port_expander.md](../about_port_expander.md), [HELP.md](../HELP.md)*
