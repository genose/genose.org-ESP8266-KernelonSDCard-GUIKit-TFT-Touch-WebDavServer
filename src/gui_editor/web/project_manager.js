/**
 * GUIKit Web Editor - Project Manager
 * 
 * Manages projects stored as {project_name}.GUIKIT directories
 * containing all GUI resources (JSON files, assets, etc.)
 */

// ============================================================================
// Project State
// ============================================================================

const ProjectState = {
    // Current active project
    currentProject: null,
    
    // Available projects
    projects: {
        webdav: [],
        local: []
    },
    
    // Default save location: 'webdav' or 'local'
    defaultLocation: 'webdav',
    
    // Project directory suffix
    PROJECT_SUFFIX: '.GUIKIT',
    
    // Default GUI file name within project
    DEFAULT_GUI_FILE: 'main_gui.json'
};

// ============================================================================
// Project Structure
// ============================================================================

/**
 * GUIKit Project Structure:
 * {project_name}.GUIKIT/
 * ├── main_gui.json          (Default GUI file)
 * ├── assets/               (Images, fonts, etc.)
 * ├── gui/                  (Additional GUI files)
 * ├── scripts/              (Lua/JavaScript scripts)
 * ├── styles/               (CSS/Theme files)
 * └── project.meta.json     (Project metadata)
 */

// ============================================================================
// Project Metadata
// ============================================================================

function createProjectMetadata(name, description = '', author = '', version = '1.0') {
    return {
        name: name,
        description: description,
        author: author,
        version: version,
        created: new Date().toISOString(),
        modified: new Date().toISOString(),
        gui_files: [ProjectState.DEFAULT_GUI_FILE],
        dependencies: []
    };
}

function getProjectMetadataPath(projectName) {
    return `${projectName}${ProjectState.PROJECT_SUFFIX}/project.meta.json`;
}

function getProjectPath(projectName) {
    return `${projectName}${ProjectState.PROJECT_SUFFIX}/`;
}

function getMainGUIPath(projectName) {
    return `${projectName}${ProjectState.PROJECT_SUFFIX}/${ProjectState.DEFAULT_GUI_FILE}`;
}

// ============================================================================
// Project Operations
// ============================================================================

/**
 * Create a new project
 * @param {string} name - Project name (without suffix)
 * @param {string} location - 'webdav' or 'local'
 * @param {object} metadata - Optional project metadata
 * @returns {Promise<boolean>} - Success status
 */
async function createProject(name, location = ProjectState.defaultLocation, metadata = null) {
    if (!name || name.trim() === '') {
        logToConsole('Error: Project name cannot be empty');
        return false;
    }
    
    const projectName = name.trim();
    const projectPath = getProjectPath(projectName);
    const metaPath = getProjectMetadataPath(projectName);
    const mainGUIPath = getMainGUIPath(projectName);
    
    try {
        if (location === 'webdav') {
            // Create project directory on WebDAV
            const dirResponse = await fetch(`/webdav/gui/${projectPath}`, {
                method: 'MKCOL'
            });
            
            if (!dirResponse.ok) {
                logToConsole(`Error: Failed to create project directory on WebDAV`);
                return false;
            }
            
            // Create subdirectories
            const subDirs = ['assets/', 'gui/', 'scripts/', 'styles/'];
            for (const subDir of subDirs) {
                await fetch(`/webdav/gui/${projectPath}${subDir}`, {
                    method: 'MKCOL'
                });
            }
            
            // Create project metadata
            const meta = metadata || createProjectMetadata(projectName);
            const metaResponse = await fetch(`/webdav/gui/${metaPath}`, {
                method: 'PUT',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(meta, null, 2)
            });
            
            if (!metaResponse.ok) {
                logToConsole('Error: Failed to create project metadata');
                return false;
            }
            
            // Create default GUI file
            const defaultGUI = {
                version: "1.0",
                name: `${projectName} Main GUI`,
                size: { width: 320, height: 240 },
                background: "#000000",
                widgets: []
            };
            
            const guiResponse = await fetch(`/webdav/gui/${mainGUIPath}`, {
                method: 'PUT',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(defaultGUI, null, 2)
            });
            
            if (!guiResponse.ok) {
                logToConsole('Error: Failed to create default GUI file');
                return false;
            }
            
            logToConsole(`Project "${projectName}" created on WebDAV`);
            await loadProjects('webdav');
            return true;
            
        } else {
            // Local project creation
            if (!window.showDirectoryPicker) {
                logToConsole('Error: File System Access API not available');
                return false;
            }
            
            const dirHandle = await window.showDirectoryPicker({
                mode: 'readwrite'
            });
            
            const projectDirHandle = await dirHandle.getDirectoryHandle(
                `${projectName}${ProjectState.PROJECT_SUFFIX}`,
                { create: true }
            );
            
            // Create subdirectories
            const subDirs = ['assets', 'gui', 'scripts', 'styles'];
            for (const subDir of subDirs) {
                await projectDirHandle.getDirectoryHandle(subDir, { create: true });
            }
            
            // Create metadata file
            const meta = metadata || createProjectMetadata(projectName);
            const metaFile = await projectDirHandle.getFileHandle(
                'project.meta.json',
                { create: true }
            );
            const metaWritable = await metaFile.createWritable();
            await metaWritable.write(JSON.stringify(meta, null, 2));
            await metaWritable.close();
            
            // Create default GUI file
            const defaultGUI = {
                version: "1.0",
                name: `${projectName} Main GUI`,
                size: { width: 320, height: 240 },
                background: "#000000",
                widgets: []
            };
            
            const guiFile = await projectDirHandle.getFileHandle(
                ProjectState.DEFAULT_GUI_FILE,
                { create: true }
            );
            const guiWritable = await guiFile.createWritable();
            await guiWritable.write(JSON.stringify(defaultGUI, null, 2));
            await guiWritable.close();
            
            // Store the directory handle
            if (!ProjectState.localHandles) {
                ProjectState.localHandles = {};
            }
            ProjectState.localHandles[projectName] = projectDirHandle;
            
            logToConsole(`Project "${projectName}" created locally`);
            await loadProjects('local');
            return true;
        }
        
    } catch (error) {
        logToConsole(`Error creating project: ${error.message}`);
        return false;
    }
}

/**
 * Open a project
 * @param {string} projectName - Project name (without suffix)
 * @param {string} location - 'webdav' or 'local'
 * @returns {Promise<boolean>} - Success status
 */
async function openProject(projectName, location = ProjectState.defaultLocation) {
    try {
        const projectPath = getProjectPath(projectName);
        const metaPath = getProjectMetadataPath(projectName);
        
        let metadata, guiContent;
        
        if (location === 'webdav') {
            // Load project metadata
            const metaResponse = await fetch(`/webdav/gui/${metaPath}`);
            if (!metaResponse.ok) {
                logToConsole(`Error: Project metadata not found for ${projectName}`);
                return false;
            }
            metadata = JSON.parse(await metaResponse.text());
            
            // Load default GUI
            const guiResponse = await fetch(`/webdav/gui/${projectPath}${ProjectState.DEFAULT_GUI_FILE}`);
            if (!guiResponse.ok) {
                logToConsole(`Error: Default GUI not found for ${projectName}`);
                return false;
            }
            guiContent = await guiResponse.text();
            
        } else {
            // Local project
            const dirHandle = ProjectState.localHandles?.[projectName];
            if (!dirHandle) {
                // Try to get the handle
                if (!window.showDirectoryPicker) {
                    logToConsole('Error: File System Access API not available');
                    return false;
                }
                // For now, use the stored handle or prompt
                logToConsole('Error: Local project handle not found');
                return false;
            }
            
            // Load metadata
            const metaFile = await dirHandle.getFileHandle('project.meta.json');
            const metaContent = await metaFile.text();
            metadata = JSON.parse(metaContent);
            
            // Load default GUI
            const guiFile = await dirHandle.getFileHandle(ProjectState.DEFAULT_GUI_FILE);
            guiContent = await guiFile.text();
        }
        
        // Set current project
        ProjectState.currentProject = {
            name: projectName,
            location: location,
            path: projectPath,
            metadata: metadata
        };
        
        // Load the GUI into editor
        openGUIFromJSON(guiContent);
        logToConsole(`Project "${projectName}" opened from ${location}`);
        
        // Update UI
        updateProjectUI();
        
        return true;
        
    } catch (error) {
        logToConsole(`Error opening project: ${error.message}`);
        return false;
    }
}

/**
 * Save current GUI to project
 * @returns {Promise<boolean>} - Success status
 */
async function saveProject() {
    if (!ProjectState.currentProject) {
        logToConsole('Error: No active project');
        return false;
    }
    
    const project = ProjectState.currentProject;
    const guiJson = saveGUIToJSON();
    const mainGUIPath = getMainGUIPath(project.name);
    
    try {
        if (project.location === 'webdav') {
            const response = await fetch(`/webdav/gui/${mainGUIPath}`, {
                method: 'PUT',
                headers: { 'Content-Type': 'application/json' },
                body: guiJson
            });
            
            if (!response.ok) {
                logToConsole('Error: Failed to save GUI to project');
                return false;
            }
            
            // Update metadata
            const metaPath = getProjectMetadataPath(project.name);
            project.metadata.modified = new Date().toISOString();
            
            const metaResponse = await fetch(`/webdav/gui/${metaPath}`, {
                method: 'PUT',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(project.metadata, null, 2)
            });
            
            if (!metaResponse.ok) {
                logToConsole('Warning: Failed to update project metadata');
            }
            
            logToConsole(`Project "${project.name}" saved to WebDAV`);
            EditorState.isSaved = true;
            updateStatusBar();
            return true;
            
        } else {
            // Local save
            const dirHandle = ProjectState.localHandles?.[project.name];
            if (!dirHandle) {
                logToConsole('Error: Local project handle not found');
                return false;
            }
            
            // Save GUI
            const guiFile = await dirHandle.getFileHandle(
                ProjectState.DEFAULT_GUI_FILE,
                { create: true }
            );
            const guiWritable = await guiFile.createWritable();
            await guiWritable.write(guiJson);
            await guiWritable.close();
            
            // Update metadata
            project.metadata.modified = new Date().toISOString();
            const metaFile = await dirHandle.getFileHandle('project.meta.json');
            const metaWritable = await metaFile.createWritable();
            await metaWritable.write(JSON.stringify(project.metadata, null, 2));
            await metaWritable.close();
            
            logToConsole(`Project "${project.name}" saved locally`);
            EditorState.isSaved = true;
            updateStatusBar();
            return true;
        }
        
    } catch (error) {
        logToConsole(`Error saving project: ${error.message}`);
        return false;
    }
}

/**
 * Save current GUI as a new project
 * @param {string} projectName - Project name
 * @param {string} location - 'webdav' or 'local'
 * @returns {Promise<boolean>} - Success status
 */
async function saveAsProject(projectName, location = ProjectState.defaultLocation) {
    if (!projectName || projectName.trim() === '') {
        logToConsole('Error: Project name cannot be empty');
        return false;
    }
    
    // Create the project first
    const created = await createProject(projectName, location);
    if (!created) return false;
    
    // Now save current GUI to the project
    ProjectState.currentProject = {
        name: projectName,
        location: location,
        path: getProjectPath(projectName)
    };
    
    const saved = await saveProject();
    if (!saved) {
        // Cleanup if save failed
        ProjectState.currentProject = null;
        return false;
    }
    
    updateProjectUI();
    logToConsole(`GUI saved as project "${projectName}" to ${location}`);
    return true;
}

/**
 * Close current project
 */
function closeProject() {
    ProjectState.currentProject = null;
    EditorState.currentFile = null;
    updateProjectUI();
    logToConsole('Project closed');
}

/**
 * Load list of projects from a location
 * @param {string} location - 'webdav' or 'local'
 */
async function loadProjects(location) {
    try {
        if (location === 'webdav') {
            // List all directories ending with .GUIKIT
            const response = await fetch('/webdav/gui/');
            if (!response.ok) throw new Error('Failed to list files');
            
            const data = await response.text();
            let files = [];
            
            try {
                files = JSON.parse(data).files || JSON.parse(data);
            } catch {
                files = parseDirectoryListing(data, '/');
            }
            
            // Filter for .GUIKIT directories
            const projects = files.filter(f => 
                f.isDirectory && f.name.endsWith(ProjectState.PROJECT_SUFFIX)
            ).map(f => ({
                name: f.name.replace(ProjectState.PROJECT_SUFFIX, ''),
                fullName: f.name,
                path: f.path,
                location: 'webdav'
            }));
            
            ProjectState.projects.webdav = projects;
        } else {
            // Local projects - list all .GUIKIT directories
            if (window.showDirectoryPicker) {
                // For now, we need to track local projects differently
                // This will be populated when projects are created/opened
                // ProjectState.projects.local = [];
            }
        }
        
        logToConsole(`Loaded ${ProjectState.projects[location].length} projects from ${location}`);
        
    } catch (error) {
        logToConsole(`Error loading projects: ${error.message}`);
    }
}

/**
 * Delete a project
 * @param {string} projectName - Project name
 * @param {string} location - 'webdav' or 'local'
 * @returns {Promise<boolean>} - Success status
 */
async function deleteProject(projectName, location = ProjectState.defaultLocation) {
    if (!confirm(`Delete project "${projectName}" and all its contents? This cannot be undone.`)) {
        return false;
    }
    
    try {
        if (location === 'webdav') {
            const projectPath = getProjectPath(projectName);
            const response = await fetch(`/webdav/gui/${projectPath}`, {
                method: 'DELETE'
            });
            
            if (!response.ok) {
                logToConsole('Error: Failed to delete project');
                return false;
            }
            
            // Remove from list
            ProjectState.projects.webdav = ProjectState.projects.webdav.filter(
                p => p.name !== projectName
            );
            
            // Clear current project if it's the one being deleted
            if (ProjectState.currentProject?.name === projectName && 
                ProjectState.currentProject?.location === 'webdav') {
                closeProject();
            }
            
            logToConsole(`Project "${projectName}" deleted from WebDAV`);
            return true;
            
        } else {
            const dirHandle = ProjectState.localHandles?.[projectName];
            if (!dirHandle) {
                logToConsole('Error: Local project handle not found');
                return false;
            }
            
            // Remove all files and directories
            for await (const entry of dirHandle.values()) {
                if (entry.kind === 'file') {
                    await dirHandle.removeEntry(entry.name);
                } else if (entry.kind === 'directory') {
                    await dirHandle.removeEntry(entry.name, { recursive: true });
                }
            }
            
            // Remove the directory itself
            const parentHandle = await dirHandle.getParent();
            if (parentHandle) {
                await parentHandle.removeEntry(`${projectName}${ProjectState.PROJECT_SUFFIX}`, { recursive: true });
            }
            
            // Remove from handles
            delete ProjectState.localHandles[projectName];
            
            // Remove from list
            ProjectState.projects.local = ProjectState.projects.local.filter(
                p => p.name !== projectName
            );
            
            // Clear current project if it's the one being deleted
            if (ProjectState.currentProject?.name === projectName && 
                ProjectState.currentProject?.location === 'local') {
                closeProject();
            }
            
            logToConsole(`Project "${projectName}" deleted locally`);
            return true;
        }
        
    } catch (error) {
        logToConsole(`Error deleting project: ${error.message}`);
        return false;
    }
}

/**
 * Rename a project
 * @param {string} oldName - Current project name
 * @param {string} newName - New project name
 * @param {string} location - 'webdav' or 'local'
 * @returns {Promise<boolean>} - Success status
 */
async function renameProject(oldName, newName, location = ProjectState.defaultLocation) {
    if (!newName || newName.trim() === '') {
        logToConsole('Error: Project name cannot be empty');
        return false;
    }
    
    try {
        if (location === 'webdav') {
            const oldPath = getProjectPath(oldName);
            const newPath = getProjectPath(newName);
            
            // Use MOVE to rename the directory
            const response = await fetch(`/webdav/gui/${oldPath}`, {
                method: 'MOVE',
                headers: {
                    'Destination': `/webdav/gui/${newPath}`
                }
            });
            
            if (!response.ok) {
                logToConsole('Error: Failed to rename project directory');
                return false;
            }
            
            // Update project list
            const index = ProjectState.projects.webdav.findIndex(p => p.name === oldName);
            if (index !== -1) {
                ProjectState.projects.webdav[index].name = newName;
                ProjectState.projects.webdav[index].fullName = `${newName}${ProjectState.PROJECT_SUFFIX}`;
                ProjectState.projects.webdav[index].path = newPath;
            }
            
            // Update current project if it's the one being renamed
            if (ProjectState.currentProject?.name === oldName && 
                ProjectState.currentProject?.location === 'webdav') {
                ProjectState.currentProject.name = newName;
                ProjectState.currentProject.path = newPath;
                updateProjectUI();
            }
            
            logToConsole(`Project "${oldName}" renamed to "${newName}" on WebDAV`);
            return true;
            
        } else {
            const dirHandle = ProjectState.localHandles?.[oldName];
            if (!dirHandle) {
                logToConsole('Error: Local project handle not found');
                return false;
            }
            
            const parentHandle = await dirHandle.getParent();
            if (!parentHandle) {
                logToConsole('Error: Cannot access parent directory');
                return false;
            }
            
            // Get all entries
            const entries = [];
            for await (const entry of dirHandle.values()) {
                entries.push(entry);
            }
            
            // Create new directory
            const newDirHandle = await parentHandle.getDirectoryHandle(
                `${newName}${ProjectState.PROJECT_SUFFIX}`,
                { create: true }
            );
            
            // Move all entries to new directory
            for (const entry of entries) {
                const newFileHandle = await newDirHandle.getFileHandle(entry.name, { create: true });
                const newWritable = await newFileHandle.createWritable();
                
                if (entry.kind === 'file') {
                    const file = await entry.getFile();
                    const content = await file.text();
                    await newWritable.write(content);
                }
                
                await newWritable.close();
            }
            
            // Remove old directory
            await parentHandle.removeEntry(`${oldName}${ProjectState.PROJECT_SUFFIX}`, { recursive: true });
            
            // Update handles
            delete ProjectState.localHandles[oldName];
            ProjectState.localHandles[newName] = newDirHandle;
            
            // Update project list
            const index = ProjectState.projects.local.findIndex(p => p.name === oldName);
            if (index !== -1) {
                ProjectState.projects.local[index].name = newName;
            }
            
            // Update current project if it's the one being renamed
            if (ProjectState.currentProject?.name === oldName && 
                ProjectState.currentProject?.location === 'local') {
                ProjectState.currentProject.name = newName;
                ProjectState.currentProject.path = getProjectPath(newName);
                updateProjectUI();
            }
            
            logToConsole(`Project "${oldName}" renamed to "${newName}" locally`);
            return true;
        }
        
    } catch (error) {
        logToConsole(`Error renaming project: ${error.message}`);
        return false;
    }
}

// ============================================================================
// Project UI Updates
// ============================================================================

function updateProjectUI() {
    const projectInfoEl = document.getElementById('project-info');
    const projectNameEl = document.getElementById('project-name');
    const projectLocationEl = document.getElementById('project-location');
    
    if (ProjectState.currentProject) {
        const project = ProjectState.currentProject;
        if (projectInfoEl) {
            projectInfoEl.textContent = `Project: ${project.name} (${project.location})`;
        }
        if (projectNameEl) {
            projectNameEl.textContent = project.name;
        }
        if (projectLocationEl) {
            projectLocationEl.textContent = project.location.toUpperCase();
        }
    } else {
        if (projectInfoEl) {
            projectInfoEl.textContent = 'No project open';
        }
        if (projectNameEl) {
            projectNameEl.textContent = 'Untitled';
        }
    }
}

/**
 * Show project manager modal
 */
function showProjectManager() {
    const modal = document.getElementById('modal-projects');
    if (modal) {
        modal.classList.remove('hidden');
        refreshProjectList();
    }
}

function hideProjectManager() {
    const modal = document.getElementById('modal-projects');
    if (modal) {
        modal.classList.add('hidden');
    }
}

async function refreshProjectList() {
    await loadProjects('webdav');
    // Local projects will be shown when available
    renderProjectList();
}

function renderProjectList() {
    const webdavList = document.getElementById('project-list-webdav');
    const localList = document.getElementById('project-list-local');
    
    if (webdavList) {
        const projects = ProjectState.projects.webdav;
        let html = '';
        
        for (const project of projects) {
            html += `
                <div class="project-item" onclick="openProject('${project.name}', 'webdav')">
                    <span class="project-icon">📁</span>
                    <span class="project-name">${project.name}</span>
                    <span class="project-location">WebDAV</span>
                </div>
            `;
        }
        
        webdavList.innerHTML = html || '<div class="project-empty">No WebDAV projects found</div>';
    }
    
    if (localList) {
        const projects = ProjectState.projects.local;
        let html = '';
        
        for (const project of projects) {
            html += `
                <div class="project-item" onclick="openProject('${project.name}', 'local')">
                    <span class="project-icon">💻</span>
                    <span class="project-name">${project.name}</span>
                    <span class="project-location">Local</span>
                </div>
            `;
        }
        
        localList.innerHTML = html || '<div class="project-empty">No local projects found</div>';
    }
}

function showNewProjectModal() {
    const modal = document.getElementById('modal-new-project');
    if (modal) {
        const input = document.getElementById('new-project-name');
        if (input) input.value = '';
        
        // Set default location
        const locationSelect = document.getElementById('new-project-location');
        if (locationSelect) {
            locationSelect.value = ProjectState.defaultLocation;
        }
        
        modal.classList.remove('hidden');
    }
}

async function createNewProjectFromModal() {
    const input = document.getElementById('new-project-name');
    const locationSelect = document.getElementById('new-project-location');
    
    if (!input || !locationSelect) return;
    
    const name = input.value.trim();
    const location = locationSelect.value;
    
    if (!name) {
        logToConsole('Error: Project name cannot be empty');
        return;
    }
    
    const modal = document.getElementById('modal-new-project');
    if (modal) modal.classList.add('hidden');
    
    const created = await createProject(name, location);
    if (created) {
        // Optionally open the project
        await openProject(name, location);
    }
}

function showProjectSettingsModal() {
    const modal = document.getElementById('modal-project-settings');
    if (modal) {
        const locationSelect = document.getElementById('default-location-select');
        if (locationSelect) {
            locationSelect.value = ProjectState.defaultLocation;
        }
        
        modal.classList.remove('hidden');
    }
}

function setDefaultLocation(location) {
    ProjectState.defaultLocation = location;
    const select = document.getElementById('default-location-select');
    if (select) {
        select.value = location;
    }
    logToConsole(`Default location set to: ${location}`);
}

// ============================================================================
// Initialize
// ============================================================================

function initProjectManager() {
    // Load projects from default location
    loadProjects(ProjectState.defaultLocation);
    
    // Set up event listeners
    const newProjectBtn = document.getElementById('btn-new-project');
    if (newProjectBtn) {
        newProjectBtn.addEventListener('click', showNewProjectModal);
    }
    
    const projectManagerBtn = document.getElementById('btn-projects');
    if (projectManagerBtn) {
        projectManagerBtn.addEventListener('click', showProjectManager);
    }
    
    const saveProjectBtn = document.getElementById('btn-save-project');
    if (saveProjectBtn) {
        saveProjectBtn.addEventListener('click', saveProject);
    }
    
    const closeProjectBtn = document.getElementById('btn-close-project');
    if (closeProjectBtn) {
        closeProjectBtn.addEventListener('click', closeProject);
    }
    
    // Keyboard shortcuts
    document.addEventListener('keydown', handleProjectKeyDown);
    
    updateProjectUI();
}

function handleProjectKeyDown(event) {
    if (event.target.tagName === 'INPUT' || event.target.tagName === 'TEXTAREA') return;
    
    switch (event.key) {
        case 'p':
            if (event.ctrlKey) {
                showProjectManager();
                event.preventDefault();
            }
            break;
        case 's':
            if (event.ctrlKey && event.shiftKey && ProjectState.currentProject) {
                saveProject();
                event.preventDefault();
            }
            break;
    }
}

// Add to global scope
window.ProjectState = ProjectState;
window.createProject = createProject;
window.openProject = openProject;
window.saveProject = saveProject;
window.saveAsProject = saveAsProject;
window.closeProject = closeProject;
window.deleteProject = deleteProject;
window.renameProject = renameProject;
window.loadProjects = loadProjects;
window.setDefaultLocation = setDefaultLocation;
window.showProjectManager = showProjectManager;
window.showNewProjectModal = showNewProjectModal;
window.createNewProjectFromModal = createNewProjectFromModal;
window.showProjectSettingsModal = showProjectSettingsModal;
window.refreshProjectList = refreshProjectList;
