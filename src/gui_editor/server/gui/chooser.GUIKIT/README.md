# GUIKit Project Chooser

The **GUIKit Project Chooser** is a system GUI that provides a visual interface for selecting and loading GUI projects on the ESP8266 device.

## Overview

This GUI serves as the default entry point when the ESP8266 boots. It:

1. **Scans** the `/gui/` directory on the SD card for all `.GUIKIT` project directories
2. **Displays** a list of available projects with their names and descriptions
3. **Allows** users to select a project and load it
4. **Supports** setting a project as the default to load on future boots
5. **Provides** visual feedback showing which project is currently set as default

## Directory Structure

```
chooser.GUIKIT/
├── main_gui.json          # Main GUI definition with widgets
├── project.meta.json     # Project metadata
├── README.md             # This file
├── assets/               # Static resources (icons, images)
├── gui/                  # Additional GUI files (if needed)
├── scripts/
│   └── chooser.js        # JavaScript logic for the chooser
└── styles/
    └── chooser.css       # CSS theme for the chooser
```

## GUI Layout

The chooser GUI has the following layout (320x240):

```
┌─────────────────────────────────────────┐
│ GUIKit Project Chooser              Default: │
├─────────────────────────────────────────┤
│ 🔄 Refresh                                    │
├─────────────────────────────────────────┤
│ ┌─────────────────────────────────────┐ │
│ │ Project 1 ★                            │ │
│ │ Project 2                              │ │
│ │ Project 3                              │ │
│ │ Project 4                              │ │
│ │ ...                                   │ │
│ └─────────────────────────────────────┘ │
├─────────────────────────────────────────┤
│ Select a project to load                   │
├─────────────────────────────────────────┤
│                         [ Set Default ]     │
│                         [ Load       ]     │
└─────────────────────────────────────────┘
```

### Widgets

| ID | Type | Purpose |
|----|------|---------|
| `title_bar` | view | Blue title bar with name and default indicator |
| `title` | label | "GUIKit Project Chooser" text |
| `default_indicator` | label | Shows current default project |
| `refresh_btn` | button | Rescans for projects |
| `projects_container` | view (scrollable) | Container for project list |
| `projects_list` | view | Holds dynamically created project buttons |
| `info_bar` | view | Status/info message area |
| `info_text` | label | Shows info messages |
| `set_default_btn` | button | Set selected project as default |
| `load_btn` | button | Load the selected project |

## Configuration

The chooser reads from `/etc/guikitloader.conf`:

```ini
# Default GUI project name
default_gui=chooser

# Path to GUI directory
gui_path=/gui

# Use .GUIKIT directories
use_project_dirs=true

# Auto-load last used GUI
auto_load_last=true

# Last loaded GUI
last_gui=
```

## JavaScript API

The chooser provides the following JavaScript API:

### Functions

- `init_chooser()` - Initialize the chooser state
- `refresh_projects()` - Rescan for projects and rebuild the list
- `select_project(widget, event)` - Handle project selection
- `load_selected_project(widget, event)` - Load the selected project
- `set_as_default(widget, event)` - Set selected project as default
- `loadProject(projectName)` - Load a project by name
- `loadDefaultGUI()` - Load the default GUI (kernel entry point)
- `scanProjects()` - Scan and return list of projects

### Global Object

```javascript
GUIKitChooser = {
    init: function() {},
    refresh: function() {},
    loadDefault: function() {},
    loadProject: function(projectName) {},
    scanProjects: function() {},
    getSelected: function() {},
    getProjects: function() {},
    setDefault: function(projectName) {}
}
```

## Integration with kernel.bin

The ESP8266 kernel should call `loadDefaultGUI()` on boot:

```c
// In kernel.bin initialization
#include "guikit_config.h"

void setup() {
    // Initialize SD card
    if (!SD.begin(SS, SPI)) {
        // Handle error
        return;
    }
    
    // Load and execute the default GUI
    GuikitConfig config;
    guikit_config_init(&config);
    guikit_config_load(&config);
    
    char buffer[GUI_LOADER_MAX_SIZE];
    int bytes_read = guikit_config_load_default_gui(buffer, sizeof(buffer));
    
    if (bytes_read > 0) {
        // Parse and execute the GUI
        buffer[bytes_read] = '\0';
        void* gui = gui_loader_parse_json(buffer);
        if (gui) {
            gui_execute(gui);
            free(gui);
        }
    }
}
```

## Project Discovery

The chooser scans the `/gui/` directory for all subdirectories ending in `.GUIKIT`:

```
/gui/
├── chooser.GUIKIT/          ← This chooser
├── MyProject.GUIKIT/        ← User project 1
├── Dashboard.GUIKIT/        ← User project 2
└── Settings.GUIKIT/         ← User project 3
```

Each project must contain at least a `main_gui.json` file to be valid.

## Styling

The chooser uses a dark theme optimized for TFT displays:

- **Background**: `#1E1E1E` (dark gray)
- **Title bar**: Blue gradient (`#0078D4` to `#005A9E`)
- **Project buttons**: Dark gray (`#2D2D2D`) with light text
- **Selected button**: Blue (`#0078D4`) with white text
- **Default marker**: Gold star (★) next to default project name

## Usage Flow

1. **User presses Refresh**: `refresh_projects()` scans `/gui/` for `.GUIKIT` dirs
2. **User selects project**: `select_project()` highlights the button
3. **User clicks "Set Default"**: `set_as_default()` updates `/etc/guikitloader.conf`
4. **User clicks "Load"**: `load_selected_project()` loads the selected project's `main_gui.json`

## Customization

To customize the chooser:

1. Edit `main_gui.json` to change layout, colors, or add/remove widgets
2. Edit `scripts/chooser.js` to modify the logic
3. Edit `styles/chooser.css` to change the visual theme
4. Update `project.meta.json` to change metadata

## Example: Adding a Project

To add a new project that appears in the chooser:

1. Create a directory: `/gui/MyProject.GUIKIT/`
2. Add `main_gui.json` with your GUI definition
3. Optionally add `project.meta.json` with metadata:
   ```json
   {
     "name": "MyProject",
     "description": "My custom GUI project",
     "version": "1.0.0"
   }
   ```
4. The chooser will automatically detect and list it on next refresh

## Fallback Behavior

If `default_gui` is set to `chooser` and the chooser itself fails to load:
1. Try `last_gui` if `auto_load_last` is true
2. Try `/gui/main_gui.json` (flat file fallback)
3. Show error message on display
