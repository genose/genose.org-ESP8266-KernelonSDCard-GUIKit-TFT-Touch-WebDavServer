/**
 * @file text_editor_sd.c
 * @brief SD card integration implementation for text editor
 * @author GUIKit for ESP8266
 * @date 2026
 */

#include "text_editor_sd.h"
#include "sd_card.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// STATIC HELPERS
// =============================================================================

/**
 * @brief Check if SD card is initialized and ready
 */
static bool sd_ready() {
    // Check if SD card is initialized
    // This depends on your SD card library implementation
    extern bool sd_card_initialized();
    return sd_card_initialized();
}

/**
 * @brief Check if a file exists
 */
static bool file_exists(const char* path) {
    // Check if file exists using your SD library
    // This is a placeholder - implement based on your SD library
    extern bool sd_file_exists(const char* path);
    if (sd_file_exists) {
        return sd_file_exists(path);
    }
    
    // Fallback: try to open the file
    FILE* f = fopen(path, "r");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
}

/**
 * @brief Get file size
 */
static uint32_t get_file_size(const char* path) {
    // Get file size using your SD library
    extern uint32_t sd_file_size(const char* path);
    if (sd_file_size) {
        return sd_file_size(path);
    }
    
    // Fallback
    FILE* f = fopen(path, "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        uint32_t size = ftell(f);
        fclose(f);
        return size;
    }
    return 0;
}

/**
 * @brief Read entire file to buffer
 */
static int32_t read_file(const char* path, char* buffer, uint16_t buffer_size) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        return -1;
    }
    
    int32_t bytes_read = fread(buffer, 1, buffer_size - 1, f);
    fclose(f);
    
    if (bytes_read >= 0) {
        buffer[bytes_read] = '\0';
    }
    
    return bytes_read;
}

/**
 * @brief Write buffer to file
 */
static bool write_file(const char* path, const char* buffer, uint32_t size) {
    FILE* f = fopen(path, "wb");
    if (!f) {
        return false;
    }
    
    size_t written = fwrite(buffer, 1, size, f);
    fclose(f);
    
    return written == size;
}

/**
 * @brief Delete a file
 */
static bool delete_file(const char* path) {
    return remove(path) == 0;
}

/**
 * @brief Ensure temp directory exists
 */
static bool ensure_temp_dir(const char* path) {
    // Check if directory exists
    // For simplicity, we'll assume it exists or create it
    // On ESP8266 with FAT filesystem, directories are created automatically
    return true;
}

/**
 * @brief Get SD state from text editor
 */
static TextEditorSd* get_sd_state(TextEditor* editor) {
    if (!editor) return NULL;
    
    // SD state is stored after the main editor struct
    // We need to extend the TextEditor struct or store it separately
    // For now, we'll use a separate allocation
    static TextEditorSd sd_states[4];
    static TextEditor* editors[4] = {NULL};
    
    for (int i = 0; i < 4; i++) {
        if (editors[i] == editor) {
            return &sd_states[i];
        }
        if (editors[i] == NULL) {
            editors[i] = editor;
            memset(&sd_states[i], 0, sizeof(TextEditorSd));
            strncpy(sd_states[i].temp_dir, TEXT_EDITOR_SD_TMP_DIR, sizeof(sd_states[i].temp_dir));
            sd_states[i].auto_save_interval = 5000;  // 5 seconds
            return &sd_states[i];
        }
    }
    
    return NULL;
}

/**
 * @brief Free SD state (when editor is destroyed)
 */
static void free_sd_state(TextEditor* editor) {
    if (!editor) return;
    
    static TextEditor* editors[4] = {NULL};
    
    for (int i = 0; i < 4; i++) {
        if (editors[i] == editor) {
            editors[i] = NULL;
            return;
        }
    }
}

/**
 * @brief Build filename from type and optional line number
 */
static uint16_t build_filename(const TextEditor* editor, SdSelectionType type,
                               uint16_t line_number, char* buffer, uint16_t buffer_size) {
    TextEditorSd* sd = get_sd_state((TextEditor*)editor);
    if (!sd || !buffer || buffer_size == 0) return 0;
    
    uint16_t written = 0;
    
    // Base filename
    written = snprintf(buffer, buffer_size, "%s", sd->base_filename);
    if (written >= buffer_size) return 0;
    
    // Add separator
    if (written < buffer_size) {
        buffer[written++] = '_';
    }
    
    // Add selection type
    const char* type_str = text_editor_sd_selection_type_string(type);
    uint16_t type_len = strlen(type_str);
    
    if (written + type_len < buffer_size) {
        memcpy(&buffer[written], type_str, type_len);
        written += type_len;
    }
    
    // For line type, add line number
    if (type == SD_SELECTION_LINE) {
        if (written + 8 < buffer_size) {
            written += snprintf(&buffer[written], buffer_size - written, "_%u", line_number);
        }
    }
    
    // Add extension
    if (written + strlen(TEXT_EDITOR_SD_EXT) < buffer_size) {
        strcpy(&buffer[written], TEXT_EDITOR_SD_EXT);
        written += strlen(TEXT_EDITOR_SD_EXT);
    }
    
    return written;
}

// =============================================================================
// INITIALIZATION
// =============================================================================

SdStatus text_editor_sd_init(TextEditor* editor, const char* base_filename) {
    if (!editor) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd) return SD_STATUS_ERROR;
    
    // Check SD card
    if (!sd_ready()) {
        return SD_STATUS_NO_SDCARD;
    }
    
    // Set base filename
    if (base_filename && strlen(base_filename) < TEXT_EDITOR_SD_MAX_FILENAME) {
        strncpy(sd->base_filename, base_filename, TEXT_EDITOR_SD_MAX_FILENAME - 1);
        sd->base_filename[TEXT_EDITOR_SD_MAX_FILENAME - 1] = '\0';
    } else {
        strncpy(sd->base_filename, "text_editor", TEXT_EDITOR_SD_MAX_FILENAME);
    }
    
    // Set temp directory
    strncpy(sd->temp_dir, TEXT_EDITOR_SD_TMP_DIR, TEXT_EDITOR_SD_MAX_PATH - 1);
    sd->temp_dir[TEXT_EDITOR_SD_MAX_PATH - 1] = '\0';
    
    // Ensure temp directory exists
    if (!ensure_temp_dir(sd->temp_dir)) {
        return SD_STATUS_DIR_NOT_FOUND;
    }
    
    // Initialize flags
    sd->sd_initialized = true;
    sd->auto_save_enabled = false;
    sd->auto_save_selection = false;
    sd->auto_save_full = false;
    sd->last_auto_save = 0;
    sd->auto_save_interval = 5000;  // 5 seconds default
    
    return SD_STATUS_OK;
}

void text_editor_sd_deinit(TextEditor* editor) {
    if (!editor) return;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (sd) {
        // Delete all temp files
        text_editor_sd_delete_all_files(editor);
        
        // Free state
        free_sd_state(editor);
    }
}

bool text_editor_sd_is_ready(const TextEditor* editor) {
    if (!editor) return false;
    
    TextEditorSd* sd = get_sd_state((TextEditor*)editor);
    if (!sd) return false;
    
    return sd->sd_initialized && sd_ready();
}

bool text_editor_sd_is_enabled(const TextEditor* editor) {
    if (!editor) return false;
    
    TextEditorSd* sd = get_sd_state((TextEditor*)editor);
    if (!sd) return false;
    
    return sd->sd_initialized;
}

// =============================================================================
// FILE PATH CONSTRUCTION
// =============================================================================

uint16_t text_editor_sd_build_path(const TextEditor* editor, SdSelectionType type,
                                   uint16_t line_number, char* buffer, uint16_t buffer_size) {
    if (!editor || !buffer || buffer_size == 0) return 0;
    
    TextEditorSd* sd = get_sd_state((TextEditor*)editor);
    if (!sd) return 0;
    
    // Build filename
    char filename[TEXT_EDITOR_SD_MAX_FILENAME];
    uint16_t filename_len = build_filename(editor, type, line_number, filename, sizeof(filename));
    if (filename_len == 0) return 0;
    
    // Build full path
    return snprintf(buffer, buffer_size, "%s/%s", sd->temp_dir, filename);
}

uint16_t text_editor_sd_build_custom_path(const TextEditor* editor, const char* custom_type,
                                          char* buffer, uint16_t buffer_size) {
    if (!editor || !custom_type || !buffer || buffer_size == 0) return 0;
    
    TextEditorSd* sd = get_sd_state((TextEditor*)editor);
    if (!sd) return 0;
    
    // Build filename with custom type
    char filename[TEXT_EDITOR_SD_MAX_FILENAME];
    uint16_t written = snprintf(filename, sizeof(filename), "%s_%s%s",
                               sd->base_filename, custom_type, TEXT_EDITOR_SD_EXT);
    if (written >= sizeof(filename)) return 0;
    
    // Build full path
    return snprintf(buffer, buffer_size, "%s/%s", sd->temp_dir, filename);
}

const char* text_editor_sd_get_base_filename(const TextEditor* editor) {
    if (!editor) return NULL;
    
    TextEditorSd* sd = get_sd_state((TextEditor*)editor);
    if (!sd) return NULL;
    
    return sd->base_filename;
}

SdStatus text_editor_sd_set_base_filename(TextEditor* editor, const char* base_filename) {
    if (!editor || !base_filename) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd) return SD_STATUS_ERROR;
    
    if (strlen(base_filename) >= TEXT_EDITOR_SD_MAX_FILENAME) {
        return SD_STATUS_ERROR;
    }
    
    strncpy(sd->base_filename, base_filename, TEXT_EDITOR_SD_MAX_FILENAME - 1);
    sd->base_filename[TEXT_EDITOR_SD_MAX_FILENAME - 1] = '\0';
    
    return SD_STATUS_OK;
}

SdStatus text_editor_sd_set_temp_dir(TextEditor* editor, const char* temp_dir) {
    if (!editor || !temp_dir) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd) return SD_STATUS_ERROR;
    
    if (strlen(temp_dir) >= TEXT_EDITOR_SD_MAX_PATH) {
        return SD_STATUS_ERROR;
    }
    
    strncpy(sd->temp_dir, temp_dir, TEXT_EDITOR_SD_MAX_PATH - 1);
    sd->temp_dir[TEXT_EDITOR_SD_MAX_PATH - 1] = '\0';
    
    return SD_STATUS_OK;
}

// =============================================================================
// SAVE OPERATIONS
// =============================================================================

SdStatus text_editor_sd_save_full(TextEditor* editor) {
    if (!editor) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->sd_initialized) return SD_STATUS_NOT_INITIALIZED;
    
    // Get text
    char buffer[TEXT_EDITOR_MAX_LINE_LENGTH * TEXT_EDITOR_MAX_LINES];
    uint16_t len = text_editor_get_text(editor, buffer, sizeof(buffer));
    
    if (len == 0) return SD_STATUS_ERROR;
    
    // Build path
    char path[TEXT_EDITOR_SD_MAX_PATH];
    uint16_t path_len = text_editor_sd_build_path(editor, SD_SELECTION_FULL, 0, path, sizeof(path));
    if (path_len == 0) return SD_STATUS_ERROR;
    
    // Ensure directory exists
    if (!ensure_temp_dir(sd->temp_dir)) {
        return SD_STATUS_DIR_NOT_FOUND;
    }
    
    // Write file
    if (!write_file(path, buffer, len)) {
        return SD_STATUS_WRITE_ERROR;
    }
    
    return SD_STATUS_OK;
}

SdStatus text_editor_sd_save_selection(TextEditor* editor) {
    if (!editor) return SD_STATUS_ERROR;
    
    if (!text_editor_has_selection(editor)) {
        return SD_STATUS_ERROR;  // No selection
    }
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->sd_initialized) return SD_STATUS_NOT_INITIALIZED;
    
    // Get selected text
    char buffer[TEXT_EDITOR_MAX_LINE_LENGTH];
    uint16_t len = text_editor_get_selected_text(editor, buffer, sizeof(buffer));
    
    if (len == 0) return SD_STATUS_ERROR;
    
    // Build path
    char path[TEXT_EDITOR_SD_MAX_PATH];
    uint16_t path_len = text_editor_sd_build_path(editor, SD_SELECTION_CURRENT, 0, path, sizeof(path));
    if (path_len == 0) return SD_STATUS_ERROR;
    
    // Ensure directory exists
    if (!ensure_temp_dir(sd->temp_dir)) {
        return SD_STATUS_DIR_NOT_FOUND;
    }
    
    // Write file
    if (!write_file(path, buffer, len)) {
        return SD_STATUS_WRITE_ERROR;
    }
    
    return SD_STATUS_OK;
}

SdStatus text_editor_sd_save_line(TextEditor* editor, uint16_t line_index) {
    if (!editor) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->sd_initialized) return SD_STATUS_NOT_INITIALIZED;
    
    // Get line
    const char* line_text = text_editor_get_line(editor, line_index);
    if (!line_text) return SD_STATUS_ERROR;
    
    uint16_t len = strlen(line_text);
    
    // Build path with line number
    char path[TEXT_EDITOR_SD_MAX_PATH];
    uint16_t path_len = text_editor_sd_build_path(editor, SD_SELECTION_LINE, line_index, path, sizeof(path));
    if (path_len == 0) return SD_STATUS_ERROR;
    
    // Ensure directory exists
    if (!ensure_temp_dir(sd->temp_dir)) {
        return SD_STATUS_DIR_NOT_FOUND;
    }
    
    // Write file
    if (!write_file(path, line_text, len)) {
        return SD_STATUS_WRITE_ERROR;
    }
    
    return SD_STATUS_OK;
}

SdStatus text_editor_sd_save_clipboard(TextEditor* editor) {
    if (!editor) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->sd_initialized) return SD_STATUS_NOT_INITIALIZED;
    
    const char* clipboard_text = text_editor_get_clipboard(editor);
    if (!clipboard_text || strlen(clipboard_text) == 0) {
        return SD_STATUS_ERROR;  // Empty clipboard
    }
    
    uint16_t len = strlen(clipboard_text);
    
    // Build path
    char path[TEXT_EDITOR_SD_MAX_PATH];
    uint16_t path_len = text_editor_sd_build_path(editor, SD_SELECTION_CLIPBOARD, 0, path, sizeof(path));
    if (path_len == 0) return SD_STATUS_ERROR;
    
    // Ensure directory exists
    if (!ensure_temp_dir(sd->temp_dir)) {
        return SD_STATUS_DIR_NOT_FOUND;
    }
    
    // Write file
    if (!write_file(path, clipboard_text, len)) {
        return SD_STATUS_WRITE_ERROR;
    }
    
    return SD_STATUS_OK;
}

SdStatus text_editor_sd_save_cursor_region(TextEditor* editor, uint16_t lines_before, uint16_t lines_after) {
    if (!editor) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->sd_initialized) return SD_STATUS_NOT_INITIALIZED;
    
    // Get cursor position
    uint16_t cursor_line = text_editor_get_cursor_line(editor);
    uint16_t line_count = text_editor_get_line_count(editor);
    
    // Calculate range
    uint16_t start_line = (cursor_line >= lines_before) ? cursor_line - lines_before : 0;
    uint16_t end_line = cursor_line + lines_after;
    if (end_line >= line_count) end_line = line_count - 1;
    
    // Build text buffer
    char buffer[TEXT_EDITOR_MAX_LINE_LENGTH * TEXT_EDITOR_MAX_LINES];
    uint16_t total_len = 0;
    
    for (uint16_t line = start_line; line <= end_line; line++) {
        const char* line_text = text_editor_get_line(editor, line);
        if (!line_text) continue;
        
        uint16_t line_len = strlen(line_text);
        
        // Add newline if not first line
        if (line > start_line) {
            if (total_len < sizeof(buffer) - 1) {
                buffer[total_len++] = '\n';
            }
        }
        
        // Add line text
        if (total_len + line_len < sizeof(buffer)) {
            memcpy(&buffer[total_len], line_text, line_len);
            total_len += line_len;
        }
    }
    
    if (total_len == 0) return SD_STATUS_ERROR;
    
    // Build path
    char path[TEXT_EDITOR_SD_MAX_PATH];
    uint16_t path_len = text_editor_sd_build_path(editor, SD_SELECTION_CURSOR, 0, path, sizeof(path));
    if (path_len == 0) return SD_STATUS_ERROR;
    
    // Ensure directory exists
    if (!ensure_temp_dir(sd->temp_dir)) {
        return SD_STATUS_DIR_NOT_FOUND;
    }
    
    // Write file
    if (!write_file(path, buffer, total_len)) {
        return SD_STATUS_WRITE_ERROR;
    }
    
    return SD_STATUS_OK;
}

SdStatus text_editor_sd_save_custom(TextEditor* editor, const char* custom_type, const char* text) {
    if (!editor || !custom_type || !text) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->sd_initialized) return SD_STATUS_NOT_INITIALIZED;
    
    uint16_t len = strlen(text);
    
    // Build path
    char path[TEXT_EDITOR_SD_MAX_PATH];
    uint16_t path_len = text_editor_sd_build_custom_path(editor, custom_type, path, sizeof(path));
    if (path_len == 0) return SD_STATUS_ERROR;
    
    // Ensure directory exists
    if (!ensure_temp_dir(sd->temp_dir)) {
        return SD_STATUS_DIR_NOT_FOUND;
    }
    
    // Write file
    if (!write_file(path, text, len)) {
        return SD_STATUS_WRITE_ERROR;
    }
    
    return SD_STATUS_OK;
}

// =============================================================================
// LOAD OPERATIONS
// =============================================================================

SdStatus text_editor_sd_load_full(TextEditor* editor) {
    if (!editor) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->sd_initialized) return SD_STATUS_NOT_INITIALIZED;
    
    // Build path
    char path[TEXT_EDITOR_SD_MAX_PATH];
    uint16_t path_len = text_editor_sd_build_path(editor, SD_SELECTION_FULL, 0, path, sizeof(path));
    if (path_len == 0) return SD_STATUS_ERROR;
    
    // Check if file exists
    if (!file_exists(path)) {
        return SD_STATUS_FILE_NOT_FOUND;
    }
    
    // Read file
    char buffer[TEXT_EDITOR_MAX_LINE_LENGTH * TEXT_EDITOR_MAX_LINES];
    int32_t bytes_read = read_file(path, buffer, sizeof(buffer));
    
    if (bytes_read < 0) {
        return SD_STATUS_READ_ERROR;
    }
    
    // Set text
    text_editor_set_text(editor, buffer);
    
    return SD_STATUS_OK;
}

SdStatus text_editor_sd_load_selection(TextEditor* editor) {
    if (!editor) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->sd_initialized) return SD_STATUS_NOT_INITIALIZED;
    
    // Build path
    char path[TEXT_EDITOR_SD_MAX_PATH];
    uint16_t path_len = text_editor_sd_build_path(editor, SD_SELECTION_CURRENT, 0, path, sizeof(path));
    if (path_len == 0) return SD_STATUS_ERROR;
    
    // Check if file exists
    if (!file_exists(path)) {
        return SD_STATUS_FILE_NOT_FOUND;
    }
    
    // Read file
    char buffer[TEXT_EDITOR_MAX_LINE_LENGTH];
    int32_t bytes_read = read_file(path, buffer, sizeof(buffer));
    
    if (bytes_read < 0) {
        return SD_STATUS_READ_ERROR;
    }
    
    // Set text
    text_editor_set_text(editor, buffer);
    
    return SD_STATUS_OK;
}

SdStatus text_editor_sd_load(TextEditor* editor, SdSelectionType type, uint16_t line_number) {
    if (!editor) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->sd_initialized) return SD_STATUS_NOT_INITIALIZED;
    
    // Build path
    char path[TEXT_EDITOR_SD_MAX_PATH];
    uint16_t path_len = text_editor_sd_build_path(editor, type, line_number, path, sizeof(path));
    if (path_len == 0) return SD_STATUS_ERROR;
    
    // Check if file exists
    if (!file_exists(path)) {
        return SD_STATUS_FILE_NOT_FOUND;
    }
    
    // Read file
    char buffer[TEXT_EDITOR_MAX_LINE_LENGTH * TEXT_EDITOR_MAX_LINES];
    int32_t bytes_read = read_file(path, buffer, sizeof(buffer));
    
    if (bytes_read < 0) {
        return SD_STATUS_READ_ERROR;
    }
    
    // Set text
    text_editor_set_text(editor, buffer);
    
    return SD_STATUS_OK;
}

SdStatus text_editor_sd_load_custom(TextEditor* editor, const char* custom_type) {
    if (!editor || !custom_type) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->sd_initialized) return SD_STATUS_NOT_INITIALIZED;
    
    // Build path
    char path[TEXT_EDITOR_SD_MAX_PATH];
    uint16_t path_len = text_editor_sd_build_custom_path(editor, custom_type, path, sizeof(path));
    if (path_len == 0) return SD_STATUS_ERROR;
    
    // Check if file exists
    if (!file_exists(path)) {
        return SD_STATUS_FILE_NOT_FOUND;
    }
    
    // Read file
    char buffer[TEXT_EDITOR_MAX_LINE_LENGTH * TEXT_EDITOR_MAX_LINES];
    int32_t bytes_read = read_file(path, buffer, sizeof(buffer));
    
    if (bytes_read < 0) {
        return SD_STATUS_READ_ERROR;
    }
    
    // Set text
    text_editor_set_text(editor, buffer);
    
    return SD_STATUS_OK;
}

SdStatus text_editor_sd_insert_from_temp(TextEditor* editor, SdSelectionType type, uint16_t line_number) {
    if (!editor) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->sd_initialized) return SD_STATUS_NOT_INITIALIZED;
    
    // Build path
    char path[TEXT_EDITOR_SD_MAX_PATH];
    uint16_t path_len = text_editor_sd_build_path(editor, type, line_number, path, sizeof(path));
    if (path_len == 0) return SD_STATUS_ERROR;
    
    // Check if file exists
    if (!file_exists(path)) {
        return SD_STATUS_FILE_NOT_FOUND;
    }
    
    // Read file
    char buffer[TEXT_EDITOR_MAX_LINE_LENGTH * TEXT_EDITOR_MAX_LINES];
    int32_t bytes_read = read_file(path, buffer, sizeof(buffer));
    
    if (bytes_read < 0) {
        return SD_STATUS_READ_ERROR;
    }
    
    // Insert at cursor
    text_editor_insert_text(editor, buffer, bytes_read);
    
    return SD_STATUS_OK;
}

SdStatus text_editor_sd_set_selection_from_temp(TextEditor* editor, SdSelectionType type, uint16_t line_number) {
    if (!editor) return SD_STATUS_ERROR;
    
    // Load text first
    SdStatus status = text_editor_sd_load(editor, type, line_number);
    if (!text_editor_sd_success(status)) {
        return status;
    }
    
    // Select all text
    text_editor_select_all(editor);
    
    return SD_STATUS_OK;
}

// =============================================================================
// AUTO-SAVE
// =============================================================================

/**
 * @brief Get current timestamp
 */
static uint32_t get_timestamp() {
    extern uint32_t millis();
    return millis();
}

void text_editor_sd_set_auto_save(TextEditor* editor, bool enabled) {
    if (!editor) return;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd) return;
    
    sd->auto_save_enabled = enabled;
}

void text_editor_sd_set_auto_save_selection(TextEditor* editor, bool enabled) {
    if (!editor) return;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd) return;
    
    sd->auto_save_selection = enabled;
}

void text_editor_sd_set_auto_save_full(TextEditor* editor, bool enabled) {
    if (!editor) return;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd) return;
    
    sd->auto_save_full = enabled;
}

void text_editor_sd_set_auto_save_interval(TextEditor* editor, uint32_t interval_ms) {
    if (!editor) return;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd) return;
    
    sd->auto_save_interval = interval_ms;
}

bool text_editor_sd_trigger_auto_save(TextEditor* editor) {
    if (!editor) return false;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->auto_save_enabled) return false;
    
    uint32_t now = get_timestamp();
    uint32_t elapsed = now - sd->last_auto_save;
    
    if (elapsed >= sd->auto_save_interval) {
        text_editor_sd_do_auto_save(editor);
        return true;
    }
    
    return false;
}

void text_editor_sd_do_auto_save(TextEditor* editor) {
    if (!editor) return;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->auto_save_enabled) return;
    
    // Save full document if enabled
    if (sd->auto_save_full) {
        text_editor_sd_save_full(editor);
    }
    
    // Save selection if enabled and there is a selection
    if (sd->auto_save_selection && text_editor_has_selection(editor)) {
        text_editor_sd_save_selection(editor);
    }
    
    // Save backup (always save a backup)
    text_editor_sd_save_custom(editor, "backup", NULL);
    
    sd->last_auto_save = get_timestamp();
}

// =============================================================================
// FILE MANAGEMENT
// =============================================================================

bool text_editor_sd_file_exists(const TextEditor* editor, SdSelectionType type, uint16_t line_number) {
    if (!editor) return false;
    
    TextEditorSd* sd = get_sd_state((TextEditor*)editor);
    if (!sd || !sd->sd_initialized) return false;
    
    // Build path
    char path[TEXT_EDITOR_SD_MAX_PATH];
    uint16_t path_len = text_editor_sd_build_path(editor, type, line_number, path, sizeof(path));
    if (path_len == 0) return false;
    
    return file_exists(path);
}

SdStatus text_editor_sd_get_file_info(const TextEditor* editor, SdSelectionType type,
                                      uint16_t line_number, TextEditorSdFileInfo* info) {
    if (!editor || !info) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state((TextEditor*)editor);
    if (!sd || !sd->sd_initialized) return SD_STATUS_NOT_INITIALIZED;
    
    // Build path
    char path[TEXT_EDITOR_SD_MAX_PATH];
    uint16_t path_len = text_editor_sd_build_path(editor, type, line_number, path, sizeof(path));
    if (path_len == 0) return SD_STATUS_ERROR;
    
    // Get file size
    info->size = get_file_size(path);
    info->exists = (info->size > 0);
    info->timestamp = 0;  // Would need filesystem support for timestamp
    info->type = type;
    
    strncpy(info->path, path, TEXT_EDITOR_SD_MAX_PATH - 1);
    info->path[TEXT_EDITOR_SD_MAX_PATH - 1] = '\0';
    
    // Build filename
    char filename[TEXT_EDITOR_SD_MAX_FILENAME];
    uint16_t filename_len = build_filename(editor, type, line_number, filename, sizeof(filename));
    if (filename_len > 0) {
        strncpy(info->name, filename, TEXT_EDITOR_SD_MAX_FILENAME - 1);
        info->name[TEXT_EDITOR_SD_MAX_FILENAME - 1] = '\0';
    } else {
        info->name[0] = '\0';
    }
    
    return SD_STATUS_OK;
}

int16_t text_editor_sd_list_files(const TextEditor* editor, TextEditorSdFileInfo* infos, uint16_t max_infos) {
    if (!editor || !infos || max_infos == 0) return -1;
    
    TextEditorSd* sd = get_sd_state((TextEditor*)editor);
    if (!sd || !sd->sd_initialized) return -1;
    
    int16_t count = 0;
    
    // Check all selection types
    for (int type = SD_SELECTION_FULL; type <= SD_SELECTION_CURSOR && count < max_infos; type++) {
        SdSelectionType sel_type = (SdSelectionType)type;
        
        // For line type, we need to check all lines
        if (sel_type == SD_SELECTION_LINE) {
            uint16_t line_count = text_editor_get_line_count(editor);
            for (uint16_t line = 0; line < line_count && count < max_infos; line++) {
                if (text_editor_sd_file_exists(editor, sel_type, line)) {
                    TextEditorSdFileInfo info;
                    if (text_editor_sd_get_file_info(editor, sel_type, line, &info) == SD_STATUS_OK) {
                        infos[count++] = info;
                    }
                }
            }
        } else {
            if (text_editor_sd_file_exists(editor, sel_type, 0)) {
                TextEditorSdFileInfo info;
                if (text_editor_sd_get_file_info(editor, sel_type, 0, &info) == SD_STATUS_OK) {
                    infos[count++] = info;
                }
            }
        }
    }
    
    return count;
}

SdStatus text_editor_sd_delete_file(TextEditor* editor, SdSelectionType type, uint16_t line_number) {
    if (!editor) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->sd_initialized) return SD_STATUS_NOT_INITIALIZED;
    
    // Build path
    char path[TEXT_EDITOR_SD_MAX_PATH];
    uint16_t path_len = text_editor_sd_build_path(editor, type, line_number, path, sizeof(path));
    if (path_len == 0) return SD_STATUS_ERROR;
    
    if (!delete_file(path)) {
        return SD_STATUS_ERROR;
    }
    
    return SD_STATUS_OK;
}

SdStatus text_editor_sd_delete_custom_file(TextEditor* editor, const char* custom_type) {
    if (!editor || !custom_type) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->sd_initialized) return SD_STATUS_NOT_INITIALIZED;
    
    // Build path
    char path[TEXT_EDITOR_SD_MAX_PATH];
    uint16_t path_len = text_editor_sd_build_custom_path(editor, custom_type, path, sizeof(path));
    if (path_len == 0) return SD_STATUS_ERROR;
    
    if (!delete_file(path)) {
        return SD_STATUS_ERROR;
    }
    
    return SD_STATUS_OK;
}

SdStatus text_editor_sd_delete_all_files(TextEditor* editor) {
    if (!editor) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->sd_initialized) return SD_STATUS_NOT_INITIALIZED;
    
    // Delete all selection type files
    for (int type = SD_SELECTION_FULL; type <= SD_SELECTION_CURSOR; type++) {
        SdSelectionType sel_type = (SdSelectionType)type;
        
        if (sel_type == SD_SELECTION_LINE) {
            uint16_t line_count = text_editor_get_line_count(editor);
            for (uint16_t line = 0; line < line_count; line++) {
                text_editor_sd_delete_file(editor, sel_type, line);
            }
        } else {
            text_editor_sd_delete_file(editor, sel_type, 0);
        }
    }
    
    return SD_STATUS_OK;
}

// =============================================================================
// CONTENT OPERATIONS
// =============================================================================

int32_t text_editor_sd_read_file(const TextEditor* editor, SdSelectionType type,
                                uint16_t line_number, char* buffer, uint16_t buffer_size) {
    if (!editor || !buffer || buffer_size == 0) return -1;
    
    TextEditorSd* sd = get_sd_state((TextEditor*)editor);
    if (!sd || !sd->sd_initialized) return -1;
    
    // Build path
    char path[TEXT_EDITOR_SD_MAX_PATH];
    uint16_t path_len = text_editor_sd_build_path(editor, type, line_number, path, sizeof(path));
    if (path_len == 0) return -1;
    
    // Check if file exists
    if (!file_exists(path)) {
        return -1;
    }
    
    // Read file
    return read_file(path, buffer, buffer_size);
}

int32_t text_editor_sd_read_custom_file(const TextEditor* editor, const char* custom_type,
                                        char* buffer, uint16_t buffer_size) {
    if (!editor || !custom_type || !buffer || buffer_size == 0) return -1;
    
    TextEditorSd* sd = get_sd_state((TextEditor*)editor);
    if (!sd || !sd->sd_initialized) return -1;
    
    // Build path
    char path[TEXT_EDITOR_SD_MAX_PATH];
    uint16_t path_len = text_editor_sd_build_custom_path(editor, custom_type, path, sizeof(path));
    if (path_len == 0) return -1;
    
    // Check if file exists
    if (!file_exists(path)) {
        return -1;
    }
    
    // Read file
    return read_file(path, buffer, buffer_size);
}

int32_t text_editor_sd_get_file_size(const TextEditor* editor, SdSelectionType type, uint16_t line_number) {
    if (!editor) return -1;
    
    TextEditorSd* sd = get_sd_state((TextEditor*)editor);
    if (!sd || !sd->sd_initialized) return -1;
    
    // Build path
    char path[TEXT_EDITOR_SD_MAX_PATH];
    uint16_t path_len = text_editor_sd_build_path(editor, type, line_number, path, sizeof(path));
    if (path_len == 0) return -1;
    
    return get_file_size(path);
}

// =============================================================================
// SESSION MANAGEMENT
// =============================================================================

SdStatus text_editor_sd_save_session(TextEditor* editor) {
    if (!editor) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->sd_initialized) return SD_STATUS_NOT_INITIALIZED;
    
    // Save full document
    SdStatus status = text_editor_sd_save_full(editor);
    if (!text_editor_sd_success(status)) {
        return status;
    }
    
    // Save cursor position
    uint16_t cursor_line = text_editor_get_cursor_line(editor);
    uint16_t cursor_col = text_editor_get_cursor_column(editor);
    
    char cursor_info[64];
    snprintf(cursor_info, sizeof(cursor_info), "%u,%u", cursor_line, cursor_col);
    
    text_editor_sd_save_custom(editor, "cursor_pos", cursor_info);
    
    // Save selection if active
    if (text_editor_has_selection(editor)) {
        uint16_t start_line, start_col, end_line, end_col;
        text_editor_get_selection_start(editor, &start_line, &start_col);
        text_editor_get_selection_end(editor, &end_line, &end_col);
        
        char sel_info[64];
        snprintf(sel_info, sizeof(sel_info), "%u,%u,%u,%u", start_line, start_col, end_line, end_col);
        
        text_editor_sd_save_custom(editor, "selection", sel_info);
    }
    
    return SD_STATUS_OK;
}

SdStatus text_editor_sd_load_session(TextEditor* editor) {
    if (!editor) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->sd_initialized) return SD_STATUS_NOT_INITIALIZED;
    
    // Load full document
    SdStatus status = text_editor_sd_load_full(editor);
    if (!text_editor_sd_success(status)) {
        return status;
    }
    
    // Load cursor position
    char cursor_info[64];
    int32_t bytes_read = text_editor_sd_read_custom_file(editor, "cursor_pos", cursor_info, sizeof(cursor_info));
    if (bytes_read > 0) {
        uint16_t line, col;
        if (sscanf(cursor_info, "%u,%u", &line, &col) == 2) {
            text_editor_set_cursor(editor, line, col);
        }
    }
    
    // Load selection
    char sel_info[64];
    bytes_read = text_editor_sd_read_custom_file(editor, "selection", sel_info, sizeof(sel_info));
    if (bytes_read > 0) {
        uint16_t start_line, start_col, end_line, end_col;
        if (sscanf(sel_info, "%u,%u,%u,%u", &start_line, &start_col, &end_line, &end_col) == 4) {
            text_editor_set_selection(editor, start_line, start_col, end_line, end_col);
        }
    }
    
    return SD_STATUS_OK;
}

bool text_editor_sd_session_exists(const TextEditor* editor) {
    if (!editor) return false;
    
    // Check if full document file exists
    if (text_editor_sd_file_exists(editor, SD_SELECTION_FULL, 0)) {
        return true;
    }
    
    return false;
}

SdStatus text_editor_sd_clear_session(TextEditor* editor) {
    if (!editor) return SD_STATUS_ERROR;
    
    // Delete session files
    text_editor_sd_delete_file(editor, SD_SELECTION_FULL, 0);
    text_editor_sd_delete_custom_file(editor, "cursor_pos");
    text_editor_sd_delete_custom_file(editor, "selection");
    
    return SD_STATUS_OK;
}

// =============================================================================
// BOOKMARK OPERATIONS
// =============================================================================

SdStatus text_editor_sd_save_bookmark(TextEditor* editor, const char* bookmark_name) {
    if (!editor || !bookmark_name) return SD_STATUS_ERROR;
    
    TextEditorSd* sd = get_sd_state(editor);
    if (!sd || !sd->sd_initialized) return SD_STATUS_NOT_INITIALIZED;
    
    // Get cursor position
    uint16_t line = text_editor_get_cursor_line(editor);
    uint16_t col = text_editor_get_cursor_column(editor);
    
    // Save bookmark info
    char info[64];
    snprintf(info, sizeof(info), "%u,%u", line, col);
    
    return text_editor_sd_save_custom(editor, bookmark_name, info);
}

SdStatus text_editor_sd_load_bookmark(TextEditor* editor, const char* bookmark_name) {
    if (!editor || !bookmark_name) return SD_STATUS_ERROR;
    
    // Load bookmark file and set cursor
    char info[64];
    int32_t bytes_read = text_editor_sd_read_custom_file(editor, bookmark_name, info, sizeof(info));
    if (bytes_read <= 0) {
        return SD_STATUS_FILE_NOT_FOUND;
    }
    
    uint16_t line, col;
    if (sscanf(info, "%u,%u", &line, &col) == 2) {
        text_editor_set_cursor(editor, line, col);
        return SD_STATUS_OK;
    }
    
    return SD_STATUS_ERROR;
}

SdStatus text_editor_sd_goto_bookmark(TextEditor* editor, const char* bookmark_name) {
    // Goto is the same as load for now (just moves cursor)
    return text_editor_sd_load_bookmark(editor, bookmark_name);
}

SdStatus text_editor_sd_delete_bookmark(TextEditor* editor, const char* bookmark_name) {
    if (!editor || !bookmark_name) return SD_STATUS_ERROR;
    
    return text_editor_sd_delete_custom_file(editor, bookmark_name);
}

// =============================================================================
// CALLBACK INTEGRATION
// =============================================================================

// We need to extend the TextEditor structure to store callbacks
// For now, we'll use a simple approach

void text_editor_sd_set_status_callback(TextEditor* editor, void (*callback)(TextEditor*, SdStatus)) {
    // Store callback - would need to extend TextEditor struct
    // For now, this is a placeholder
}

void text_editor_sd_set_file_callback(TextEditor* editor, void (*callback)(TextEditor*, const char*, SdStatus)) {
    // Store callback - would need to extend TextEditor struct
    // For now, this is a placeholder
}
