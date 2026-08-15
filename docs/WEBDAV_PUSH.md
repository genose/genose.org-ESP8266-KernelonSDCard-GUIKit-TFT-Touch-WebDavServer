# WebDAV Push Notification System

## Overview

The **WebDAV Push Notification System** enables real-time notifications from the ESP8266/ESP32 device to Linux/macOS WebDAV clients. When files or folders change on the device, connected clients receive immediate notifications.

## Features

- **Multiple Transport Mechanisms**
  - Server-Sent Events (SSE) for modern HTTP clients
  - WebSocket for persistent connections
  - Long polling for legacy client compatibility

- **Event Types**
  - File created, modified, deleted, moved
  - Folder created, deleted
  - Project updates
  - GUI updates
  - Custom events

- **Path Watching**
  - Watch specific paths for changes
  - Recursive directory watching
  - Event type filtering

- **Low Memory Footprint**
  - Supports up to 8 concurrent clients
  - JSON-based notifications (~300 bytes each)
  - Minimal RAM usage

## Use Cases

1. **Linux clients (davfs2, nautilus, etc.)**: Receive notifications when mounted WebDAV share changes
2. **macOS Finder**: Real-time updates when WebDAV files change
3. **Custom applications**: Build real-time file monitoring apps
4. **GUIKit Editor**: Live preview of GUI changes
5. **Project collaboration**: Multiple users see changes immediately

---

## File Structure

```
src/gui_editor/server/
├── webdav_push.h           # Header with types and API
├── webdav_push.c           # Implementation
├── webdav_push_auth.h      # Authentication header
└── webdav_push_auth.c      # Authentication implementation

docs/
└── WEBDAV_PUSH.md          # This documentation
```

---

## API Reference

### Configuration

```c
#include "webdav_push.h"

// Default configuration
WebDAVPushConfig config = {
    .enabled = true,
    .port = 8080,
    .use_sse = true,
    .use_websocket = true,
    .use_long_poll = true,
    .watch_count = 0,
};

// Initialize with custom config
webdav_push_init(&config);

// Initialize with defaults
webdav_push_init(NULL);

// Shutdown
webdav_push_shutdown();

// Check if enabled
if (webdav_push_is_enabled()) {
    // System is ready
}
```

### Event Types

```c
typedef enum {
    WEBDAV_PUSH_FILE_CREATED,      // New file created
    WEBDAV_PUSH_FILE_MODIFIED,    // File modified
    WEBDAV_PUSH_FILE_DELETED,     // File deleted
    WEBDAV_PUSH_FILE_MOVED,       // File moved/renamed
    WEBDAV_PUSH_FOLDER_CREATED,   // New folder created
    WEBDAV_PUSH_FOLDER_DELETED,  // Folder deleted
    WEBDAV_PUSH_PROJECT_UPDATED,  // Project file changed
    WEBDAV_PUSH_GUI_UPDATED,      // GUI definition changed
    WEBDAV_PUSH_SYSTEM_EVENT,     // System event
    WEBDAV_PUSH_CUSTOM,           // Custom user-defined event
} WebDAVPushEventType;
```

### Notification Structure

```c
typedef struct {
    WebDAVPushEventType type;      // Event type
    uint64_t timestamp;            // Milliseconds since boot
    char path[256];               // File/folder path
    char old_path[256];           // Old path (for move/rename)
    uint32_t size;                 // File size (for create/modify)
    uint32_t client_id;            // Target client ID (0 = all)
    char custom_data[64];         // Custom data
} WebDAVPushNotification;
```

### Client Connection Types

```c
typedef enum {
    WEBDAV_PUSH_CLIENT_SSE,       // Server-Sent Events
    WEBDAV_PUSH_CLIENT_WEBSOCKET, // WebSocket
    WEBDAV_PUSH_CLIENT_LONG_POLL, // Long polling
} WebDAVPushClientType;
```

### Client Management

```c
// Register a client
uint32_t client_id = webdav_push_client_register(
    WEBDAV_PUSH_CLIENT_SSE,       // Connection type
    "/projects/my_project"       // Subscription path (empty = all)
);

// Unregister a client
webdav_push_client_unregister(client_id);

// Get client info
WebDAVPushClient* client = webdav_push_client_get(client_id);

// Ping client (keep-alive)
webdav_push_client_ping(client_id);

// Cleanup disconnected clients
webdav_push_client_cleanup();
```

### Sending Notifications

```c
// Manual notification
WebDAVPushNotification notif = {
    .type = WEBDAV_PUSH_FILE_CREATED,
    .timestamp = millis(),
    .path = "/projects/my_project/file.txt",
    .size = 1024,
};
webdav_push_notify(&notif);

// Convenience: File change
webdav_push_notify_file_change(
    "/projects/my_project/file.txt",
    WEBDAV_PUSH_FILE_CREATED
);

// Convenience: Project update
webdav_push_notify_project_update("/projects/my_project");

// Convenience: GUI update
webdav_push_notify_gui_update("/gui/chooser.GUIKIT");

// To specific client
webdav_push_notify_client(client_id, &notif);
```

### Path Watching

```c
// Watch a path for specific events
webdav_push_watch_add(
    "/projects",           // Path to watch
    true,                 // Recursive (watch subdirectories)
    WEBDAV_PUSH_FILE_CREATED | WEBDAV_PUSH_FILE_MODIFIED
);

// Stop watching a path
webdav_push_watch_remove("/projects");

// Check if path is watched
if (webdav_push_watch_is_watched("/projects/my_project")) {
    // Path is being monitored
}

// Check for changes (call periodically)
webdav_push_check_changes();
```

### Transport-Specific Functions

#### Server-Sent Events (SSE)

```c
// Get SSE stream headers
const char* headers = webdav_push_sse_stream(client_id);

// Send SSE event
webdav_push_sse_send_event(
    client_id,
    "file_created",
    "{\"path\":\"/file.txt\",\"size\":1024}"
);
```

SSE Output Format:
```
event: file_created
data: {"path":"/file.txt","size":1024}

```

#### WebSocket

```c
// Handle connection
webdav_push_websocket_connect(client_id);

// Handle disconnect
webdav_push_websocket_disconnect(client_id);

// Send message
webdav_push_websocket_send(client_id, "{\"event\":\"file_created\"}");
```

#### Long Polling

```c
// Handle request (returns JSON array)
const char* response = webdav_push_long_poll(client_id);
// Returns: [] or [{"type":"file_created",...}]

// Queue notification for client
webdav_push_long_poll_queue(client_id, &notification);
```

---

## Linux Client Setup

### Using `davfs2` with Inotify

Since `davfs2` doesn't natively support push notifications, you can use a helper script:

```bash
#!/bin/bash
# Monitor WebDAV share for changes using inotify

SHARE_MOUNT="/mnt/webdav"
WEBDAV_URL="http://esp8266.local/webdav"

# Mount WebDAV
mount.davfs $WEBDAV_URL $SHARE_MOUNT

# Monitor for changes
inotifywait -m -r -e create -e modify -e delete -e move $SHARE_MOUNT | \
while read path action file; do
    echo "Change detected: $action $file"
    # Trigger your application
    ./on_webdav_change.sh "$path$file" "$action"
done
```

### Using `rclone` with Polling

```bash
# Sync with polling (rclone can detect changes)
rclone sync --poll-interval 1s webdav: /local/path
```

---

## macOS Client Setup

### Using Finder

1. Connect to WebDAV: `Go > Connect to Server > http://esp8266.local/webdav`
2. Finder will automatically refresh when files change (with push notifications)

### Using Custom Script

```bash
#!/bin/bash
# macOS: Use fswatch to monitor WebDAV mount

SHARE_MOUNT="/Volumes/WebDAV"

fswatch -r -l 1.0 $SHARE_MOUNT | \
while read path; do
    echo "Change detected: $path"
    # Trigger your application
    osascript -e "display notification \"WebDAV file changed: $path\" with title \"GUIKit\""
done
```

---

## Notification Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                        ESP8266/ESP32 Device                        │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────────┐  │
│  │ File System │───▶│  Watch      │───▶│ WebDAV Push     │  │
│  │  /gui       │    │  Manager    │    │  Notification    │  │
│  │  /projects  │    │             │    │  System          │  │
│  └─────────────┘    └─────────────┘    └────────┬────────┘  │
│                                                   │            │
│                                                   ▼            │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │                 Transport Layer                           │  │
│  │  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐   │  │
│  │  │    SSE       │ │  WebSocket   │ │ Long Polling │   │  │
│  │  │  (HTTP)      │ │  (Persistent) │ │  (Fallback)  │   │  │
│  │  └──────────────┘ └──────────────┘ └──────────────┘   │  │
│  └─────────────────────────────────────────────────────────┘  │
│                              │                                   │
│                              ▼                                   │
└──────────────────────────────┬──────────────────────────────────┘
                               │
                  ┌────────────▼────────────┐
                  │        Clients          │
                  │  ┌────────────────────┐ │
                  │  │  Linux (davfs2)     │ │
                  │  │  macOS (Finder)      │ │
                  │  │  Custom Apps        │ │
                  │  └────────────────────┘ │
                  └──────────────────────────┘
```

---

## JSON Notification Format

All notifications are sent as JSON objects:

```json
{
  "type": "file_created",
  "timestamp": 1234567890,
  "path": "/projects/my_project/file.txt",
  "old_path": "",
  "size": 1024,
  "custom": ""
}
```

### SSE Format

```
event: file_created
data: {"type":"file_created","timestamp":1234567890,"path":"/projects/my_project/file.txt","old_path":"","size":1024,"custom":""}

```

### WebSocket Format

```json
{"type":"file_created","timestamp":1234567890,"path":"/projects/my_project/file.txt","old_path":"","size":1024,"custom":""}
```

### Long Polling Format

```json
[{"type":"file_created","timestamp":1234567890,"path":"/projects/my_project/file.txt","old_path":"","size":1024,"custom":""}]
```

---

## Default Watched Paths

The system automatically watches:

| Path | Recursive | Events |
|------|-----------|--------|
| `/gui` | Yes | File create/modify/delete |
| `/projects` | Yes | File create/modify/delete |
| `/tmp/task_comm` | No | File create |

---

## Memory Usage

| Component | Size | Notes |
|-----------|------|-------|
| Client state (per client) | ~300 bytes | 8 max clients |
| Notification struct | ~600 bytes | Temporary |
| Watch config | ~50 bytes per watch | 16 max watches |
| **Total (max)** | **~3KB** | All clients + watches |

---

## Integration with GUIKit

### Automatic Notifications

The WebDAV push system automatically notifies when:
- GUI files are updated
- Project files change
- Task communication files are created

### Manual Integration

```c
// In your code when a file changes:
webdav_push_notify_file_change("/gui/my_gui.GUIKIT", WEBDAV_PUSH_FILE_MODIFIED);

// Or let the watch system detect it automatically:
webdav_push_check_changes();  // Call in main loop
```

---

## Authentication System

The WebDAV Push Notification System includes a comprehensive authentication system for securing push notifications to clients.

### Authentication Methods

The system supports multiple authentication methods:

```c
typedef enum {
    WEBDAV_PUSH_AUTH_BASIC = 0,       // HTTP Basic Auth (username/password)
    WEBDAV_PUSH_AUTH_TOKEN,          // Bearer token
    WEBDAV_PUSH_AUTH_DIGEST,         // HTTP Digest Auth
    WEBDAV_PUSH_AUTH_ANONYMOUS,      // No authentication (read-only)
} WebDAVPushAuthMethod;
```

### Permission System

Permission flags control what authenticated clients can do:

```c
typedef enum {
    WEBDAV_PUSH_PERM_READ = 1 << 0,    // Can receive notifications
    WEBDAV_PUSH_PERM_WRITE = 1 << 1,   // Can subscribe to paths
    WEBDAV_PUSH_PERM_ADMIN = 1 << 2,   // Can manage watches
    WEBDAV_PUSH_PERM_ALL = 0xFF,      // All permissions
} WebDAVPushPermission;
```

### Quick Start - Authentication

```c
#include "webdav_push_auth.h"

// Initialize authentication system
WebDAVPushAuthConfig auth_config = {
    .require_auth = true,
    .default_method = WEBDAV_PUSH_AUTH_BASIC,
    .allow_anonymous = false,
    .use_session_tokens = true,
    .token_expiry = 86400000,  // 24 hours in ms
};

webdav_push_auth_init(&auth_config);

// Or use defaults (authentication required, Basic auth, session tokens enabled)
webdav_push_auth_init(NULL);
```

### User Management

```c
// Add a user
webdav_push_user_add("username", "password", WEBDAV_PUSH_PERM_READ | WEBDAV_PUSH_PERM_WRITE);

// Remove a user
webdav_push_user_remove("username");

// Update password
webdav_push_user_update_password("username", "new_password");

// Update permissions
webdav_push_user_update_permissions("username", WEBDAV_PUSH_PERM_ADMIN);

// Enable/disable user
webdav_push_user_set_enabled("username", false);

// Get user by name
WebDAVPushUser* user = webdav_push_user_get("username");
```

### Session Management

Sessions manage authenticated client connections:

```c
// Create a session for a user (called after successful authentication)
const char* session_id = webdav_push_session_create("username", WEBDAV_PUSH_PERM_READ);

// Get session by ID
WebDAVPushSession* session = webdav_push_session_get(session_id);

// Get session by client ID
WebDAVPushSession* session = webdav_push_session_get_by_client(client_id);

// Ping session (update last activity - prevents timeout)
webdav_push_session_ping(session_id);

// Destroy session
webdav_push_session_destroy(session_id);

// Cleanup expired sessions (call periodically)
webdav_push_session_cleanup();
```

**Session Configuration:**
- Maximum: 16 concurrent sessions
- Timeout: 5 minutes (300,000 ms)
- Lockout: 5 failed attempts, 1-minute duration

### Authentication Flow

#### Username/Password Authentication

```c
// Client attempts login
bool success = webdav_push_auth_login(
    client_id,
    "username",
    "password"
);

// Check if client is authenticated
if (webdav_push_auth_is_authenticated(client_id)) {
    // Client is authenticated
}

// Check permissions
if (webdav_push_auth_has_permission(client_id, WEBDAV_PUSH_PERM_WRITE)) {
    // Client can subscribe to paths
}

// Get auth state
WebDAVPushAuthState* state = webdav_push_auth_get_state(client_id);
```

#### Token Authentication

```c
// Generate a token for a user
const char* token = webdav_push_token_generate(
    "username",
    WEBDAV_PUSH_PERM_READ | WEBDAV_PUSH_PERM_WRITE,
    86400000  // 24 hours expiry
);

// Client authenticates with token
bool success = webdav_push_auth_token(client_id, token);

// Validate a token
bool valid = webdav_push_token_validate(token);

// Get token info
WebDAVPushToken* token_info = webdav_push_token_get(token);

// Revoke a token
webdav_push_token_revoke(token);

// Revoke all tokens for a user
int count = webdav_push_token_revoke_all("username");

// List all active tokens
WebDAVPushToken** tokens = webdav_push_token_list();

// Cleanup expired tokens
webdav_push_token_cleanup();
```

**Token Configuration:**
- Maximum: 32 active tokens
- Default expiry: 24 hours
- Token length: 32 characters (secure random)

### Rate Limiting

The system includes rate limiting to prevent abuse:

```c
// Check if client is rate limited
if (webdav_push_rate_limit_check(client_id)) {
    // Client has exceeded rate limit
    // Return HTTP 429 Too Many Requests
}

// Record a notification send
webdav_push_rate_limit_record(client_id);

// Get remaining notifications for client
uint32_t remaining = webdav_push_rate_limit_remaining(client_id);
```

**Rate Limit Configuration:**
- Default: 60 notifications per minute per client
- Configurable per client

### Password Hashing

```c
// Hash a password
char hash[64];
webdav_push_password_hash("mypassword", hash);

// Verify a password against a hash
bool match = webdav_push_password_verify("mypassword", hash);
```

**Note:** The implementation uses a simple hash function suitable for embedded systems. For production use, implement SHA-256 or similar cryptographic hash.

### Default Users

The system automatically creates a default admin user:
- **Username:** `admin`
- **Password:** `admin`
- **Permissions:** All (READ, WRITE, ADMIN)

### Authentication Configuration

```c
typedef struct {
    bool require_auth;                    // Require authentication
    WebDAVPushAuthMethod default_method; // Default auth method
    bool allow_anonymous;                 // Allow anonymous connections (read-only)
    bool use_session_tokens;             // Use session tokens
    uint32_t token_expiry;                // Token expiry in ms (0 = never expire)
} WebDAVPushAuthConfig;
```

### Security Utilities

```c
// Generate a random string
char buffer[33];
webdav_push_generate_random_string(buffer, 32);

// Create a secure session ID
const char* session_id = webdav_push_create_session_id();

// Create a secure token
const char* token = webdav_push_create_token();
```

### Shutdown

```c
// Shutdown authentication system (clears sensitive data)
webdav_push_auth_shutdown();
```

### Integration with Push Notification System

The authentication system integrates seamlessly with the push notification system:

```c
// Check if authentication is required before sending notification
if (webdav_push_auth_required()) {
    if (!webdav_push_auth_is_authenticated(client_id)) {
        // Return 401 Unauthorized
        return;
    }
}

// Send notification only to authenticated clients with READ permission
if (webdav_push_auth_has_permission(client_id, WEBDAV_PUSH_PERM_READ)) {
    webdav_push_notify_client(client_id, &notification);
}
```

---

## Client Examples

### Python SSE Client

```python
import requests
import sseclient

# Connect to SSE endpoint
url = 'http://esp8266.local:8080/webdav/push/sse'
headers = {'Accept': 'text/event-stream'}

# Stream events
for event in sseclient.SSEClient(url, headers=headers):
    print(f"Event: {event.event}")
    print(f"Data: {event.data}")
    
    # Parse JSON
    import json
    data = json.loads(event.data)
    print(f"File changed: {data['path']}")
```

### Python WebSocket Client

```python
import websocket
import json

def on_message(ws, message):
    data = json.loads(message)
    print(f"Notification: {data['type']} at {data['path']}")

def on_error(ws, error):
    print(f"Error: {error}")

def on_close(ws, close_status, close_msg):
    print("Connection closed")

def on_open(ws):
    print("Connected")
    # Subscribe to path
    ws.send(json.dumps({"action": "subscribe", "path": "/projects"}))

# Connect
ws = websocket.WebSocketApp(
    "ws://esp8266.local:8080/webdav/push/ws",
    on_open=on_open,
    on_message=on_message,
    on_error=on_error,
    on_close=on_close
)
ws.run_forever()
```

### JavaScript (Browser) SSE Client

```javascript
// Connect to SSE
const eventSource = new EventSource('http://esp8266.local:8080/webdav/push/sse');

eventSource.onopen = () => {
    console.log('Connected to WebDAV push');
};

eventSource.onerror = (error) => {
    console.error('Connection error:', error);
};

// Handle specific event types
eventSource.addEventListener('file_created', (event) => {
    const data = JSON.parse(event.data);
    console.log(`File created: ${data.path}, size: ${data.size}`);
});

eventSource.addEventListener('file_modified', (event) => {
    const data = JSON.parse(event.data);
    console.log(`File modified: ${data.path}`);
});

eventSource.addEventListener('project_updated', (event) => {
    const data = JSON.parse(event.data);
    console.log(`Project updated: ${data.path}`);
    // Refresh project in UI
    refreshProject(data.path);
});
```

---

## HTTP Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/webdav/push/sse` | SSE event stream |
| GET | `/webdav/push/ws` | WebSocket connection |
| GET | `/webdav/push/poll` | Long polling |
| POST | `/webdav/push/subscribe` | Subscribe to path |
| POST | `/webdav/push/unsubscribe` | Unsubscribe |

---

## Files

| File | Description |
|------|-------------|
| `src/gui_editor/server/webdav_push.h` | Header with API |
| `src/gui_editor/server/webdav_push.c` | Implementation |
| `src/gui_editor/server/webdav_push_auth.h` | Authentication types and API |
| `src/gui_editor/server/webdav_push_auth.c` | Authentication implementation |
| `docs/WEBDAV_PUSH.md` | This documentation |

---

## Limitations

1. **SSE**: Requires HTTP/1.1 persistent connection support
2. **WebSocket**: Requires WebSocket library (not included in base system)
3. **Long Polling**: Higher latency, more HTTP overhead
4. **Client Count**: Maximum 8 concurrent clients (configurable)
5. **Watch Count**: Maximum 16 watched paths (configurable)

---

## Future Enhancements

- [ ] Add WebSocket library integration
- [ ] Implement inotify-like for SD card (if supported)
- [ ] Add notification history/replay
- [x] Add client authentication
- [x] Add rate limiting
- [ ] Add notification filtering by client
- [ ] Support for binary data in notifications
- [ ] Add reconnection handling

---

## Version History

- **Latest**: WebDAV push notification system with SSE, WebSocket, long polling
- **Latest**: Multiple event types (file, folder, project, GUI)
- **Latest**: Path watching with recursive support
- **Latest**: Low memory footprint (~3KB max)

---

## References

- [Server-Sent Events Spec](https://html.spec.whatwg.org/multipage/server-sent-events.html)
- [WebSocket API](https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API)
- [WebDAV RFC 4918](https://tools.ietf.org/html/rfc4918)
- [davfs2](https://savannah.nongnu.org/projects/davfs2)
