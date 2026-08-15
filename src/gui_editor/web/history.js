/**
 * GUIKit Web Editor - History Management (Undo/Redo)
 */

// ============================================================================
// History Management
// ============================================================================

function saveToHistory(action) {
    // Remove future history
    EditorState.history = EditorState.history.slice(0, EditorState.historyIndex + 1);

    // Save current state
    EditorState.history.push({
        action,
        gui: JSON.parse(JSON.stringify(EditorState.gui)),
        selection: new Set(EditorState.selectedWidgets)
    });

    // Limit history size
    if (EditorState.history.length > EditorState.maxHistory) {
        EditorState.history.shift();
    }

    EditorState.historyIndex = EditorState.history.length - 1;
    EditorState.isSaved = false;
    updateStatusBar();
}

function undo() {
    if (EditorState.historyIndex > 0) {
        EditorState.historyIndex--;
        const state = EditorState.history[EditorState.historyIndex];

        EditorState.gui = JSON.parse(JSON.stringify(state.gui));
        EditorState.selectedWidgets = new Set(state.selection);
        EditorState.selectedWidget = EditorState.selectedWidgets.size > 0 ?
            [...EditorState.selectedWidgets][0] : null;

        updatePropertyInspector();
        updateHierarchy();
        updateJSONEditor();
        renderCanvas();
        updateStatusBar();
    }
}

function redo() {
    if (EditorState.historyIndex < EditorState.history.length - 1) {
        EditorState.historyIndex++;
        const state = EditorState.history[EditorState.historyIndex];

        EditorState.gui = JSON.parse(JSON.stringify(state.gui));
        EditorState.selectedWidgets = new Set(state.selection);
        EditorState.selectedWidget = EditorState.selectedWidgets.size > 0 ?
            [...EditorState.selectedWidgets][0] : null;

        updatePropertyInspector();
        updateHierarchy();
        updateJSONEditor();
        renderCanvas();
        updateStatusBar();
    }
}

// ============================================================================
// Status Bar
// ============================================================================

function updateStatusBar() {
    const widgetCountEl = document.getElementById('status-widget-count');
    const selectionEl = document.getElementById('status-selection');
    const fileEl = document.getElementById('status-file');

    if (widgetCountEl) {
        widgetCountEl.textContent = `${countAllWidgets(EditorState.gui.widgets)} widgets`;
    }

    if (selectionEl) {
        if (EditorState.selectedWidgets.size === 0) {
            selectionEl.textContent = '| No selection';
        } else if (EditorState.selectedWidgets.size === 1) {
            selectionEl.textContent = ` | Selected: ${EditorState.selectedWidget.id}`;
        } else {
            selectionEl.textContent = ` | ${EditorState.selectedWidgets.size} widgets selected`;
        }
    }

    if (fileEl) {
        const filename = EditorState.currentFile || 'New GUI';
        fileEl.textContent = ` | ${filename}${EditorState.isSaved ? '' : '*'}`;
    }
}

function countAllWidgets(widgets) {
    let count = widgets.length;
    for (const widget of widgets) {
        if (widget.type === 'view' && widget.children) {
            count += countAllWidgets(widget.children);
        }
    }
    return count;
}
