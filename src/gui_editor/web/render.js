/**
 * GUIKit Web Editor - Canvas Rendering
 */

// ============================================================================
// Main Render Function
// ============================================================================

function renderCanvas() {
    if (!canvasCtx) return;

    const renderCanvas = canvasElement.querySelector('#render-canvas');
    if (!renderCanvas) return;

    // Clear canvas
    canvasCtx.clearRect(0, 0, renderCanvas.width, renderCanvas.height);

    // Draw background
    canvasCtx.fillStyle = EditorState.gui.background || '#000000';
    canvasCtx.fillRect(0, 0, renderCanvas.width, renderCanvas.height);

    // Draw grid if enabled
    if (EditorState.gridEnabled) {
        drawGrid();
    }

    // Draw widgets
    const scale = EditorState.zoom;
    canvasCtx.save();
    canvasCtx.scale(scale, scale);

    drawWidgets(EditorState.gui.widgets);

    canvasCtx.restore();

    // Draw selection rectangle (box selection)
    if (EditorState.isBoxSelecting) {
        drawBoxSelection();
    }

    // Draw resize handles for selected widget
    if (EditorState.selectedWidget) {
        drawResizeHandles();
    }
}

// ============================================================================
// Grid Drawing
// ============================================================================

function drawGrid() {
    if (!canvasCtx) return;

    const renderCanvas = canvasElement.querySelector('#render-canvas');
    const width = renderCanvas.width / EditorState.zoom;
    const height = renderCanvas.height / EditorState.zoom;

    canvasCtx.strokeStyle = '#333333';
    canvasCtx.lineWidth = 1;

    // Vertical lines
    for (let x = 0; x <= width; x += EditorState.gridSize) {
        canvasCtx.beginPath();
        canvasCtx.moveTo(x, 0);
        canvasCtx.lineTo(x, height);
        canvasCtx.stroke();
    }

    // Horizontal lines
    for (let y = 0; y <= height; y += EditorState.gridSize) {
        canvasCtx.beginPath();
        canvasCtx.moveTo(0, y);
        canvasCtx.lineTo(width, y);
        canvasCtx.stroke();
    }
}

// ============================================================================
// Widget Drawing
// ============================================================================

function drawWidgets(widgets, parentX = 0, parentY = 0) {
    if (!canvasCtx) return;

    for (const widget of widgets) {
        const x = parentX + (widget.x || 0);
        const y = parentY + (widget.y || 0);

        drawWidget(widget, x, y);

        // Draw children for view widgets with clipping
        if (widget.type === 'view' && widget.children) {
            canvasCtx.save();
            canvasCtx.beginPath();
            canvasCtx.rect(x, y, widget.width, widget.height);
            canvasCtx.clip();

            drawWidgets(widget.children, x, y);

            canvasCtx.restore();
        }
    }
}

function drawWidget(widget, x, y) {
    if (!canvasCtx) return;

    const style = widget.style || {};
    const selected = EditorState.selectedWidgets.has(widget);

    // Draw background
    const bgColor = style.bg_color || (widget.type === 'button' ? '#4CAF50' : '#404040');
    canvasCtx.fillStyle = bgColor;
    canvasCtx.fillRect(x, y, widget.width, widget.height);

    // Draw border
    const borderWidth = style.border_width || 1;
    const borderColor = style.border_color || '#808080';

    if (borderWidth > 0) {
        canvasCtx.strokeStyle = borderColor;
        canvasCtx.lineWidth = borderWidth;
        canvasCtx.strokeRect(x, y, widget.width, widget.height);
    }

    // Draw selection highlight
    if (selected) {
        canvasCtx.strokeStyle = '#FF0000';
        canvasCtx.lineWidth = 2;
        canvasCtx.strokeRect(x - 1, y - 1, widget.width + 2, widget.height + 2);
    }

    // Draw widget-specific content
    drawWidgetContent(widget, x, y);
}

function drawWidgetContent(widget, x, y) {
    if (!canvasCtx) return;

    const style = widget.style || {};
    const fgColor = style.fg_color || '#FFFFFF';
    const fontSize = style.font_size || 12;
    const font = style.font || 'default';
    const textAlign = style.text_align || 'left';

    canvasCtx.fillStyle = fgColor;
    canvasCtx.font = `${fontSize}px ${getCanvasFont(font)}`;
    canvasCtx.textAlign = textAlign;

    switch (widget.type) {
        case 'label':
        case 'button':
            canvasCtx.fillText(widget.text || widget.type, x + widget.width / 2, y + widget.height / 2 + fontSize / 3);
            break;

        case 'text_input':
            canvasCtx.fillStyle = '#000000';
            canvasCtx.fillRect(x + 2, y + 2, widget.width - 4, widget.height - 4);
            canvasCtx.fillStyle = fgColor;
            canvasCtx.fillText(widget.text || widget.placeholder || 'Text', x + 5, y + fontSize + 5);
            break;

        case 'text_editor':
            canvasCtx.fillStyle = '#000000';
            canvasCtx.fillRect(x + 1, y + 1, widget.width - 2, widget.height - 2);
            canvasCtx.fillStyle = fgColor;
            canvasCtx.fillText('Text Editor', x + 5, y + fontSize + 5);

            // Draw line numbers if enabled
            if (widget.show_line_numbers) {
                canvasCtx.fillStyle = '#808080';
                canvasCtx.textAlign = 'right';
                for (let i = 1; i <= Math.floor(widget.height / fontSize); i++) {
                    canvasCtx.fillText(i.toString(), x + widget.width - 5, y + i * fontSize);
                }
                canvasCtx.textAlign = textAlign;
            }
            break;

        case 'slider':
            const sx = x + widget.width / 2;
            const sy = y + widget.height / 2;
            const sw = widget.width - 20;
            const minVal = widget.min_value || 0;
            const maxVal = widget.max_value || 100;
            const sv = ((widget.value || 50) - minVal) / (maxVal - minVal) * sw;

            // Track
            canvasCtx.strokeStyle = '#808080';
            canvasCtx.lineWidth = 4;
            canvasCtx.beginPath();
            canvasCtx.moveTo(x + 10, sy);
            canvasCtx.lineTo(x + widget.width - 10, sy);
            canvasCtx.stroke();

            // Thumb
            canvasCtx.fillStyle = '#FFFFFF';
            canvasCtx.beginPath();
            canvasCtx.arc(x + 10 + sv, sy, 8, 0, Math.PI * 2);
            canvasCtx.fill();
            break;

        case 'progress_bar':
            const progress = (widget.value || 0) / (widget.max || 100);
            const barWidth = widget.width * progress;

            canvasCtx.fillStyle = '#4CAF50';
            canvasCtx.fillRect(x + 1, y + 1, barWidth, widget.height - 2);

            // Text
            canvasCtx.fillStyle = fgColor;
            if (widget.show_text) {
                canvasCtx.fillText(`${Math.round(progress * 100)}%`, x + widget.width / 2, y + widget.height / 2 + fontSize / 3);
            }
            break;

        case 'checkbox':
            // Box
            canvasCtx.strokeStyle = '#FFFFFF';
            canvasCtx.lineWidth = 2;
            canvasCtx.strokeRect(x + 2, y + 2, 16, 16);

            // Check mark
            if (widget.checked) {
                canvasCtx.beginPath();
                canvasCtx.moveTo(x + 5, y + 9);
                canvasCtx.lineTo(x + 10, y + 14);
                canvasCtx.lineTo(x + 15, y + 6);
                canvasCtx.stroke();
            }

            // Text
            canvasCtx.fillText(widget.text || 'Checkbox', x + 22, y + 14);
            break;

        case 'radio_button':
            // Circle
            canvasCtx.strokeStyle = '#FFFFFF';
            canvasCtx.lineWidth = 2;
            canvasCtx.beginPath();
            canvasCtx.arc(x + 10, y + 10, 8, 0, Math.PI * 2);
            canvasCtx.stroke();

            // Fill if checked
            if (widget.checked) {
                canvasCtx.fillStyle = '#FFFFFF';
                canvasCtx.beginPath();
                canvasCtx.arc(x + 10, y + 10, 5, 0, Math.PI * 2);
                canvasCtx.fill();
            }

            // Text
            canvasCtx.fillText(widget.text || 'Radio', x + 22, y + 12);
            break;

        case 'dropdown':
            canvasCtx.fillStyle = '#FFFFFF';
            canvasCtx.fillRect(x + 2, y + 2, widget.width - 4, widget.height - 4);
            canvasCtx.fillStyle = '#000000';
            const selectedText = (widget.items && widget.items[widget.selected_index || 0]) || 'Select...';
            canvasCtx.fillText(selectedText, x + 5, y + fontSize + 5);
            break;

        case 'image':
            canvasCtx.fillStyle = '#808080';
            canvasCtx.fillRect(x + 1, y + 1, widget.width - 2, widget.height - 2);
            canvasCtx.fillStyle = '#FFFFFF';
            canvasCtx.textAlign = 'center';
            canvasCtx.fillText('Image', x + widget.width / 2, y + widget.height / 2);
            canvasCtx.textAlign = textAlign;
            break;

        case 'canvas':
            canvasCtx.fillStyle = '#202020';
            canvasCtx.fillRect(x + 1, y + 1, widget.width - 2, widget.height - 2);
            canvasCtx.fillStyle = '#FFFFFF';
            canvasCtx.textAlign = 'center';
            canvasCtx.fillText('Canvas', x + widget.width / 2, y + widget.height / 2);
            canvasCtx.textAlign = textAlign;
            break;

        case 'view':
            canvasCtx.fillStyle = '#303030';
            canvasCtx.globalAlpha = 0.3;
            canvasCtx.fillRect(x + 1, y + 1, widget.width - 2, widget.height - 2);
            canvasCtx.globalAlpha = 1;
            break;
    }
}

// ============================================================================
// Resize Handles and Box Selection
// ============================================================================

function drawResizeHandles() {
    if (!canvasCtx || !EditorState.selectedWidget) return;

    const widget = EditorState.selectedWidget;
    const x = widget.x + widget.width;
    const y = widget.y + widget.height;

    canvasCtx.fillStyle = '#FF0000';

    // Draw 8 resize handles
    const handles = [
        { x: widget.x, y: widget.y },
        { x: x, y: widget.y },
        { x: widget.x, y: y },
        { x: x, y: y },
        { x: widget.x + widget.width / 2, y: widget.y },
        { x: x, y: widget.y + widget.height / 2 },
        { x: widget.x + widget.width / 2, y: y },
        { x: widget.x, y: widget.y + widget.height / 2 }
    ];

    for (const handle of handles) {
        canvasCtx.fillRect(handle.x - 3, handle.y - 3, 6, 6);
    }
}

function drawBoxSelection() {
    if (!canvasCtx) return;

    const minX = Math.min(boxSelectStartX, EditorState.dragStartX);
    const maxX = Math.max(boxSelectStartX, EditorState.dragStartX);
    const minY = Math.min(boxSelectStartY, EditorState.dragStartY);
    const maxY = Math.max(boxSelectStartY, EditorState.dragStartY);

    canvasCtx.fillStyle = 'rgba(0, 100, 255, 0.2)';
    canvasCtx.strokeStyle = '#0064FF';
    canvasCtx.lineWidth = 1;

    canvasCtx.fillRect(minX, minY, maxX - minX, maxY - minY);
    canvasCtx.strokeRect(minX, minY, maxX - minX, maxY - minY);
}
