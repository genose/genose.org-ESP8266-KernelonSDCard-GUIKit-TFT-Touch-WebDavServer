# Network Architecture

This document describes the network services, WebDAV server, and enhanced features architecture.

---

## Table of Contents

1. [Network Services Stack](#network-services-stack)
2. [WebDAV Server](#webdav-server)
3. [Enhanced Features](#enhanced-features)
4. [Network Mounting](#network-mounting)
5. [Quota System](#quota-system)
6. [File Locking System](#file-locking-system)
7. [History Tracking](#history-tracking)
8. [Share Links System](#share-links-system)
9. [Remote Access System](#remote-access-system)
10. [API Reference](#api-reference)

---

## Network Services Stack

```
┌─────────────────────────────────────────────────────────────┐
│                     NETWORK LAYER ARCHITECTURE                    │
├─────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │                    HTTP Server (Port 80)                    │  │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐         │  │
│  │  │   /         │  │   /webdav    │  │   /ui/       │         │  │
│  │  │  Root Page  │  │  WebDAV      │  │  UI Files    │         │  │
│  │  │             │  │  Redirect     │  │  JSON        │         │  │
│  │  └─────────────┘  └─────────────┘  └─────────────┘         │  │
│  └─────────────────────────────────────────────────────────┘  │
│                              │                                  │
│                              ▼                                  │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │                   WebDAV Server (Port 80)                   │  │
│  │                                                                  │  │
│  │  Core Features:                                               │  │
│  │  - File operations (GET, PUT, DELETE, MKCOL)               │  │
│  │  - Directory listing                                         │  │
│  │  - Basic authentication (username/password)                │  │
│  │                                                                  │  │
│  │  Enhanced Features:                                          │  │
│  │  - Network Mounting                                          │  │
│  │  - Quota System                                              │  │
│  │  - File Locking                                              │  │
│  │  - History Tracking                                           │  │
│  │  - Share Links                                               │  │
│  │  - Remote Access                                             │  │
│  │                                                                  │  │
│  └─────────────────────────────────────────────────────────┘  │
│                                                                  │
└─────────────────────────────────────────────────────────────┘
```

---

## WebDAV Server

### Implementation

The WebDAV server is implemented using the [ESPWebDAV](https://github.com/hoonie/ESPWebDAV) library, which provides WebDAV functionality for ESP8266.

**Key Features:**
- Full WebDAV protocol support (RFC 4918)
- File upload, download, delete, and directory creation
- Directory listing
- Basic authentication
- Runs on port 80

### Configuration

```ini
; WebDAV Configuration in platformio.ini
build_flags =
    -D WEBDAV_USERNAME="admin"
    -D WEBDAV_PASSWORD="esp8266"
    -D WEBDAV_PORT=80
```

### WebDAV Module Structure

```
Web Module/
├── webdav_server.h/cpp    # WebDAV server wrapper
│   ├── init_webdav_server()
│   ├── handle_webdav_requests()
│   ├── set_authentication()
│   └── ESPWebDAV integration
│
├── web_server.h/cpp        # HTTP server with port forwarding
│   ├── init_web_server()
│   ├── handle_web_requests()
│   ├── handle_root()
│   ├── handle_webdav()
│   ├── handle_ui_file()
│   └── ESP8266WebServer integration
│
└── remote_access.h/cpp     # Remote access configuration
    ├── configure_remote_access()
    ├── get_remote_access_url()
    └── DNS update handling
```

---

## Enhanced Features

### Enhanced Features Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                 WEBDAV ENHANCEMENTS LAYER                        │
├─────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌──────────────────┐  ┌──────────────────┐                   │
│  │  Network Mounting  │  │  Quota System     │                   │
│  │  Path Mapping     │  │  Per-user limits   │                   │
│  └──────────────────┘  └──────────────────┘                   │
│                                                                  │
│  ┌──────────────────┐  ┌──────────────────┐                   │
│  │  File Locking     │  │  History Log      │                   │
│  │  Prevents race    │  │  Audit trail      │                   │
│  │  conditions      │  │                  │                   │
│  └──────────────────┘  └──────────────────┘                   │
│                                                                  │
│  ┌──────────────────┐  ┌──────────────────┐                   │
│  │  Remote Access    │  │  Share Links      │                   │
│  │  Internet access  │  │  Temporary access │                   │
│  └──────────────────┘  └──────────────────┘                   │
│                                                                  │
└─────────────────────────────────────────────────────────────┘
```

---

## Network Mounting

### Overview

**Purpose:** Map physical SD card paths to virtual mount points for easier access.

**Implementation:**
- Simple path mapping (no full FUSE implementation)
- Configuration stored in memory (RAM)
- Multiple mount points supported
- Cached for performance

### API

```bash
# Create a mount point
curl -u admin:esp8266 -X POST http://<IP>/mount \
  -d "path=/system/ui&mount_point=/ui"

# Access a mounted file
curl -u admin:esp8266 http://<IP>/webdav/ui/main_ui.json

# Remove a mount point
curl -u admin:esp8266 -X DELETE http://<IP>/mount \
  -d "mount_point=/ui"

# List all mount points
curl -u admin:esp8266 http://<IP>/mount
```

### Example Mappings

| Physical Path | Mount Point | Access Path | Description |
|---------------|-------------|-------------|-------------|
| `/system/ui` | `/ui` | `/webdav/ui/` | UI definitions |
| `/system/config` | `/config` | `/webdav/config/` | Configurations |
| `/system/logs` | `/logs` | `/webdav/logs/` | Log files |

### Implementation Code

```c
typedef struct {
    char physical_path[64];
    char mount_point[32];
} MountPoint;

#define MAX_MOUNT_POINTS 10
MountPoint mount_points[MAX_MOUNT_POINTS];
uint8_t mount_point_count = 0;

// Resolve a path through mount points
const char* resolve_mount_point(const char* path) {
    for (int i = 0; i < mount_point_count; i++) {
        if (strcmp(path, mount_points[i].mount_point) == 0) {
            return mount_points[i].physical_path;
        }
    }
    return path;  // No mount point found, return original
}
```

---

## Quota System

### Overview

**Purpose:** Limit storage usage per user to prevent abuse and ensure fair resource allocation.

**Implementation:**
- Per-user storage limits
- Stored in `/system/config/quotas.json` on SD card
- Updated incrementally on file writes
- Admin has unlimited quota (`max_space: 0`)

### Data Structure

```json
[
  {
    "username": "user1",
    "max_space": 10485760,  // 10MB
    "used_space": 1048576   // 1MB used
  },
  {
    "username": "admin",
    "max_space": 0,          // Unlimited
    "used_space": 5242880    // 5MB used
  }
]
```

### C Structure

```c
typedef struct {
    char username[32];
    uint32_t max_space;   // 0 = unlimited
    uint32_t used_space;
} UserQuota;

#define MAX_USERS 20
UserQuota quotas[MAX_USERS];
```

### API

```bash
# Get user quota usage
curl -u admin:esp8266 http://<IP>/quota?username=user1

# Response:
{
  "username": "user1",
  "max_space": 10485760,
  "used_space": 1048576,
  "available_space": 9437184
}

# Set user quota (admin only)
curl -u admin:esp8266 -X POST http://<IP>/quota \
  -d "username=user2&max_space=5242880"
```

### Quota Check Flow

```
User writes file (size N bytes)
    │
    ▼
Check quota for user
    │
    ▼
User.used_space + N <= User.max_space?
    │
    ├── YES → Allow write, update used_space += N
    │
    └── NO → Reject with 402 Payment Required (or 413)
              Error: "Quota exceeded"
```

---

## File Locking System

### Overview

**Purpose:** Prevent concurrent write conflicts and ensure data integrity.

**Implementation:**
- Per-file, per-user locking
- Locks stored in `/system/config/file_locks.json` on SD card
- Automatic timeout expiration (configurable, default: 1 hour)
- Automatic cleanup of expired locks on system startup

### Data Structure

```json
[
  {
    "path": "/system/ui/main_ui.json",
    "username": "admin",
    "locked_at": 1637145600000,
    "timeout": 3600000
  },
  {
    "path": "/system/config/settings.json",
    "username": "user1",
    "locked_at": 1637145610000,
    "timeout": 3600000
  }
]
```

### C Structure

```c
typedef struct {
    char path[64];
    char username[32];
    uint32_t locked_at;
    uint32_t timeout;  // milliseconds
} FileLock;

#define MAX_FILE_LOCKS 50
FileLock file_locks[MAX_FILE_LOCKS];
```

### API

```bash
# Lock a file (1 hour timeout)
curl -u admin:esp8266 -X POST http://<IP>/file_lock \
  -d "path=/system/ui/main_ui.json&username=admin&timeout=3600000"

# Response:
{
  "success": true,
  "message": "File locked",
  "locked_at": 1637145600000,
  "expires_at": 1637149200000
}

# Unlock a file
curl -u admin:esp8266 -X DELETE http://<IP>/file_lock \
  -d "path=/system/ui/main_ui.json"

# Check if file is locked
curl -u admin:esp8266 http://<IP>/file_lock?path=/system/ui/main_ui.json

# Response:
{
  "locked": true,
  "username": "admin",
  "locked_at": 1637145600000,
  "expires_at": 1637149200000
}
```

### Lock Flow

```
User requests to write to file
    │
    ▼
Check if file is locked
    │
    ▼
Is file locked?
    │
    ├── NO → Check quota → Allow write
    │
    └── YES
        │
        ▼
    Is lock expired?
        │
        ├── YES → Remove lock → Allow write
        │
        └── NO
            │
            ▼
        Is current user the lock owner?
            │
            ├── YES → Allow write (extend lock)
            │
            └── NO → Reject with 423 Locked
                      Error: "File locked by user X"
```

---

## History Tracking

### Overview

**Purpose:** Maintain an audit trail of file modifications for debugging and security.

**Implementation:**
- Stored in `/system/config/history.log` on SD card
- CSV format for easy parsing
- Automatic rotation when file grows too large
- Limited to 100 entries (configurable via `MAX_HISTORY_ENTRIES`)

### Log Format

```csv
# Format: timestamp,username,action,path
timestamp,username,action,path
1637145600000,admin,write,/system/ui/main_ui.json
1637145601000,user1,read,/system/config/settings.json
1637145602000,admin,delete,/system/logs/old.log
1637145603000,user2,create,/system/ui/new_ui.json
```

### C Structure

```c
typedef struct {
    uint32_t timestamp;
    char username[32];
    char action[16];    // "read", "write", "delete", "create", "rename"
    char path[64];
} HistoryEntry;

#define MAX_HISTORY_ENTRIES 100
HistoryEntry history[MAX_HISTORY_ENTRIES];
```

### API

```bash
# Get history (last 50 entries)
curl -u admin:esp8266 http://<IP>/history?limit=50

# Response:
[
  {
    "timestamp": 1637145600000,
    "username": "admin",
    "action": "write",
    "path": "/system/ui/main_ui.json"
  },
  {
    "timestamp": 1637145601000,
    "username": "user1",
    "action": "read",
    "path": "/system/config/settings.json"
  }
]

# Get history for specific user
curl -u admin:esp8266 http://<IP>/history?username=admin&limit=20

# Get history for specific file
curl -u admin:esp8266 http://<IP>/history?path=/system/ui/main_ui.json

# Get history in date range
curl -u admin:esp8266 http://<IP>/history?since=1637100000000&until=1637200000000
```

---

## Share Links System

### Overview

**Purpose:** Generate temporary, shareable download links for files.

**Implementation:**
- Unique 32-character random tokens
- Expiration timer per link (configurable)
- Stored in `/system/config/share_links.json` on SD card
- Access via `/s/<token>` endpoint (no authentication required)

### Data Structure

```json
[
  {
    "token": "abc123def456ghi789jkl012mno345pqr678",
    "path": "/system/ui/main_ui.json",
    "username": "admin",
    "created_at": 1637145600000,
    "expires_at": 1637232000000,
    "access_count": 2
  }
]
```

### C Structure

```c
typedef struct {
    char token[33];    // 32 chars + null terminator
    char path[64];
    char username[32];
    uint32_t created_at;
    uint32_t expires_at;
    uint16_t access_count;
} ShareLink;

#define MAX_SHARE_LINKS 50
ShareLink share_links[MAX_SHARE_LINKS];
```

### Token Generation

```c
#include <ESP8266WiFi.h>

void generate_share_token(char* token, size_t length) {
    // Use hardware random number generator if available
    // or WiFi MAC address + timestamp for entropy
    
    uint8_t random_bytes[16];
    for (int i = 0; i < 16; i++) {
        random_bytes[i] = random(256);
    }
    
    // Convert to hex string
    for (int i = 0; i < length / 2; i++) {
        sprintf(&token[i * 2], "%02x", random_bytes[i]);
    }
    token[length] = '\0';
}
```

### API

```bash
# Create share link (24h expiration)
curl -u admin:esp8266 -X POST http://<IP>/share \
  -d "path=/system/ui/main_ui.json&expiration=24"

# Response:
{
  "success": true,
  "token": "abc123def456ghi789jkl012mno345pqr678",
  "expires_at": 1637232000000,
  "path": "/system/ui/main_ui.json",
  "url": "http://<IP>/s/abc123def456ghi789jkl012mno345pqr678"
}

# List all share links (admin only)
curl -u admin:esp8266 http://<IP>/share

# Response:
[
  {
    "token": "abc123...",
    "path": "/system/ui/main_ui.json",
    "username": "admin",
    "created_at": 1637145600000,
    "expires_at": 1637232000000,
    "access_count": 2
  }
]

# Access shared file (no authentication)
curl http://<IP>/s/abc123def456ghi789jkl012mno345pqr678

# Delete share link
curl -u admin:esp8266 -X DELETE http://<IP>/share \
  -d "token=abc123def456ghi789jkl012mno345pqr678"
```

---

## Remote Access System

### Overview

**Purpose:** Enable access to WebDAV server from the Internet.

**Implementation:**
- Port forwarding on router (external port → ESP8266 port 80)
- DNS dynamic service (No-IP or DynDNS) for domain name
- Configuration stored in `/system/config/remote_access.json` on SD card
- Automatic DNS updates (configurable interval, default: 1 hour)

### Data Structure

```json
{
  "enabled": true,
  "external_port": 8080,
  "dns_hostname": "esp8266.ddns.net",
  "dns_provider": "noip",
  "dns_username": "your_noip_username",
  "dns_password": "your_noip_password",
  "dns_update_interval": 3600000
}
```

### C Structure

```c
typedef struct {
    bool enabled;
    uint16_t external_port;
    char dns_hostname[64];
    char dns_provider[16];
    char dns_username[32];
    char dns_password[32];
    uint32_t dns_update_interval;  // milliseconds
    uint32_t last_dns_update;
} RemoteAccessConfig;
```

### API

```bash
# Enable remote access
curl -u admin:esp8266 -X POST http://<IP>/remote_access \
  -d "enabled=true&external_port=8080&dns_hostname=esp8266.ddns.net&dns_provider=noip&dns_username=user&dns_password=pass"

# Response:
{
  "success": true,
  "message": "Remote access configured",
  "url": "http://esp8266.ddns.net:8080"
}

# Get remote access URL
curl -u admin:esp8266 http://<IP>/remote_url

# Response:
{
  "enabled": true,
  "url": "http://esp8266.ddns.net:8080"
}

# Disable remote access
curl -u admin:esp8266 -X POST http://<IP>/remote_access \
  -d "enabled=false"
```

### Router Configuration

1. **Port Forwarding:**
   - External port: 8080 (or any available port)
   - Internal IP: ESP8266's local IP address
   - Internal port: 80
   - Protocol: TCP

2. **DNS Dynamic:**
   - Create account at No-IP.com or DynDNS.com
   - Add hostname (e.g., `esp8266.ddns.net`)
   - Configure ESP8266 with credentials

3. **Access:**
   - From Internet: `http://esp8266.ddns.net:8080/webdav`
   - From local network: `http://<local-ip>/webdav`

---

## API Reference

### WebDAV API

| Endpoint | Method | Description | Authentication |
|----------|--------|-------------|----------------|
| `/webdav/` | GET/POST/PUT/DELETE | WebDAV operations | Required |
| `/webdav/{path}` | GET | Download file | Required |
| `/webdav/{path}` | PUT | Upload file | Required |
| `/webdav/{path}` | DELETE | Delete file | Required |
| `/webdav/{path}` | MKCOL | Create directory | Required |

### HTTP API

| Endpoint | Method | Description | Authentication |
|----------|--------|-------------|----------------|
| `/` | GET | Web interface | None |
| `/ui/{filename}` | GET | Load UI file | None |

### Enhanced API

| Endpoint | Method | Description | Authentication |
|----------|--------|-------------|----------------|
| `/mount` | POST | Create mount point | Required |
| `/mount` | GET | List mount points | Required |
| `/mount` | DELETE | Remove mount point | Required |
| `/quota` | GET | Check user quota | Required |
| `/quota` | POST | Set user quota | Required (admin) |
| `/file_lock` | POST | Lock file | Required |
| `/file_lock` | GET | Check file lock | Required |
| `/file_lock` | DELETE | Unlock file | Required |
| `/history` | GET | View history | Required (admin) |
| `/share` | POST | Create share link | Required |
| `/share` | GET | List share links | Required (admin) |
| `/share` | DELETE | Delete share link | Required (admin) |
| `/remote_access` | POST | Configure remote access | Required (admin) |
| `/remote_url` | GET | Get remote access URL | Required |
| `/s/{token}` | GET | Access shared file | None |

---

*See [ARCHITECTURE.md](ARCHITECTURE.md) for overall system architecture*  
*See [HARDWARE.md](HARDWARE.md) for hardware architecture*  
*See [SOFTWARE.md](SOFTWARE.md) for software components*  
*See [DATA_FLOW.md](DATA_FLOW.md) for data flow diagrams*  

---

*Generated from architecture analysis of discussion_guikit.txt*
*Documentation extracted and organized by Mistral Vibe*
