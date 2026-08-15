# PNG to RGB Converter Task

## Overview

The **PNG Converter** is a heavy task (Task B) that performs PNG to RGB conversion. It demonstrates the task switching system in action with a real-world use case.

### Key Characteristics

- **Heavy Operation**: PNG decoding is CPU and memory intensive
- **Task Switching**: Runs as Task B, launched from Task A (GUI)
- **Communication**: Results saved to SD card for parent task
- **Standalone**: Can be compiled as a separate kernel (`/kernel/png_converter.bin`)
- **Lightweight**: Designed for embedded systems (ESP8266/ESP32)

### Use Case

```
User in GUI (Task A) selects a PNG image
  ↓
GUI calls: task_switch_to("/kernel/png_converter.bin")
  ↓
PNG Converter (Task B) runs
  - Freezes Task A RAM to SD card
  - Loads PNG file from SD
  - Decodes PNG to RGB
  - Saves RGB data to /tmp/task_comm/
  ↓
User clicks "Back" button
  ↓
PNG Converter calls: task_restore()
  ↓
Task A (GUI) resumes with full state
  - Loads RGB data from /tmp/task_comm/
  - Displays converted image
```

---

## File Structure

```
/src/kernel/
├── png_converter.h      # Header file with API
└── png_converter.c      # Implementation

/kernel/
└── png_converter.bin    # Compiled binary (created by build system)

SD Card (after conversion):
├── /tmp/task_comm/
│   ├── converted.hdr     # RGB image header (PngRgbHeader struct)
│   └── converted.rgb     # Raw RGB pixel data
```

---

## API Reference

### Main Functions

```c
#include "png_converter.h"

// Run the converter (called by Task B)
bool png_converter_run(const char* png_path);

// Standalone entry point (when compiled as kernel)
void png_converter_task_main(void);
```

### Integration with Task A (GUI)

```c
// Trigger conversion from GUI
bool gui_trigger_png_conversion(const char* png_path);

// Load results after restore
uint32_t gui_load_png_conversion_results(
    PngRgbHeader* header, 
    uint8_t* rgb_buffer, 
    uint32_t buffer_size
);
```

### Data Structures

```c
// RGB image header for communication
typedef struct {
    uint16_t width;            // Image width in pixels
    uint16_t height;           // Image height in pixels
    uint8_t bits_per_pixel;    // Bits per pixel (24 for RGB888)
    uint32_t data_size;        // Size of RGB data in bytes
    uint32_t crc32;            // CRC32 checksum of RGB data
    uint8_t reserved[16];      // Reserved for future use
} PngRgbHeader;
```

### Error Handling

```c
PngConverterError err = png_converter_get_error();
const char* msg = png_converter_error_to_string(err);

// Error codes:
PNG_ERR_NONE              // No error
PNG_ERR_FILE_NOT_FOUND    // PNG file not found
PNG_ERR_INVALID_PNG       // Invalid PNG signature
PNG_ERR_UNSUPPORTED_FORMAT // PNG format not supported
PNG_ERR_MEMORY_ERROR      // Memory allocation failed
PNG_ERR_SD_CARD_ERROR      // SD card read/write error
PNG_ERR_TOO_LARGE         // Image exceeds max dimensions
```

---

## Usage Example

### Task A: GUI Code

```c
#include "png_converter.h"

void on_image_selected(const char* png_path) {
    // Show conversion button
    show_button("Convert to RGB", on_convert_clicked, (void*)png_path);
}

void on_convert_clicked(void* arg) {
    const char* png_path = (const char*)arg;
    
    if (gui_trigger_png_conversion(png_path)) {
        // After restore, we continue here
        // Task A state is fully restored
        
        PngRgbHeader header;
        uint8_t rgb_buffer[1024 * 768 * 3];  // Max supported size
        
        uint32_t loaded = gui_load_png_conversion_results(
            &header, rgb_buffer, sizeof(rgb_buffer)
        );
        
        if (loaded > 0) {
            // Display the converted image
            display_rgb_image(&header, rgb_buffer);
        }
    }
}
```

### Task B: PNG Converter (Standalone)

```c
#include "png_converter.h"

// This is the entry point when compiled as a separate kernel
// In PlatformIO, this would be in main.cpp

extern "C" void setup() {
    // Hardware initialization
    init_serial(115200);
    init_sd_card();
    init_tft();
    
    // Run PNG converter
    png_converter_task_main();
}

extern "C" void loop() {
    // In a real system with task switching,
    // the loop would be in the bootloader
    // After task_restore(), execution returns to parent
}
```

### Task B: With Event Loop

```c
void png_converter_with_event_loop(const char* png_path) {
    task_switcher_init();
    
    // Run conversion
    bool success = png_converter_run(png_path);
    
    if (!success) {
        // Show error on TFT
        tft_clear(BG_COLOR);
        tft_draw_text(10, 10, "Conversion Failed!", ERROR_COLOR, BG_COLOR);
        tft_draw_text(10, 30, png_converter_error_to_string(g_last_error), 
                     TEXT_COLOR, BG_COLOR);
    } else {
        // Show success
        tft_clear(BG_COLOR);
        tft_draw_text(10, 10, "Conversion Complete!", SUCCESS_COLOR, BG_COLOR);
        tft_draw_text(10, 30, "Press Back to return", TEXT_COLOR, BG_COLOR);
    }
    
    // Event loop
    while (1) {
        check_input();
        
        if (back_button_pressed()) {
            task_restore();
        }
        
        delay(10);
    }
}
```

---

## Implementation Details

### PNG Format Support

This implementation supports basic PNG features:

| Feature | Supported | Notes |
|---------|-----------|-------|
| PNG Signature | ✅ | 8-byte magic number |
| IHDR Chunk | ✅ | Width, height, bit depth, color type |
| IDAT Chunks | ✅ | Image data (decoded in mock) |
| IEND Chunk | ✅ | End of image |
| PLTE Chunk | ❌ | Palette (indexed color) |
| Alpha Channel | ❌ | RGBA not yet supported |
| Interlacing | ❌ | Only non-interlaced |
| Compression | ❌ | Uses mock decompression |
| Filtering | ❌ | Uses mock filter removal |

### Memory Requirements

| Image Size | RGB Size | RAM Needed |
|------------|----------|------------|
| 320x240 | 230KB | ~250KB |
| 480x320 | 460KB | ~500KB |
| 640x480 | 900KB | ~1MB |
| 800x600 | 1.4MB | ~1.5MB |
| 1024x768 | 2.3MB | ~2.5MB |

**Note:** The PNG converter runs in the same RAM space as Task A. Ensure you have enough free RAM before switching.

### Performance

| Platform | Decode Time (320x240) | Notes |
|----------|---------------------|-------|
| ESP8266 @ 80MHz | ~500ms | With optimized library |
| ESP8266 @ 160MHz | ~250ms | With optimized library |
| ESP32 @ 240MHz | ~100ms | With PSRAM |

---

## Production Integration

### Recommended Libraries

For production use, replace the mock decoder with a real library:

#### 1. PNGdec (Recommended)

```c
#include <PNGdec.h>

PNG png;

bool decode_with_pngdec(const char* filename, uint8_t* buffer) {
    png.openRAM((uint8_t*)buffer, buffer_size, png_callback);
    png.decodeFile(filename);
    png.close();
    return true;
}
```

#### 2. LodePNG

```c
#include "lodepng.h"

bool decode_with_lodepng(const char* filename, 
                        uint8_t* buffer, 
                        uint32_t* width, 
                        uint32_t* height) {
    return lodepng_decode24_file(buffer, *width, *height, filename) == 0;
}
```

#### 3. Custom Minimal Decoder

For maximum control and minimum footprint:

```c
// Read PNG chunks
while (!end_of_file) {
    read_chunk_length();
    read_chunk_type();
    
    if (type == IHDR) parse_ihdr();
    if (type == IDAT) decode_idat();
    if (type == IEND) break;
}
```

---

## Build System Integration

### PlatformIO Configuration

Add to `platformio.ini`:

```ini
[env:esp8266_png_converter]
platform = espressif8266
board = nodemcuv2
framework = arduino
src_file = src/kernel/png_converter.cpp
build_flags = -D PNG_CONVERTER_STANDALONE
```

### Build Script

The `build.sh` script should create the PNG converter binary:

```bash
# Build PNG converter
pio run -e esp8266_png_converter

# Copy to SD card structure
cp .pio/build/esp8266_png_converter/firmware.bin /sdcard/system/png_converter.bin
```

---

## Testing

### Test with Mock Data

```c
// Test PNG validation
bool test_png_validation() {
    return png_is_valid_file("/test/test.png");
}

// Test dimensions
bool test_png_dimensions() {
    uint16_t w, h;
    bool result = png_get_dimensions("/test/test.png", &w, &h);
    printf("Dimensions: %ux%u\n", w, h);
    return result;
}

// Test full conversion
bool test_png_conversion() {
    PngRgbHeader header;
    uint8_t buffer[320 * 240 * 3];
    return png_converter_run("/test/test.png");
}
```

### Expected Output

```
[PNG_CONVERTER] Starting conversion
[PNG_CONVERTER] Input: /images/input.png
[PNG_CONVERTER] Image: 800x600
[PNG_CONVERTER] Decoded to RGB: 1440000 bytes
[PNG_CONVERTER] CRC32: A1B2C3D4
[PNG_CONVERTER] Results saved to SD card
[PNG_CONVERTER] Header: /tmp/task_comm/converted.hdr
[PNG_CONVERTER] RGB: /tmp/task_comm/converted.rgb
[PNG_CONVERTER] Conversion complete!
[PNG_CONVERTER] Press Back button to return to GUI...
[PNG_CONVERTER] Auto-returning to parent (demo mode)...
```

---

## Files

| File | Description |
|------|-------------|
| `src/kernel/png_converter.h` | Header with API and data structures |
| `src/kernel/png_converter.c` | Implementation with mock decoder |
| `docs/PNG_CONVERTER.md` | This documentation |

---

## Limitations

1. **Mock Decoder**: The current implementation creates a gradient pattern instead of actual PNG decoding. Replace with a real library for production.

2. **Memory**: Large images may not fit in RAM. Consider streaming or chunked processing.

3. **PNG Features**: Only basic PNG features are validated. Advanced features (palette, alpha, interlacing) need library support.

4. **Performance**: PNG decoding is slow on ESP8266. Consider hardware acceleration or ESP32.

5. **Error Recovery**: Limited error recovery. Invalid PNG files may cause crashes.

---

## Future Enhancements

- [ ] Integrate PNGdec library for real decoding
- [ ] Add support for RGBA (alpha channel)
- [ ] Add support for indexed color (palette)
- [ ] Add support for interlaced PNGs
- [ ] Add progressive rendering (show partial results)
- [ ] Add memory-efficient streaming decoder
- [ ] Add hardware-accelerated decoding (ESP32)
- [ ] Add support for transparent backgrounds

---

## Version History

- **Latest**: PNG to RGB converter task with task switcher integration
- **Latest**: Mock decoder with gradient pattern for demo
- **Latest**: Full API for Task A/Task B communication
- **Latest**: Documentation and examples

---

## References

- [PNG Specification](https://www.w3.org/TR/PNG/)
- [PNGdec Library](https://github.com/bitbank2/PNGdec)
- [LodePNG](https://lodev.org/lodepng/)
- [ESP8266 PNG Decoding](https://github.com/bitbank2/PNGdec)
