/**
 * GUIKit Web Editor - Context Menu
 */

// ============================================================================
// Context Menus
// ============================================================================

function showWidgetContextMenu(widget, x, y) {
    const menu = document.createElement('div');
    menu.className = 'context-menu';
    menu.style.cssText = 'position:absolute;left:' + x + 'px;top:' + y + 'px;z-index:1000;background:#2D2D2D;border:1px solid #666;border-radius:4px;min-width:150px;box-shadow:2px 2px 10px rgba(0,0,0,0.5)';

    menu.innerHTML = `
        <div class="context-item" onclick="cutWidget()">Cut</div>
        <div class="context-item" onclick="copyWidget()">Copy</div>
        <div class="context-item" onclick="pasteWidget()">Paste</div>
        <hr style="margin:2px 0;border:none;border-top:1px solid #444">
        <div class="context-item" onclick="deleteWidget()">Delete</div>
        <hr style="margin:2px 0;border:none;border-top:1px solid #444">
        <div class="context-item" onclick="bringToFrontWidget()">Bring to Front</div>
        <div class="context-item" onclick="sendToBackWidget()">Send to Back</div>
    `;

    menu.addEventListener('click', (e) => e.stopPropagation());
    document.addEventListener('click', () => menu.remove(), { once: true });
    document.body.appendChild(menu);
}

function showCanvasContextMenu(x, y) {
    const menu = document.createElement('div');
    menu.className = 'context-menu';
    menu.style.cssText = 'position:absolute;left:' + x + 'px;top:' + y + 'px;z-index:1000;background:#2D2D2D;border:1px solid #666;border-radius:4px;min-width:150px;box-shadow:2px 2px 10px rgba(0,0,0,0.5)';

    menu.innerHTML = `
        <div class="context-item" onclick="pasteWidget()">Paste</div>
        <div class="context-item" onclick="clearSelection()">Deselect All</div>
    `;

    menu.addEventListener('click', (e) => e.stopPropagation());
    document.addEventListener('click', () => menu.remove(), { once: true });
    document.body.appendChild(menu);
}

// ============================================================================
// Context Menu Actions
// ============================================================================

function cutWidget() {
    if (EditorState.selectedWidget) {
        copiedWidget = JSON.parse(JSON.stringify(EditorState.selectedWidget));
        removeWidget(EditorState.selectedWidget);
        clearSelection();
    }
}

function copyWidget() {
    if (EditorState.selectedWidget) {
        copiedWidget = JSON.parse(JSON.stringify(EditorState.selectedWidget));
        clearSelection();
    }
}

function pasteWidget() {
    if (copiedWidget) {
        const widget = JSON.parse(JSON.stringify(copiedWidget));
        widget.id = generateWidgetId(widget.type);
        widget.x += 20;
        widget.y += 20;
        addWidget(widget);
        saveToHistory('paste');
    }
}

function deleteWidget() {
    if (EditorState.selectedWidget) {
        removeWidget(EditorState.selectedWidget);
        clearSelection();
    }
}

function bringToFrontWidget() {
    if (EditorState.selectedWidget) {
        bringToFront(EditorState.selectedWidget);
    }
}

function sendToBackWidget() {
    if (EditorState.selectedWidget) {
        sendToBack(EditorState.selectedWidget);
    }
}
