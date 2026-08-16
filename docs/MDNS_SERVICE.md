# mDNS Service Discovery (Bonjour/Zeroconf)

## Overview

The **mDNS Service Discovery** module enables ESP8266/ESP32 devices to advertise themselves on the local network using **Bonjour/Zeroconf** technology. This allows any device on the same local network to discover and connect to the ESP8266 using a simple hostname like `http://esp8266.local` without requiring:

- A DNS server
- Manual `/etc/hosts` file configuration
- Hard-coded IP addresses

This is implemented as part of **Kernel.bin** (SD card) because it requires network connectivity which is established after the bootloader loads the kernel.

---

## How It Works

### mDNS Protocol Basics

- Uses **UDP multicast** on port **5353**
- Devices advertise their **hostname** + `.local` (e.g., `esp8266.local`)
- Services are advertised with **service types** (e.g., `_http._tcp`, `_webdav._tcp`)
- Each service can include **TXT records** with metadata (model, manufacturer, version, etc.)

### Service Discovery Flow

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│   ESP8266/ESP32   │     │   mDNS Multicast │     │  Linux/macOS    │
│   Device          │────▶│   on port 5353  │────▶│  Client         │
└─────────────────┘     └─────────────────┘     └─────────────────┘
                          │
                          ▼
                    ┌─────────────────┐
                    │  Service Cache   │
                    │  (Browser/OS)    │
                    └─────────────────┘
                          │
                          ▼
                    ┌─────────────────┐
                    │  Resolves:       │
                    │  esp8266.local   │
                    │  → 192.168.1.50 │
                    └─────────────────┘
```

### Service Types Advertised

| Service Type | Port | Description | Path |
|--------------|------|-------------|------|
| `_http._tcp` | 80 | HTTP/Web server | `/` |
| `_webdav._tcp` | 80 | WebDAV file server | `/webdav` |
| `_guikit._tcp` | 8080 | GUIKit custom service | `/gui` |

Each service includes TXT records with device metadata:
- `model` - Device model (e.g., "ESP8266 NodeMCU")
- `manufacturer` - Manufacturer (e.g., "Genose.org")
- `serial` - Serial number / device ID
- `version` - Firmware version
- `path` - Service path

---

## Quick Start

### Basic Initialization

```c
#include "mdns_service.h"

// After WiFi connection is established
void setup() {
    // Connect to WiFi first
    WiFi.begin("SSID", "password");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    // Initialize mDNS with defaults
    mdns_init(NULL);
    
    // Device is now discoverable as esp8266.local
}

void loop() {
    // Process mDNS events periodically
    mdns_process();
}
```

### Custom Configuration

```c
#include "mdns_service.h"

void setup() {
    WiFi.begin("SSID", "password");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    // Custom configuration
    mDNSConfig config = {
        .hostname = "my-esp8266",      // → my-esp8266.local
        .enable_http = true,
        .enable_webdav = true,
        .enable_guikit = true,
        .http_port = 80,
        .webdav_port = 80,
        .guikit_port = 8080,
        .model = "ESP8266 NodeMCU v3",
        .manufacturer = "Genose.org",
        .serial_number = "DEV-001",
        .version = "1.0.0"
    };

    mdns_init(&config);
}
```

### Kernel Integration

For automatic initialization in Kernel.bin:

```c
#include "mdns_service.h"

void kernel_setup() {
    // ... other initialization ...
    
    // Initialize mDNS after WiFi
    kernel_mdns_init("GUIKit-Device");
}
```

---

## API Reference

### Configuration Structure

```c
typedef struct {
    const char* hostname;           // Device hostname (max 63 chars)
    bool enable_http;              // Advertise HTTP service
    bool enable_webdav;           // Advertise WebDAV service
    bool enable_guikit;            // Advertise GUIKit service
    uint16_t http_port;            // HTTP port (default: 80)
    uint16_t webdav_port;          // WebDAV port (default: 80)
    uint16_t guikit_port;          // GUIKit port (default: 8080)
    const char* model;             // Device model
    const char* manufacturer;      // Manufacturer
    const char* serial_number;     // Serial number
    const char* version;           // Version
} mDNSConfig;
```

### Initialization Functions

| Function | Description |
|----------|-------------|
| `mdns_init(config)` | Initialize mDNS (NULL for defaults) |
| `mdns_shutdown()` | Stop mDNS service |
| `mdns_is_running()` | Check if mDNS is active |
| `mdns_reinit(config)` | Restart mDNS with new config |

### Hostname Management

| Function | Description |
|----------|-------------|
| `mdns_set_hostname(name)` | Change device hostname |
| `mdns_get_hostname()` | Get current hostname |
| `mdns_get_fqdn()` | Get FQDN (e.g., "esp8266.local") |
| `mdns_is_valid_hostname(name)` | Validate hostname format |
| `mdns_generate_unique_hostname(base, buf)` | Generate unique hostname |

### Service Management

| Function | Description |
|----------|-------------|
| `mdns_add_service()` | Add custom service |
| `mdns_add_service_with_txt()` | Add service with multiple TXT records |
| `mdns_remove_service()` | Remove a service |
| `mdns_remove_all_services()` | Remove all custom services |
| `mdns_get_service_info()` | Get info about a service |

### Maintenance

| Function | Description |
|----------|-------------|
| `mdns_process()` | Process mDNS events (call in loop) |
| `mdns_get_error()` | Get last error code |
| `mdns_error_to_string()` | Convert error code to string |

### Kernel Integration Helpers

| Function | Description |
|----------|-------------|
| `kernel_mdns_init(name)` | Initialize mDNS for kernel |
| `mdns_get_webdav_url(buf, size)` | Get WebDAV discovery URL |
| `mdns_get_guikit_url(buf, size)` | Get GUIKit discovery URL |

---

## Usage Examples

### Example 1: Basic Web Server Discovery

```c
#include "mdns_service.h"
#include "ESP8266WebServer.h"

ESP8266WebServer server(80);

void setup() {
    WiFi.begin("SSID", "password");
    while (WiFi.status() != WL_CONNECTED) delay(500);

    // Start mDNS
    mdns_init(NULL);

    // Start web server
    server.on("/", []() {
        server.send(200, "text/html", "<h1>ESP8266 Web Server</h1>");
    });
    server.begin();

    Serial.println("Device ready at:");
    Serial.print("  http://");
    Serial.println(mdns_get_fqdn());
}

void loop() {
    server.handleClient();
    mdns_process();
}
```

### Example 2: Custom Service with TXT Records

```c
#include "mdns_service.h"

void setup() {
    WiFi.begin("SSID", "password");
    while (WiFi.status() != WL_CONNECTED) delay(500);

    mdns_init(NULL);

    // Add custom service for API
    const char* txt_records[] = {
        "api_version", "1.0",
        "description", "REST API for device control",
        NULL
    };
    
    mdns_add_service_with_txt(
        "ESP8266-API",      // Instance name
        "_api._tcp",        // Service type
        8080,              // Port
        "/api",            // Path
        txt_records        // TXT records
    );
}
```

### Example 3: Multiple Devices with Unique Names

```c
#include "mdns_service.h"
#include "ESP8266WiFi.h"

void setup() {
    WiFi.begin("SSID", "password");
    while (WiFi.status() != WL_CONNECTED) delay(500);

    // Generate unique hostname based on MAC address
    char hostname[64];
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(hostname, sizeof(hostname), "esp-%02x%02x%02x", mac[3], mac[4], mac[5]);

    mDNSConfig config = {
        .hostname = hostname,
        .enable_http = true,
        .enable_webdav = true,
        .model = "ESP8266",
        .manufacturer = "Genose.org",
        .serial_number = "DEV-001",
        .version = "1.0.0"
    };

    mdns_init(&config);

    Serial.print("Device ready at: http://");
    Serial.println(mdns_get_fqdn());
}
```

---

## Client Discovery

### Linux (avahi-browser)

```bash
# List all mDNS services on network
avahi-browse -a -r

# List HTTP services
avahi-browse -a -r _http._tcp

# List WebDAV services
avahi-browse -a -r _webdav._tcp

# List GUIKit services
avahi-browse -a -r _guikit._tcp
```

Output:
```
+   eth0 IPv4 esp8266                      _http._tcp            local
+   eth0 IPv4 esp8266                      _webdav._tcp          local
+   eth0 IPv4 esp8266                      _guikit._tcp          local
```

### macOS (dns-sd)

```bash
# List all services
dns-sd -B _services._dns-sd._udp

# List HTTP services
dns-sd -B _http._tcp

# Resolve service to IP
dns-sd -L "ESP8266 Web Server" _http._tcp
```

### Windows (Bonjour)

Use **Bonjour Browser** utility or PowerShell:

```powershell
# Using dns-sd (if installed via Bonjour SDK)
dns-sd -B _http._tcp
```

### Python Discovery

```python
import socket
import struct
import time

def query_mdns(service_type="_http._tcp"):
    """Query for mDNS services"""
    # This is a simplified example
    # In practice, use zeroconf library
    pass

# Better: Use zeroconf library
from zeroconf import ServiceBrowser, Zeroconf

class MyListener:
    def add_service(self, zeroconf, type, name):
        print(f"Found: {name} ({type})")

zeroconf = Zeroconf()
listener = MyListener()
browser = ServiceBrowser(zeroconf, "_http._tcp", listener)

try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    zeroconf.close()
```

### JavaScript (Node.js)

```javascript
const mdns = require('multicast-dns')();

mdns.on('response', (response) => {
    console.log('Found service:', response.answers);
});

// Query for HTTP services
mdns.query({
    questions: [{ type: 'PTR', name: '_http._tcp.local' }]
});
```

---

## Platform Support

### ESP8266

- **Library**: `ESP8266mDNS` (built-in to Arduino core)
- **Dependencies**: WiFi connection must be established first
- **Port**: 5353 (UDP)
- **Limitations**: None

### ESP32

- **Library**: `ESPmDNS` (built-in to Arduino core)
- **Dependencies**: WiFi connection must be established first
- **Port**: 5353 (UDP)
- **Limitations**: None

---

## Configuration Options

### Hostname Rules

- **Maximum length**: 63 characters
- **Valid characters**: a-z, A-Z, 0-9, hyphen (-)
- **Cannot start with hyphen**
- **Cannot be all numeric**
- **Case-insensitive** (but preserve case for display)

### Default Hostname Generation

If no hostname is provided or invalid:
1. Uses `"esp8266"` as default
2. Can generate unique names using `mdns_generate_unique_hostname()`
3. Can include MAC address for uniqueness

### Service Configuration

| Service | Port | Enabled by Default | TXT Records |
|---------|------|-------------------|-------------|
| HTTP | 80 | Yes | model, manufacturer, path |
| WebDAV | 80 | Yes | model, manufacturer, path |
| GUIKit | 8080 | Yes | model, manufacturer, serial, version |

---

## Integration with GUIKit

### Automatic Initialization

The mDNS service is automatically initialized as part of Kernel.bin startup:

```c
// In kernel_main.cpp
void setup() {
    // 1. Initialize hardware
    init_hardware();
    
    // 2. Connect to WiFi
    connect_wifi();
    
    // 3. Initialize mDNS
    kernel_mdns_init("GUIKit-Device");
    
    // 4. Start servers
    start_web_server();
    start_webdav_server();
    
    // 5. Initialize push notifications
    webdav_push_init(NULL);
    webdav_push_auth_init(NULL);
}
```

### Configuration File Integration

mDNS configuration can be loaded from `/etc/GUIKIT_autostart.ini`:

```ini
[mdns]
hostname = my-esp8266
enable_http = true
enable_webdav = true
enable_guikit = true
model = ESP8266 NodeMCU
manufacturer = Genose.org
```

---

## Troubleshooting

### Common Issues

#### Service Not Discoverable

**Symptoms**: Clients cannot resolve `esp8266.local`

**Solutions:**
1. Verify WiFi is connected before calling `mdns_init()`
2. Check firewall allows UDP port 5353 multicast
3. Verify mDNS is running: `mdns_is_running()`
4. Check hostname is valid: `mdns_is_valid_hostname(name)`
5. Test with `avahi-browse -a -r` on Linux

#### Name Conflict

**Symptoms**: Multiple ESP8266 devices with same hostname

**Solutions:**
1. Use unique hostnames for each device
2. Include MAC address in hostname
3. Use `mdns_generate_unique_hostname()`

#### mDNS Not Starting

**Symptoms**: `mdns_init()` returns false

**Solutions:**
1. Check WiFi connection status
2. Verify hostname is valid
3. Check for memory constraints
4. Verify ESP8266mDNS library is included

### Debug Output

Enable debug output:

```c
#include "mdns_service.h"

void setup() {
    // ...
    if (!mdns_init(NULL)) {
        Serial.println("mDNS init failed:");
        Serial.println(mdns_error_to_string(mdns_get_error()));
    } else {
        Serial.print("mDNS running: ");
        Serial.println(mdns_get_fqdn());
    }
}
```

---

## Memory Usage

| Component | Size | Notes |
|-----------|------|-------|
| mDNS state | ~500 bytes | Hostname, FQDN, config |
| Custom services | ~64 bytes each | Max 8 services |
| **Total (max)** | **~1.1KB** | All services |

---

## Client Connection Examples

### Using the Discovery URL

Once mDNS is running, clients can connect using:

```bash
# Direct access
curl http://esp8266.local/

# WebDAV access
curl http://esp8266.local/webdav/

# GUIKit web editor
# Open in browser: http://esp8266.local/gui/editor.GUIKIT
```

### Python Request

```python
import requests

# Using mDNS hostname
response = requests.get('http://esp8266.local/')
print(response.text)
```

### JavaScript Fetch

```javascript
// In browser
fetch('http://esp8266.local/api/status')
  .then(response => response.json())
  .then(data => console.log(data));
```

---

## Comparison with Alternatives

| Method | Pros | Cons |
|--------|------|------|
| **mDNS** | No configuration, works on local network, standard | Requires mDNS support on client |
| **DNS** | Works everywhere, reliable | Requires DNS server configuration |
| **Static IP** | Simple, no discovery needed | Manual configuration, changes break |
| **IP Broadcast** | Simple | Not reliable, network-dependent |
| **UPnP** | More features | Complex, Windows-centric |

**Recommendation**: Use mDNS as primary method, with fallback to manual IP configuration.

---

## Standards Compliance

- **RFC 6761**: mDNS Multicast DNS
- **RFC 6762**: mDNS Multicast DNS-SD
- **RFC 3927**: DNS-SD Service Discovery
- **Apple Bonjour**: Compatible implementation

---

## Files

| File | Description |
|------|-------------|
| `src/system/mdns_service.h` | Header with types and API |
| `src/system/mdns_service.c` | Implementation |
| `docs/MDNS_SERVICE.md` | This documentation |

---

## Version History

- **Latest**: mDNS service discovery with Bonjour/Zeroconf support
- **Latest**: HTTP, WebDAV, and GUIKit service advertisement
- **Latest**: TXT records for device metadata
- **Latest**: Custom service support
- **Latest**: Integration with Kernel.bin

---

## References

- [RFC 6761 - mDNS](https://tools.ietf.org/html/rfc6761)
- [RFC 6762 - mDNS-SD](https://tools.ietf.org/html/rfc6762)
- [Apple Bonjour](https://developer.apple.com/bonjour/)
- [ESP8266mDNS Library](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266mDNS)
- [ESP32 mDNS Library](https://github.com/espressif/arduino-esp32/tree/master/libraries/ESPmDNS)

---

*Generated by Mistral Vibe*  
*Date: 2026-08-16*
