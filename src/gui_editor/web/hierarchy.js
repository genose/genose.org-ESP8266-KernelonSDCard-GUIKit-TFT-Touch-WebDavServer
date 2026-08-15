/**
 * GUIKit Web Editor - Hierarchy View
 */

// ============================================================================
// Hierarchy View
// ============================================================================

function updateHierarchy() {
    const tree = document.getElementById('hierarchy-tree');
    if (!tree) return;

    let html = '<div class="tree-item root" data-id="root">';
    html += '<span class="tree-toggle">\u25BC</span>';
    html += '<span class="tree-icon">\uD83C\uDF10</span>';
    html += '<span class="tree-name">GUI Root</span>';
    html += '</div>';

    html += buildHierarchyTree(EditorState.gui.widgets, 0);
    tree.innerHTML = html;
    setupHierarchyEvents();
}

function buildHierarchyTree(widgets, depth) {
    let html = '';

    for (const widget of widgets) {
        const selected = EditorState.selectedWidgets.has(widget) ? 'selected' : '';
        const hasChildren = widget.type === 'view' && widget.children && widget.children.length > 0;
        const collapsed = widget.collapsed ? 'collapsed' : '';

        html += `<div class="tree-item ${selected} ${collapsed}" data-id="${widget.id}" style="padding-left: ${depth * 20 + 20}px">`;
        html += `<span class="tree-toggle ${hasChildren ? '' : 'hidden'}">${widget.collapsed ? '\u25B6' : '\u25BC'}</span>`;
        html += `<span class="tree-icon">${getWidgetIcon(widget.type)}</span>`;
        html += `<span class="tree-name">${widget.id} (${widget.type})</span>`;
        html += '</div>';

        if (hasChildren && !widget.collapsed) {
            html += buildHierarchyTree(widget.children, depth + 1);
        }
    }

    return html;
}

function setupHierarchyEvents() {
    document.querySelectorAll('.tree-toggle').forEach(toggle => {
        if (toggle.classList.contains('hidden')) return;
        toggle.addEventListener('click', (e) => {
            e.stopPropagation();
            const item = toggle.parentElement;
            const widgetId = item.dataset.id;
            const widget = findWidgetById(widgetId);
            if (widget) {
                widget.collapsed = !widget.collapsed;
                updateHierarchy();
            }
        });
    });

    document.querySelectorAll('.tree-item').forEach(item => {
        item.addEventListener('click', (e) => {
            const widgetId = item.dataset.id;
            if (widgetId === 'root') {
                clearSelection();
                return;
            }
            const widget = findWidgetById(widgetId);
            if (widget) selectWidget(widget);
        });
    });
}
