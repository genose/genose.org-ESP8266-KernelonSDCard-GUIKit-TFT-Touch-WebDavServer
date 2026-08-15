/**
 * GUIKit Web Editor - Drag and Drop
 */

// ============================================================================
// Drag and Drop Setup
// ============================================================================

function setupDragAndDrop() {
    // Widget palette drag and drop
    const widgetItems = document.querySelectorAll('.widget-item');

    widgetItems.forEach(item => {
        item.addEventListener('dragstart', (e) => {
            EditorState.createWidgetType = item.dataset.type;
            e.dataTransfer.setData('text/plain', item.dataset.type);
            e.dataTransfer.effectAllowed = 'copy';
            document.body.classList.add('dragging-widget');
        });

        item.addEventListener('dragend', () => {
            document.body.classList.remove('dragging-widget');
        });
    });

    // Canvas drop
    if (canvasElement) {
        canvasElement.addEventListener('dragover', (e) => {
            e.preventDefault();
            e.dataTransfer.dropEffect = 'copy';
        });

        canvasElement.addEventListener('drop', (e) => {
            e.preventDefault();
            document.body.classList.remove('dragging-widget');

            const rect = canvasElement.getBoundingClientRect();
            const x = Math.floor((e.clientX - rect.left) / EditorState.zoom);
            const y = Math.floor((e.clientY - rect.top) / EditorState.zoom);

            const type = e.dataTransfer.getData('text/plain');
            if (type && isValidWidgetType(type)) {
                const widget = createNewWidget(type, x, y);
                addWidget(widget);
                saveToHistory('add');
            }

            EditorState.createWidgetType = null;
        });
    }
}

function startWidgetCreation(x, y) {
    EditorState.createWidgetType = null;
}
