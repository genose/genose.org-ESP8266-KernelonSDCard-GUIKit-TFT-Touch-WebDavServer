/**
 * GUIKit Web Editor Script
 * 
 * Full-featured web-based editor for GUIKit projects with WebDAV integration
 * Supports: project management, JSON/GUI editing, script editing, temp buffers,
 * contextual menus, and concurrent file management
 * 
 * USAGE:
 *   --help     Show this help message
 *   --version  Show version information
 *   
 * EXAMPLES:
 *   GUIKitEditor.init();                    // Initialize editor
 *   GUIKitEditor.newProject();              // Create new project
 *   GUIKitEditor.openFile('/path/to/file'); // Open a file
 *   GUIKitEditor.saveFile(path, content);   // Save a file
 *   
 * FEATURES:
 *   - Project templates (empty, basic_ui)
 *   - Tabbed file editor
 *   - Contextual menu (long press ~2sec)
 *   - Temp buffer support (/tmp/filename_edit.txt)
 *   - WebDAV integration with user home directories
 */

// ============================================================================
// Version Information
// ============================================================================

var EDITOR_VERSION = '1.0.0';
var EDITOR_NAME = 'GUIKit Web Editor';

// ============================================================================
// Global State
// ============================================================================

var EditorState = {
    // WebDAV connection
    webdavConnected: false,
    webdavManager: null,
    currentUser: null,
    userHome: '/home/admin',
    
    // Project management
    currentProject: null,
    currentProjectPath: '',
    projects: [],
    
    // File management
    openFiles: [],
    currentFileIndex: -1,
    currentFile: null,
    
    // Editor state
    fileContent: '',
    originalContent: '',
    isModified: false,
    cursorPosition: { line: 1, col: 1 },
    selection: { start: 0, end: 0, text: '' },
    
    // Clipboard
    clipboard: '',
    
    // Context menu
    contextMenuVisible: false,
    contextMenuX: 0,
    contextMenuY: 0,
    
    // Temp buffers
    tempBuffers: {},
    
    // Project tree
    projectTree: [],
    
    // UI references
    ui: {
        projectTree: null,
        tabBar: null,
        fileEditor: null,
        statusText: null,
        cursorPos: null,
        webdavStatus: null,
        contextMenu: null
    }
};

// ============================================================================
// Project Types and Templates
// ============================================================================

var ProjectTemplates = {
    'empty': {
        name: 'Empty Project',
        description: 'Blank GUIKit project',
        files: {
            'main_gui.json': {
                content: `{\n  "version": "1.0",\n  "name": "NewProject",\n  "size": { "width": 320, "height": 240 },\n  "background": "#000000",\n  "widgets": []\n}`
            },
            'project.meta.json': {
                content: `{\n  "name": "NewProject",\n  "description": "",\n  "author": "",\n  "version": "1.0.0",\n  "created": "${DATE}",\n  "modified": "${DATE}",\n  "gui_files": ["main_gui.json"],\n  "scripts": [],\n  "styles": []\n}`
            }
        }
    },
    'basic_ui': {
        name: 'Basic UI',
        description: 'GUI with a button and label',
        files: {
            'main_gui.json': {
                content: `{\n  "version": "1.0",\n  "name": "BasicUI",\n  "size": { "width": 320, "height": 240 },\n  "background": "#1E1E1E",\n  "widgets": [\n    {\n      "id": "title",\n      "type": "label",\n      "x": 10,\n      "y": 10,\n      "width": 300,\n      "height": 24,\n      "text": "Hello GUIKit!",\n      "text_size": 18,\n      "text_color": "#FFFFFF",\n      "text_align": "center"\n    },\n    {\n      "id": "my_button",\n      "type": "button",\n      "x": 100,\n      "y": 50,\n      "width": 120,\n      "height": 40,\n      "text": "Click Me",\n      "text_size": 14,\n      "text_color": "#000000",\n      "background": "#1177BB",\n      "action": "my_action"\n    }\n  ]\n}`
            },
            'scripts/main.js': {
                content: `/**\n * Main script\n */\n\nfunction my_action(widget, event) {\n    GUI.getWidgetById('title').text = 'Button Clicked!'\n    GUI.redraw()\n}`
            },
            'project.meta.json': {
                content: `{\n  "name": "BasicUI",\n  "description": "Basic UI with button",\n  "author": "",\n  "version": "1.0.0",\n  "created": "${DATE}",\n  "modified": "${DATE}",\n  "gui_files": ["main_gui.json"],\n  "scripts": ["scripts/main.js"],\n  "styles": []\n}`
            }
        }
    }
};

// File type definitions
var FileTypes = {
    'json': { icon: 'J', color: '#FFD700', editor: 'json' },
    'js': { icon: 'JS', color: '#FFA500', editor: 'code' },
    'css': { icon: 'CS', color: '#FF1493', editor: 'code' },
    'txt': { icon: 'T', color: '#FFFFFF', editor: 'text' },
    'md': { icon: 'MD', color: '#90EE90', editor: 'text' },
    'lua': { icon: 'L', color: '#0000FF', editor: 'code' },
    'unknown': { icon: '?', color: '#808080', editor: 'text' }
};

// ============================================================================
// Initialization
// ============================================================================

function editor_init() {
    // Initialize WebDAV
    initWebDAV();
    
    // Cache UI references
    cacheUIReferences();
    
    // Initialize state
    initEditorState();
    
    // Update UI
    updateWebDAVStatus();
    updateStatusBar();
    
    // Load recent projects if available
    loadRecentProjects();
    
    // Show project tree
    refreshProjectTree();
}

/**
 * Initialize WebDAV connection
 */
function initWebDAV() {
    if (typeof WebDAVManager !== 'undefined') {
        EditorState.webdavManager = WebDAVManager;
        
        // Check if already connected
        if (WebDAVManager.isConnected()) {
            EditorState.webdavConnected = true;
            EditorState.currentUser = WebDAVManager.getCredentials().username;
            EditorState.userHome = WebDAVManager.getUserHome();
        }
    }
}

/**
 * Cache UI widget references
 */
function cacheUIReferences() {
    EditorState.ui.projectTree = GUI.getWidgetById('project_tree');
    EditorState.ui.tabBar = GUI.getWidgetById('tab_bar');
    EditorState.ui.fileEditor = GUI.getWidgetById('file_editor');
    EditorState.ui.statusText = GUI.getWidgetById('status_text');
    EditorState.ui.cursorPos = GUI.getWidgetById('cursor_pos');
    EditorState.ui.webdavStatus = GUI.getWidgetById('webdav_status');
    EditorState.ui.contextMenu = GUI.getWidgetById('context_menu');
}

/**
 * Initialize editor state
 */
function initEditorState() {
    EditorState.openFiles = [];
    EditorState.currentFileIndex = -1;
    EditorState.currentFile = null;
    EditorState.fileContent = '';
    EditorState.originalContent = '';
    EditorState.isModified = false;
    EditorState.clipboard = '';
    EditorState.contextMenuVisible = false;
    EditorState.tempBuffers = {};
}

/**
 * Load recent projects from storage
 */
function loadRecentProjects() {
    if (typeof Storage !== 'undefined') {
        var recentJson = localStorage.getItem('editor_recent_projects');
        if (recentJson) {
            EditorState.projects = JSON.parse(recentJson);
        }
    }
}

/**
 * Save recent projects to storage
 */
function saveRecentProjects() {
    if (typeof Storage !== 'undefined') {
        localStorage.setItem('editor_recent_projects', JSON.stringify(EditorState.projects));
    }
}

// ============================================================================
// WebDAV Connection Management
// ============================================================================

/**
 * Connect to WebDAV
 */
function editor_connect_webdav(widget, event) {
    if (typeof WebDAVManager !== 'undefined') {
        if (WebDAVManager.isConnected()) {
            // Already connected
            refreshProjectTree();
            return;
        }
        
        // Connect using stored or default credentials
        WebDAVManager.connect(null, null);
        
        // Check connection after a short delay
        setTimeout(function() {
            if (WebDAVManager.isConnected()) {
                EditorState.webdavConnected = true;
                EditorState.currentUser = WebDAVManager.getCredentials().username;
                EditorState.userHome = WebDAVManager.getUserHome();
                updateWebDAVStatus();
                refreshProjectTree();
                updateStatus('Connected to WebDAV as ' + EditorState.currentUser);
            }
        }, 500);
    }
}

/**
 * Update WebDAV status display
 */
function updateWebDAVStatus() {
    if (EditorState.ui.webdavStatus) {
        if (EditorState.webdavConnected) {
            EditorState.ui.webdavStatus.text = 'WebDAV: ON';
            EditorState.ui.webdavStatus.text_color = '#4CAF50';
        } else {
            EditorState.ui.webdavStatus.text = 'WebDAV: OFF';
            EditorState.ui.webdavStatus.text_color = '#FF4444';
        }
    }
}

// ============================================================================
// Project Management
// ============================================================================

/**
 * Create a new project
 */
function editor_new_project(widget, event) {
    // Show project creation dialog (simulated)
    var projectName = prompt('Enter project name:', 'MyProject');
    
    if (!projectName || projectName.trim() === '') {
        updateStatus('Project creation cancelled');
        return;
    }
    
    var cleanName = projectName.trim().replace(/\s+/g, '_');
    var projectDir = cleanName + '.GUIKIT';
    
    // Check if project already exists
    if (projectExists(projectDir)) {
        updateStatus('Error: Project already exists');
        return;
    }
    
    // Create project using 'basic_ui' template
    createProjectFromTemplate(projectDir, 'basic_ui');
    
    // Add to recent projects
    addRecentProject(projectDir);
    
    // Open the project
    openProject(projectDir);
}

/**
 * Check if a project exists
 */
function projectExists(projectDir) {
    if (typeof FileSystem !== 'undefined') {
        var userProjectsPath = EditorState.userHome + '/projects/';
        return FileSystem.exists(userProjectsPath + projectDir);
    }
    return false;
}

/**
 * Create project from template
 */
function createProjectFromTemplate(projectDir, templateName) {
    var template = ProjectTemplates[templateName];
    if (!template) {
        template = ProjectTemplates.empty;
    }
    
    var userProjectsPath = EditorState.userHome + '/projects/';
    var projectPath = userProjectsPath + projectDir;
    
    if (typeof FileSystem !== 'undefined') {
        try {
            // Create project directory
            FileSystem.mkdir(projectPath);
            
            // Create all files from template
            var date = new Date().toISOString();
            for (var filename in template.files) {
                var filePath = projectPath + '/' + filename;
                var fileDir = filePath.substring(0, filePath.lastIndexOf('/'));
                
                // Create subdirectories if needed
                if (fileDir !== projectPath && !FileSystem.exists(fileDir)) {
                    FileSystem.mkdir(fileDir);
                }
                
                // Write file content
                var content = template.files[filename].content.replace(/\$\{DATE\}/g, date);
                FileSystem.write(filePath, content);
            }
            
            updateStatus('Created project: ' + projectDir);
            refreshProjectTree();
            return true;
        } catch (error) {
            updateStatus('Error creating project: ' + error.message);
            return false;
        }
    } else {
        // Simulated
        updateStatus('Created project (simulated): ' + projectDir);
        refreshProjectTree();
        return true;
    }
}

/**
 * Open a project
 */
function editor_open_project(widget, event) {
    // Show project list (from project tree)
    // For now, we'll list available projects
    listProjects();
}

/**
 * List available projects
 */
function listProjects() {
    if (typeof FileSystem !== 'undefined') {
        var userProjectsPath = EditorState.userHome + '/projects/';
        var systemProjectsPath = '/gui/';
        
        try {
            var projects = [];
            
            // List user projects
            if (FileSystem.exists(userProjectsPath)) {
                var userFiles = FileSystem.list(userProjectsPath);
                for (var i = 0; i < userFiles.length; i++) {
                    if (userFiles[i].isDirectory && userFiles[i].name.endsWith('.GUIKIT')) {
                        projects.push({
                            name: userFiles[i].name,
                            path: userProjectsPath + userFiles[i].name,
                            type: 'user'
                        });
                    }
                }
            }
            
            // List system projects
            if (FileSystem.exists(systemProjectsPath)) {
                var systemFiles = FileSystem.list(systemProjectsPath);
                for (var i = 0; i < systemFiles.length; i++) {
                    if (systemFiles[i].isDirectory && systemFiles[i].name.endsWith('.GUIKIT')) {
                        projects.push({
                            name: systemFiles[i].name,
                            path: systemProjectsPath + systemFiles[i].name,
                            type: 'system'
                        });
                    }
                }
            }
            
            EditorState.projects = projects;
            saveRecentProjects();
            
            // For now, open the first project
            if (projects.length > 0) {
                openProject(projects[0]);
            } else {
                updateStatus('No projects found. Create a new one.');
            }
        } catch (error) {
            updateStatus('Error listing projects: ' + error.message);
        }
    } else {
        // Simulated - use stored projects
        if (EditorState.projects.length > 0) {
            openProject(EditorState.projects[0]);
        } else {
            updateStatus('No projects (simulated). Create a new one.');
        }
    }
}

/**
 * Open a specific project
 */
function openProject(project) {
    var projectName = typeof project === 'string' ? project : project.name;
    var projectPath = typeof project === 'string' ? 
        (EditorState.userHome + '/projects/' + projectName) : project.path;
    
    EditorState.currentProject = projectName;
    EditorState.currentProjectPath = projectPath;
    
    // Load project files
    loadProjectFiles(projectPath);
    
    updateStatus('Opened project: ' + projectName);
    refreshProjectTree();
}

/**
 * Load all files from a project
 */
function loadProjectFiles(projectPath) {
    if (typeof FileSystem !== 'undefined') {
        try {
            var files = listFilesRecursive(projectPath);
            EditorState.projectTree = buildTreeFromFiles(files, projectPath);
            
            // Find main_gui.json and open it
            var mainGuiPath = projectPath + '/main_gui.json';
            if (FileSystem.exists(mainGuiPath)) {
                openFile(mainGuiPath);
            }
        } catch (error) {
            updateStatus('Error loading project: ' + error.message);
        }
    } else {
        // Simulated
        updateStatus('Loaded project (simulated): ' + projectPath);
    }
}

/**
 * List files recursively
 */
function listFilesRecursive(path) {
    var files = [];
    
    if (typeof FileSystem !== 'undefined') {
        var items = FileSystem.list(path);
        if (items) {
            for (var i = 0; i < items.length; i++) {
                var item = items[i];
                var fullPath = path + '/' + item.name;
                
                if (item.isDirectory) {
                    files.push({
                        name: item.name,
                        path: fullPath,
                        type: 'directory',
                        isDirectory: true
                    });
                    files = files.concat(listFilesRecursive(fullPath));
                } else {
                    files.push({
                        name: item.name,
                        path: fullPath,
                        type: getFileType(item.name),
                        isDirectory: false,
                        size: item.size || 0
                    });
                }
            }
        }
    }
    
    return files;
}

/**
 * Get file type from extension
 */
function getFileType(filename) {
    var ext = filename.substring(filename.lastIndexOf('.') + 1).toLowerCase();
    return FileTypes[ext] ? ext : 'unknown';
}

/**
 * Build tree structure from files
 */
function buildTreeFromFiles(files, rootPath) {
    var tree = [];
    var pathMap = {};
    
    // Sort by path depth
    files.sort(function(a, b) {
        return a.path.split('/').length - b.path.split('/').length;
    });
    
    for (var i = 0; i < files.length; i++) {
        var file = files[i];
        var relativePath = file.path.substring(rootPath.length + 1);
        var parts = relativePath.split('/');
        
        var currentLevel = tree;
        var pathSoFar = '';
        
        for (var j = 0; j < parts.length; j++) {
            var part = parts[j];
            pathSoFar = pathSoFar + (pathSoFar ? '/' : '') + part;
            
            var existing = findInTree(currentLevel, part);
            if (existing) {
                currentLevel = existing.children || [];
            } else {
                var node = {
                    name: part,
                    path: rootPath + '/' + pathSoFar,
                    type: j === parts.length - 1 ? file.type : 'directory',
                    isDirectory: j < parts.length - 1 || file.isDirectory,
                    children: []
                };
                currentLevel.push(node);
                currentLevel = node.children;
            }
        }
    }
    
    return tree;
}

/**
 * Find node in tree by name
 */
function findInTree(tree, name) {
    for (var i = 0; i < tree.length; i++) {
        if (tree[i].name === name) {
            return tree[i];
        }
    }
    return null;
}

/**
 * Add project to recent list
 */
function addRecentProject(projectDir) {
    // Remove if already exists
    EditorState.projects = EditorState.projects.filter(function(p) {
        return p !== projectDir;
    });
    
    // Add to beginning
    EditorState.projects.unshift(projectDir);
    
    // Keep only last 10
    if (EditorState.projects.length > 10) {
        EditorState.projects = EditorState.projects.slice(0, 10);
    }
    
    saveRecentProjects();
}

/**
 * Refresh project tree display
 */
function refreshProjectTree() {
    var treeContainer = EditorState.ui.projectTree;
    if (!treeContainer) return;
    
    treeContainer.children = [];
    
    // Add project root
    if (EditorState.currentProject) {
        var projectNode = {
            id: 'project_root',
            type: 'label',
            x: 5,
            y: 2,
            width: 70,
            height: 18,
            text: EditorState.currentProject,
            text_size: 11,
            text_color: '#FFFFFF',
            background: '#1E639C'
        };
        treeContainer.children.push(projectNode);
        
        var y = 22;
        
        // Add project tree items
        if (EditorState.projectTree && EditorState.projectTree.length > 0) {
            renderTreeNodes(EditorState.projectTree, treeContainer, 5, y, 0);
        } else {
            // Show message
            var noFiles = {
                id: 'no_files',
                type: 'label',
                x: 5,
                y: y,
                width: 70,
                height: 16,
                text: 'No files',
                text_size: 10,
                text_color: '#858585'
            };
            treeContainer.children.push(noFiles);
        }
    } else {
        var noProject = {
            id: 'no_project',
            type: 'label',
            x: 5,
            y: 2,
            width: 70,
            height: 16,
            text: 'No project open',
            text_size: 10,
            text_color: '#858585'
        };
        treeContainer.children.push(noProject);
    }
    
    GUI.redraw();
}

/**
 * Render tree nodes recursively
 */
function renderTreeNodes(nodes, container, x, y, depth) {
    for (var i = 0; i < nodes.length; i++) {
        var node = nodes[i];
        var icon = node.isDirectory ? '📁' : getFileIcon(node.type);
        var color = node.isDirectory ? '#FFA500' : getFileColor(node.type);
        
        var treeItem = {
            id: 'tree_' + node.path.replace(/\/|\./g, '_'),
            type: 'button',
            x: x + (depth * 10),
            y: y,
            width: 70 - (depth * 10),
            height: 16,
            text: (depth > 0 ? String.fromCharCode(9679) : '') + ' ' + node.name,
            text_size: 10,
            text_color: color,
            background: '#252526',
            border_color: '#3E3E42',
            border_width: 0,
            action: 'editor_open_file',
            data: { path: node.path, type: node.type, isDirectory: node.isDirectory }
        };
        
        container.children.push(treeItem);
        y += 16;
        
        // Render children
        if (node.children && node.children.length > 0) {
            y = renderTreeNodes(node.children, container, x, y, depth + 1);
        }
    }
    
    return y;
}

/**
 * Get file icon based on type
 */
function getFileIcon(fileType) {
    var icons = { json: 'J', js: 'JS', css: 'CS', txt: 'T', md: 'MD', lua: 'L' };
    return icons[fileType] || '?';
}

/**
 * Get file color based on type
 */
function getFileColor(fileType) {
    var types = FileTypes[fileType];
    return types ? types.color : '#FFFFFF';
}

// ============================================================================
// File Management
// ============================================================================

/**
 * Open a file from the project tree
 */
function editor_open_file(widget, event) {
    if (widget.data && widget.data.path) {
        var path = widget.data.path;
        var type = widget.data.type;
        var isDirectory = widget.data.isDirectory;
        
        if (isDirectory) {
            // Expand/collapse directory in tree
            // For now, just show the directory
            updateStatus('Directory: ' + path);
        } else {
            openFile(path);
        }
    }
}

/**
 * Open a file by path
 */
function openFile(filePath) {
    if (typeof FileSystem !== 'undefined') {
        try {
            var content = FileSystem.read(filePath);
            
            // Check if already open
            var existingIndex = findOpenFileIndex(filePath);
            
            if (existingIndex !== -1) {
                // Switch to existing tab
                EditorState.currentFileIndex = existingIndex;
                EditorState.currentFile = EditorState.openFiles[existingIndex];
                EditorState.fileContent = EditorState.openFiles[existingIndex].content;
                EditorState.originalContent = EditorState.openFiles[existingIndex].originalContent;
            } else {
                // Open as new tab
                var fileInfo = {
                    path: filePath,
                    filename: filePath.substring(filePath.lastIndexOf('/') + 1),
                    type: getFileType(filePath),
                    content: content,
                    originalContent: content,
                    isModified: false
                };
                
                EditorState.openFiles.push(fileInfo);
                EditorState.currentFileIndex = EditorState.openFiles.length - 1;
                EditorState.currentFile = fileInfo;
                EditorState.fileContent = content;
                EditorState.originalContent = content;
                EditorState.isModified = false;
            }
            
            // Update editor
            updateFileEditor();
            updateTabBar();
            updateStatus('Opened: ' + filePath);
            
            // Create temp buffer
            createTempBuffer(filePath);
        } catch (error) {
            updateStatus('Error opening file: ' + error.message);
        }
    } else {
        // Simulated
        updateStatus('Opened (simulated): ' + filePath);
    }
}

/**
 * Find index of open file by path
 */
function findOpenFileIndex(filePath) {
    for (var i = 0; i < EditorState.openFiles.length; i++) {
        if (EditorState.openFiles[i].path === filePath) {
            return i;
        }
    }
    return -1;
}

/**
 * Create temp buffer for a file
 */
function createTempBuffer(filePath) {
    var filename = filePath.substring(filePath.lastIndexOf('/') + 1);
    var cleanFilename = filename.replace(/\./g, '_');
    var tempPath = '/tmp/' + cleanFilename + '_edit.txt';
    
    EditorState.tempBuffers[filePath] = {
        path: tempPath,
        content: EditorState.fileContent,
        originalContent: EditorState.originalContent
    };
    
    // Write to temp if FileSystem available
    if (typeof FileSystem !== 'undefined') {
        try {
            FileSystem.write(tempPath, EditorState.fileContent);
        } catch (error) {
            // Ignore temp write errors
        }
    }
}

/**
 * Update file editor display
 */
function updateFileEditor() {
    var editor = EditorState.ui.fileEditor;
    if (editor) {
        editor.text = EditorState.fileContent;
    }
    updateStatusBar();
}

/**
 * Update tab bar
 */
function updateTabBar() {
    var tabBar = EditorState.ui.tabBar;
    if (!tabBar) return;
    
    tabBar.children = [];
    
    for (var i = 0; i < EditorState.openFiles.length; i++) {
        var file = EditorState.openFiles[i];
        var isActive = i === EditorState.currentFileIndex;
        
        var tab = {
            id: 'tab_' + i,
            type: 'button',
            x: i * 60,
            y: 0,
            width: 58,
            height: 22,
            text: file.filename.substring(0, 10) + (file.isModified ? '*' : ''),
            text_size: 10,
            text_color: isActive ? '#FFFFFF' : '#D4D4D4',
            background: isActive ? '#0E639C' : '#2D2D2D',
            border_color: isActive ? '#1177BB' : '#3E3E42',
            border_width: 1,
            action: 'editor_switch_tab',
            data: { index: i }
        };
        
        tabBar.children.push(tab);
        
        // Close button on tab
        if (tabBar.children.length <= 8) { // Limit tabs
            var closeBtn = {
                id: 'tab_close_' + i,
                type: 'button',
                x: i * 60 + 48,
                y: 2,
                width: 16,
                height: 16,
                text: 'X',
                text_size: 8,
                text_color: '#FF4444',
                background: 'transparent',
                border_color: 'transparent',
                border_width: 0,
                action: 'editor_close_tab',
                data: { index: i }
            };
            tabBar.children.push(closeBtn);
        }
    }
    
    GUI.redraw();
}

/**
 * Switch to a different tab
 */
function editor_switch_tab(widget, event) {
    if (widget.data && widget.data.index !== undefined) {
        var index = widget.data.index;
        
        // Save current file content
        saveCurrentFileContent();
        
        EditorState.currentFileIndex = index;
        EditorState.currentFile = EditorState.openFiles[index];
        EditorState.fileContent = EditorState.openFiles[index].content;
        EditorState.originalContent = EditorState.openFiles[index].originalContent;
        EditorState.isModified = EditorState.openFiles[index].isModified;
        
        updateFileEditor();
        updateTabBar();
        updateStatusBar();
    }
}

/**
 * Save current file content to openFiles
 */
function saveCurrentFileContent() {
    if (EditorState.currentFile && EditorState.ui.fileEditor) {
        EditorState.currentFile.content = EditorState.ui.fileEditor.text;
        EditorState.currentFile.isModified = 
            EditorState.ui.fileEditor.text !== EditorState.currentFile.originalContent;
        EditorState.fileContent = EditorState.ui.fileEditor.text;
        EditorState.isModified = EditorState.currentFile.isModified;
    }
}

/**
 * Close a tab
 */
function editor_close_tab(widget, event) {
    if (widget.data && widget.data.index !== undefined) {
        var index = widget.data.index;
        
        // Save content before closing
        saveCurrentFileContent();
        
        if (EditorState.openFiles[index].isModified) {
            // Prompt to save (simulated)
            var save = confirm('Save changes to ' + EditorState.openFiles[index].filename + '?');
            if (save) {
                saveFile(EditorState.openFiles[index].path, EditorState.openFiles[index].content);
            }
        }
        
        // Remove from open files
        EditorState.openFiles.splice(index, 1);
        
        // Adjust current index
        if (EditorState.currentFileIndex >= index) {
            EditorState.currentFileIndex--;
        }
        
        if (EditorState.openFiles.length > 0) {
            if (EditorState.currentFileIndex < 0) {
                EditorState.currentFileIndex = 0;
            }
            EditorState.currentFile = EditorState.openFiles[EditorState.currentFileIndex];
            EditorState.fileContent = EditorState.currentFile.content;
            EditorState.originalContent = EditorState.currentFile.originalContent;
            EditorState.isModified = EditorState.currentFile.isModified;
        } else {
            EditorState.currentFile = null;
            EditorState.fileContent = '';
            EditorState.originalContent = '';
            EditorState.isModified = false;
        }
        
        updateFileEditor();
        updateTabBar();
        updateStatusBar();
    }
}

/**
 * Save current file
 */
function editor_save(widget, event) {
    if (EditorState.currentFile) {
        saveFile(EditorState.currentFile.path, EditorState.fileContent);
    }
}

/**
 * Save all open files
 */
function editor_save_all(widget, event) {
    for (var i = 0; i < EditorState.openFiles.length; i++) {
        saveFile(EditorState.openFiles[i].path, EditorState.openFiles[i].content);
    }
    updateStatus('All files saved');
}

/**
 * Save file to filesystem
 */
function saveFile(filePath, content) {
    if (typeof FileSystem !== 'undefined') {
        try {
            FileSystem.write(filePath, content);
            
            // Update original content
            if (EditorState.currentFile && EditorState.currentFile.path === filePath) {
                EditorState.currentFile.originalContent = content;
                EditorState.currentFile.isModified = false;
                EditorState.originalContent = content;
                EditorState.isModified = false;
                updateTabBar();
            }
            
            // Update temp buffer
            if (EditorState.tempBuffers[filePath]) {
                EditorState.tempBuffers[filePath].content = content;
                EditorState.tempBuffers[filePath].originalContent = content;
            }
            
            updateStatus('Saved: ' + filePath);
            return true;
        } catch (error) {
            updateStatus('Error saving: ' + error.message);
            return false;
        }
    } else {
        // Simulated
        updateStatus('Saved (simulated): ' + filePath);
        if (EditorState.currentFile && EditorState.currentFile.path === filePath) {
            EditorState.currentFile.originalContent = content;
            EditorState.currentFile.isModified = false;
            EditorState.isModified = false;
        }
        return true;
    }
}

// ============================================================================
// Text Editing & Context Menu
// ============================================================================

/**
 * Handle text changes in editor
 */
function editor_text_changed(widget, event) {
    if (widget.id === 'file_editor') {
        EditorState.fileContent = widget.text;
        EditorState.isModified = widget.text !== EditorState.originalContent;
        
        if (EditorState.currentFile) {
            EditorState.currentFile.content = widget.text;
            EditorState.currentFile.isModified = EditorState.isModified;
        }
        
        updateTabBar();
        updateStatusBar();
        
        // Update temp buffer
        if (EditorState.currentFile) {
            EditorState.tempBuffers[EditorState.currentFile.path].content = widget.text;
        }
    }
}

/**
 * Show context menu on long press
 */
function editor_show_context_menu(widget, event) {
    // Show context menu at appropriate position
    if (EditorState.ui.contextMenu) {
        EditorState.contextMenuVisible = true;
        EditorState.ui.contextMenu.visible = true;
        
        // Position menu (simplified - center for now)
        EditorState.ui.contextMenu.x = 100;
        EditorState.ui.contextMenu.y = 80;
        
        // Store selection
        if (widget.selection) {
            EditorState.selection = widget.selection;
        }
        
        GUI.redraw();
    }
}

/**
 * Hide context menu
 */
function editor_hide_context_menu() {
    if (EditorState.ui.contextMenu) {
        EditorState.contextMenuVisible = false;
        EditorState.ui.contextMenu.visible = false;
        GUI.redraw();
    }
}

/**
 * Copy selected text
 */
function editor_copy(widget, event) {
    if (EditorState.ui.fileEditor && EditorState.ui.fileEditor.selection) {
        var selection = EditorState.ui.fileEditor.selection;
        if (selection.start !== selection.end) {
            EditorState.clipboard = EditorState.fileContent.substring(
                selection.start, selection.end
            );
            updateStatus('Copied to clipboard');
        }
    }
    editor_hide_context_menu();
}

/**
 * Cut selected text
 */
function editor_cut(widget, event) {
    editor_copy(widget, event);
    
    if (EditorState.ui.fileEditor && EditorState.ui.fileEditor.selection) {
        var selection = EditorState.ui.fileEditor.selection;
        if (selection.start !== selection.end) {
            var before = EditorState.fileContent.substring(0, selection.start);
            var after = EditorState.fileContent.substring(selection.end);
            EditorState.fileContent = before + after;
            
            EditorState.ui.fileEditor.text = EditorState.fileContent;
            EditorState.isModified = true;
            
            if (EditorState.currentFile) {
                EditorState.currentFile.content = EditorState.fileContent;
                EditorState.currentFile.isModified = true;
            }
            
            updateTabBar();
            updateStatus('Cut to clipboard');
        }
    }
    editor_hide_context_menu();
}

/**
 * Paste from clipboard
 */
function editor_paste(widget, event) {
    if (EditorState.clipboard) {
        if (EditorState.ui.fileEditor && EditorState.ui.fileEditor.selection) {
            var selection = EditorState.ui.fileEditor.selection;
            var before = EditorState.fileContent.substring(0, selection.start);
            var after = EditorState.fileContent.substring(selection.end);
            EditorState.fileContent = before + EditorState.clipboard + after;
            
            EditorState.ui.fileEditor.text = EditorState.fileContent;
            EditorState.isModified = true;
            
            if (EditorState.currentFile) {
                EditorState.currentFile.content = EditorState.fileContent;
                EditorState.currentFile.isModified = true;
            }
            
            updateTabBar();
            updateStatus('Pasted from clipboard');
        }
    }
    editor_hide_context_menu();
}

/**
 * Select all text
 */
function editor_select_all(widget, event) {
    // Set selection on file editor
    if (EditorState.ui.fileEditor) {
        var textLength = EditorState.fileContent.length;
        EditorState.ui.fileEditor.selection = {
            start: 0,
            end: textLength,
            text: EditorState.fileContent
        };
        EditorState.selection = EditorState.ui.fileEditor.selection;
        updateStatus('All text selected');
    }
    editor_hide_context_menu();
}

// ============================================================================
// Status and UI Updates
// ============================================================================

/**
 * Update status bar
 */
function updateStatusBar() {
    if (EditorState.ui.statusText) {
        var status = EditorState.currentFile ? 
            ('Editing: ' + EditorState.currentFile.filename + (EditorState.isModified ? ' *' : '')) : 
            'Ready';
        EditorState.ui.statusText.text = status;
    }
    
    if (EditorState.ui.cursorPos) {
        // Calculate line and column
        var lines = EditorState.fileContent.split('\n');
        var totalChars = 0;
        var line = 1;
        var col = 1;
        
        if (EditorState.ui.fileEditor && EditorState.ui.fileEditor.selection) {
            var pos = EditorState.ui.fileEditor.selection.start;
            for (var i = 0; i < lines.length; i++) {
                if (totalChars + lines[i].length >= pos) {
                    line = i + 1;
                    col = pos - totalChars + 1;
                    break;
                }
                totalChars += lines[i].length + 1; // +1 for newline
            }
        }
        
        EditorState.cursorPos.text = 'Line: ' + line + ', Col: ' + col;
    }
    
    GUI.redraw();
}

/**
 * Update status message
 */
function updateStatus(message) {
    if (EditorState.ui.statusText) {
        EditorState.ui.statusText.text = message;
        GUI.redraw();
    }
    
    // Clear after 3 seconds
    setTimeout(function() {
        updateStatusBar();
    }, 3000);
}

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * Get file extension
 */
function getFileExtension(filename) {
    return filename.substring(filename.lastIndexOf('.') + 1);
}

/**
 * Check if JSON is valid
 */
function isValidJSON(content) {
    try {
        JSON.parse(content);
        return true;
    } catch (error) {
        return false;
    }
}

/**
 * Format JSON
 */
function formatJSON(content) {
    try {
        var obj = JSON.parse(content);
        return JSON.stringify(obj, null, 2);
    } catch (error) {
        return content;
    }
}

// ============================================================================
// Global API
// ============================================================================

var GUIKitEditor = {
    init: editor_init,
    newProject: editor_new_project,
    openProject: editor_open_project,
    save: editor_save,
    saveAll: editor_save_all,
    connectWebDAV: editor_connect_webdav,
    
    // File operations
    openFile: openFile,
    saveFile: saveFile,
    
    // Editor operations
    copy: editor_copy,
    cut: editor_cut,
    paste: editor_paste,
    selectAll: editor_select_all,
    
    // State getters
    getCurrentFile: function() { return EditorState.currentFile; },
    getFileContent: function() { return EditorState.fileContent; },
    getCurrentProject: function() { return EditorState.currentProject; },
    getOpenFiles: function() { return EditorState.openFiles; },
    isModified: function() { return EditorState.isModified; },
    
    // WebDAV
    isConnected: function() { return EditorState.webdavConnected; },
    getUserHome: function() { return EditorState.userHome; }
};

// ============================================================================
// Help System
// ============================================================================

/**
 * Show help information
 * Called with --help argument or help command
 */
function editor_help(widget, event) {
    var helpText = EDITOR_NAME + ' v' + EDITOR_VERSION + '\n\n' +
        'USAGE:\n' +
        '  --help     Show this help message\n' +
        '  --version  Show version information\n\n' +
        
        'COMMANDS:\n' +
        '  init                    Initialize editor\n' +
        '  newProject              Create new project\n' +
        '  openProject             Open existing project\n' +
        '  save                    Save current file\n' +
        '  saveAll                 Save all open files\n' +
        '  connectWebDAV           Connect to WebDAV server\n\n' +
        
        'FEATURES:\n' +
        '  - Project Management: Create, open, browse projects\n' +
        '  - File Editing: Tabbed editor with multiple files\n' +
        '  - Context Menu: Long press (~2sec) for Copy/Cut/Paste/Select All\n' +
        '  - Temp Buffers: Auto-saved to /tmp/filename_edit.txt\n' +
        '  - WebDAV: User home directory as root\n' +
        '  - Templates: empty, basic_ui project templates\n\n' +
        
        'EXAMPLES:\n' +
        '  GUIKitEditor.init();\n' +
        '  GUIKitEditor.newProject();\n' +
        '  GUIKitEditor.openFile("/home/user/projects/MyApp.GUIKIT/main_gui.json");\n' +
        '  GUIKitEditor.saveFile(path, content);\n\n' +
        
        'KEYBINDINGS:\n' +
        '  Long Press: Show context menu\n' +
        '  Tab Click: Switch between open files\n' +
        '  Tab X: Close file\n\n' +
        
        'PROJECT TEMPLATES:\n' +
        '  empty      - Blank project (main_gui.json, project.meta.json)\n' +
        '  basic_ui   - Button + label example (main_gui.json, scripts/main.js)\n';
    
    updateStatus('Help: See console for details');
    
    if (typeof console !== 'undefined' && console.log) {
        console.log(helpText);
    }
    
    return helpText;
}

/**
 * Show version information
 */
function editor_version(widget, event) {
    var versionText = EDITOR_NAME + ' v' + EDITOR_VERSION + '\n' +
        'Build: 2026-08-15\n' +
        'Features: project_management, json_editor, script_editor, webdav_integration, temp_buffer_support, contextual_menu\n' +
        'License: MIT\n' +
        'Author: GUIKit System';
    
    updateStatus('Version: ' + EDITOR_VERSION);
    
    if (typeof console !== 'undefined' && console.log) {
        console.log(versionText);
    }
    
    return versionText;
}

/**
 * Process command line arguments
 * Supports --help and --version
 */
function processEditorArgs(args) {
    if (!args) return;
    
    for (var i = 0; i < args.length; i++) {
        switch (args[i]) {
            case '--help':
            case '-h':
            case 'help':
            case '?':
                editor_help(null, null);
                return true;
            case '--version':
            case '-v':
                editor_version(null, null);
                return true;
        }
    }
    return false;
}

// ============================================================================
// Global API Extension with Help
// ============================================================================

// Extend GUIKitEditor with help functions
GUIKitEditor.help = editor_help;
GUIKitEditor.version = editor_version;
GUIKitEditor.showHelp = editor_help;
GUIKitEditor.showVersion = editor_version;
GUIKitEditor.processArgs = processEditorArgs;

// Make available globally
if (typeof window !== 'undefined') {
    window.GUIKitEditor = GUIKitEditor;
    window.GUIKitEditorHelp = helpText;  // Pre-generated help
}
if (typeof global !== 'undefined') {
    global.GUIKitEditor = GUIKitEditor;
    global.GUIKitEditorHelp = helpText;
}

// Export help text for external use
var helpText = editor_help(null, null);
