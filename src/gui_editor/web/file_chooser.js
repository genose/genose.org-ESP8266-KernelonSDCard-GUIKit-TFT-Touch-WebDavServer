/**
 * GUIKit Web Editor - Dual Source File Chooser
 * Shows both WebDAV (ESP8266) and Local Computer files
 */

const FileChooserState = {
    activeSource: 'webdav',
    webdav: { currentPath: '/', files: [], selectedFile: null, isLoading: false },
    local: { currentPath: null, files: [], selectedFile: null, isLoading: false, handle: null },
    filter: '', showHidden: false, sortBy: 'name', sortOrder: 'asc'
};

function getActiveSourceState() { return FileChooserState[FileChooserState.activeSource]; }
function getActiveFiles() { return getActiveSourceState().files; }
function getActiveSelectedFile() { return getActiveSourceState().selectedFile; }

function setActiveSource(source) {
    if (FileChooserState.activeSource !== source) {
        FileChooserState.activeSource = source;
        refreshFileChooser();
    }
    const webdavBtn = document.getElementById('btn-source-webdav');
    const localBtn = document.getElementById('btn-source-local');
    if (webdavBtn && localBtn) {
        webdavBtn.classList.toggle('active', source === 'webdav');
        localBtn.classList.toggle('active', source === 'local');
    }
    updateSourceIndicator();
}

function updateSourceIndicator() {
    const indicator = document.getElementById('file-chooser-source-indicator');
    if (indicator) indicator.textContent = FileChooserState.activeSource === 'webdav' ? 'WebDAV' : 'Local';
}

function showFileChooser() {
    const panel = document.getElementById('file-chooser-panel');
    if (panel) { panel.classList.remove('hidden'); refreshFileChooser(); }
}
function hideFileChooser() { const panel = document.getElementById('file-chooser-panel'); if (panel) panel.classList.add('hidden'); }
function toggleFileChooser() {
    const panel = document.getElementById('file-chooser-panel');
    if (panel) { panel.classList.toggle('hidden'); if (!panel.classList.contains('hidden')) refreshFileChooser(); }
}

async function refreshFileChooser(path = null) {
    const state = getActiveSourceState();
    if (path) state.currentPath = path;
    const fileListEl = document.getElementById('file-chooser-list');
    const pathEl = document.getElementById('file-chooser-path');
    const loadingEl = document.getElementById('file-chooser-loading');
    if (fileListEl) fileListEl.innerHTML = '';
    if (loadingEl) loadingEl.classList.remove('hidden');
    try {
        if (FileChooserState.activeSource === 'webdav') await loadWebDAVFiles();
        else await loadLocalFiles();
        renderFileChooser();
    } catch (error) { logToConsole(`Error: ${error.message}`); if (fileListEl) fileListEl.innerHTML = '<div class="file-chooser-error">Error loading files</div>';
    } finally {
        if (loadingEl) loadingEl.classList.add('hidden');
        if (pathEl) pathEl.textContent = FileChooserState.activeSource === 'webdav' ? (state.currentPath || '/') : (state.currentPath || 'Local Files');
    }
}

async function loadWebDAVFiles() {
    const state = FileChooserState.webdav;
    try {
        const response = await fetch(`/webdav/gui${state.currentPath}`);
        if (!response.ok) throw new Error('Failed to list files');
        let files = []; const data = await response.text();
        try { const json = JSON.parse(data); files = json.files || json; }
        catch { files = parseDirectoryListing(data, state.currentPath); }
        state.files = files; state.selectedFile = null;
    } catch (error) { logToConsole(`WebDAV: ${error.message}`); state.files = []; }
}

async function loadLocalFiles() {
    const state = FileChooserState.local;
    try {
        if (!window.showDirectoryPicker) {
            const input = document.createElement('input');
            input.type = 'file'; input.multiple = true; input.accept = '.json,.txt';
            return new Promise(resolve => {
                input.onchange = (e) => {
                    state.files = Array.from(e.target.files).map(f => ({
                        name: f.name, path: f.name, isDirectory: false, isFile: true,
                        extension: f.name.split('.').pop() || '', size: f.size,
                        date: new Date(f.lastModified).toLocaleDateString(), fileHandle: f
                    }));
                    state.selectedFile = null; resolve();
                };
                input.click();
            });
        }
        if (!state.handle) {
            state.handle = await window.showDirectoryPicker({ mode: 'readwrite' });
        }
        const files = [];
        for await (const entry of state.handle.values()) {
            if (entry.kind === 'file') {
                const file = await entry.getFile();
                files.push({ name: entry.name, path: entry.name, isDirectory: false, isFile: true,
                    extension: entry.name.split('.').pop() || '', size: file.size,
                    date: new Date(file.lastModified).toLocaleDateString(), fileHandle: entry });
            } else if (entry.kind === 'directory') {
                files.push({ name: entry.name, path: entry.name, isDirectory: true, isFile: false, extension: '', directoryHandle: entry });
            }
        }
        state.files = files; state.selectedFile = null;
    } catch (error) { if (error.name !== 'AbortError') logToConsole(`Local: ${error.message}`); state.files = []; }
}

function parseDirectoryListing(html, currentPath) {
    const files = []; const parser = new DOMParser(); const doc = parser.parseFromString(html, 'text/html');
    for (const link of doc.querySelectorAll('a')) {
        const href = link.getAttribute('href'); let name = link.textContent.trim();
        if (href && name && !href.startsWith('?') && !href.startsWith('#')) {
            if (name === 'Parent Directory') name = '..';
            const isDir = href.endsWith('/') || name.endsWith('/');
            let fullPath = href; if (isDir && !href.endsWith('/')) fullPath = href + '/';
            if (name === '..') {
                const parentPath = getParentPath(currentPath);
                files.unshift({ name: '..', path: parentPath, isDirectory: true, isFile: false, extension: '' });
            } else if (isDir) {
                files.push({ name: name.replace(/\/$/, ''), path: fullPath, isDirectory: true, isFile: false, extension: '' });
            } else {
                files.push({ name: name, path: fullPath, isDirectory: false, isFile: true, extension: name.split('.').pop() || '' });
            }
        }
    }
    return files;
}

function getParentPath(path) {
    if (path === '/') return '/';
    const parts = path.split('/').filter(p => p.length > 0);
    if (parts.length <= 1) return '/';
    parts.pop(); return '/' + parts.join('/');
}

function getFileIcon(filename) {
    if (filename.endsWith('.json')) return '\uD83D\uDCC4';
    if (filename.endsWith('.txt')) return '\uD83D\uDC40';
    if (/\.(jpg|jpeg|png|gif)$/i.test(filename)) return '\uD83D\uDDBC';
    if (filename.endsWith('.js')) return '\uD83D\uDCBB';
    if (filename.endsWith('.css')) return '\uD83C\uDFA8';
    if (filename.endsWith('.html')) return '\uD83D\uDDD2';
    return '\uD83D\uDCC1';
}

function formatFileSize(bytes) {
    if (bytes < 1024) return bytes + ' B';
    if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
    return (bytes / (1024 * 1024)).toFixed(1) + ' MB';
}

function renderFileChooser() {
    const fileListEl = document.getElementById('file-chooser-list');
    if (!fileListEl) return; const state = getActiveSourceState(); const files = state.files || []; let html = '';
    if (FileChooserState.activeSource === 'webdav') {
        const parentPath = getParentPath(state.currentPath);
        if (parentPath !== state.currentPath) html += `<div class="file-chooser-item directory" onclick="navigateFileChooser('${parentPath}')"><span class="file-chooser-icon">\u25B6</span><span class="file-chooser-name">..</span><span class="file-chooser-type">Directory</span></div>`;
    }
    const filteredFiles = files.filter(file => (!FileChooserState.showHidden && file.name.startsWith('.')) ? false : !FileChooserState.filter || file.name.toLowerCase().includes(FileChooserState.filter.toLowerCase()));
    filteredFiles.sort((a, b) => { if (a.isDirectory && !b.isDirectory) return -1; if (!a.isDirectory && b.isDirectory) return 1;
        let aVal, bVal; switch (FileChooserState.sortBy) { case 'name': aVal = a.name.toLowerCase(); bVal = b.name.toLowerCase(); break; case 'date': aVal = a.date || ''; bVal = b.date || ''; break; case 'size': aVal = a.size || 0; bVal = b.size || 0; break; default: aVal = a.name.toLowerCase(); bVal = b.name.toLowerCase(); }
        return FileChooserState.sortOrder === 'asc' ? (aVal > bVal ? 1 : -1) : (aVal < bVal ? 1 : -1); });
    const directories = filteredFiles.filter(f => f.isDirectory); const filesOnly = filteredFiles.filter(f => !f.isDirectory);
    for (const file of [...directories, ...filesOnly]) {
        const icon = file.isDirectory ? '\uD83D\uDCC1' : getFileIcon(file.name);
        const type = file.isDirectory ? 'Directory' : file.extension.toUpperCase();
        const size = file.size ? formatFileSize(file.size) : ''; const date = file.date || '';
        const selectedClass = getActiveSelectedFile()?.name === file.name ? 'selected' : '';
        const isJson = file.name.endsWith('.json'); const isDir = file.isDirectory;
        const onclick = isDir ? `navigateFileChooser('${state.currentPath}${state.currentPath.endsWith('/') ? '' : '/'}${file.name}')` : `selectFileChooser('${file.name}', false)`;
        html += `<div class="file-chooser-item ${isDir ? 'directory' : 'file'} ${selectedClass}" onclick="${onclick}" ${isJson && !isDir ? 'ondblclick="loadSelectedFileChooser()"' : ''}><span class="file-chooser-icon">${icon}</span><span class="file-chooser-name">${file.name}</span><span class="file-chooser-type">${type}</span>${size ? `<span class="file-chooser-size">${size}</span>` : ''}${date ? `<span class="file-chooser-date">${date}</span>` : ''}</div>`;
    }
    fileListEl.innerHTML = html || '<div class="file-chooser-empty">No files found</div>';
    updateFileChooserActions();
}

function navigateFileChooser(path) { const state = getActiveSourceState(); state.currentPath = path; state.selectedFile = null; refreshFileChooser(); }
function selectFileChooser(name, isDirectory) { const state = getActiveSourceState(); const file = state.files.find(f => f.name === name); if (file) { state.selectedFile = isDirectory ? null : file; renderFileChooser(); updateFileChooserActions(); } }

function updateFileChooserActions() {
    const openBtn = document.getElementById('file-chooser-open'); const hasSelection = getActiveSelectedFile() && !getActiveSelectedFile().isDirectory; const isWebDAV = FileChooserState.activeSource === 'webdav';
    if (openBtn) openBtn.disabled = !hasSelection; const deleteBtn = document.getElementById('file-chooser-delete'); if (deleteBtn) deleteBtn.disabled = !hasSelection || !isWebDAV;
    const renameBtn = document.getElementById('file-chooser-rename'); if (renameBtn) renameBtn.disabled = !hasSelection || !isWebDAV; const newFolderBtn = document.getElementById('file-chooser-new-folder'); if (newFolderBtn) newFolderBtn.disabled = !isWebDAV;
    const uploadBtn = document.getElementById('file-chooser-upload'); if (uploadBtn) uploadBtn.disabled = !isWebDAV;
}

async function loadSelectedFileChooser() {
    const file = getActiveSelectedFile();
    if (file && FileChooserState.activeSource === 'webdav') { const fullPath = FileChooserState.webdav.currentPath + '/' + file.name; await loadGUIFromFile(fullPath); hideFileChooser(); }
    else if (file && FileChooserState.activeSource === 'local') {
        try { const content = await file.fileHandle.text(); openGUIFromJSON(content); hideFileChooser(); } catch (error) { logToConsole(`Local load error: ${error.message}`); }
    }
}

function showNewFolderModal() { const modal = document.getElementById('modal-new-folder'); if (modal) { const input = document.getElementById('new-folder-name'); if (input) input.value = ''; modal.classList.remove('hidden'); } }
async function createNewFolder() {
    const input = document.getElementById('new-folder-name'); if (!input) return; const folderName = input.value.trim(); if (!folderName) return;
    const modal = document.getElementById('modal-new-folder'); if (modal) modal.classList.add('hidden');
    try { const path = FileChooserState.webdav.currentPath + '/' + folderName; const response = await fetch(`/webdav/gui${path}`, { method: 'MKCOL' });
        if (response.ok) { logToConsole(`Created: ${path}`); refreshFileChooser(); } else logToConsole(`Failed: ${path}`); } catch (error) { logToConsole(`Error: ${error.message}`); }
}

async function deleteSelectedFileChooser() {
    const file = getActiveSelectedFile(); if (!file || FileChooserState.activeSource !== 'webdav') return;
    if (!confirm(`Delete ${file.name}?`)) return; try { const path = FileChooserState.webdav.currentPath + '/' + file.name; const response = await fetch(`/webdav/gui${path}`, { method: 'DELETE' });
        if (response.ok) { logToConsole(`Deleted: ${path}`); FileChooserState.webdav.selectedFile = null; refreshFileChooser(); } else logToConsole(`Failed`); } catch (error) { logToConsole(`Error: ${error.message}`); }
}

function showRenameModal() { const file = getActiveSelectedFile(); if (!file || FileChooserState.activeSource !== 'webdav') return; const modal = document.getElementById('modal-rename-file'); if (modal) { const input = document.getElementById('rename-new-name'); if (input) input.value = file.name; modal.classList.remove('hidden'); } }
async function renameFileChooser() {
    const file = getActiveSelectedFile(); if (!file || FileChooserState.activeSource !== 'webdav') return; const input = document.getElementById('rename-new-name'); if (!input) return;
    const newName = input.value.trim(); if (!newName || newName === file.name) return; const modal = document.getElementById('modal-rename-file'); if (modal) modal.classList.add('hidden');
    try { const oldPath = FileChooserState.webdav.currentPath + '/' + file.name; const newPath = FileChooserState.webdav.currentPath + '/' + newName;
        const response = await fetch(`/webdav/gui${oldPath}`, { method: 'MOVE', headers: { 'Destination': `/webdav/gui${newPath}` } });
        if (response.ok) { logToConsole(`Renamed: ${oldPath} -> ${newPath}`); FileChooserState.webdav.selectedFile = null; refreshFileChooser(); } else logToConsole(`Failed`); } catch (error) { logToConsole(`Error: ${error.message}`); }
}

function showUploadModal() { const modal = document.getElementById('modal-upload'); if (modal) { const input = document.getElementById('upload-file-input'); if (input) input.value = ''; const nameEl = document.getElementById('upload-file-name'); if (nameEl) nameEl.textContent = ''; modal.classList.remove('hidden'); } }
function hideUploadModal() { const modal = document.getElementById('modal-upload'); if (modal) modal.classList.add('hidden'); }
function triggerFileUpload() { const input = document.getElementById('upload-file-input'); if (input) input.click(); }
function updateUploadFileName() { const input = document.getElementById('upload-file-input'); const nameEl = document.getElementById('upload-file-name'); if (input && nameEl) { if (input.files.length > 0) nameEl.textContent = `${input.files.length} file(s)`; else nameEl.textContent = ''; } }

async function handleFileUpload(event) {
    const input = event.target; updateUploadFileName(); if (!input.files || input.files.length === 0) return; const statusEl = document.getElementById('upload-status'); const modal = document.getElementById('modal-upload');
    if (statusEl) statusEl.textContent = `Uploading ${input.files.length} file(s)...`; try {
        for (const file of input.files) { if (statusEl) statusEl.textContent = `Uploading ${file.name}...`; const content = await file.text();
            if (file.name.endsWith('.json')) try { JSON.parse(content); } catch (e) { if (statusEl) statusEl.textContent = `Invalid JSON: ${e.message}`; continue; }
            const path = FileChooserState.webdav.currentPath + '/' + file.name; const response = await fetch(`/webdav/gui${path}`, { method: 'PUT', headers: { 'Content-Type': file.type || 'application/octet-stream' }, body: content });
            if (!response.ok && statusEl) statusEl.textContent = `Failed: ${file.name}`; } if (statusEl) statusEl.textContent = `Uploaded ${input.files.length} file(s)!`;
        setTimeout(() => { if (modal) modal.classList.add('hidden'); refreshFileChooser(); }, 1000);
    } catch (error) { if (statusEl) statusEl.textContent = `Error: ${error.message}`; logToConsole(`Upload: ${error.message}`); }
}

function setFileChooserFilter(filter) { FileChooserState.filter = filter; renderFileChooser(); }
function toggleHiddenFiles() { FileChooserState.showHidden = !FileChooserState.showHidden; renderFileChooser(); }
function sortFilesBy(field) { if (FileChooserState.sortBy === field) FileChooserState.sortOrder = FileChooserState.sortOrder === 'asc' ? 'desc' : 'asc'; else { FileChooserState.sortBy = field; FileChooserState.sortOrder = 'asc'; } renderFileChooser(); }

function handleFileChooserKeyDown(event) {
    if (event.target.tagName === 'INPUT') return; switch (event.key) {
        case 'Escape': hideFileChooser(); event.preventDefault(); break; case 'Enter': if (getActiveSelectedFile()) { loadSelectedFileChooser(); event.preventDefault(); } break;
        case 'Delete': deleteSelectedFileChooser(); event.preventDefault(); break; case 'F2': showRenameModal(); event.preventDefault(); break;
        case 'n': if (event.ctrlKey) { showNewFolderModal(); event.preventDefault(); } break; case 'u': if (event.ctrlKey) { showUploadModal(); event.preventDefault(); } break;
    }
}

async function downloadCurrentGUI() {
    const json = saveGUIToJSON(); const name = EditorState.gui.name || 'untitled'; const filename = `${name.replace(/[^a-z0-9]/gi, '_')}.json`;
    const blob = new Blob([json], { type: 'application/json' }); const url = URL.createObjectURL(blob);
    const a = document.createElement('a'); a.href = url; a.download = filename; document.body.appendChild(a); a.click(); document.body.removeChild(a); URL.revokeObjectURL(url);
    logToConsole(`Downloaded: ${filename}`);
}

function initFileChooser() {
    document.addEventListener('keydown', handleFileChooserKeyDown); const panel = document.getElementById('file-chooser-panel');
    if (panel) panel.addEventListener('keydown', handleFileChooserKeyDown); const webdavBtn = document.getElementById('btn-source-webdav'); const localBtn = document.getElementById('btn-source-local');
    if (webdavBtn && localBtn) webdavBtn.classList.add('active'); updateSourceIndicator();
}

window.FileChooserState = FileChooserState; window.showFileChooser = showFileChooser; window.toggleFileChooser = toggleFileChooser;
window.refreshFileChooser = refreshFileChooser; window.setSource = setSource; window.downloadCurrentGUI = downloadCurrentGUI;
