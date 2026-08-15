/**
 * GUIKit Web Editor - File Chooser Panel
 * 
 * Provides file browsing, upload, and management functionality
 * for GUI JSON files via WebDAV.
 */

// ============================================================================
// File Chooser State
// ============================================================================

const FileChooserState = {
    currentPath: '/',
    files: [],
    selectedFile: null,
    isLoading: false,
    filter: '',
    showHidden: false,
    sortBy: 'name', // name, date, size
    sortOrder: 'asc' // asc, desc
};

// ============================================================================
// File Chooser UI
// ============================================================================

function showFileChooser() {
    const panel = document.getElementById('file-chooser-panel');
    if (panel) {
        panel.classList.remove('hidden');
        refreshFileChooser();
    }
}

function hideFileChooser() {
    const panel = document.getElementById('file-chooser-panel');
    if (panel) {
        panel.classList.add('hidden');
    }
}

function toggleFileChooser() {
    const panel = document.getElementById('file-chooser-panel');
    if (panel) {
        panel.classList.toggle('hidden');
        if (!panel.classList.contains('hidden')) {
            refreshFileChooser();
        }
    }
}

// ============================================================================
// File Operations
// ============================================================================

async function refreshFileChooser(path = null) {
    const panel = document.getElementById('file-chooser-panel');
    if (!panel) return;

    const fileListEl = document.getElementById('file-chooser-list');
    const pathEl = document.getElementById('file-chooser-path');
    const loadingEl = document.getElementById('file-chooser-loading');

    if (path) {
        FileChooserState.currentPath = path;
    }

    // Show loading
    if (fileListEl) fileListEl.innerHTML = '';
    if (loadingEl) loadingEl.classList.remove('hidden');

    try {
        const response = await fetch(`/webdav/gui${FileChooserState.currentPath}`);
        if (!response.ok) throw new Error('Failed to list files');
        
        // Parse directory listing
        // WebDAV typically returns XML, but we'll assume JSON for simplicity
        // or parse the directory listing
        let files = [];
        
        try {
            const data = await response.text();
            // Try to parse as JSON first
            try {
                const json = JSON.parse(data);
                files = json.files || json;
            } catch {
                // Parse as directory listing HTML or XML
                files = parseDirectoryListing(data);
            }
        } catch (error) {
            logToConsole(`Error parsing directory: ${error.message}`);
        }

        FileChooserState.files = files;
        renderFileChooser();

    } catch (error) {
        logToConsole(`Error loading files: ${error.message}`);
        if (fileListEl) {
            fileListEl.innerHTML = '<div class="file-chooser-error">Error loading files. Check WebDAV connection.</div>';
        }
    } finally {
        if (loadingEl) loadingEl.classList.add('hidden');
        if (pathEl) pathEl.textContent = FileChooserState.currentPath || '/';
    }
}

function parseDirectoryListing(html) {
    // Parse HTML directory listing
    const files = [];
    const parser = new DOMParser();
    const doc = parser.parseFromString(html, 'text/html');
    
    // Look for file links in the listing
    const links = doc.querySelectorAll('a');
    for (const link of links) {
        const href = link.getAttribute('href');
        const name = link.textContent.trim();
        
        if (href && name && !href.startsWith('?') && !href.startsWith('#')) {
            // Skip parent directory
            if (name === 'Parent Directory' || name === '..') continue;
            
            // Determine if it's a directory
            const isDir = href.endsWith('/') || name.endsWith('/');
            
            files.push({
                name: isDir ? name.replace(/\/$/, '') : name,
                path: href,
                isDirectory: isDir,
                isFile: !isDir,
                extension: isDir ? '' : name.split('.').pop() || ''
            });
        }
    }
    
    return files;
}

function renderFileChooser() {
    const fileListEl = document.getElementById('file-chooser-list');
    if (!fileListEl) return;

    let html = '';
    
    // Parent directory link
    const parentPath = getParentPath(FileChooserState.currentPath);
    if (parentPath !== FileChooserState.currentPath) {
        html += `
            <div class="file-chooser-item directory" 
                 onclick="navigateFileChooser('${parentPath}')">
                <span class="file-chooser-icon">\uD83D\uDC19</span>
                <span class="file-chooser-name">.. (Parent)</span>
                <span class="file-chooser-type">Directory</span>
            </div>
        `;
    }

    // Files and directories
    const filteredFiles = FileChooserState.files.filter(file => {
        if (!FileChooserState.showHidden && file.name.startsWith('.')) return false;
        if (FileChooserState.filter && !file.name.toLowerCase().includes(FileChooserState.filter.toLowerCase())) return false;
        return true;
    });

    // Sort files
    filteredFiles.sort((a, b) => {
        // Directories first
        if (a.isDirectory && !b.isDirectory) return -1;
        if (!a.isDirectory && b.isDirectory) return 1;
        
        // Sort by field
        let aVal, bVal;
        switch (FileChooserState.sortBy) {
            case 'name':
                aVal = a.name.toLowerCase();
                bVal = b.name.toLowerCase();
                break;
            case 'date':
                aVal = a.date || '';
                bVal = b.date || '';
                break;
            case 'size':
                aVal = a.size || 0;
                bVal = b.size || 0;
                break;
            default:
                aVal = a.name.toLowerCase();
                bVal = b.name.toLowerCase();
        }
        
        if (FileChooserState.sortOrder === 'asc') {
            return aVal > bVal ? 1 : -1;
        } else {
            return aVal < bVal ? 1 : -1;
        }
    });

    for (const file of filteredFiles) {
        const icon = file.isDirectory ? '\uD83D\uDCC1' : getFileIcon(file.name);
        const type = file.isDirectory ? 'Directory' : file.extension.toUpperCase();
        const size = file.size ? formatFileSize(file.size) : '';
        const date = file.date || '';
        const selectedClass = FileChooserState.selectedFile?.name === file.name ? 'selected' : '';
        const isJson = file.name.endsWith('.json');
        
        html += `
            <div class="file-chooser-item ${file.isDirectory ? 'directory' : 'file'} ${selectedClass}"
                 onclick="selectFileChooser('${file.name}', ${file.isDirectory})"
                 ${isJson ? 'ondblclick="loadSelectedFileChooser()"' : ''}>
                <span class="file-chooser-icon">${icon}</span>
                <span class="file-chooser-name">${file.name}</span>
                <span class="file-chooser-type">${type}</span>
                ${size ? `<span class="file-chooser-size">${size}</span>` : ''}
                ${date ? `<span class="file-chooser-date">${date}</span>` : ''}
            </div>
        `;
    }

    fileListEl.innerHTML = html;
    updateFileChooserActions();
}

function getParentPath(path) {
    if (path === '/') return '/';
    const parts = path.split('/').filter(p => p.length > 0);
    if (parts.length <= 1) return '/';
    parts.pop();
    return '/' + parts.join('/');
}

function getFileIcon(filename) {
    if (filename.endsWith('.json')) return '\uD83D\uDCC4';
    if (filename.endsWith('.txt')) return '\uD83D\uDC40';
    if (filename.endsWith('.jpg') || filename.endsWith('.jpeg') || filename.endsWith('.png') || filename.endsWith('.gif')) return '\uD83D\uDDBC';
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

function navigateFileChooser(path) {
    FileChooserState.currentPath = path;
    FileChooserState.selectedFile = null;
    refreshFileChooser();
}

function selectFileChooser(name, isDirectory) {
    const file = FileChooserState.files.find(f => f.name === name);
    if (file) {
        FileChooserState.selectedFile = isDirectory ? null : file;
        renderFileChooser();
        updateFileChooserActions();
    }
}

function updateFileChooserActions() {
    const openBtn = document.getElementById('file-chooser-open');
    const deleteBtn = document.getElementById('file-chooser-delete');
    const renameBtn = document.getElementById('file-chooser-rename');
    
    const hasSelection = FileChooserState.selectedFile && !FileChooserState.selectedFile.isDirectory;
    
    if (openBtn) openBtn.disabled = !hasSelection;
    if (deleteBtn) deleteBtn.disabled = !hasSelection;
    if (renameBtn) renameBtn.disabled = !hasSelection;
}

async function loadSelectedFileChooser() {
    if (FileChooserState.selectedFile) {
        const fullPath = FileChooserState.currentPath + '/' + FileChooserState.selectedFile.name;
        await loadGUIFromFile(fullPath);
        hideFileChooser();
    }
}

// ============================================================================
// File Actions
// ============================================================================

function showNewFolderModal() {
    const modal = document.getElementById('modal-new-folder');
    if (modal) {
        const input = document.getElementById('new-folder-name');
        if (input) input.value = '';
        modal.classList.remove('hidden');
    }
}

async function createNewFolder() {
    const input = document.getElementById('new-folder-name');
    if (!input) return;
    
    const folderName = input.value.trim();
    if (!folderName) return;
    
    const modal = document.getElementById('modal-new-folder');
    if (modal) modal.classList.add('hidden');
    
    try {
        const path = FileChooserState.currentPath + '/' + folderName;
        const response = await fetch(`/webdav/gui${path}`, {
            method: 'MKCOL'
        });
        
        if (response.ok) {
            logToConsole(`Created directory: ${path}`);
            refreshFileChooser();
        } else {
            logToConsole(`Failed to create directory: ${path}`);
        }
    } catch (error) {
        logToConsole(`Error creating directory: ${error.message}`);
    }
}

async function deleteSelectedFileChooser() {
    if (!FileChooserState.selectedFile) return;
    
    if (!confirm(`Delete ${FileChooserState.selectedFile.name}? This cannot be undone.`)) {
        return;
    }
    
    try {
        const path = FileChooserState.currentPath + '/' + FileChooserState.selectedFile.name;
        const response = await fetch(`/webdav/gui${path}`, {
            method: 'DELETE'
        });
        
        if (response.ok) {
            logToConsole(`Deleted: ${path}`);
            FileChooserState.selectedFile = null;
            refreshFileChooser();
        } else {
            logToConsole(`Failed to delete: ${path}`);
        }
    } catch (error) {
        logToConsole(`Error deleting file: ${error.message}`);
    }
}

function showRenameModal() {
    if (!FileChooserState.selectedFile) return;
    
    const modal = document.getElementById('modal-rename-file');
    if (modal) {
        const input = document.getElementById('rename-new-name');
        if (input) input.value = FileChooserState.selectedFile.name;
        modal.classList.remove('hidden');
    }
}

async function renameFileChooser() {
    if (!FileChooserState.selectedFile) return;
    
    const input = document.getElementById('rename-new-name');
    if (!input) return;
    
    const newName = input.value.trim();
    if (!newName || newName === FileChooserState.selectedFile.name) return;
    
    const modal = document.getElementById('modal-rename-file');
    if (modal) modal.classList.add('hidden');
    
    try {
        const oldPath = FileChooserState.currentPath + '/' + FileChooserState.selectedFile.name;
        const newPath = FileChooserState.currentPath + '/' + newName;
        
        // WebDAV MOVE request
        const response = await fetch(`/webdav/gui${oldPath}`, {
            method: 'MOVE',
            headers: {
                'Destination': `/webdav/gui${newPath}`
            }
        });
        
        if (response.ok) {
            logToConsole(`Renamed: ${oldPath} -> ${newPath}`);
            FileChooserState.selectedFile = null;
            refreshFileChooser();
        } else {
            logToConsole(`Failed to rename file`);
        }
    } catch (error) {
        logToConsole(`Error renaming file: ${error.message}`);
    }
}

// ============================================================================
// Upload Functionality
// ============================================================================

function showUploadModal() {
    const modal = document.getElementById('modal-upload');
    if (modal) {
        const input = document.getElementById('upload-file-input');
        if (input) input.value = '';
        modal.classList.remove('hidden');
    }
}

function hideUploadModal() {
    const modal = document.getElementById('modal-upload');
    if (modal) {
        modal.classList.add('hidden');
    }
}

function triggerFileUpload() {
    const input = document.getElementById('upload-file-input');
    if (input) {
        input.click();
    }
}

function updateUploadFileName() {
    const input = document.getElementById('upload-file-input');
    const nameEl = document.getElementById('upload-file-name');
    if (input && nameEl && input.files.length > 0) {
        nameEl.textContent = input.files[0].name;
    } else if (nameEl) {
        nameEl.textContent = '';
    }
}

async function handleFileUpload(event) {
    const input = event.target;
    updateUploadFileName();
    if (!input.files || input.files.length === 0) return;
    
    const file = input.files[0];
    const modal = document.getElementById('modal-upload');
    const statusEl = document.getElementById('upload-status');
    
    if (statusEl) statusEl.textContent = `Uploading ${file.name}...`;
    
    try {
        // Read file content
        const content = await file.text();
        
        // Validate if it's a JSON file
        if (file.name.endsWith('.json')) {
            try {
                JSON.parse(content);
            } catch (error) {
                if (statusEl) statusEl.textContent = `Invalid JSON: ${error.message}`;
                return;
            }
        }
        
        // Upload via WebDAV
        const path = FileChooserState.currentPath + '/' + file.name;
        const response = await fetch(`/webdav/gui${path}`, {
            method: 'PUT',
            headers: {
                'Content-Type': file.type || 'application/octet-stream'
            },
            body: content
        });
        
        if (response.ok) {
            if (statusEl) statusEl.textContent = `Uploaded: ${file.name}`;
            logToConsole(`Uploaded: ${file.name}`);
            setTimeout(() => {
                if (modal) modal.classList.add('hidden');
                refreshFileChooser();
            }, 1000);
        } else {
            if (statusEl) statusEl.textContent = `Upload failed: ${response.statusText}`;
        }
    } catch (error) {
        if (statusEl) statusEl.textContent = `Error: ${error.message}`;
        logToConsole(`Upload error: ${error.message}`);
    }
}

// ============================================================================
// Filter and Sort
// ============================================================================

function setFileChooserFilter(filter) {
    FileChooserState.filter = filter;
    renderFileChooser();
}

function toggleHiddenFiles() {
    FileChooserState.showHidden = !FileChooserState.showHidden;
    renderFileChooser();
}

function sortFilesBy(field) {
    if (FileChooserState.sortBy === field) {
        FileChooserState.sortOrder = FileChooserState.sortOrder === 'asc' ? 'desc' : 'asc';
    } else {
        FileChooserState.sortBy = field;
        FileChooserState.sortOrder = 'asc';
    }
    renderFileChooser();
}

// ============================================================================
// Keyboard Shortcuts
// ============================================================================

function handleFileChooserKeyDown(event) {
    if (event.target.tagName === 'INPUT') return;
    
    switch (event.key) {
        case 'Escape':
            hideFileChooser();
            event.preventDefault();
            break;
        case 'Enter':
            if (FileChooserState.selectedFile) {
                loadSelectedFileChooser();
                event.preventDefault();
            }
            break;
        case 'Delete':
            deleteSelectedFileChooser();
            event.preventDefault();
            break;
        case 'F2':
            showRenameModal();
            event.preventDefault();
            break;
    }
}

// ============================================================================
// Initialize
// ============================================================================

function initFileChooser() {
    // Add event listeners
    document.addEventListener('keydown', handleFileChooserKeyDown);
    
    // Initialize file chooser panel
    const panel = document.getElementById('file-chooser-panel');
    if (panel) {
        panel.addEventListener('keydown', handleFileChooserKeyDown);
    }
}

// Add to global scope
window.FileChooserState = FileChooserState;
window.showFileChooser = showFileChooser;
window.toggleFileChooser = toggleFileChooser;
window.refreshFileChooser = refreshFileChooser;
