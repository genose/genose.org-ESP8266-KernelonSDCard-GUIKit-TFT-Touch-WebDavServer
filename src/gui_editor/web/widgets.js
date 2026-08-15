/**
 * GUIKit Web Editor - Widget Operations
 */

// ============================================================================
// Widget Creation and Management
// ============================================================================

function createNewWidget(type, x, y) {
    const defaults = {
        view: { type: 'view', x: x, y: y, width: 100, height: 100, children: [] },
        button: { type: 'button', x: x, y: y, width: 80, height: 30, text: 'Button', style: { bg_color: '#4CAF50', fg_color: '#FFFFFF' } },
        label: { type: 'label', x: x, y: y, width: 80, height: 20, text: 'Label', style: { fg_color: '#FFFFFF' } },
        text_input: { type: 'text_input', x: x, y: y, width: 150, height: 24, text: '', placeholder: 'Enter text...' },
        text_editor: { type: 'text_editor', x: x, y: y, width: 200, height: 150, text: '', word_wrap: true, scrollable: { x: false, y: true } },
        slider: { type: 'slider', x: x, y: y, width: 150, height: 20, min_value: 0, max_value: 100, value: 50 },
        progress_bar: { type: 'progress_bar', x: x, y: y, width: 150, height: 20, value: 50, max: 100 },
        image: { type: 'image', x: x, y: y, width: 100, height: 100, src: '/images/default.png' },
        canvas: { type: 'canvas', x: x, y: y, width: 150, height: 100 },
        checkbox: { type: 'checkbox', x: x, y: y, width: 150, height: 20, text: 'Checkbox' },
        radio_button: { type: 'radio_button', x: x, y: y, width: 150, height: 20, text: 'Radio', group: 'radio_group_1' },
        dropdown: { type: 'dropdown', x: x, y: y, width: 150, height: 30, items: ['Option 1', 'Option 2', 'Option 3'], selected_index: 0 }
    };
    const definition = defaults[type] || defaults.view;
    const widget = { id: generateWidgetId(type), ...definition };
    if (EditorState.snapToGrid) {
        widget.x = Math.floor(x / EditorState.gridSize) * EditorState.gridSize;
        widget.y = Math.floor(y / EditorState.gridSize) * EditorState.gridSize;
    }
    return widget;
}

function generateWidgetId(type) {
    const count = EditorState.gui.widgets.filter(w => w.type === type).length + 1;
    return `${type}_${count}`;
}

function addWidget(widget, parent = null) {
    if (parent) {
        if (!parent.children) parent.children = [];
        parent.children.push(widget);
    } else {
        EditorState.gui.widgets.push(widget);
    }
    selectWidget(widget);
    saveToHistory('add');
    updateHierarchy();
    updateJSONEditor();
    return widget;
}

function removeWidget(widget, parent = null) {
    const list = parent ? parent.children : EditorState.gui.widgets;
    const index = list.findIndex(w => w.id === widget.id);
    if (index !== -1) {
        list.splice(index, 1);
        EditorState.selectedWidgets.delete(widget);
        if (EditorState.selectedWidget?.id === widget.id) EditorState.selectedWidget = null;
        saveToHistory('remove');
        updatePropertyInspector();
        updateHierarchy();
        updateJSONEditor();
    }
}

function findWidgetAtPosition(x, y) {
    const widgets = [...EditorState.gui.widgets].reverse();
    for (const widget of widgets) {
        if (isPointInWidget(widget, x, y)) return widget;
        if (widget.type === 'view' && widget.children) {
            for (const child of [...widget.children].reverse()) {
                if (isPointInWidget(child, x - widget.x, y - widget.y)) return child;
            }
        }
    }
    return null;
}

function isPointInWidget(widget, x, y) {
    return x >= widget.x && x < widget.x + widget.width &&
           y >= widget.y && y < widget.y + widget.height;
}

function findWidgetById(widgetId, widgets = EditorState.gui.widgets) {
    for (const widget of widgets) {
        if (widget.id === widgetId) return widget;
        if (widget.type === 'view' && widget.children) {
            const found = findWidgetById(widgetId, widget.children);
            if (found) return found;
        }
    }
    return null;
}

// ============================================================================
// Selection
// ============================================================================

function selectWidget(widget) {
    if (EditorState.multiSelect && (EditorState.selectedWidgets.size > 0 || widget !== EditorState.selectedWidget)) {
        EditorState.selectedWidgets.add(widget);
        EditorState.selectedWidget = widget;
    } else {
        EditorState.selectedWidgets.clear();
        EditorState.selectedWidgets.add(widget);
        EditorState.selectedWidget = widget;
    }
    updatePropertyInspector();
    updateStatusBar();
    renderCanvas();
}

function clearSelection() {
    EditorState.selectedWidgets.clear();
    EditorState.selectedWidget = null;
    updatePropertyInspector();
    updateStatusBar();
    renderCanvas();
}

function toggleWidgetSelection(widget) {
    if (EditorState.selectedWidgets.has(widget)) {
        EditorState.selectedWidgets.delete(widget);
        if (EditorState.selectedWidget?.id === widget.id) {
            EditorState.selectedWidget = EditorState.selectedWidgets.size > 0 ?
                [...EditorState.selectedWidgets][0] : null;
        }
    } else {
        EditorState.selectedWidgets.add(widget);
        EditorState.selectedWidget = widget;
    }
    updatePropertyInspector();
    updateStatusBar();
    renderCanvas();
}

function getParentWidgetList(widget) {
    for (const w of EditorState.gui.widgets) {
        if (w.type === 'view' && w.children?.some(c => c.id === widget.id)) {
            return w.children;
        }
    }
    return EditorState.gui.widgets;
}

function bringToFront(widget) {
    const list = getParentWidgetList(widget);
    if (!list) return;
    const index = list.findIndex(w => w.id === widget.id);
    if (index !== -1 && index < list.length - 1) {
        list.splice(index, 1);
        list.push(widget);
        saveToHistory('z-order');
        renderCanvas();
    }
}

function sendToBack(widget) {
    const list = getParentWidgetList(widget);
    if (!list) return;
    const index = list.findIndex(w => w.id === widget.id);
    if (index !== -1 && index > 0) {
        list.splice(index, 1);
        list.unshift(widget);
        saveToHistory('z-order');
        renderCanvas();
    }
}

// ============================================================================
// Property Inspector
// ============================================================================

function updatePropertyInspector() {
    const container = document.getElementById('properties-container');
    const title = document.getElementById('inspector-title');
    if (!container || !title) return;

    if (EditorState.selectedWidgets.size === 0) {
        container.innerHTML = '<div class="no-selection"><p>Select a widget to edit properties</p></div>';
        title.textContent = 'No Selection';
        return;
    }

    if (EditorState.selectedWidgets.size > 1) {
        title.textContent = `${EditorState.selectedWidgets.size} Widgets Selected`;
        container.innerHTML = '<div class="multi-selection"><p>Edit multiple widgets simultaneously</p></div>';
        return;
    }

    const widget = EditorState.selectedWidget;
    title.textContent = `${widget.type} - ${widget.id}`;

    let html = '<div class="property-section">';
    html += '<div class="property-group"><label>ID</label>';
    html += `<input type="text" onchange="updateWidgetProperty('${widget.id}', 'id', this.value)" value="${widget.id}"></div>`;

    // Position & Size
    html += '<h4>Position & Size</h4>';
    html += `<div class="property-row">`;
    html += `<div class="property-item"><label>X</label><input type="number" value="${widget.x}" onchange="updateWidgetProperty('${widget.id}', 'x', parseInt(this.value) || 0)"></div>`;
    html += `<div class="property-item"><label>Y</label><input type="number" value="${widget.y}" onchange="updateWidgetProperty('${widget.id}', 'y', parseInt(this.value) || 0)"></div>`;
    html += `</div><div class="property-row">`;
    html += `<div class="property-item"><label>Width</label><input type="number" value="${widget.width}" onchange="updateWidgetProperty('${widget.id}', 'width', parseInt(this.value) || 1)"></div>`;
    html += `<div class="property-item"><label>Height</label><input type="number" value="${widget.height}" onchange="updateWidgetProperty('${widget.id}', 'height', parseInt(this.value) || 1)"></div>`;
    html += `</div>`;

    // Visibility & Enabled
    html += `<div class="property-item checkbox"><label><input type="checkbox" ${widget.visible !== false ? 'checked' : ''} onchange="updateWidgetProperty('${widget.id}', 'visible', this.checked)"> Visible</label></div>`;
    html += `<div class="property-item checkbox"><label><input type="checkbox" ${widget.enabled !== false ? 'checked' : ''} onchange="updateWidgetProperty('${widget.id}', 'enabled', this.checked)"> Enabled</label></div>`;

    // Type-specific properties
    html += getTypeSpecificProperties(widget);

    // Style
    html += getStyleProperties(widget);

    // Scrollable
    if (widget.scrollable) {
        html += '<h4>Scrollable</h4><div class="property-row">';
        html += `<div class="property-item checkbox"><label><input type="checkbox" ${widget.scrollable.x ? 'checked' : ''} onchange="updateScrollable('${widget.id}', 'x', this.checked)"> X</label></div>`;
        html += `<div class="property-item checkbox"><label><input type="checkbox" ${widget.scrollable.y ? 'checked' : ''} onchange="updateScrollable('${widget.id}', 'y', this.checked)"> Y</label></div>`;
        html += `</div>`;
    }

    // Children count for view
    if (widget.type === 'view' && widget.children) {
        html += `<h4>Children</h4><div class="property-item"><label>Count: ${widget.children.length}</label></div>`;
    }

    html += '</div>';
    container.innerHTML = html;
}

function getTypeSpecificProperties(widget) {
    let html = '';
    switch (widget.type) {
        case 'button':
            html += '<h4>Button</h4>';
            html += `<div class="property-item full"><label>Text</label><input type="text" value="${widget.text || ''}" onchange="updateWidgetProperty('${widget.id}', 'text', this.value)"></div>`;
            html += `<div class="property-item full"><label>On Click</label><input type="text" value="${widget.on_click || ''}" onchange="updateWidgetProperty('${widget.id}', 'on_click', this.value)"></div>`;
            break;
        case 'label':
            html += '<h4>Label</h4>';
            html += `<div class="property-item full"><label>Text</label><textarea rows="3" onchange="updateWidgetProperty('${widget.id}', 'text', this.value)">${widget.text || ''}</textarea></div>`;
            break;
        case 'text_input':
            html += '<h4>Text Input</h4>';
            html += `<div class="property-item full"><label>Text</label><input type="text" value="${widget.text || ''}" onchange="updateWidgetProperty('${widget.id}', 'text', this.value)"></div>`;
            html += `<div class="property-item full"><label>Placeholder</label><input type="text" value="${widget.placeholder || ''}" onchange="updateWidgetProperty('${widget.id}', 'placeholder', this.value)"></div>`;
            html += `<div class="property-item"><label>Max Length</label><input type="number" value="${widget.max_length || 256}" onchange="updateWidgetProperty('${widget.id}', 'max_length', parseInt(this.value) || 256)"></div>`;
            html += `<div class="property-item checkbox"><label><input type="checkbox" ${widget.password ? 'checked' : ''} onchange="updateWidgetProperty('${widget.id}', 'password', this.checked)"> Password</label></div>`;
            break;
        case 'text_editor':
            html += '<h4>Text Editor</h4>';
            html += `<div class="property-item checkbox"><label><input type="checkbox" ${widget.read_only ? 'checked' : ''} onchange="updateWidgetProperty('${widget.id}', 'read_only', this.checked)"> Read Only</label></div>`;
            html += `<div class="property-item checkbox"><label><input type="checkbox" ${widget.show_line_numbers ? 'checked' : ''} onchange="updateWidgetProperty('${widget.id}', 'show_line_numbers', this.checked)"> Line Numbers</label></div>`;
            html += `<div class="property-item checkbox"><label><input type="checkbox" ${widget.word_wrap !== false ? 'checked' : ''} onchange="updateWidgetProperty('${widget.id}', 'word_wrap', this.checked)"> Word Wrap</label></div>`;
            break;
        case 'slider':
            html += '<h4>Slider</h4><div class="property-row">';
            html += `<div class="property-item"><label>Min</label><input type="number" value="${widget.min_value || 0}" onchange="updateWidgetProperty('${widget.id}', 'min_value', parseInt(this.value) || 0)"></div>`;
            html += `<div class="property-item"><label>Max</label><input type="number" value="${widget.max_value || 100}" onchange="updateWidgetProperty('${widget.id}', 'max_value', parseInt(this.value) || 100)"></div>`;
            html += `</div><div class="property-item"><label>Value</label><input type="number" value="${widget.value || 50}" onchange="updateWidgetProperty('${widget.id}', 'value', parseInt(this.value) || 50)"></div>`;
            html += `<div class="property-item checkbox"><label><input type="checkbox" ${widget.vertical ? 'checked' : ''} onchange="updateWidgetProperty('${widget.id}', 'vertical', this.checked)"> Vertical</label></div>`;
            break;
        case 'progress_bar':
            html += '<h4>Progress Bar</h4><div class="property-row">';
            html += `<div class="property-item"><label>Value</label><input type="number" value="${widget.value || 0}" onchange="updateWidgetProperty('${widget.id}', 'value', parseInt(this.value) || 0)"></div>`;
            html += `<div class="property-item"><label>Max</label><input type="number" value="${widget.max || 100}" onchange="updateWidgetProperty('${widget.id}', 'max', parseInt(this.value) || 100)"></div>`;
            html += `</div><div class="property-item"><label>Direction</label><select onchange="updateWidgetProperty('${widget.id}', 'direction', this.value)"><option value="horizontal" ${widget.direction === 'horizontal' ? 'selected' : ''}>Horizontal</option><option value="vertical" ${widget.direction === 'vertical' ? 'selected' : ''}>Vertical</option></select></div>`;
            break;
        case 'image':
            html += '<h4>Image</h4>';
            html += `<div class="property-item full"><label>Source</label><input type="text" value="${widget.src || ''}" onchange="updateWidgetProperty('${widget.id}', 'src', this.value)"></div>`;
            html += `<div class="property-item"><label>Scale</label><select onchange="updateWidgetProperty('${widget.id}', 'scale', this.value)"><option value="stretch" ${widget.scale === 'stretch' ? 'selected' : ''}>Stretch</option><option value="fit" ${widget.scale === 'fit' ? 'selected' : ''}>Fit</option><option value="center" ${widget.scale === 'center' ? 'selected' : ''}>Center</option><option value="tile" ${widget.scale === 'tile' ? 'selected' : ''}>Tile</option></select></div>`;
            break;
        case 'canvas':
            html += '<h4>Canvas</h4>';
            html += `<div class="property-item full"><label>On Draw</label><input type="text" value="${widget.on_draw || ''}" onchange="updateWidgetProperty('${widget.id}', 'on_draw', this.value)"></div>`;
            break;
        case 'checkbox':
            html += '<h4>Checkbox</h4>';
            html += `<div class="property-item full"><label>Text</label><input type="text" value="${widget.text || ''}" onchange="updateWidgetProperty('${widget.id}', 'text', this.value)"></div>`;
            html += `<div class="property-item checkbox"><label><input type="checkbox" ${widget.checked ? 'checked' : ''} onchange="updateWidgetProperty('${widget.id}', 'checked', this.checked)"> Checked</label></div>`;
            break;
        case 'radio_button':
            html += '<h4>Radio Button</h4>';
            html += `<div class="property-item full"><label>Text</label><input type="text" value="${widget.text || ''}" onchange="updateWidgetProperty('${widget.id}', 'text', this.value)"></div>`;
            html += `<div class="property-item full"><label>Group</label><input type="text" value="${widget.group || ''}" onchange="updateWidgetProperty('${widget.id}', 'group', this.value)"></div>`;
            html += `<div class="property-item checkbox"><label><input type="checkbox" ${widget.checked ? 'checked' : ''} onchange="updateWidgetProperty('${widget.id}', 'checked', this.checked)"> Checked</label></div>`;
            break;
        case 'dropdown':
            html += '<h4>Dropdown</h4>';
            html += `<div class="property-item full"><label>Items (comma)</label><input type="text" value="${(widget.items || []).join(',')}" onchange="updateDropdownItems('${widget.id}', this.value)"></div>`;
            html += `<div class="property-item"><label>Selected</label><input type="number" value="${widget.selected_index || 0}" onchange="updateWidgetProperty('${widget.id}', 'selected_index', parseInt(this.value) || 0)"></div>`;
            break;
    }
    return html;
}

function getStyleProperties(widget) {
    const style = widget.style || {};
    let html = '<h4>Style</h4><div class="property-row">';
    html += `<div class="property-item"><label>BG</label><input type="color" value="${style.bg_color || '#000000'}" onchange="updateWidgetStyle('${widget.id}', 'bg_color', this.value)"></div>`;
    html += `<div class="property-item"><label>FG</label><input type="color" value="${style.fg_color || '#FFFFFF'}" onchange="updateWidgetStyle('${widget.id}', 'fg_color', this.value)"></div>`;
    html += `</div><div class="property-row">`;
    html += `<div class="property-item"><label>Border</label><input type="color" value="${style.border_color || '#808080'}" onchange="updateWidgetStyle('${widget.id}', 'border_color', this.value)"></div>`;
    html += `<div class="property-item"><label>Border W</label><input type="number" value="${style.border_width || 1}" min="0" max="10" onchange="updateWidgetStyle('${widget.id}', 'border_width', parseInt(this.value) || 1)"></div>`;
    html += `</div><div class="property-row">`;
    html += `<div class="property-item"><label>Font Size</label><input type="number" value="${style.font_size || 12}" min="6" max="32" onchange="updateWidgetStyle('${widget.id}', 'font_size', parseInt(this.value) || 12)"></div>`;
    html += `<div class="property-item"><label>Font</label><select onchange="updateWidgetStyle('${widget.id}', 'font', this.value)"><option value="default" ${style.font === 'default' ? 'selected' : ''}>Default</option><option value="small">Small</option><option value="medium">Medium</option><option value="large">Large</option><option value="monospace">Monospace</option></select></div>`;
    html += `</div><div class="property-item"><label>Align</label><select onchange="updateWidgetStyle('${widget.id}', 'text_align', this.value)"><option value="left" ${style.text_align === 'left' ? 'selected' : ''}>Left</option><option value="center" ${style.text_align === 'center' ? 'selected' : ''}>Center</option><option value="right">Right</option></select></div>`;
    return html;
}

// ============================================================================
// Property Update Handlers
// ============================================================================

function updateWidgetProperty(widgetId, property, value) {
    const widget = findWidgetById(widgetId);
    if (!widget) return;
    widget[property] = value;
    saveToHistory('property');
    updateJSONEditor();
    renderCanvas();
}

function updateWidgetStyle(widgetId, property, value) {
    const widget = findWidgetById(widgetId);
    if (!widget) return;
    if (!widget.style) widget.style = {};
    widget.style[property] = value;
    saveToHistory('style');
    updateJSONEditor();
    renderCanvas();
}

function updateScrollable(widgetId, axis, value) {
    const widget = findWidgetById(widgetId);
    if (!widget) return;
    if (!widget.scrollable) widget.scrollable = { x: false, y: false };
    widget.scrollable[axis] = value;
    saveToHistory('scrollable');
    updateJSONEditor();
}

function updateDropdownItems(widgetId, itemsString) {
    const widget = findWidgetById(widgetId);
    if (!widget) return;
    widget.items = itemsString.split(',').map(s => s.trim());
    saveToHistory('dropdown-items');
    updateJSONEditor();
    updatePropertyInspector();
    renderCanvas();
}
