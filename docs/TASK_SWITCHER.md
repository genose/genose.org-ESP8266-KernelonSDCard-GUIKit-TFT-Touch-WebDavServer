# Task Switcher - Single-Level Context Switching

## Overview

The **Task Switcher** implements a simple but powerful context-switching mechanism that allows your GUIKit system to temporarily switch from one task to another, then restore the original task exactly as it was.

### Key Concepts

- **Single-level switching**: Only one frozen task at a time (A → B → back to A)
- **RAM freeze/thaw**: Uses the existing freeze system to save/restore complete RAM state
- **Task B is freed**: When returning to Task A, Task B is discarded (not saved)
- **SD card required**: Frozen state is stored on SD card
- **Task communication**: Tasks communicate via files on SD card

### Use Cases

- Heavy operations (JPEG decoding, complex calculations)
- Specialized utilities (file browsers, settings menus)
- Modal dialogs that need full RAM
- Any operation that requires dedicated RAM

---

## How It Works

### Flow Diagram

```
┌─────────────────────────┐
│      Task A (GUI)        │
│  - Running normally      │
│  - Has state in RAM      │
└─────────────┬───────────┘
              │
              ▼
┌─────────────────────────┐
│   task_switch_to()       │
│   1. Freeze RAM → SD    │
│   2. Load Task B kernel  │
│   3. Jump to Task B      │
└─────────────┬───────────┘
              │
              ▼
┌─────────────────────────┐
│      Task B (Converter)  │
│  - Runs in same RAM     │
│  - Does heavy work       │
│  - Saves results to SD  │
└─────────────┬───────────┘
              │
              ▼
┌─────────────────────────┐
│    task_restore()        │
│   1. Free Task B (no save)│
│   2. Thaw Task A from SD │
│   3. Resume Task A       │
└─────────────┬───────────┘
              │
              ▼
┌─────────────────────────┐
│      Task A (GUI)        │
│  - Restored to exact     │
│    state before switch   │
│  - Can load results from │
│    SD card               │
└─────────────────────────┘
```

### Memory Layout

```
Before Switch:
┌─────────────────────────────────────┐
│           RAM                       │
│  ┌───────────────────────────────┐  │
│  │      Task A State               │  │
│  │  - GUI widgets                 │  │
│  │  - Image data                  │  │
│  │  - Variables                   │  │
│  └───────────────────────────────┘  │
└─────────────────────────────────────┘

After Freeze (Task A saved to SD):
┌─────────────────────────────────────┐
│           SD Card                    │
│  ┌───────────────────────────────┐  │
│  │  /tmp/frozen_task.bin           │  │
│  │  (Complete RAM snapshot)        │  │
│  └───────────────────────────────┘  │
└─────────────────────────────────────┘

After Load Task B:
┌─────────────────────────────────────┐
│           RAM                       │
│  ┌───────────────────────────────┐  │
│  │      Task B (Converter)         │  │
│  │  - JPEG decoder                │  │
│  │  - Temporary buffers           │  │
│  │  - Working data                │  │
│  └───────────────────────────────┘  │
└─────────────────────────────────────┘

After Restore:
┌─────────────────────────────────────┐
│           RAM                       │
│  ┌───────────────────────────────┐  │
│  │      Task A State               │  │
│  │  - GUI widgets (restored!)      │  │
│  │  - Image data (restored!)       │  │
│  │  - Variables (restored!)        │  │
│  └───────────────────────────────┘  │
└─────────────────────────────────────┘
```

---

## API Reference

### Initialization

```c
#include "task_switcher.h"

// Initialize with defaults (80% free RAM required, auto cleanup)
task_switcher_init();

// Or customize configuration
TaskSwitcherConfig config = {
    .frozen_task_file = "/tmp/frozen.bin",
    .min_free_ram = 80,  // Require 80% free RAM
    .auto_cleanup = true,
    .debug = true
};
task_switcher_init_with_config(&config);
```

### Switching Tasks

```c
// Switch from current task to new task
bool success = task_switch_to("/kernel/jpeg_converter.bin");
if (!success) {
    TaskSwitcherError err = task_switcher_get_error();
    printf("Error: %s\n", task_switcher_error_to_string(err));
}
// Note: Does not return if successful - new task starts running
```

```c
// Restore parent task (called by Task B or "Back" button)
bool success = task_restore();
// Note: Does not return if successful - parent task resumes
```

### State Checking

```c
// Check if we have a frozen parent
if (task_has_frozen_parent()) {
    // We are Task B (child)
    // Can call task_restore() to return
}

// Get parent task info
TaskInfo parent;
if (task_get_frozen_parent(&parent)) {
    printf("Parent: %s\n", parent.kernel_path);
}

// Get current task info
TaskInfo current;
task_get_current(&current);

// Check state
TaskSwitcherState state = task_switcher_get_state();
```

### Error Handling

```c
TaskSwitcherError err = task_switcher_get_error();

switch (err) {
    case TASK_ERROR_NONE:
        break;
    case TASK_ERROR_NO_SDCARD:
        show_tft_error("SD Card Required!");
        break;
    case TASK_ERROR_FREEZE_FAILED:
        show_tft_error("Freeze Failed!");
        break;
    case TASK_ERROR_LOAD_FAILED:
        show_tft_error("Load Failed!");
        break;
    case TASK_ERROR_RESTORE_FAILED:
        show_tft_error("Restore Failed!");
        break;
    case TASK_ERROR_NO_FROZEN_TASK:
        show_tft_error("Nothing to Restore!");
        break;
    case TASK_ERROR_INSUFFICIENT_RAM:
        show_tft_error("Need 80%+ Free RAM!");
        break;
}
```

### Task Communication

Tasks communicate via files on SD card in `/tmp/task_comm/`:

```c
// Task B: Save data for parent
uint8_t results[1024] = { ... };
task_save_for_parent("results.bin", results, sizeof(results));

// Task A (after restore): Load data from child
uint8_t buffer[1024];
uint32_t loaded = task_load_from_child("results.bin", buffer, sizeof(buffer));

// Check if child saved a file
if (task_child_has_file("results.bin")) {
    // File exists
}
```

---

## Example: JPEG to RGB Conversion

This is the canonical example that demonstrates the task switcher in action.

### Task A: GUI Image Viewer

```c
#include "task_switcher.h"
#include "task_switcher_example.h"

void on_convert_button_click(const char* jpeg_path) {
    // Trigger conversion - will switch to converter task
    if (!gui_trigger_jpeg_conversion(jpeg_path)) {
        show_error("Failed to start conversion");
    }
    // After restore, execution continues here
    // Task A state is exactly as it was before the switch
}

void main_loop(void) {
    // Check for conversion results
    RgbImageHeader header;
    uint8_t rgb_buffer[1024 * 768 * 3];
    
    uint32_t loaded = gui_load_conversion_result(
        &header, rgb_buffer, sizeof(rgb_buffer)
    );
    
    if (loaded > 0) {
        display_rgb_image(&header, rgb_buffer);
        gui_cleanup_conversion();
    }
}
```

### Task B: JPEG Converter

```c
#include "task_switcher.h"
#include "task_switcher_example.h"

void main(void) {
    // Initialize task switcher
    task_switcher_init();
    
    // Check if we were launched from a parent
    if (task_has_frozen_parent()) {
        // We are Task B (JPEG Converter)
        jpeg_converter_run("/images/input.jpg");
    }
    
    // Main event loop
    while (1) {
        // Check for button presses
        if (back_button_pressed()) {
            task_restore();  // Returns to Task A
        }
        
        // Other processing...
    }
}

bool jpeg_converter_run(const char* jpeg_path) {
    // Decode JPEG to RGB
    // In real code: use JPEG decoder library
    
    RgbImageHeader header;
    uint8_t* rgb_data = decode_jpeg(jpeg_path, &header);
    
    if (!rgb_data) {
        return false;
    }
    
    // Save results for parent task
    task_save_for_parent("converted.hdr", &header, sizeof(header));
    task_save_for_parent("converted.rgb", rgb_data, header.data_size);
    
    free(rgb_data);
    return true;
}
```

---

## Configuration Reference

### `/etc/GUIKIT_autostart.ini` Integration

The task switcher works with the autostart system. You can configure which kernel to load:

```ini
[kernel]
path = /kernel/main.bin

[gui]
path = /gui/main.GUIKIT
auto_start = true
```

### Task Switcher Configuration

Create a custom configuration if needed:

```c
TaskSwitcherConfig config = {
    .frozen_task_file = "/tmp/custom_frozen.bin",
    .min_free_ram = 75,  // 75% free RAM required
    .auto_cleanup = false,  // Keep frozen file after restore
    .debug = true  // Enable debug output
};
```

---

## Requirements & Limitations

### Requirements

- ✅ SD card must be available and initialized
- ✅ At least `min_free_ram` percentage of RAM must be free (default: 80%)
- ✅ RAM freeze/thaw system must be initialized
- ✅ Kernel files must be present on SD card

### Limitations

- **Single-level only**: Cannot nest task switches (A → B → C is not supported)
- **Task B is not saved**: When restoring, Task B's state is discarded
- **SD card dependency**: Requires working SD card for freeze/thaw
- **RAM requirements**: Needs significant free RAM for the freeze operation

### Error States

If an error occurs during switching:

| Error | Behavior |
|-------|----------|
| `TASK_ERROR_NO_SDCARD` | Shows error on TFT, returns to current task |
| `TASK_ERROR_FREEZE_FAILED` | Shows error, current task continues |
| `TASK_ERROR_LOAD_FAILED` | Shows error, current task continues |
| `TASK_ERROR_INSUFFICIENT_RAM` | Shows error with % required |
| `TASK_ERROR_NO_FROZEN_TASK` | `task_restore()` called with no parent |

---

## Implementation Notes

### Real Implementation Requirements

For production use, you need to replace the mock functions in `task_switcher.c`:

1. **RAM Functions**:
   - `get_total_ram()` - Return actual total RAM
   - `get_free_ram()` - Return actual free RAM

2. **SD Card Functions**:
   - `sd_card_available()` - Check if SD initialized
   - `sd_file_exists()` - Check file existence
   - `sd_delete_file()` - Delete file
   - `sd_write_file()` - Write data to file
   - `sd_read_file()` - Read file data

3. **Kernel Loading**:
   - `load_and_jump_to_kernel()` - Actual kernel loading and execution

### Integration with Existing Code

The task switcher uses:
- `ram_freeze.h/c` - For freezing/thawing RAM
- `sd_freeze_wrapper.h/cpp` - For SD card operations
- `crc32.h/c` - For data verification

---

## File Locations

| File | Purpose |
|------|---------|
| `src/boot/task_switcher.h` | Main header |
| `src/boot/task_switcher.c` | Implementation |
| `src/boot/task_switcher_example.h` | JPEG conversion example header |
| `src/boot/task_switcher_example.c` | JPEG conversion example implementation |
| `docs/TASK_SWITCHER.md` | This documentation |

---

## Version History

- **Latest**: Single-level task switching with RAM freeze/thaw
- **Latest**: JPEG to RGB conversion example
- **Latest**: Task communication via SD card files
- **Latest**: Error handling with TFT display support
