/**
 * User Management Script
 * 
 * Handles user account management on the ESP8266
 * Allows creating, editing, deleting users and managing permissions
 */

// ============================================================================
// Global State
// ============================================================================

var UsersState = {
    users: [],              // List of all users
    filteredUsers: [],     // Users after search filter
    selectedIndex: -1,      // Currently selected user index
    selectedUser: null,    // Currently selected username
    searchFilter: '',       // Current search filter text
    editingUser: null,     // User being edited (temporary storage)
    newUser: {
        username: '',
        password: '',
        confirmPassword: '',
        permissions: 'user'
    },
    permissions: ['admin', 'user', 'guest']
};

// ============================================================================
// Initialization
// ============================================================================

function users_init() {
    // Load users from storage
    loadUsers();
    
    // Initialize UI
    updateUserCount();
    refreshUsers();
}

/**
 * Load users from storage
 */
function loadUsers() {
    if (typeof UserStorage !== 'undefined') {
        UsersState.users = UserStorage.getAllUsers();
    } else if (typeof Storage !== 'undefined') {
        // Try localStorage fallback
        var usersJson = localStorage.getItem('guikit_users');
        if (usersJson) {
            UsersState.users = JSON.parse(usersJson);
        }
    }
    
    // If no users, create default admin
    if (UsersState.users.length === 0) {
        UsersState.users.push({
            username: 'admin',
            password: 'admin',  // In real implementation, this would be hashed
            permissions: 'admin',
            created: new Date().toISOString(),
            lastLogin: null
        });
        saveUsers();
    }
    
    UsersState.filteredUsers = UsersState.users;
}

/**
 * Save users to storage
 */
function saveUsers() {
    if (typeof UserStorage !== 'undefined') {
        UserStorage.saveAllUsers(UsersState.users);
    } else if (typeof Storage !== 'undefined') {
        localStorage.setItem('guikit_users', JSON.stringify(UsersState.users));
    }
}

// ============================================================================
// UI Update Functions
// ============================================================================

function updateUserCount() {
    var userCount = GUI.getWidgetById('user_count');
    if (userCount) {
        userCount.text = UsersState.users.length + ' user' + (UsersState.users.length !== 1 ? 's' : '');
    }
}

function updateInfoText(message) {
    var infoText = GUI.getWidgetById('info_text');
    if (infoText) {
        infoText.text = message;
    }
}

// ============================================================================
// User List Management
// ============================================================================

/**
 * Refresh the user list display
 */
function user_refresh(widget, event) {
    refreshUsers();
}

/**
 * Internal: Refresh users list
 */
function refreshUsers() {
    // Apply search filter
    var searchInput = GUI.getWidgetById('search_input');
    if (searchInput) {
        UsersState.searchFilter = searchInput.text.toLowerCase();
    }
    
    UsersState.filteredUsers = UsersState.users.filter(function(user) {
        return user.username.toLowerCase().includes(UsersState.searchFilter);
    });
    
    // Clear existing user buttons
    var usersList = GUI.getWidgetById('users_list');
    if (usersList) {
        usersList.children = [];
    }
    
    // Create buttons for each user
    for (var i = 0; i < UsersState.filteredUsers.length; i++) {
        var user = UsersState.filteredUsers[i];
        
        var userBtn = {
            id: 'user_' + i,
            type: 'button',
            x: 5,
            y: i * 26,
            width: 290,
            height: 22,
            text: user.username + ' (' + user.permissions + ')',
            text_color: '#FFFFFF',
            background: '#2D2D2D',
            border_color: '#3E3E42',
            border_width: 1,
            text_size: 12,
            text_align: 'left',
            action: 'user_select',
            data: { index: i }
        };
        
        usersList.children.push(userBtn);
    }
    
    // Update info text
    updateInfoText(UsersState.filteredUsers.length + ' user(s) found');
    
    // Disable edit/delete buttons
    disableUserActionButtons();
    
    GUI.redraw();
}

/**
 * Disable user action buttons
 */
function disableUserActionButtons() {
    var editBtn = GUI.getWidgetById('edit_user_btn');
    var deleteBtn = GUI.getWidgetById('delete_user_btn');
    
    var disabled = UsersState.selectedIndex === -1 || UsersState.selectedUser === null;
    
    if (editBtn) editBtn.disabled = disabled;
    if (deleteBtn) deleteBtn.disabled = disabled;
}

// ============================================================================
// User Selection
// ============================================================================

/**
 * Select a user from the list
 */
function user_select(widget, event) {
    if (widget.data && widget.data.index !== undefined) {
        var index = widget.data.index;
        
        // Clear previous selection
        for (var i = 0; i < UsersState.filteredUsers.length; i++) {
            var btn = GUI.getWidgetById('user_' + i);
            if (btn) {
                btn.background = '#2D2D2D';
                btn.border_color = '#3E3E42';
            }
        }
        
        // Select new user
        UsersState.selectedIndex = index;
        UsersState.selectedUser = UsersState.filteredUsers[index].username;
        
        var selectedBtn = GUI.getWidgetById('user_' + index);
        if (selectedBtn) {
            selectedBtn.background = '#4B0082';
            selectedBtn.border_color = '#36005D';
        }
        
        // Enable action buttons
        var editBtn = GUI.getWidgetById('edit_user_btn');
        var deleteBtn = GUI.getWidgetById('delete_user_btn');
        
        if (editBtn) editBtn.disabled = false;
        if (deleteBtn) deleteBtn.disabled = false;
        
        // Update info text
        updateInfoText('Selected: ' + UsersState.selectedUser);
        
        GUI.redraw();
    }
}

// ============================================================================
// User CRUD Operations
// ============================================================================

/**
 * Show new user dialog
 */
function user_new(widget, event) {
    // Reset new user form
    UsersState.editingUser = null;
    UsersState.newUser = {
        username: '',
        password: '',
        confirmPassword: '',
        permissions: 'user'
    };
    
    // In a real implementation, this would open a modal dialog
    // For now, we'll add a new user with default values
    showNewUserDialog();
}

/**
 * Show new user dialog (simulated)
 */
function showNewUserDialog() {
    // This would normally open a modal with form fields
    // For this implementation, we'll use the GUI to show a message
    updateInfoText('Enter new user details (simulated)');
    
    // Pre-fill with a new username
    UsersState.newUser.username = 'new_user_' + (UsersState.users.length + 1);
    UsersState.newUser.password = 'password';
    UsersState.newUser.confirmPassword = 'password';
    UsersState.newUser.permissions = 'user';
    
    // In real implementation, show modal with:
    // - Username input
    // - Password input
    // - Confirm password input
    // - Permissions dropdown (admin, user, guest)
    // - Create/Cancel buttons
    
    // For now, auto-create with defaults
    createNewUser();
}

/**
 * Create a new user
 */
function createNewUser() {
    var newUser = UsersState.newUser;
    
    // Validate
    if (!newUser.username || newUser.username.trim() === '') {
        updateInfoText('Error: Username is required');
        return;
    }
    
    if (newUser.password !== newUser.confirmPassword) {
        updateInfoText('Error: Passwords do not match');
        return;
    }
    
    if (newUser.password.length < 4) {
        updateInfoText('Error: Password must be at least 4 characters');
        return;
    }
    
    // Check if username exists
    for (var i = 0; i < UsersState.users.length; i++) {
        if (UsersState.users[i].username === newUser.username) {
            updateInfoText('Error: Username already exists');
            return;
        }
    }
    
    // Create user
    var user = {
        username: newUser.username.trim(),
        password: newUser.password,  // In real implementation, hash this
        permissions: newUser.permissions,
        home: '/home/' + newUser.username.trim(),
        created: new Date().toISOString(),
        lastLogin: null
    };
    
    UsersState.users.push(user);
    saveUsers();
    
    // Create user home directory from skeleton
    createUserHomeDirectory(user.username, user.home);
    
    // Reset new user
    UsersState.newUser = {
        username: '',
        password: '',
        confirmPassword: '',
        permissions: 'user'
    };
    
    // Refresh list
    refreshUsers();
    updateUserCount();
    updateInfoText('User created: ' + user.username + ' with home: ' + user.home);
}

/**
 * Create user home directory from /etc/user.skel/ template
 * @param {string} username - The username
 * @param {string} homePath - The home directory path (e.g., /home/username)
 */
function createUserHomeDirectory(username, homePath) {
    if (typeof FileSystem !== 'undefined') {
        // Using FileSystem API (ESP8266 SD card filesystem)
        try {
            // Create home directory
            FileSystem.mkdir(homePath);
            
            // Copy contents from /etc/user.skel/ to home directory
            var skelPath = '/etc/user.skel/';
            var skelFiles = FileSystem.list(skelPath);
            
            if (skelFiles && skelFiles.length > 0) {
                for (var i = 0; i < skelFiles.length; i++) {
                    var file = skelFiles[i];
                    var sourcePath = skelPath + file.name;
                    var destPath = homePath + '/' + file.name;
                    
                    if (file.isDirectory) {
                        // Recursively copy directories
                        copyDirectoryRecursive(sourcePath, destPath);
                    } else {
                        // Copy file
                        var content = FileSystem.read(sourcePath);
                        if (content !== null) {
                            FileSystem.write(destPath, content);
                        }
                    }
                }
            }
            
            // Ensure projects directory exists
            var projectsPath = homePath + '/projects';
            if (!FileSystem.exists(projectsPath)) {
                FileSystem.mkdir(projectsPath);
            }
            
            return true;
        } catch (error) {
            updateInfoText('Warning: Failed to create home dir: ' + error.message);
            return false;
        }
    } else if (typeof FS !== 'undefined') {
        // Alternative filesystem API
        try {
            FS.mkdir(homePath);
            copySkeletonToHome(username, homePath);
            return true;
        } catch (error) {
            updateInfoText('Warning: Failed to create home dir: ' + error.message);
            return false;
        }
    } else {
        // Simulated/fallback - log the action
        updateInfoText('Home directory would be created: ' + homePath + ' (from /etc/user.skel/)');
        return false;
    }
}

/**
 * Recursively copy a directory
 * @param {string} sourcePath - Source directory path
 * @param {string} destPath - Destination directory path
 */
function copyDirectoryRecursive(sourcePath, destPath) {
    if (typeof FileSystem !== 'undefined') {
        try {
            FileSystem.mkdir(destPath);
            var files = FileSystem.list(sourcePath);
            
            if (files && files.length > 0) {
                for (var i = 0; i < files.length; i++) {
                    var file = files[i];
                    var sourceFile = sourcePath + '/' + file.name;
                    var destFile = destPath + '/' + file.name;
                    
                    if (file.isDirectory) {
                        copyDirectoryRecursive(sourceFile, destFile);
                    } else {
                        var content = FileSystem.read(sourceFile);
                        if (content !== null) {
                            FileSystem.write(destFile, content);
                        }
                    }
                }
            }
        } catch (error) {
            // Log but don't fail - some files may not copy
        }
    }
}

/**
 * Copy skeleton contents to user home directory (alternative implementation)
 * @param {string} username - The username
 * @param {string} homePath - The home directory path
 */
function copySkeletonToHome(username, homePath) {
    // This function provides alternative implementation for different filesystem APIs
    var skelPath = '/etc/user.skel/';
    
    // Create standard structure
    var projectsPath = homePath + '/projects';
    
    // Copy README.md from skeleton
    if (typeof FS !== 'undefined') {
        if (FS.exists(skelPath + 'README.md')) {
            var readmeContent = FS.read(skelPath + 'README.md');
            FS.write(homePath + '/README.md', readmeContent);
        }
        FS.mkdir(projectsPath);
    }
}

/**
 * Edit selected user
 */
function user_edit(widget, event) {
    if (UsersState.selectedIndex === -1 || !UsersState.selectedUser) {
        return;
    }
    
    var user = UsersState.filteredUsers[UsersState.selectedIndex];
    UsersState.editingUser = JSON.parse(JSON.stringify(user));
    
    // In real implementation, open edit dialog with:
    // - Username (read-only or changeable)
    // - New password input
    // - Confirm new password input
    // - Permissions dropdown
    // - Save/Cancel buttons
    
    // For now, show message
    updateInfoText('Editing user: ' + user.username);
    
    // Simulate editing
    showEditUserDialog();
}

/**
 * Show edit user dialog (simulated)
 */
function showEditUserDialog() {
    // For now, just cycle through permissions
    var user = UsersState.filteredUsers[UsersState.selectedIndex];
    var currentPermIndex = UsersState.permissions.indexOf(user.permissions);
    var newPermIndex = (currentPermIndex + 1) % UsersState.permissions.length;
    user.permissions = UsersState.permissions[newPermIndex];
    
    saveUsers();
    refreshUsers();
    updateInfoText('Updated: ' + user.username + ' -> ' + user.permissions);
}

/**
 * Delete selected user
 */
function user_delete(widget, event) {
    if (UsersState.selectedIndex === -1 || !UsersState.selectedUser) {
        return;
    }
    
    var user = UsersState.filteredUsers[UsersState.selectedIndex];
    
    // Prevent deleting the last admin
    var adminCount = UsersState.users.filter(function(u) { return u.permissions === 'admin'; }).length;
    if (user.permissions === 'admin' && adminCount <= 1) {
        updateInfoText('Error: Cannot delete the only admin user');
        return;
    }
    
    // Confirm deletion (in real implementation, show confirmation dialog)
    // For now, just delete
    var actualIndex = UsersState.users.findIndex(function(u) { 
        return u.username === user.username; 
    });
    
    if (actualIndex !== -1) {
        UsersState.users.splice(actualIndex, 1);
        saveUsers();
        
        // Clear selection
        UsersState.selectedIndex = -1;
        UsersState.selectedUser = null;
        
        refreshUsers();
        updateUserCount();
        updateInfoText('User deleted: ' + user.username);
    }
}

// ============================================================================
// Search Functionality
// ============================================================================

/**
 * Handle search input changes
 */
function handleSearchInput(widget, event) {
    if (widget.id === 'search_input') {
        refreshUsers();
    }
}

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * Get user by username
 */
function getUserByUsername(username) {
    for (var i = 0; i < UsersState.users.length; i++) {
        if (UsersState.users[i].username === username) {
            return UsersState.users[i];
        }
    }
    return null;
}

/**
 * Get all users
 */
function getAllUsers() {
    return UsersState.users;
}

/**
 * Get user count
 */
function getUserCount() {
    return UsersState.users.length;
}

// ============================================================================
// Global API
// ============================================================================

var UserManager = {
    init: users_init,
    refresh: refreshUsers,
    getAll: getAllUsers,
    getCount: getUserCount,
    getByUsername: getUserByUsername,
    create: createNewUser,
    edit: user_edit,
    remove: user_delete,
    getSelected: function() { return UsersState.selectedUser; },
    setSelected: function(username) {
        UsersState.selectedUser = username;
        UsersState.selectedIndex = UsersState.filteredUsers.findIndex(function(u) {
            return u.username === username;
        });
    },
    add: function(user) {
        UsersState.users.push(user);
        saveUsers();
        refreshUsers();
        updateUserCount();
    },
    update: function(username, updates) {
        var user = getUserByUsername(username);
        if (user) {
            for (var key in updates) {
                user[key] = updates[key];
            }
            saveUsers();
            refreshUsers();
        }
    }
};

// Make available globally
if (typeof window !== 'undefined') {
    window.UserManager = UserManager;
}
if (typeof global !== 'undefined') {
    global.UserManager = UserManager;
}
