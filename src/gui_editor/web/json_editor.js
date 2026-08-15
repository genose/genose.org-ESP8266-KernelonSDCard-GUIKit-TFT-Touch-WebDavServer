/**
 * GUIKit Web Editor - JSON Editor
 */

// ============================================================================
// JSON Editor
// ============================================================================

function updateJSONEditor() {
    const editor = document.getElementById('json-editor');
    if (!editor) return;
    editor.value = JSON.stringify(EditorState.gui, null, 2);
}

function formatJSON() {
    const editor = document.getElementById('json-editor');
    if (!editor) return;

    try {
        const obj = JSON.parse(editor.value);
        editor.value = JSON.stringify(obj, null, 2);
        logToConsole('JSON formatted');
    } catch (error) {
        logToConsole(`Format error: ${error.message}`);
    }
}

function validateJSON() {
    const editor = document.getElementById('json-editor');
    if (!editor) return;

    try {
        const obj = JSON.parse(editor.value);
        const result = validateGUI(obj);

        if (result.valid) {
            logToConsole('JSON is valid');
        } else {
            logToConsole(`Validation errors: ${result.errors.join(', ')}`);
        }
    } catch (error) {
        logToConsole(`JSON error: ${error.message}`);
    }
}
