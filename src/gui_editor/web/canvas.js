/**
 * GUIKit Web Editor - Canvas Interaction Handlers
 */

// ============================================================================
// Mouse & Touch Handlers
// ============================================================================

function handleCanvasMouseDown(e) {
    if (e.button !== 0) return;
    const pos = getCanvasMousePosition(e);
    const canvasX = Math.floor(pos.x / EditorState.zoom);
    const canvasY = Math.floor(pos.y / EditorState.zoom);

    if (EditorState.selectedWidget && checkResizeHandleClick(canvasX, canvasY)) {
        startResizing(canvasX, canvasY);
        return;
    }

    const widget = findWidgetAtPosition(canvasX, canvasY);
    if (widget) {
        if (e.ctrlKey || e.metaKey) {
            toggleWidgetSelection(widget);
        } else if (!EditorState.selectedWidgets.has(widget)) {
            selectWidget(widget);
        }
        EditorState.isDragging = true;
        EditorState.dragWidget = widget;
        EditorState.dragStartX = canvasX;
        EditorState.dragStartY = canvasY;
        EditorState.dragOffsetX = canvasX - widget.x;
        EditorState.dragOffsetY = canvasY - widget.y;
    } else {
        if (!e.ctrlKey && !e.metaKey) clearSelection();
        startBoxSelection(canvasX, canvasY);
    }
    renderCanvas();
}

function handleCanvasMouseMove(e) {
    const pos = getCanvasMousePosition(e);
    const canvasX = Math.floor(pos.x / EditorState.zoom);
    const canvasY = Math.floor(pos.y / EditorState.zoom);
    updateCursorPosition(pos.x, pos.y);

    if (EditorState.isDragging && EditorState.dragWidget) {
        handleWidgetDrag(canvasX, canvasY);
    } else if (EditorState.isResizing) {
        handleWidgetResize(canvasX, canvasY);
    } else if (EditorState.isBoxSelecting) {
        updateBoxSelection(canvasX, canvasY);
    }
    checkWidgetHover(canvasX, canvasY);
    renderCanvas();
}

function handleCanvasMouseUp(e) {
    if (EditorState.isDragging) {
        EditorState.isDragging = false;
        EditorState.dragWidget = null;
        if (EditorState.snapToGrid && EditorState.selectedWidget) {
            snapToGrid(EditorState.selectedWidget);
        }
        saveToHistory('drag');
        updatePropertyInspector();
        updateHierarchy();
        updateJSONEditor();
    }
    if (EditorState.isResizing) {
        EditorState.isResizing = false;
        EditorState.resizeHandle = null;
        if (EditorState.selectedWidget) {
            EditorState.selectedWidget.width = Math.max(1, EditorState.selectedWidget.width);
            EditorState.selectedWidget.height = Math.max(1, EditorState.selectedWidget.height);
            saveToHistory('resize');
            updatePropertyInspector();
            updateJSONEditor();
        }
    }
    if (EditorState.isBoxSelecting) finishBoxSelection();
    renderCanvas();
}

function handleCanvasWheel(e) {
    e.preventDefault();
    const delta = e.deltaY > 0 ? -0.1 : 0.1;
    EditorState.zoom = Math.max(0.1, Math.min(3.0, EditorState.zoom + delta));
    updateZoomIndicator();
    renderCanvas();
}

function handleCanvasTouchStart(e) {
    if (e.touches.length > 1) return;
    const touch = e.touches[0];
    const rect = canvasElement.getBoundingClientRect();
    const canvasX = Math.floor((touch.clientX - rect.left) / EditorState.zoom);
    const canvasY = Math.floor((touch.clientY - rect.top) / EditorState.zoom);

    lastTouch = { x: canvasX, y: canvasY, time: Date.now() };
    touchTimeout = setTimeout(() => showContextMenu(canvasX, canvasY), 2000);

    const widget = findWidgetAtPosition(canvasX, canvasY);
    if (widget && !EditorState.selectedWidgets.has(widget)) selectWidget(widget);
    EditorState.isDragging = true;
    if (widget) {
        EditorState.dragWidget = widget;
        EditorState.dragOffsetX = canvasX - widget.x;
        EditorState.dragOffsetY = canvasY - widget.y;
    }
    renderCanvas();
    e.preventDefault();
}

function handleCanvasTouchMove(e) {
    if (e.touches.length > 1 || !EditorState.isDragging) return;
    if (touchTimeout) { clearTimeout(touchTimeout); touchTimeout = null; }
    const touch = e.touches[0];
    const rect = canvasElement.getBoundingClientRect();
    const canvasX = Math.floor((touch.clientX - rect.left) / EditorState.zoom);
    const canvasY = Math.floor((touch.clientY - rect.top) / EditorState.zoom);
    if (EditorState.dragWidget) handleWidgetDrag(canvasX, canvasY);
    renderCanvas();
    e.preventDefault();
}

function handleCanvasTouchEnd(e) {
    if (touchTimeout) { clearTimeout(touchTimeout); touchTimeout = null; }
    if (EditorState.isDragging) {
        EditorState.isDragging = false;
        EditorState.dragWidget = null;
        if (EditorState.snapToGrid && EditorState.selectedWidget) snapToGrid(EditorState.selectedWidget);
        saveToHistory('drag');
        updatePropertyInspector();
        updateHierarchy();
        updateJSONEditor();
    }
    renderCanvas();
    e.preventDefault();
}

function showContextMenu(x, y) {
    logToConsole(`Context menu at (${x}, ${y})`);
    const widget = findWidgetAtPosition(x, y);
    if (widget) {
        selectWidget(widget);
        showWidgetContextMenu(widget, x, y);
    } else {
        showCanvasContextMenu(x, y);
    }
}

// ============================================================================
// Drag, Resize, Box Selection
// ============================================================================

function handleWidgetDrag(canvasX, canvasY) {
    if (!EditorState.dragWidget) return;
    const widget = EditorState.dragWidget;
    let newX = canvasX - EditorState.dragOffsetX;
    let newY = canvasY - EditorState.dragOffsetY;
    if (EditorState.snapToGrid) {
        newX = Math.floor(newX / EditorState.gridSize) * EditorState.gridSize;
        newY = Math.floor(newY / EditorState.gridSize) * EditorState.gridSize;
    }
    widget.x = newX;
    widget.y = newY;
    if (EditorState.selectedWidgets.size > 1) {
        const dx = widget.x - EditorState.dragStartX + EditorState.dragOffsetX;
        const dy = widget.y - EditorState.dragStartY + EditorState.dragOffsetY;
        for (const w of EditorState.selectedWidgets) {
            if (w !== widget) { w.x += dx; w.y += dy; }
        }
    }
    EditorState.dragStartX = canvasX;
    EditorState.dragStartY = canvasY;
}

function startResizing(canvasX, canvasY) {
    if (!EditorState.selectedWidget) return;
    const widget = EditorState.selectedWidget;
    const handles = getResizeHandles(widget);
    for (const handle of handles) {
        if (Math.abs(canvasX - handle.x) <= 6 && Math.abs(canvasY - handle.y) <= 6) {
            EditorState.resizeHandle = handle.name;
            EditorState.resizeStartX = canvasX;
            EditorState.resizeStartY = canvasY;
            EditorState.resizeStartWidth = widget.width;
            EditorState.resizeStartHeight = widget.height;
            EditorState.isResizing = true;
            return;
        }
    }
}

function checkResizeHandleClick(x, y) {
    if (!EditorState.selectedWidget) return false;
    startResizing(x, y);
    return EditorState.isResizing;
}

function getResizeHandles(widget) {
    const x = widget.x + widget.width;
    const y = widget.y + widget.height;
    return [
        { name: 'top-left', x: widget.x, y: widget.y },
        { name: 'top-right', x: x, y: widget.y },
        { name: 'bottom-left', x: widget.x, y: y },
        { name: 'bottom-right', x: x, y: y },
        { name: 'top', x: widget.x + widget.width / 2, y: widget.y },
        { name: 'right', x: x, y: widget.y + widget.height / 2 },
        { name: 'bottom', x: widget.x + widget.width / 2, y: y },
        { name: 'left', x: widget.x, y: widget.y + widget.height / 2 }
    ];
}

function handleWidgetResize(canvasX, canvasY) {
    if (!EditorState.selectedWidget || !EditorState.resizeHandle) return;
    const widget = EditorState.selectedWidget;
    const dx = canvasX - EditorState.resizeStartX;
    const dy = canvasY - EditorState.resizeStartY;
    let newWidth = EditorState.resizeStartWidth;
    let newHeight = EditorState.resizeStartHeight;
    let newX = widget.x;
    let newY = widget.y;

    switch (EditorState.resizeHandle) {
        case 'top-left':
            newWidth = EditorState.resizeStartWidth - dx;
            newHeight = EditorState.resizeStartHeight - dy;
            newX = widget.x + dx;
            newY = widget.y + dy;
            break;
        case 'top-right':
            newWidth = EditorState.resizeStartWidth + dx;
            newHeight = EditorState.resizeStartHeight - dy;
            newY = widget.y + dy;
            break;
        case 'bottom-left':
            newWidth = EditorState.resizeStartWidth - dx;
            newHeight = EditorState.resizeStartHeight + dy;
            newX = widget.x + dx;
            break;
        case 'bottom-right':
            newWidth = EditorState.resizeStartWidth + dx;
            newHeight = EditorState.resizeStartHeight + dy;
            break;
        case 'top':
            newHeight = EditorState.resizeStartHeight - dy;
            newY = widget.y + dy;
            break;
        case 'right':
            newWidth = EditorState.resizeStartWidth + dx;
            break;
        case 'bottom':
            newHeight = EditorState.resizeStartHeight + dy;
            break;
        case 'left':
            newWidth = EditorState.resizeStartWidth - dx;
            newX = widget.x + dx;
            break;
    }

    const grid = EditorState.gridSize;
    widget.x = newX;
    widget.y = newY;
    widget.width = Math.max(1, EditorState.snapToGrid ? Math.floor(newWidth / grid) * grid : newWidth);
    widget.height = Math.max(1, EditorState.snapToGrid ? Math.floor(newHeight / grid) * grid : newHeight);
}

function snapToGrid(widget) {
    const grid = EditorState.gridSize;
    widget.x = Math.floor(widget.x / grid) * grid;
    widget.y = Math.floor(widget.y / grid) * grid;
    widget.width = Math.floor(widget.width / grid) * grid;
    widget.height = Math.floor(widget.height / grid) * grid;
}

function startBoxSelection(x, y) {
    EditorState.isBoxSelecting = true;
    boxSelectStartX = x;
    boxSelectStartY = y;
}

function updateBoxSelection(x, y) {
    EditorState.selectedWidgets.clear();
    const minX = Math.min(boxSelectStartX, x);
    const maxX = Math.max(boxSelectStartX, x);
    const minY = Math.min(boxSelectStartY, y);
    const maxY = Math.max(boxSelectStartY, y);

    for (const widget of EditorState.gui.widgets) {
        if (isWidgetInBox(widget, minX, minY, maxX, maxY)) {
            EditorState.selectedWidgets.add(widget);
        }
        if (widget.type === 'view' && widget.children) {
            for (const child of widget.children) {
                if (isWidgetInBox(child, minX - widget.x, minY - widget.y,
                    maxX - widget.x, maxY - widget.y)) {
                    EditorState.selectedWidgets.add(child);
                }
            }
        }
    }
    if (EditorState.selectedWidgets.size > 0) {
        EditorState.selectedWidget = [...EditorState.selectedWidgets][0];
    }
}

function isWidgetInBox(widget, minX, minY, maxX, maxY) {
    return widget.x + widget.width > minX &&
           widget.x < maxX &&
           widget.y + widget.height > minY &&
           widget.y < maxY;
}

function finishBoxSelection() {
    EditorState.isBoxSelecting = false;
    updatePropertyInspector();
    updateStatusBar();
}

function checkWidgetHover(x, y) {
    // Hover detection logic can be added here
}

// ============================================================================
// Utility Functions
// ============================================================================

function getCanvasMousePosition(e) {
    const rect = canvasElement.getBoundingClientRect();
    return { x: e.clientX - rect.left, y: e.clientY - rect.top };
}

function updateCursorPosition(x, y) {
    const cursorEl = document.getElementById('status-cursor');
    if (cursorEl) {
        const canvasX = Math.floor(x / EditorState.zoom);
        const canvasY = Math.floor(y / EditorState.zoom);
        cursorEl.textContent = ` | Cursor: (${canvasX}, ${canvasY})`;
    }
}

function updateZoomIndicator() {
    const indicator = document.querySelector('.zoom-indicator');
    if (indicator) {
        indicator.textContent = `${Math.round(EditorState.zoom * 100)}%`;
    }
}
