/**
 * GUIKit Web Editor - Main Editor Logic
 * Core editor functions and initialization
 */

// Import state (loaded via script tag order)
// <script src="state.js"></script> must come before this

// ============================================================================
// Initialization
// ============================================================================

async function loadSchemaValidator() {
    try {
        const response = await fetch('json/gui_schema.json');
        if (!response.ok) throw new Error('Schema not found');
        const schema = await response.json();
        guiSchemaValidator = new ajv({ strict: false, allErrors: true });
        guiSchemaValidator.addSchema(schema);
        logToConsole('Schema validator loaded');
    } catch (error) {
        logToConsole(`Schema error: ${error.message}`);
    }
}

function validateGUI(gui) {
    if (!guiSchemaValidator) return { valid: true, errors: [] };
    const validate = guiSchemaValidator.getSchema('https://genose.org/gui-editor/gui-schema-v1.0.json');
    if (!validate) return { valid: true, errors: [] };
    const valid = validate(gui);
    const errors = valid ? [] : validate.errors.map(e => `${e.instancePath} ${e.message}`);
    return { valid, errors };
}

function initEditor() {
    logToConsole('Initializing GUIKit Web Editor...');
    loadSchemaValidator();
    initCanvas();
    setupEventListeners();
    setupDragAndDrop();
    initFileChooser();
    initProjectManager();
    createNewGUI();
    updateStatusBar();
    updateHierarchy();
    updateJSONEditor();
    logToConsole('Editor initialized.');
}

function initCanvas() {
    canvasElement = document.getElementById('gui-canvas');
    if (!canvasElement) return;
    const renderCanvas = document.createElement('canvas');
    renderCanvas.id = 'render-canvas';
    renderCanvas.style.position = 'absolute';
    renderCanvas.style.top = '0';
    renderCanvas.style.left = '0';
    canvasElement.appendChild(renderCanvas);
    canvasCtx = renderCanvas.getContext('2d');
    updateCanvasSize();
    setupCanvasEvents();
}

function updateCanvasSize() {
    if (!canvasElement) return;
    const rect = canvasElement.getBoundingClientRect();
    const renderCanvas = canvasElement.querySelector('#render-canvas');
    if (renderCanvas) {
        renderCanvas.width = rect.width;
        renderCanvas.height = rect.height;
    }
}

function setupCanvasEvents() {
    if (!canvasElement) return;
    canvasElement.addEventListener('mousedown', handleCanvasMouseDown);
    canvasElement.addEventListener('mousemove', handleCanvasMouseMove);
    canvasElement.addEventListener('mouseup', handleCanvasMouseUp);
    canvasElement.addEventListener('mouseleave', handleCanvasMouseUp);
    canvasElement.addEventListener('wheel', handleCanvasWheel);
    canvasElement.addEventListener('touchstart', handleCanvasTouchStart);
    canvasElement.addEventListener('touchmove', handleCanvasTouchMove);
    canvasElement.addEventListener('touchend', handleCanvasTouchEnd);
}

// ============================================================================
// GUI Management
// ============================================================================

function createNewGUI() {
    EditorState.gui = {
        version: "1.0",
        name: "Untitled GUI",
        size: { width: 320, height: 240 },
        background: "#000000",
        widgets: []
    };
    EditorState.currentFile = null;
    EditorState.isSaved = true;
    EditorState.history = [];
    EditorState.historyIndex = -1;
    clearSelection();
    updateGUIInfo();
    updateStatusBar();
    updateHierarchy();
    updateJSONEditor();
    renderCanvas();
    logToConsole('New GUI created');
}

function updateGUIInfo() {
    const nameEl = document.getElementById('gui-name');
    const sizeEl = document.getElementById('gui-size');
    if (nameEl) nameEl.textContent = EditorState.gui.name || 'Untitled';
    if (sizeEl) sizeEl.textContent = `${EditorState.gui.size.width}x${EditorState.gui.size.height}`;
}

function openGUIFromJSON(json) {
    try {
        const gui = JSON.parse(json);
        const result = validateGUI(gui);
        if (!result.valid) {
            logToConsole(`Validation errors: ${result.errors.join(', ')}`);
            return false;
        }
        EditorState.gui = gui;
        EditorState.isSaved = true;
        EditorState.history = [];
        EditorState.historyIndex = -1;
        clearSelection();
        updateGUIInfo();
        updateStatusBar();
        updateHierarchy();
        updateJSONEditor();
        renderCanvas();
        logToConsole(`GUI loaded: ${gui.name || 'Untitled'}`);
        return true;
    } catch (error) {
        logToConsole(`Load error: ${error.message}`);
        return false;
    }
}

function saveGUIToJSON() {
    return JSON.stringify(EditorState.gui, null, 2);
}

async function saveGUIToFile(filename) {
    try {
        const json = saveGUIToJSON();
        if (window.WebDAVClient) {
            const success = await WebDAVClient.saveFile(filename, json);
            if (success) {
                EditorState.currentFile = filename;
                EditorState.isSaved = true;
                logToConsole(`Saved: ${filename}`);
                updateStatusBar();
                return true;
            }
        } else {
            localStorage.setItem(filename, json);
            EditorState.currentFile = filename;
            EditorState.isSaved = true;
            logToConsole(`Saved: ${filename}`);
            updateStatusBar();
            return true;
        }
        return false;
    } catch (error) {
        logToConsole(`Save error: ${error.message}`);
        return false;
    }
}

async function loadGUIFromFile(filename) {
    try {
        let content;
        if (window.WebDAVClient) {
            content = await WebDAVClient.loadFile(filename);
        } else {
            content = localStorage.getItem(filename);
        }
        if (content) {
            EditorState.currentFile = filename;
            return openGUIFromJSON(content);
        }
        return false;
    } catch (error) {
        logToConsole(`Load error: ${error.message}`);
        return false;
    }
}

// ============================================================================
// WebDAV Client
// ============================================================================

class WebDAVClient {
    static async listFiles(path = '/') {
        try {
            const response = await fetch(`/webdav/gui${path}`, {
                headers: { 'Accept': 'application/json' }
            });
            if (!response.ok) throw new Error('Failed to list');
            return await response.json();
        } catch (error) {
            logToConsole(`WebDAV list error: ${error.message}`);
            return [];
        }
    }

    static async saveFile(filename, content) {
        try {
            const response = await fetch(`/webdav/gui/${filename}`, {
                method: 'PUT',
                headers: { 'Content-Type': 'application/json' },
                body: content
            });
            return response.ok;
        } catch (error) {
            logToConsole(`WebDAV save error: ${error.message}`);
            return false;
        }
    }

    static async loadFile(filename) {
        try {
            const response = await fetch(`/webdav/gui/${filename}`);
            if (!response.ok) throw new Error('Not found');
            return await response.text();
        } catch (error) {
            logToConsole(`WebDAV load error: ${error.message}`);
            return null;
        }
    }

    static async deleteFile(filename) {
        try {
            const response = await fetch(`/webdav/gui/${filename}`, {
                method: 'DELETE'
            });
            return response.ok;
        } catch (error) {
            logToConsole(`WebDAV delete error: ${error.message}`);
            return false;
        }
    }
}

window.WebDAVClient = WebDAVClient;

// ============================================================================
// Event Listeners Setup
// ============================================================================

function setupEventListeners() {
    // Toolbar
    setupButton('btn-new', () => showNewModal());
    setupButton('btn-open', () => toggleFileChooser());
    setupButton('btn-save', () => saveCurrentGUI());
    setupButton('btn-save-as', () => showSaveAsModal());
    setupButton('btn-preview', () => showPreview());
    setupButton('btn-export', () => exportToC());
    setupButton('btn-settings', () => logToConsole('Settings not implemented'));

    // Project buttons
    setupButton('btn-new-project', () => window.showNewProjectModal ? window.showNewProjectModal() : logToConsole('Project manager not loaded'));
    setupButton('btn-projects', () => window.showProjectManager ? window.showProjectManager() : logToConsole('Project manager not loaded'));
    setupButton('btn-save-project', () => window.saveProject ? window.saveProject() : logToConsole('No project open'));
    setupButton('btn-close-project', () => window.closeProject ? window.closeProject() : logToConsole('Project manager not loaded'));
    setupButton('btn-project-settings', () => window.showProjectSettingsModal ? window.showProjectSettingsModal() : logToConsole('Project manager not loaded'));

    // Canvas controls
    setupButton('btn-zoom-in', () => setZoom(EditorState.zoom + 0.1));
    setupButton('btn-zoom-out', () => setZoom(EditorState.zoom - 0.1));
    setupButton('btn-zoom-reset', () => setZoom(1.0));
    setupButton('btn-grid', () => toggleGrid());
    setupButton('btn-snap', () => toggleSnap());
    setupButton('btn-close-preview', () => hidePreview());

    // JSON editor
    setupButton('btn-format-json', () => formatJSON());
    setupButton('btn-validate-json', () => validateJSON());

    // Project manager tabs
    const projectTabs = document.querySelectorAll('.project-manager-tabs .tab-btn');
    if (projectTabs.length > 0) {
        projectTabs.forEach(btn => {
            btn.addEventListener('click', () => {
                projectTabs.forEach(b => b.classList.remove('active'));
                document.querySelectorAll('.project-tabs-content .tab-content').forEach(c => c.classList.add('hidden'));
                btn.classList.add('active');
                const content = document.getElementById(`tab-${btn.dataset.tab}`);
                if (content) content.classList.remove('hidden');
            });
        });
    }

    // Keyboard
    document.addEventListener('keydown', handleKeyDown);

    // Window resize
    window.addEventListener('resize', () => {
        updateCanvasSize();
        renderCanvas();
    });

    // Tabs
    document.querySelectorAll('.tab-btn').forEach(btn => {
        btn.addEventListener('click', () => {
            document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
            document.querySelectorAll('.tab-content').forEach(c => c.classList.add('hidden'));
            btn.classList.add('active');
            const content = document.getElementById(`tab-${btn.dataset.tab}`);
            if (content) content.classList.remove('hidden');
        });
    });
}

function setupButton(id, callback) {
    const btn = document.getElementById(id);
    if (btn) btn.addEventListener('click', callback);
}

// ============================================================================
// Modals
// ============================================================================

function showNewModal() {
    const modal = document.getElementById('modal-new');
    if (modal) modal.classList.remove('hidden');
}

function showOpenModal() {
    const modal = document.getElementById('modal-open');
    if (modal) {
        refreshFileList();
        modal.classList.remove('hidden');
    }
}

function showSaveAsModal() {
    const modal = document.getElementById('modal-save');
    const input = document.getElementById('save-name');
    if (modal && input) {
        input.value = EditorState.currentFile || 'untitled.json';
        modal.classList.remove('hidden');
    }
}

function hidePreview() {
    const panel = document.getElementById('preview-panel');
    if (panel) panel.classList.add('hidden');
}

function showPreview() {
    const panel = document.getElementById('preview-panel');
    if (panel) panel.classList.remove('hidden');
    const previewCanvas = document.getElementById('preview-canvas');
    if (previewCanvas) {
        const ctx = previewCanvas.getContext('2d');
        ctx.fillStyle = EditorState.gui.background || '#000000';
        ctx.fillRect(0, 0, previewCanvas.width, previewCanvas.height);
        const scaleX = previewCanvas.width / EditorState.gui.size.width;
        const scaleY = previewCanvas.height / EditorState.gui.size.height;
        ctx.save();
        ctx.scale(scaleX, scaleY);
        drawWidgets(EditorState.gui.widgets);
        ctx.restore();
    }
}

function exportToC() {
    logToConsole('Export to C not implemented');
}

async function refreshFileList() {
    const fileList = document.getElementById('file-list');
    if (!fileList) return;
    fileList.innerHTML = '<div class="file-item"><span class="file-name">Loading...</span></div>';
    try {
        const files = await WebDAVClient.listFiles('/');
        let html = '<div class="file-item folder" data-path="/" onclick="navigateToPath(this)"><span class="file-icon">\uD83D\uDCC1</span><span class="file-name">/ (Root)</span></div>';
        for (const file of files) {
            if (file.isDirectory) {
                html += `<div class="file-item folder" data-path="${file.path}" onclick="navigateToPath(this)"><span class="file-icon">\uD83D\uDCC1</span><span class="file-name">${file.name}/</span></div>`;
            } else if (file.name.endsWith('.json')) {
                html += `<div class="file-item file" data-path="${file.path}" onclick="selectFile(this)"><span class="file-icon">\uD83D\uDCC4</span><span class="file-name">${file.name}</span></div>`;
            }
        }
        fileList.innerHTML = html;
    } catch {
        fileList.innerHTML = '<div class="file-item"><span class="file-name">Error loading files</span></div>';
    }
}

function navigateToPath(element) {
    logToConsole(`Navigate: ${element.dataset.path}`);
}

function selectFile(element) {
    logToConsole(`Selected: ${element.dataset.path}`);
    document.querySelectorAll('.file-item').forEach(item => item.classList.remove('selected'));
    element.classList.add('selected');
}

async function saveCurrentGUI() {
    // Check if we're in a project context
    if (window.ProjectState && window.ProjectState.currentProject) {
        // Save to project
        await window.saveProject();
    } else if (EditorState.currentFile) {
        await saveGUIToFile(EditorState.currentFile);
    } else {
        showSaveAsModal();
    }
}

// ============================================================================
// Keyboard Handler
// ============================================================================

function handleKeyDown(e) {
    if (e.target.tagName === 'INPUT' || e.target.tagName === 'TEXTAREA') {
        if (e.key === 'Escape') e.target.blur();
        return;
    }

    const keyActions = {
        'Delete': () => deleteWidget(),
        'Backspace': () => deleteWidget(),
        'Escape': () => clearSelection(),
        'a': () => e.ctrlKey && selectAllWidgets(),
        'c': () => e.ctrlKey && copyWidget(),
        'v': () => e.ctrlKey && pasteWidget(),
        'x': () => e.ctrlKey && cutWidget(),
        'z': () => e.ctrlKey && (e.shiftKey ? redo() : undo()),
        's': () => e.ctrlKey && saveCurrentGUI(),
        'n': () => e.ctrlKey && showNewModal(),
        'o': () => e.ctrlKey && toggleFileChooser(),
        'e': () => e.ctrlKey && toggleFileChooser(),
        'p': () => e.ctrlKey && (window.showProjectManager ? window.showProjectManager() : null),
        'ArrowUp': () => moveSelected(0, -EditorState.gridSize),
        'ArrowDown': () => moveSelected(0, EditorState.gridSize),
        'ArrowLeft': () => moveSelected(-EditorState.gridSize, 0),
        'ArrowRight': () => moveSelected(EditorState.gridSize, 0)
    };

    const action = keyActions[e.key];
    if (action) {
        action();
        e.preventDefault();
    }
}

function moveSelected(dx, dy) {
    if (EditorState.selectedWidget) {
        EditorState.selectedWidget.x += dx;
        EditorState.selectedWidget.y += dy;
        saveToHistory('move');
        updateJSONEditor();
        renderCanvas();
    }
}

function setZoom(zoom) {
    EditorState.zoom = Math.max(0.1, Math.min(3.0, zoom));
    updateZoomIndicator();
    renderCanvas();
}

function toggleGrid() {
    EditorState.gridEnabled = !EditorState.gridEnabled;
    const btn = document.getElementById('btn-grid');
    if (btn) btn.classList.toggle('active', EditorState.gridEnabled);
    renderCanvas();
}

function toggleSnap() {
    EditorState.snapToGrid = !EditorState.snapToGrid;
    const btn = document.getElementById('btn-snap');
    if (btn) btn.classList.toggle('active', EditorState.snapToGrid);
}

function selectAllWidgets() {
    EditorState.selectedWidgets.clear();
    collectAllWidgets(EditorState.gui.widgets, EditorState.selectedWidgets);
    EditorState.selectedWidget = EditorState.selectedWidgets.size > 0 ? [...EditorState.selectedWidgets][0] : null;
    updatePropertyInspector();
    updateStatusBar();
    renderCanvas();
}

function collectAllWidgets(widgets, set) {
    for (const widget of widgets) {
        set.add(widget);
        if (widget.type === 'view' && widget.children) {
            collectAllWidgets(widget.children, set);
        }
    }
}

// ============================================================================
// Console Logging
// ============================================================================

function logToConsole(message) {
    const output = document.getElementById('console-output');
    if (!output) return;
    const line = document.createElement('div');
    line.className = 'console-line';
    const timestamp = new Date().toLocaleTimeString();
    const timestampEl = document.createElement('span');
    timestampEl.className = 'console-timestamp';
    timestampEl.textContent = `[${timestamp}] `;
    line.prepend(timestampEl);
    line.appendChild(document.createTextNode(message));
    output.appendChild(line);
    output.scrollTop = output.scrollHeight;
}

// ============================================================================
// Global API
// ============================================================================

window.editor = {
    undo, redo,
    cutWidget, copyWidget, pasteWidget, deleteWidget,
    bringToFrontWidget, sendToBackWidget,
    createNewGUI, openGUIFromJSON, saveGUIToJSON,
    saveGUIToFile, loadGUIFromFile,
    render: renderCanvas
};

// Initialize on DOM load
document.addEventListener('DOMContentLoaded', initEditor);
