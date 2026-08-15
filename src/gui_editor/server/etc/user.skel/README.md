# User Home Directory Template

This directory (`/etc/user.skel/`) serves as a template for new user home directories on the ESP8266 system.

## Directory Structure

```
/etc/user.skel/
└── projects/                    # User's GUIKit projects directory
    └── README.md               # This file
```

When a new user is created, this directory is copied to the user's home directory (e.g., `/home/username/`).

## User Projects Directory

Each user has a `projects/` directory where they can store their GUIKit projects:

```
/home/username/projects/
├── MyProject.GUIKIT/
│   ├── main_gui.json
│   ├── project.meta.json
│   ├── assets/
│   │   └── (images, fonts, etc.)
│   ├── gui/
│   │   └── (additional GUI files)
│   ├── scripts/
│   │   └── (Lua/JavaScript files)
│   └── styles/
│       └── (CSS/theme files)
│
├── Dashboard.GUIKIT/
│   └── ...
│
└── Settings.GUIKIT/
    └── ...
```

## Project Structure

Each GUIKit project follows the standard structure:

```
{project_name}.GUIKIT/
├── main_gui.json          # Required: Root GUI definition
├── project.meta.json     # Optional: Project metadata
├── assets/               # Optional: Static resources
│   └── (images, fonts, sounds, etc.)
├── gui/                  # Optional: Additional GUI files
│   └── (sub_gui.json, etc.)
├── scripts/              # Optional: Script files
│   └── (lua, js files)
└── styles/               # Optional: CSS/theme files
    └── (theme.css, etc.)
```

## Creating a New Project

To create a new project in your user directory:

1. Navigate to your projects directory:
   ```bash
   cd ~/projects
   ```

2. Create a new project directory with `.GUIKIT` suffix:
   ```bash
   mkdir MyProject.GUIKIT
   cd MyProject.GUIKIT
   ```

3. Create the main GUI file:
   ```bash
   cat > main_gui.json << 'EOF'
   {
     "version": "1.0",
     "name": "MyProject",
     "size": { "width": 320, "height": 240 },
     "background": "#000000",
     "widgets": []
   }
   EOF
   ```

4. Optionally create metadata:
   ```bash
   cat > project.meta.json << 'EOF'
   {
     "name": "MyProject",
     "description": "My custom GUI project",
     "author": "Your Name",
     "version": "1.0.0",
     "created": "2026-08-15T00:00:00Z",
     "modified": "2026-08-15T00:00:00Z",
     "gui_files": ["main_gui.json"],
     "dependencies": []
   }
   EOF
   ```

## Deploying Projects to ESP8266

To deploy your user projects to the ESP8266 SD card:

### Option 1: WebDAV (Recommended)

Use the GUIKit Web Editor to create and save projects directly to WebDAV:

1. Open the GUIKit Web Editor
2. Create a new project with "webdav" location
3. Design your GUI
4. Save the project - it will be saved to `/webdav/gui/{name}.GUIKIT/`

### Option 2: Manual SD Card Copy

1. Insert SD card into computer
2. Copy your project directory:
   ```bash
   cp -r ~/projects/MyProject.GUIKIT /Volumes/SDCARD/gui/
   ```
3. Update the chooser (optional):
   - The chooser will automatically detect it on next refresh

## Using the Chooser

The GUIKit Project Chooser (`chooser.GUIKIT`) will automatically scan the `/gui/` directory and display all available `.GUIKIT` projects:

1. On ESP8266 boot, the chooser loads
2. It scans `/gui/` for all `.GUIKIT` directories
3. Each valid project (with `main_gui.json`) appears as a button
4. Select a project and click "Load" to run it
5. Click "Set Default" to make it load automatically on next boot

## Project Metadata

The `project.meta.json` file contains:

```json
{
  "name": "Project name (without .GUIKIT suffix)",
  "description": "Short description of the project",
  "author": "Author name",
  "version": "Version string (e.g., 1.0.0)",
  "created": "ISO 8601 timestamp",
  "modified": "ISO 8601 timestamp",
  "gui_files": ["main_gui.json", "settings.json"],
  "dependencies": ["library1", "library2"]
}
```

## Multiple Users

Each user on the system has their own `projects/` directory. The ESP8266 can support multiple users with different project sets:

```
/home/alice/projects/
├── AliceProject1.GUIKIT/
└── AliceProject2.GUIKIT/

/home/bob/projects/
├── BobProject1.GUIKIT/
└── BobProject2.GUIKIT/
```

## Sharing Projects

To share a project between users:

1. Copy the project directory:
   ```bash
   cp -r /home/alice/projects/Shared.GUIKIT /home/bob/projects/
   ```

2. Or deploy to the shared `/gui/` directory:
   ```bash
   cp -r ~/projects/Shared.GUIKIT /gui/
   ```

## System Projects vs User Projects

| Location | Type | Managed By | Visible To |
|----------|------|------------|------------|
| `/gui/` | System | Admin/Web Editor | All users |
| `/home/username/projects/` | User | Individual user | That user only |

**System projects** in `/gui/` are available to all users and appear in the chooser.
**User projects** in `~/projects/` are private to that user.

## Configuration

The main configuration file is at `/etc/guikitloader.conf`:

```ini
# Default GUI to load on boot
default_gui=chooser

# Path to GUI directory
gui_path=/gui

# Use .GUIKIT directories
use_project_dirs=true

# Auto-load last used GUI
auto_load_last=true

# Last loaded GUI
last_gui=chooser
```

## Tips

1. **Naming**: Always use `.GUIKIT` suffix for project directories
2. **Required file**: Each project must have `main_gui.json`
3. **Validation**: The chooser will only show projects with valid `main_gui.json`
4. **Refresh**: Click "Refresh" in the chooser to rescan for new projects
5. **Backup**: Regularly backup your `projects/` directory

## See Also

- `/gui/chooser.GUIKIT/` - The project chooser GUI
- `/etc/guikitloader.conf` - System configuration
- GUIKit Web Editor documentation
