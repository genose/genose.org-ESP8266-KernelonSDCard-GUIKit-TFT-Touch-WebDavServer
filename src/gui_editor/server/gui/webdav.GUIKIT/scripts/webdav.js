/**
 * WebDAV Server Management Script
 * 
 * Handles WebDAV connection management on the ESP8266
 * Allows connecting, disconnecting, testing, and browsing WebDAV servers
 * Each authenticated user gets their home directory as the WebDAV root
 */

// ============================================================================
// Global State
// ============================================================================

var WebDAVState = {
    connected: false,
    serverUrl: 'http://esp8266.local:8080/webdav',
    username: 'admin',
    password: '',
    userHome: '/home/admin',
    fileCount: 0,
    dirCount: 0,
    files: [],
    currentPath: '/',
    connectionError: null
};

// ============================================================================
// Initialization
// ============================================================================

function webdav_init() {
    // Load saved settings if available
    loadSettings();
    
    // Update UI with current settings
    updateConnectionUI();
    
    // Test connection on init
    webdav_test();
}

/**
 * Load saved settings from storage
 */
function loadSettings() {
    if (typeof Storage !== 'undefined') {
        WebDAVState.serverUrl = localStorage.getItem('webdav_server_url') || WebDAVState.serverUrl;
        WebDAVState.username = localStorage.getItem('webdav_username') || WebDAVState.username;
        WebDAVState.password = localStorage.getItem('webdav_password') || WebDAVState.password;
        WebDAVState.userHome = localStorage.getItem('webdav_user_home') || WebDAVState.userHome;
    }
}

/**
 * Save settings to storage
 */
function saveSettings() {
    if (typeof Storage !== 'undefined') {
        localStorage.setItem('webdav_server_url', WebDAVState.serverUrl);
        localStorage.setItem('webdav_username', WebDAVState.username);
        localStorage.setItem('webdav_password', WebDAVState.password);
        localStorage.setItem('webdav_user_home', WebDAVState.userHome);
    }
}

// ============================================================================
// UI Update Functions
// ============================================================================

function updateConnectionUI() {
    var serverUrl = GUI.getWidgetById('server_url');
    var username = GUI.getWidgetById('username');
    var password = GUI.getWidgetById('password');
    
    if (serverUrl) serverUrl.text = WebDAVState.serverUrl;
    if (username) username.text = WebDAVState.username;
    if (password) password.text = WebDAVState.password ? '****' : '';
    
    updateStatusUI();
}

function updateStatusUI() {
    var statusIndicator = GUI.getWidgetById('status_indicator');
    var statusText = GUI.getWidgetById('status_text');
    var filesText = GUI.getWidgetById('files_text');
    
    if (statusIndicator) {
        statusIndicator.text = 'Status: ' + (WebDAVState.connected ? 'ON' : 'OFF');
        statusIndicator.text_color = WebDAVState.connected ? '#4CAF50' : '#FF4444';
    }
    
    if (statusText) {
        if (WebDAVState.connected) {
            statusText.text = 'Connected to ' + WebDAVState.serverUrl;
            statusText.text_color = '#4CAF50';
        } else if (WebDAVState.connectionError) {
            statusText.text = 'Error: ' + WebDAVState.connectionError;
            statusText.text_color = '#FF4444';
        } else {
            statusText.text = 'Not connected';
            statusText.text_color = '#858585';
        }
    }
    
    if (filesText) {
        filesText.text = WebDAVState.fileCount + ' files, ' + WebDAVState.dirCount + ' directories';
    }
}

// ============================================================================
// Connection Functions
// ============================================================================

/**
 * Connect to WebDAV server
 */
function webdav_connect(widget, event) {
    var serverUrlWidget = GUI.getWidgetById('server_url');
    var usernameWidget = GUI.getWidgetById('username');
    var passwordWidget = GUI.getWidgetById('password');
    
    if (serverUrlWidget) WebDAVState.serverUrl = serverUrlWidget.text;
    if (usernameWidget) WebDAVState.username = usernameWidget.text;
    if (passwordWidget) WebDAVState.password = passwordWidget.text;
    
    saveSettings();
    
    // Look up user home directory from UserManager
    setUserHomeDirectory(WebDAVState.username);
    
    // Attempt connection
    if (typeof WebDAVClient !== 'undefined') {
        // Set user home as the root path for this connection
        var userRoot = WebDAVState.userHome;
        
        WebDAVClient.connect(WebDAVState.serverUrl, WebDAVState.username, WebDAVState.password, userRoot)
            .then(function(success) {
                WebDAVState.connected = success;
                WebDAVState.connectionError = null;
                updateStatusUI();
                
                if (success) {
                    // List files to get counts
                    listWebDAVFiles('/');
                }
            })
            .catch(function(error) {
                WebDAVState.connected = false;
                WebDAVState.connectionError = error.message || 'Connection failed';
                updateStatusUI();
            });
    } else {
        // Simulate connection for testing
        WebDAVState.connected = true;
        WebDAVState.connectionError = null;
        WebDAVState.fileCount = 0;
        WebDAVState.dirCount = 0;
        updateStatusUI();
    }
}

/**
 * Set user home directory based on username
 * Looks up the user's home from UserManager or uses default
 * @param {string} username - The username
 */
function setUserHomeDirectory(username) {
    if (typeof UserManager !== 'undefined') {
        var user = UserManager.getByUsername(username);
        if (user && user.home) {
            WebDAVState.userHome = user.home;
        } else {
            // Default home directory
            WebDAVState.userHome = '/home/' + username;
        }
    } else {
        // Default home directory
        WebDAVState.userHome = '/home/' + username;
    }
}

/**
 * Disconnect from WebDAV server
 */
function webdav_disconnect(widget, event) {
    if (typeof WebDAVClient !== 'undefined') {
        WebDAVClient.disconnect();
    }
    
    WebDAVState.connected = false;
    WebDAVState.connectionError = null;
    WebDAVState.fileCount = 0;
    WebDAVState.dirCount = 0;
    WebDAVState.files = [];
    
    updateStatusUI();
}

/**
 * Test WebDAV connection
 */
function webdav_test(widget, event) {
    if (!WebDAVState.connected) {
        webdav_connect(null, null);
        return;
    }
    
    // List root directory to test connection
    listWebDAVFiles('/');
}

/**
 * List files from WebDAV server
 */
function listWebDAVFiles(path) {
    if (typeof WebDAVClient !== 'undefined') {
        WebDAVClient.listFiles(path)
            .then(function(files) {
                WebDAVState.files = files;
                WebDAVState.fileCount = files.filter(function(f) { return !f.isDirectory; }).length;
                WebDAVState.dirCount = files.filter(function(f) { return f.isDirectory; }).length;
                updateStatusUI();
            })
            .catch(function(error) {
                WebDAVState.connectionError = error.message || 'Failed to list files';
                updateStatusUI();
            });
    }
}

/**
 * Browse WebDAV files
 */
function webdav_browse(widget, event) {
    if (!WebDAVState.connected) {
        webdav_connect(null, null);
        return;
    }
    
    // Launch file browser
    if (typeof GUIKitChooser !== 'undefined') {
        // Open a file browser GUI
        GUIKitChooser.loadProject('file_browser');
    } else {
        // Show message
        var infoText = GUI.getWidgetById('info_text');
        if (infoText) {
            infoText.text = 'Browsing: ' + WebDAVState.serverUrl + WebDAVState.currentPath;
        }
        
        // List files
        listWebDAVFiles(WebDAVState.currentPath);
    }
}

// ============================================================================
// Update on Show
// ============================================================================

function webdav_update_status() {
    updateConnectionUI();
}

// ============================================================================
// Global API
// ============================================================================

var WebDAVManager = {
    connect: webdav_connect,
    disconnect: webdav_disconnect,
    test: webdav_test,
    browse: webdav_browse,
    isConnected: function() { return WebDAVState.connected; },
    getServerUrl: function() { return WebDAVState.serverUrl; },
    setServerUrl: function(url) { WebDAVState.serverUrl = url; saveSettings(); },
    getCredentials: function() { 
        return { username: WebDAVState.username, password: WebDAVState.password }; 
    },
    setCredentials: function(username, password) {
        WebDAVState.username = username;
        WebDAVState.password = password;
        setUserHomeDirectory(username);
        saveSettings();
    },
    getUserHome: function() { return WebDAVState.userHome; },
    setUserHome: function(path) { WebDAVState.userHome = path; saveSettings(); },
    setUser: function(username, password, home) {
        WebDAVState.username = username;
        WebDAVState.password = password;
        if (home) {
            WebDAVState.userHome = home;
        } else {
            setUserHomeDirectory(username);
        }
        saveSettings();
    }
};

// Make available globally
if (typeof window !== 'undefined') {
    window.WebDAVManager = WebDAVManager;
}
if (typeof global !== 'undefined') {
    global.WebDAVManager = WebDAVManager;
}
