/**
 * @file text_editor_sd.h
 * @brief SD card integration for text editor - temporary buffer files
 * @author GUIKit for ESP8266
 * @date 2026
 * 
 * This module provides SD card integration for the text editor, allowing:
 * - Saving text selections to temporary files in /tmp/
 * - Loading text from temporary files
 * - Auto-saving selections when text is selected
 * - Session recovery from temp files
 * 
 * File naming pattern: /tmp/(filename)_(selection_type).txt
 * 
 * Selection types:
 * - "full" - Entire document
 * - "selection" - Currently selected text
 * - "line_N" - Specific line N
 * - "clipboard" - Clipboard content
 * - "cursor" - Text around cursor position
 * 
 * Usage:
 * @code
 * TextEditor* editor = text_editor_create(0, 0, 240, 320);
 * 
 * // Initialize SD card integration
 * text_editor_sd_init(editor, "myfile");
 * 
 * // Save current selection to temp file
 * text_editor_sd_save_selection(editor);  // Saves to /tmp/myfile_selection.txt
 * 
 * // Save entire document
 * text_editor_sd_save_full(editor);  // Saves to /tmp/myfile_full.txt
 * 
 * // Load from temp file
 * text_editor_sd_load_selection(editor, "myfile_selection");
 * 
 * // Enable auto-save on selection change
 * text_editor_sd_set_auto_save(editor, true);
 * @endcode
 */

#ifndef TEXT_EDITOR_SD_H
#define TEXT_EDITOR_SD_H

#include "text_editor.h"
#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// CONSTANTS
// =============================================================================

/**
 * @brief Maximum length of base filename (without extension)
 */
#define TEXT_EDITOR_SD_MAX_FILENAME 64

/**
 * @brief Maximum path length for temp files
 */
#define TEXT_EDITOR_SD_MAX_PATH 128

/**
 * @brief Temporary directory path
 */
#define TEXT_EDITOR_SD_TMP_DIR "/tmp"

/**
 * @brief File extension for temp files
 */
#define TEXT_EDITOR_SD_EXT ".txt"

/**
 * @brief Selection type enum for file naming
 */
typedef enum {
    SD_SELECTION_FULL = 0,        ///< Entire document
    SD_SELECTION_CURRENT,          ///< Currently selected text
    SD_SELECTION_LINE,            ///< Specific line
    SD_SELECTION_CLIPBOARD,       ///< Clipboard content
    SD_SELECTION_CURSOR,          ///< Text around cursor
    SD_SELECTION_BACKUP,          ///< Auto-save backup
    SD_SELECTION_CUSTOM           ///< Custom type (for user-defined)
} SdSelectionType;

// =============================================================================
// SD CARD STATUS
// =============================================================================

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

// =============================================================================
// TEXT EDITOR SD INTEGRATION STRUCTURE
// =============================================================================

/**
 * @brief SD card state for text editor
 */
typedef struct {
    char base_filename[TEXT_EDITOR_SD_MAX_FILENAME];  ///< Base filename without extension
    char temp_dir[TEXT_EDITOR_SD_MAX_PATH];          ///< Temporary directory path
    bool sd_initialized;                              ///< Whether SD card is initialized
    bool auto_save_enabled;                          ///< Whether to auto-save on changes
    bool auto_save_selection;                       ///< Auto-save selection when changed
    bool auto_save_full;                             ///< Auto-save full document periodically
    uint32_t last_auto_save;                         ///< Last auto-save timestamp
    uint32_t auto_save_interval;                     ///< Auto-save interval in ms
} TextEditorSd;

// =============================================================================
// FILE INFO STRUCTURE
// =============================================================================

/**
 * @brief Information about a temporary file
 */
typedef struct {
    char path[TEXT_EDITOR_SD_MAX_PATH];    ///< Full path to file
    char name[TEXT_EDITOR_SD_MAX_FILENAME]; ///< Filename without path
    SdSelectionType type;                 ///< Selection type
    uint32_t timestamp;                   ///< File modification timestamp
    uint32_t size;                        ///< File size in bytes
    bool exists;                         ///< Whether file exists
} TextEditorSdFileInfo;

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// --- Initialization ---

/**
 * @brief Initialize SD card integration for text editor
 * @param editor TextEditor pointer
 * @param base_filename Base filename (without extension)
 * @return SD status
 */
SdStatus text_editor_sd_init(TextEditor* editor, const char* base_filename);

/**
 * @brief Deinitialize SD card integration
 * @param editor TextEditor pointer
 */
void text_editor_sd_deinit(TextEditor* editor);

/**
 * @brief Check if SD card is available and initialized
 * @param editor TextEditor pointer
 * @return true if SD card is ready
 */
bool text_editor_sd_is_ready(const TextEditor* editor);

/**
 * @brief Check if SD card integration is enabled
 * @param editor TextEditor pointer
 * @return true if SD integration is active
 */
bool text_editor_sd_is_enabled(const TextEditor* editor);

// --- File Path Construction ---

/**
 * @brief Build full path for a selection type
 * @param editor TextEditor pointer
 * @param type Selection type
 * @param line_number Line number (for SD_SELECTION_LINE)
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Number of characters written, or 0 on error
 */
uint16_t text_editor_sd_build_path(const TextEditor* editor, SdSelectionType type,
                                   uint16_t line_number, char* buffer, uint16_t buffer_size);

/**
 * @brief Build path for custom selection type
 * @param editor TextEditor pointer
 * @param custom_type Custom type string (e.g., "bookmark1", "note1")
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Number of characters written, or 0 on error
 */
uint16_t text_editor_sd_build_custom_path(const TextEditor* editor, const char* custom_type,
                                          char* buffer, uint16_t buffer_size);

/**
 * @brief Get the base filename
 * @param editor TextEditor pointer
 * @return Base filename string
 */
const char* text_editor_sd_get_base_filename(const TextEditor* editor);

/**
 * @brief Set the base filename
 * @param editor TextEditor pointer
 * @param base_filename New base filename
 * @return SD status
 */
SdStatus text_editor_sd_set_base_filename(TextEditor* editor, const char* base_filename);

/**
 * @brief Set the temporary directory
 * @param editor TextEditor pointer
 * @param temp_dir Path to temporary directory
 * @return SD status
 */
SdStatus text_editor_sd_set_temp_dir(TextEditor* editor, const char* temp_dir);

// --- Save Operations ---

/**
 * @brief Save entire document to temp file
 * @param editor TextEditor pointer
 * @return SD status
 */
SdStatus text_editor_sd_save_full(TextEditor* editor);

/**
 * @brief Save current selection to temp file
 * @param editor TextEditor pointer
 * @return SD status (SD_STATUS_OK if saved, SD_STATUS_ERROR if no selection)
 */
SdStatus text_editor_sd_save_selection(TextEditor* editor);

/**
 * @brief Save a specific line to temp file
 * @param editor TextEditor pointer
 * @param line_index Line index to save
 * @return SD status
 */
SdStatus text_editor_sd_save_line(TextEditor* editor, uint16_t line_index);

/**
 * @brief Save clipboard content to temp file
 * @param editor TextEditor pointer
 * @return SD status
 */
SdStatus text_editor_sd_save_clipboard(TextEditor* editor);

/**
 * @brief Save text around cursor to temp file
 * @param editor TextEditor pointer
 * @param lines_before Number of lines before cursor to save
 * @param lines_after Number of lines after cursor to save
 * @return SD status
 */
SdStatus text_editor_sd_save_cursor_region(TextEditor* editor, uint16_t lines_before, uint16_t lines_after);

/**
 * @brief Save to custom temp file
 * @param editor TextEditor pointer
 * @param custom_type Custom type string for filename
 * @param text Text to save
 * @return SD status
 */
SdStatus text_editor_sd_save_custom(TextEditor* editor, const char* custom_type, const char* text);

// --- Load Operations ---

/**
 * @brief Load text from full document temp file
 * @param editor TextEditor pointer
 * @return SD status
 */
SdStatus text_editor_sd_load_full(TextEditor* editor);

/**
 * @brief Load text from selection temp file
 * @param editor TextEditor pointer
 * @return SD status
 */
SdStatus text_editor_sd_load_selection(TextEditor* editor);

/**
 * @brief Load text from a specific temp file
 * @param editor TextEditor pointer
 * @param type Selection type
 * @param line_number Line number (for SD_SELECTION_LINE)
 * @return SD status
 */
SdStatus text_editor_sd_load(TextEditor* editor, SdSelectionType type, uint16_t line_number);

/**
 * @brief Load text from custom temp file
 * @param editor TextEditor pointer
 * @param custom_type Custom type string
 * @return SD status
 */
SdStatus text_editor_sd_load_custom(TextEditor* editor, const char* custom_type);

/**
 * @brief Load text from any temp file into editor at cursor
 * @param editor TextEditor pointer
 * @param type Selection type
 * @param line_number Line number (for SD_SELECTION_LINE)
 * @return SD status
 */
SdStatus text_editor_sd_insert_from_temp(TextEditor* editor, SdSelectionType type, uint16_t line_number);

/**
 * @brief Load text from any temp file and set as selection
 * @param editor TextEditor pointer
 * @param type Selection type
 * @param line_number Line number (for SD_SELECTION_LINE)
 * @return SD status
 */
SdStatus text_editor_sd_set_selection_from_temp(TextEditor* editor, SdSelectionType type, uint16_t line_number);

// --- Auto-Save ---

/**
 * @brief Enable or disable auto-save
 * @param editor TextEditor pointer
 * @param enabled Whether to enable auto-save
 */
void text_editor_sd_set_auto_save(TextEditor* editor, bool enabled);

/**
 * @brief Enable or disable auto-save for selections
 * @param editor TextEditor pointer
 * @param enabled Whether to enable
 */
void text_editor_sd_set_auto_save_selection(TextEditor* editor, bool enabled);

/**
 * @brief Enable or disable auto-save for full document
 * @param editor TextEditor pointer
 * @param enabled Whether to enable
 */
void text_editor_sd_set_auto_save_full(TextEditor* editor, bool enabled);

/**
 * @brief Set auto-save interval (milliseconds)
 * @param editor TextEditor pointer
 * @param interval_ms Interval in milliseconds
 */
void text_editor_sd_set_auto_save_interval(TextEditor* editor, uint32_t interval_ms);

/**
 * @brief Trigger auto-save if interval has passed
 * @param editor TextEditor pointer
 * @return true if auto-save was performed
 */
bool text_editor_sd_trigger_auto_save(TextEditor* editor);

/**
 * @brief Perform auto-save based on current state
 * @param editor TextEditor pointer
 */
void text_editor_sd_do_auto_save(TextEditor* editor);

// --- File Management ---

/**
 * @brief Check if a temp file exists
 * @param editor TextEditor pointer
 * @param type Selection type
 * @param line_number Line number (for SD_SELECTION_LINE)
 * @return true if file exists
 */
bool text_editor_sd_file_exists(const TextEditor* editor, SdSelectionType type, uint16_t line_number);

/**
 * @brief Get file info for a selection type
 * @param editor TextEditor pointer
 * @param type Selection type
 * @param line_number Line number (for SD_SELECTION_LINE)
 * @param info Output file info
 * @return SD status
 */
SdStatus text_editor_sd_get_file_info(const TextEditor* editor, SdSelectionType type,
                                      uint16_t line_number, TextEditorSdFileInfo* info);

/**
 * @brief List all temp files for this editor
 * @param editor TextEditor pointer
 * @param infos Array of file info structures
 * @param max_infos Maximum number of infos to return
 * @return Number of files found, or -1 on error
 */
int16_t text_editor_sd_list_files(const TextEditor* editor, TextEditorSdFileInfo* infos, uint16_t max_infos);

/**
 * @brief Delete a temp file
 * @param editor TextEditor pointer
 * @param type Selection type
 * @param line_number Line number (for SD_SELECTION_LINE)
 * @return SD status
 */
SdStatus text_editor_sd_delete_file(TextEditor* editor, SdSelectionType type, uint16_t line_number);

/**
 * @brief Delete custom temp file
 * @param editor TextEditor pointer
 * @param custom_type Custom type string
 * @return SD status
 */
SdStatus text_editor_sd_delete_custom_file(TextEditor* editor, const char* custom_type);

/**
 * @brief Delete all temp files for this editor
 * @param editor TextEditor pointer
 * @return SD status
 */
SdStatus text_editor_sd_delete_all_files(TextEditor* editor);

// --- Content Operations ---

/**
 * @brief Read content of a temp file to buffer
 * @param editor TextEditor pointer
 * @param type Selection type
 * @param line_number Line number (for SD_SELECTION_LINE)
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Number of bytes read, or -1 on error
 */
int32_t text_editor_sd_read_file(const TextEditor* editor, SdSelectionType type,
                                uint16_t line_number, char* buffer, uint16_t buffer_size);

/**
 * @brief Read content of custom temp file
 * @param editor TextEditor pointer
 * @param custom_type Custom type string
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Number of bytes read, or -1 on error
 */
int32_t text_editor_sd_read_custom_file(const TextEditor* editor, const char* custom_type,
                                        char* buffer, uint16_t buffer_size);

/**
 * @brief Get the size of a temp file
 * @param editor TextEditor pointer
 * @param type Selection type
 * @param line_number Line number (for SD_SELECTION_LINE)
 * @return File size in bytes, or -1 on error
 */
int32_t text_editor_sd_get_file_size(const TextEditor* editor, SdSelectionType type, uint16_t line_number);

// --- Session Management ---

/**
 * @brief Save current session (full document + cursor + selection)
 * @param editor TextEditor pointer
 * @return SD status
 */
SdStatus text_editor_sd_save_session(TextEditor* editor);

/**
 * @brief Load session from temp files
 * @param editor TextEditor pointer
 * @return SD status
 */
SdStatus text_editor_sd_load_session(TextEditor* editor);

/**
 * @brief Check if session files exist
 * @param editor TextEditor pointer
 * @return true if session can be restored
 */
bool text_editor_sd_session_exists(const TextEditor* editor);

/**
 * @brief Clear session files
 * @param editor TextEditor pointer
 * @return SD status
 */
SdStatus text_editor_sd_clear_session(TextEditor* editor);

// --- Bookmark Operations ---

/**
 * @brief Save current cursor position as a bookmark
 * @param editor TextEditor pointer
 * @param bookmark_name Name for the bookmark
 * @return SD status
 */
SdStatus text_editor_sd_save_bookmark(TextEditor* editor, const char* bookmark_name);

/**
 * @brief Load text from a bookmark file
 * @param editor TextEditor pointer
 * @param bookmark_name Name of the bookmark
 * @return SD status
 */
SdStatus text_editor_sd_load_bookmark(TextEditor* editor, const char* bookmark_name);

/**
 * @brief Go to bookmark position
 * @param editor TextEditor pointer
 * @param bookmark_name Name of the bookmark
 * @return SD status
 */
SdStatus text_editor_sd_goto_bookmark(TextEditor* editor, const char* bookmark_name);

/**
 * @brief Delete a bookmark
 * @param editor TextEditor pointer
 * @param bookmark_name Name of the bookmark
 * @return SD status
 */
SdStatus text_editor_sd_delete_bookmark(TextEditor* editor, const char* bookmark_name);

// --- Callback Integration ---

/**
 * @brief Set callback for SD card status changes
 * @param editor TextEditor pointer
 * @param callback Callback function (SdStatus new_status)
 */
void text_editor_sd_set_status_callback(TextEditor* editor, void (*callback)(TextEditor*, SdStatus));

/**
 * @brief Set callback for file operations
 * @param editor TextEditor pointer
 * @param callback Callback function (const char* filename, SdStatus status)
 */
void text_editor_sd_set_file_callback(TextEditor* editor, void (*callback)(TextEditor*, const char*, SdStatus));

// =============================================================================
// INLINE FUNCTIONS
// =============================================================================

/**
 * @brief Check if last operation succeeded
 * @param status SD status
 * @return true if operation succeeded
 */
static inline bool text_editor_sd_success(SdStatus status) {
    return status == SD_STATUS_OK;
}

/**
 * @brief Check if SD card is not available
 * @param status SD status
 * @return true if SD card is not available
 */
static inline bool text_editor_sd_no_sdcard(SdStatus status) {
    return status == SD_STATUS_NO_SDCARD || status == SD_STATUS_NOT_INITIALIZED;
}

/**
 * @brief Get SD status as string
 * @param status SD status
 * @return String description of status
 */
static inline const char* text_editor_sd_status_string(SdStatus status) {
    switch (status) {
        case SD_STATUS_OK: return "OK";
        case SD_STATUS_ERROR: return "Error";
        case SD_STATUS_NOT_INITIALIZED: return "Not initialized";
        case SD_STATUS_FILE_NOT_FOUND: return "File not found";
        case SD_STATUS_WRITE_ERROR: return "Write error";
        case SD_STATUS_READ_ERROR: return "Read error";
        case SD_STATUS_DIR_NOT_FOUND: return "Directory not found";
        case SD_STATUS_NO_SDCARD: return "No SD card";
        default: return "Unknown";
    }
}

/**
 * @brief Get selection type as string
 * @param type Selection type
 * @return String name of type
 */
static inline const char* text_editor_sd_selection_type_string(SdSelectionType type) {
    switch (type) {
        case SD_SELECTION_FULL: return "full";
        case SD_SELECTION_CURRENT: return "selection";
        case SD_SELECTION_LINE: return "line";
        case SD_SELECTION_CLIPBOARD: return "clipboard";
        case SD_SELECTION_CURSOR: return "cursor";
        case SD_SELECTION_BACKUP: return "backup";
        case SD_SELECTION_CUSTOM: return "custom";
        default: return "unknown";
    }
}

#endif // TEXT_EDITOR_SD_H
