# Text Editor SD Card Integration

> Temporary buffer files for text editor: `/tmp/(filename)_(selection_type).txt`

## Overview

This module provides **SD card integration** for the text editor, enabling:

- **Saving text selections** to temporary files in `/tmp/`
- **Loading text** from temporary files
- **Auto-saving** selections when text is selected
- **Session recovery** from temp files
- **Bookmark support** for saving/loading cursor positions

### File Naming Pattern

```
/tmp/(filename)_(selection_type).txt
```

Where:
- `filename` - Base filename set during initialization (e.g., "myfile", "document1")
- `selection_type` - Type of content saved:
  - `full` - Entire document
  - `selection` - Currently selected text
  - `line_N` - Specific line N (e.g., `line_5`)
  - `clipboard` - Clipboard content
  - `cursor` - Text around cursor position
  - `backup` - Auto-save backup
  - `cursor_pos` - Cursor position (line, column)
  - `selection` - Selection range (start_line, start_col, end_line, end_col)
  - `custom_name` - Any custom type (e.g., `bookmark1`, `note1`)

### Examples

```
/tmp/myfile_full.txt        # Entire document
/tmp/myfile_selection.txt    # Selected text
/tmp/myfile_line_5.txt      # Line 5 content
/tmp/myfile_clipboard.txt   # Clipboard content
/tmp/myfile_cursor.txt      # Text around cursor
/tmp/myfile_bookmark1.txt   # Custom bookmark
```

---

## Integration with Existing SD Card System

This module integrates with the existing **sd_card.h** library in GUIKit:

```c
#include "sd_card.h"
#include "text_editor_sd.h"
```

### Dependencies

- `sd_card.h` - SD card initialization and file operations
- `text_editor.h` - Text editor core
- `stdio.h` - File I/O functions (fallback)

---

## Data Structures

### SdSelectionType

```c
typedef enum {
    SD_SELECTION_FULL = 0,        // Entire document
    SD_SELECTION_CURRENT,          // Currently selected text
    SD_SELECTION_LINE,            // Specific line (requires line_number)
    SD_SELECTION_CLIPBOARD,       // Clipboard content
    SD_SELECTION_CURSOR,          // Text around cursor
    SD_SELECTION_BACKUP,          // Auto-save backup
    SD_SELECTION_CUSTOM           // Custom user-defined type
} SdSelectionType;
```

### SdStatus

```c
typedef enum {
    SD_STATUS_OK = 0,
    SD_STATUS_ERROR,
    SD_STATUS_NOT_INITIALIZED,
    SD_STATUS_FILE_NOT_FOUND,
    SD_STATUS_WRITE_ERROR,
    SD_STATUS_READ_ERROR,
    SD_STATUS_DIR_NOT_FOUND,
    SD_STATUS_NO_SDCARD
} SdStatus;
```

### TextEditorSd

Internal state structure (managed automatically):

```c
typedef struct {
    char base_filename[64];       // Base filename without extension
    char temp_dir[128];           // Temporary directory path
    bool sd_initialized;          // SD card ready
    bool auto_save_enabled;      // Enable auto-save
    bool auto_save_selection;   // Auto-save selection on change
    bool auto_save_full;         // Auto-save full document periodically
    uint32_t last_auto_save;     // Last auto-save timestamp
    uint32_t auto_save_interval; // Auto-save interval (ms)
} TextEditorSd;
```

### TextEditorSdFileInfo

```c
typedef struct {
    char path[128];               // Full path to file
    char name[64];                // Filename without path
    SdSelectionType type;         // Selection type
    uint32_t timestamp;           // Modification timestamp
    uint32_t size;                // File size in bytes
    bool exists;                 // Whether file exists
} TextEditorSdFileInfo;
```

---

## API Reference

### Initialization

| Function | Description |
|----------|-------------|
| `text_editor_sd_init(editor, base_filename)` | Initialize SD integration with base filename |
| `text_editor_sd_deinit(editor)` | Deinitialize and cleanup |
| `text_editor_sd_is_ready(editor)` | Check if SD card is ready |
| `text_editor_sd_is_enabled(editor)` | Check if SD integration is active |

### Path Construction

| Function | Description |
|----------|-------------|
| `text_editor_sd_build_path(editor, type, line, buf, size)` | Build full path for selection type |
| `text_editor_sd_build_custom_path(editor, custom, buf, size)` | Build path for custom type |
| `text_editor_sd_get_base_filename(editor)` | Get base filename |
| `text_editor_sd_set_base_filename(editor, name)` | Set base filename |
| `text_editor_sd_set_temp_dir(editor, dir)` | Set temp directory |

### Save Operations

| Function | Description |
|----------|-------------|
| `text_editor_sd_save_full(editor)` | Save entire document to `/tmp/(filename)_full.txt` |
| `text_editor_sd_save_selection(editor)` | Save selected text to `/tmp/(filename)_selection.txt` |
| `text_editor_sd_save_line(editor, line)` | Save specific line to `/tmp/(filename)_line_N.txt` |
| `text_editor_sd_save_clipboard(editor)` | Save clipboard to `/tmp/(filename)_clipboard.txt` |
| `text_editor_sd_save_cursor_region(editor, before, after)` | Save text around cursor |
| `text_editor_sd_save_custom(editor, type, text)` | Save custom text to `/tmp/(filename)_type.txt` |

### Load Operations

| Function | Description |
|----------|-------------|
| `text_editor_sd_load_full(editor)` | Load from `/tmp/(filename)_full.txt` |
| `text_editor_sd_load_selection(editor)` | Load from `/tmp/(filename)_selection.txt` |
| `text_editor_sd_load(editor, type, line)` | Load from specific temp file |
| `text_editor_sd_load_custom(editor, type)` | Load from custom temp file |
| `text_editor_sd_insert_from_temp(editor, type, line)` | Insert content from temp file at cursor |
| `text_editor_sd_set_selection_from_temp(editor, type, line)` | Set selection from temp file |

### Auto-Save

| Function | Description |
|----------|-------------|
| `text_editor_sd_set_auto_save(editor, enabled)` | Enable/disable all auto-save |
| `text_editor_sd_set_auto_save_selection(editor, enabled)` | Auto-save selection on change |
| `text_editor_sd_set_auto_save_full(editor, enabled)` | Auto-save full document periodically |
| `text_editor_sd_set_auto_save_interval(editor, ms)` | Set auto-save interval |
| `text_editor_sd_trigger_auto_save(editor)` | Trigger if interval passed (return true if saved) |
| `text_editor_sd_do_auto_save(editor)` | Force auto-save now |

### File Management

| Function | Description |
|----------|-------------|
| `text_editor_sd_file_exists(editor, type, line)` | Check if temp file exists |
| `text_editor_sd_get_file_info(editor, type, line, info)` | Get file metadata |
| `text_editor_sd_list_files(editor, infos, max)` | List all temp files |
| `text_editor_sd_delete_file(editor, type, line)` | Delete specific temp file |
| `text_editor_sd_delete_custom_file(editor, type)` | Delete custom temp file |
| `text_editor_sd_delete_all_files(editor)` | Delete all temp files |

### Content Operations

| Function | Description |
|----------|-------------|
| `text_editor_sd_read_file(editor, type, line, buf, size)` | Read temp file to buffer |
| `text_editor_sd_read_custom_file(editor, type, buf, size)` | Read custom temp file |
| `text_editor_sd_get_file_size(editor, type, line)` | Get file size |

### Session Management

| Function | Description |
|----------|-------------|
| `text_editor_sd_save_session(editor)` | Save document + cursor + selection |
| `text_editor_sd_load_session(editor)` | Load session from temp files |
| `text_editor_sd_session_exists(editor)` | Check if session can be restored |
| `text_editor_sd_clear_session(editor)` | Delete session files |

### Bookmark Operations

| Function | Description |
|----------|-------------|
| `text_editor_sd_save_bookmark(editor, name)` | Save cursor position as bookmark |
| `text_editor_sd_load_bookmark(editor, name)` | Load bookmark (move cursor) |
| `text_editor_sd_goto_bookmark(editor, name)` | Same as load_bookmark |
| `text_editor_sd_delete_bookmark(editor, name)` | Delete bookmark file |

### Callbacks

| Function | Description |
|----------|-------------|
| `text_editor_sd_set_status_callback(editor, cb)` | Set SD status change callback |
| `text_editor_sd_set_file_callback(editor, cb)` | Set file operation callback |

### Inline Helpers

| Function | Description |
|----------|-------------|
| `text_editor_sd_success(status)` | Check if operation succeeded |
| `text_editor_sd_no_sdcard(status)` | Check if SD card is missing |
| `text_editor_sd_status_string(status)` | Get status as string |
| `text_editor_sd_selection_type_string(type)` | Get selection type as string |

---

## Usage Examples

### Example 1: Basic Initialization and Save

```c
#include "text_editor.h"
#include "text_editor_sd.h"

TextEditor* editor;

void setup() {
    tft.init();
    sd_card_init();  // Initialize SD card first
    
    // Create editor
    editor = text_editor_create(0, 0, 240, 320);
    
    // Initialize SD integration with base filename
    SdStatus status = text_editor_sd_init(editor, "myfile");
    if (status != SD_STATUS_OK) {
        Serial.println("SD init failed!");
    }
    
    // Set some text
    text_editor_set_text(editor, "Hello, World!");
}

void loop() {
    // Save full document when needed
    if (button_pressed) {
        SdStatus status = text_editor_sd_save_full(editor);
        if (text_editor_sd_success(status)) {
            Serial.println("Saved to /tmp/myfile_full.txt");
        }
    }
}
```

### Example 2: Auto-Save on Selection

```c
void setup() {
    editor = text_editor_create(0, 0, 240, 320);
    text_editor_sd_init(editor, "document");
    
    // Enable auto-save for selections
    text_editor_sd_set_auto_save_selection(editor, true);
    text_editor_sd_set_auto_save_interval(editor, 1000);  // 1 second
    
    // Set callback for selection changes
    text_editor_set_on_selection_change(editor, on_selection_change);
}

void on_selection_change(TextEditor* editor) {
    // Auto-save when selection changes
    if (text_editor_has_selection(editor)) {
        text_editor_sd_save_selection(editor);
    }
}

void loop() {
    // Or use trigger in main loop
    text_editor_sd_trigger_auto_save(editor);
}
```

### Example 3: Session Recovery

```c
void setup() {
    editor = text_editor_create(0, 0, 240, 320);
    
    // Initialize SD
    text_editor_sd_init(editor, "session");
    
    // Try to restore session
    if (text_editor_sd_session_exists(editor)) {
        SdStatus status = text_editor_sd_load_session(editor);
        if (text_editor_sd_success(status)) {
            Serial.println("Restored from session files");
        }
    }
}

void loop() {
    // Periodically save session
    static uint32_t last_save = 0;
    if (millis() - last_save > 30000) {  // Every 30 seconds
        text_editor_sd_save_session(editor);
        last_save = millis();
    }
}
```

### Example 4: Bookmark Support

```c
void save_bookmark() {
    // Save current position as bookmark1
    text_editor_sd_save_bookmark(editor, "bookmark1");
}

void goto_bookmark1() {
    // Jump to bookmark1
    text_editor_sd_goto_bookmark(editor, "bookmark1");
}

void delete_bookmark1() {
    text_editor_sd_delete_bookmark(editor, "bookmark1");
}
```

### Example 5: Loading Specific Lines

```c
void load_line_5() {
    // Load line 5 from temp file
    SdStatus status = text_editor_sd_load(editor, SD_SELECTION_LINE, 5);
    if (text_editor_sd_success(status)) {
        // Line 5 content is now in editor
    }
}

void insert_line_10_at_cursor() {
    // Insert content of line 10 at cursor position
    SdStatus status = text_editor_sd_insert_from_temp(editor, SD_SELECTION_LINE, 10);
}
```

### Example 6: File Management

```c
void list_all_temp_files() {
    TextEditorSdFileInfo infos[20];
    int16_t count = text_editor_sd_list_files(editor, infos, 20);
    
    for (int i = 0; i < count; i++) {
        Serial.print("File: ");
        Serial.print(infos[i].name);
        Serial.print(" (");
        Serial.print(text_editor_sd_selection_type_string(infos[i].type));
        Serial.print("), Size: ");
        Serial.print(infos[i].size);
        Serial.println(" bytes");
    }
}

void delete_all_temp_files() {
    text_editor_sd_delete_all_files(editor);
}
```

### Example 7: Custom Types

```c
void save_note() {
    // Save custom text with custom type
    text_editor_sd_save_custom(editor, "note1", "This is a note");
}

void load_note() {
    text_editor_sd_load_custom(editor, "note1");
}

void delete_note() {
    text_editor_sd_delete_custom_file(editor, "note1");
}
```

### Example 8: Clipboard Persistence

```c
void copy_to_clipboard() {
    text_editor_copy(editor);
    // Save clipboard to SD
    text_editor_sd_save_clipboard(editor);
}

void paste_from_clipboard() {
    // If clipboard is empty, try loading from SD
    if (!text_editor_clipboard_has_content(editor)) {
        text_editor_sd_load(editor, SD_SELECTION_CLIPBOARD, 0);
        // Now copy from editor to clipboard
        text_editor_select_all(editor);
        text_editor_copy(editor);
        text_editor_clear_selection(editor);
    }
    text_editor_paste(editor);
}
```

### Example 9: Integrating with Editor Callbacks

```c
void on_selection_change(TextEditor* editor) {
    if (text_editor_sd_is_ready(editor)) {
        // Auto-save selection
        text_editor_sd_save_selection(editor);
    }
}

void on_change(TextEditor* editor) {
    if (text_editor_sd_is_ready(editor)) {
        // Auto-save full document every change (if enabled)
        text_editor_sd_trigger_auto_save(editor);
    }
}

void setup() {
    editor = text_editor_create(0, 0, 240, 320);
    text_editor_sd_init(editor, "document");
    
    // Set callbacks
    text_editor_set_on_selection_change(editor, on_selection_change);
    text_editor_set_on_change(editor, on_change);
    
    // Enable auto-save
    text_editor_sd_set_auto_save_full(editor, true);
}
```

### Example 10: Cursor Region Saving

```c
void save_context() {
    // Save 3 lines before and after cursor
    text_editor_sd_save_cursor_region(editor, 3, 3);
}

void restore_context() {
    // Load the saved context
    text_editor_sd_load(editor, SD_SELECTION_CURSOR, 0);
}
```

---

## Integration with GUIKit SD Card

The module uses the existing `sd_card.h` library. The implementation provides fallback functions for file operations:

```c
// In text_editor_sd.c
static bool file_exists(const char* path);
static uint32_t get_file_size(const char* path);
static int32_t read_file(const char* path, char* buffer, uint16_t buffer_size);
static bool write_file(const char* path, const char* buffer, uint32_t size);
static bool delete_file(const char* path);
```

These functions first try to use the SD card library, then fall back to standard C file I/O if needed.

### Using with Existing SD Functions

If your project already has SD card functions, you can replace the static helpers:

```c
// Override with your own implementations
bool my_sd_file_exists(const char* path);
uint32_t my_sd_file_size(const char* path);
```

---

## Memory Considerations

### Temp File Storage

All temp files are stored on the **SD card**, not in RAM. This means:

1. **No RAM overhead** for file content (only metadata in RAM)
2. **Large files supported** (limited by SD card, not RAM)
3. **Persistent storage** across power cycles

### RAM Usage

The SD integration adds minimal RAM overhead:

- `TextEditorSd`: ~200 bytes per editor
- `TextEditorSdFileInfo`: ~200 bytes (temporary, on stack)
- Path buffers: ~256 bytes (temporary, on stack)

**Total: ~200 bytes per editor instance**

This is acceptable for ESP8266 (80KB RAM).

### File Size Limits

| **Content** | **Maximum Size** |
|-------------|------------------|
| Full document | TEXT_EDITOR_MAX_LINE_LENGTH × TEXT_EDITOR_MAX_LINES = 64KB |
| Selection | TEXT_EDITOR_MAX_LINE_LENGTH = 256 bytes |
| Single line | TEXT_EDITOR_MAX_LINE_LENGTH = 256 bytes |
| Clipboard | TEXT_EDITOR_MAX_LINE_LENGTH = 256 bytes |
| Custom | Limited by SD card |

---

## Best Practices

### Do This ✅

```c
// Always check SD status
SdStatus status = text_editor_sd_save_full(editor);
if (text_editor_sd_success(status)) {
    // Success
} else {
    Serial.print("Error: ");
    Serial.println(text_editor_sd_status_string(status));
}

// Check if SD is ready before operations
if (text_editor_sd_is_ready(editor)) {
    text_editor_sd_save_selection(editor);
}

// Use meaningful base filenames
text_editor_sd_init(editor, "document1");
text_editor_sd_init(editor, "code_file");

// Enable auto-save for important documents
text_editor_sd_set_auto_save_full(editor, true);
text_editor_sd_set_auto_save_interval(editor, 60000);  // 1 minute

// Clean up temp files when done
text_editor_sd_delete_all_files(editor);

// Use custom types for application-specific data
text_editor_sd_save_custom(editor, "config", config_data);

// Check if files exist before loading
if (text_editor_sd_file_exists(editor, SD_SELECTION_FULL, 0)) {
    text_editor_sd_load_full(editor);
}
```

### Don't Do This ❌

```c
// DON'T assume SD is always ready
text_editor_sd_save_full(editor);  // Might fail if SD not initialized

// DON'T use without checking
text_editor_sd_load_selection(editor);  // Check if file exists first

// DON'T save without checking selection
text_editor_sd_save_selection(editor);  // Will fail if no selection
if (text_editor_has_selection(editor)) {
    text_editor_sd_save_selection(editor);  // Safe
}

// DON'T use long base filenames
text_editor_sd_init(editor, "very_long_filename_that_exceeds_limits");

// DON'T forget to initialize
text_editor_sd_save_full(editor);  // Not initialized!

// DON'T leak temp files
// Remember to delete files when no longer needed
```

---

## File Structure

### Created Files

| **File** | **Path** | **Description** |
|----------|----------|-----------------|
| Header | `src/gui/text_editor_sd.h` | SD integration declarations |
| Implementation | `src/gui/text_editor_sd.c` | SD integration implementation |
| Documentation | `docs/discussion_analysis/20_TEXT_EDITOR_SD.md` | This file |

### Dependencies

```
src/gui/text_editor_sd.h  -->  text_editor.h
src/gui/text_editor_sd.c  -->  text_editor_sd.h
                               sd_card.h (or stdio.h)
```

---

## Cross-References

- **Text Editor**: See `19_TEXT_EDITOR.md` for core text editor
- **SD Card Integration**: See `11_SD_CARD_WEBDAV.md` for SD card implementation
- **File System**: See platform-specific file system docs

---

## Summary

| **Feature** | **Implementation** | **Benefit** |
|-------------|-------------------|-------------|
| Temp File Saving | `/tmp/(filename)_(type).txt` pattern | Persistent storage |
| Multiple Selection Types | full, selection, line, clipboard, cursor | Flexible saving |
| Auto-Save | Configurable intervals | Data protection |
| Session Management | Save/restore document + cursor + selection | User convenience |
| Bookmark Support | Save/load cursor positions | Navigation aids |
| File Management | List, delete, check existence | Cleanup and info |
| SD Card Integration | Uses existing sd_card.h | Consistent API |
| Error Handling | SdStatus enum with helpers | Robust operations |
| ESP8266 Friendly | SD card storage, minimal RAM | Memory efficient |

The **Text Editor SD Integration** provides **complete persistent storage** for the text editor, enabling saving selections, auto-save, session recovery, and bookmark support using the pattern `/tmp/(filename)_(selection_type).txt`.
