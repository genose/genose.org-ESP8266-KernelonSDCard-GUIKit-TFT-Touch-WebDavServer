# GUI Editor - Web-based JSON GUI Board Rendering

## Overview

A web-based editor for creating and editing GUIKit interfaces. The editor:
- Renders GUI layouts from JSON definitions
- Provides a visual drag-and-drop interface
- Generates JSON that can be loaded by GUIKit on ESP8266
- Integrates with WebDAV server for file management

## Architecture

```
src/gui_editor/
├── web/                    # Web-based editor (HTML/JS/CSS)
│   ├── index.html          # Main editor page
│   ├── editor.js           # Editor logic
│   ├── renderer.js        # GUI rendering engine
│   ├── widgets/           # Widget-specific rendering
│   └── styles/            # CSS styles
│
├── json/                   # GUI definition schemas
│   ├── gui_schema.json     # JSON Schema for GUI definitions
│   └── examples/          # Example GUI configurations
│
├── server/                # Backend integration
│   ├── webdav_bridge.h    # Bridge to WebDAV server
│   └── gui_loader.c       # Load/save GUI from SD card
│
└── gui_editor.h/c         # Native C interface (optional)
```

## JSON GUI Schema

### Basic Structure

```json
{
  "version": "1.0",
  "name": "My GUI",
  "size": {"width": 320, "height": 240},
  "background": "#000000",
  "widgets": [
    {
      "id": "btn1",
      "type": "button",
      "text": "Click Me",
      "x": 10, "y": 10, "width": 100, "height": 40,
      "style": {"bg_color": "#FF0000", "fg_color": "#FFFFFF"},
      "on_click": "handle_button_click"
    },
    {
      "id": "label1",
      "type": "label",
      "text": "Hello World",
      "x": 10, "y": 60, "width": 200, "height": 20
    }
  ]
}
```

### Widget Types Supported

| Type | Description | JSON Properties |
|------|-------------|----------------|
| `view` | Container | `children: []` |
| `button` | Clickable button | `text`, `on_click` |
| `label` | Static text | `text`, `font_size` |
| `text_input` | Text input field | `text`, `max_length` |
| `text_editor` | Multi-line editor | `text`, `read_only` |
| `slider` | Value slider | `min`, `max`, `value` |
| `progress_bar` | Progress indicator | `value`, `max` |
| `image` | Bitmap image | `src` (file path) |
| `canvas` | Drawing area | `on_draw` (callback) |

### Full Widget Schema

```json
{
  "id": "string (unique)",
  "type": "view|button|label|text_input|text_editor|slider|progress_bar|image|canvas",
  "x": 0,
  "y": 0,
  "width": 100,
  "height": 50,
  "visible": true,
  "enabled": true,
  "style": {
    "bg_color": "#RRGGBB",
    "fg_color": "#RRGGBB",
    "border_color": "#RRGGBB",
    "border_width": 1,
    "font_size": 12,
    "font": "default|small|large"
  },
  "scrollable": {
    "x": false,
    "y": false
  }
}
```

## Web Editor Features

### 1. Canvas Preview
- Live rendering of the GUI at actual screen size (320x240)
- Zoom in/out for detailed editing
- Grid overlay for precise positioning

### 2. Widget Palette
- Drag widgets from palette to canvas
- Search/filter widgets
- Recently used widgets

### 3. Property Inspector
- Edit widget properties in real-time
- Color pickers for colors
- Sliders for numeric values
- Dropdowns for enums

### 4. Hierarchy View
- Tree view of widget structure
- Drag to reorder
- Parent/child relationships

### 5. JSON Editor
- Raw JSON editing with syntax highlighting
- Validation against schema
- Auto-formatting

### 6. File Management
- Save/load GUI definitions via WebDAV
- Browse SD card files
- Preview before loading

## Integration with GUIKit

### Loading JSON GUI on ESP8266

```c
#include "gui_editor/gui_loader.h"

void setup() {
    // Initialize GUIKit
    guikit_init();
    
    // Load GUI from JSON file on SD card
    GUI* gui = gui_loader_load_from_sd("/ui/main_gui.json");
    
    // Or load from WebDAV
    GUI* gui = gui_loader_load_from_webdav("main_gui.json");
}
```

### Rendering

```c
void loop() {
    gui_render(gui);
    gui_handle_input(gui);
}
```

## Web Server Integration

The web editor communicates with the ESP8266 via:

1. **WebDAV** - For file management (load/save JSON)
2. **WebSocket** - For live preview (optional)
3. **REST API** - For GUI operations (optional)

### WebDAV Endpoints

- `GET /gui/` - List GUI files
- `GET /gui/{name}.json` - Get GUI definition
- `PUT /gui/{name}.json` - Save GUI definition
- `DELETE /gui/{name}.json` - Delete GUI

### WebSocket Events

```json
// Client -> Server
{"type": "preview", "gui": { ... }}

// Server -> Client
{"type": "render", "png": "base64..."}
{"type": "touch", "x": 100, "y": 150}
{"type": "error", "message": "Invalid GUI"}
```

## Getting Started

1. **Setup WebDAV Server** on ESP8266
2. **Upload web editor files** to SD card
3. **Open browser** to `http://<esp-ip>/editor`
4. **Create/load GUI** and start editing
5. **Save** - JSON is stored on SD card
6. **Deploy** - ESP8266 loads and runs the GUI

## File Structure

```
SD Card /
├── web/                    # Web editor files
│   ├── editor/            # Editor application
│   │   ├── index.html
│   │   ├── editor.js
│   │   ├── renderer.js
│   │   └── styles.css
│   └── gui/               # Saved GUI definitions
│       ├── main.json
│       └── settings.json
└── ui/                    # Runtime GUI files (optional)
```

## Example: Creating a Simple GUI

1. Open web editor at `http://192.168.1.100/editor`
2. Click "New GUI" → "320x240"
3. Drag a Button widget to (10, 10)
4. Set properties: text="Start", width=100, height=40
5. Drag a Label to (10, 60)
6. Set properties: text="Status: Ready"
7. Click "Save" → "main_gui.json"
8. On ESP8266: `gui_loader_load("main_gui.json")`

## Browser Compatibility

- Chrome/Edge (recommended)
- Firefox
- Safari (limited)
- Mobile browsers (view-only)

## Performance Considerations

- Web editor runs in browser, no impact on ESP8266
- JSON files are typically < 10KB
- Preview rendering uses Canvas API
- Real-time preview sends only diffs

## Security

- WebDAV authentication (username/password)
- CORS restrictions
- Read-only mode for shared GUIs
- Backup before overwrite

## Future Enhancements

- [ ] Undo/redo for editor actions
- [ ] Widget templates
- [ ] Theme editor
- [ ] Animation preview
- [ ] Export to C code
- [ ] Simulation mode
- [ ] Collaboration (multi-user)
