# GUIKit Image Assets

This directory contains sample images for GUIKit projects.

## Adding Images to Your Project

### 1. Create BMP Files

Use **ImageMagick** or similar tool to convert images to BMP format:

```bash
# Convert to 16-bit BMP (565 format - recommended for ESP8266)
convert input.png -depth 16 -colorspace RGB output.bmp

# Convert to 24-bit BMP
convert input.png output.bmp

# Resize to appropriate dimensions
convert input.png -resize 100x100 output.bmp
```

### 2. BMP Requirements for TFT_eSPI

| Property | Requirement |
|----------|-------------|
| Format | BMP (Bitmap) |
| Bit Depth | 24-bit, 16-bit (565), or 8-bit with palette |
| Color Order | BGR (standard BMP) |
| Compression | None (uncompressed) |
| Origin | Bottom-left (default) or top-left |

### 3. Supported Scale Modes in GUIKit

| Mode | Description |
|------|-------------|
| `none` | Draw at native size from top-left |
| `stretch` | Fill entire widget bounds |
| `aspect_fit` | Fit within bounds, maintain aspect, letterbox |
| `aspect_fill` | Fill bounds, maintain aspect, crop |
| `tile` | Repeat to fill area |
| `center` | Center at native size |

### 4. JSON Image Widget Example

```json
{
  "id": "my_logo",
  "type": "image",
  "x": 50,
  "y": 50,
  "width": 100,
  "height": 100,
  "source": "/assets/images/logo.bmp",
  "format": "bmp",
  "scale_mode": "aspect_fit",
  "transparent_color": "#FF00FF"
}
```

### 5. Transparency

Specify a color in hex format (`#RRGGBB`) that should be treated as transparent:

```json
{
  "type": "image",
  "source": "/assets/images/icon.bmp",
  "transparent_color": "#00FF00"  // Green is transparent
}
```

### 6. Memory Optimization

**ESP8266 RAM is limited (~80KB available)**

| Image Size | Memory (16-bit) | Recommendation |
|------------|-----------------|----------------|
| 16x16 | 512 bytes | ✅ Cache |
| 32x32 | 2,048 bytes | ✅ Cache |
| 64x64 | 8,192 bytes | ✅ Cache |
| 100x100 | 20,000 bytes | ⚠️ Load on demand |
| 200x200 | 80,000 bytes | ❌ Don't cache |
| 320x240 | 153,600 bytes | ❌ Don't cache |

### 7. Using Sprites (Advanced)

For better performance with multiple images:

```json
{
  "id": "my_sprite",
  "type": "sprite",
  "x": 0,
  "y": 0,
  "width": 100,
  "height": 100,
  "source": "/assets/images/sprite.bmp",
  "visible": true
}
```

### 8. File Naming Convention

- Use lowercase with underscores: `my_image.bmp`
- Include dimensions: `logo_100x100.bmp`
- Group by purpose: `icons/`, `backgrounds/`, `logos/`

### 9. Directory Structure

```
project.GUIKIT/
├── main_gui.json
├── project.meta.json
└── assets/
    └── images/
        ├── logo.bmp
        ├── background.bmp
        ├── icon_button.bmp
        └── README.md  (this file)
```

### 10. Pre-conversion Tools

**Online BMP Converters:**
- https://imageonline.co/?bmpconverter
- https://convertio.co/image-converter/

**Command Line (ImageMagick):**
```bash
# Install ImageMagick (macOS)
brew install imagemagick

# Convert and resize
convert input.jpg -resize 100x100 -depth 16 logo.bmp
```

**Python Script:**
```python
from PIL import Image

img = Image.open('input.png')
img = img.resize((100, 100))
img.save('output.bmp', 'BMP')
```

---

## Sample Image

A sample 8x8 checkerboard BMP would look like this in hex:

```
42 4D        # Signature 'BM'
36 06 00 00  # File size: 1654 bytes
00 00        # Reserved
00 00        # Reserved
36 00 00 00  # Data offset: 54
28 00 00 00  # DIB header size: 40
08 00 00 00  # Width: 8
08 00 00 00  # Height: 8
01 00        # Planes: 1
18 00        # Bit count: 24
...          # Pixel data
```

---

## Troubleshooting

**Image not displaying?**
- Check file path is correct
- Verify BMP format (24-bit or 16-bit)
- Ensure file is not corrupted
- Check RAM usage (don't cache large images)

**Colors look wrong?**
- BMP uses BGR order, not RGB
- Try converting with ImageMagick: `convert input.png -colorspace RGB output.bmp`

**Out of memory?**
- Reduce image size
- Don't cache large images
- Use `cache: false` in widget definition
