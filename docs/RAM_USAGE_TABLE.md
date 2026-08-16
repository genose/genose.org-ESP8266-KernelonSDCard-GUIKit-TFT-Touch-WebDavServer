# Internal RAM Usage Table - ESP8266/ESP32 GUIKit

**Platform:** ESP8266 (80KB free internal RAM) / ESP32 (320KB free internal RAM)  
**Measurement:** Bytes (B) and Kilobytes (KB)

---

## 📊 Quick Summary

| Component | ESP8266 | ESP32 | % of Free RAM | Notes |
|-----------|---------|-------|---------------|-------|
| **Bootloader RAM Usage** | ~25KB | ~25KB | ~31% / ~8% | Peak during boot |
| **Kernel RAM Usage** | ~110KB | ~110KB | Won't fit / ~34% | Without external RAM |
| **Kernel with External RAM** | ~20KB | ~20KB | ~25% / ~6% | Actual internal usage |
| **Combined (Boot + Kernel)** | ~45KB | ~135KB | ~56% / ~42% | With external RAM |

**Key Insight:** The kernel alone (~110KB) exceeds ESP8266's 80KB free RAM, which is why **external RAM or SD Card Swap is required** for ESP8266. ESP32 can run the kernel in internal RAM with room to spare.

---

## 🥾 Bootloader RAM Usage

Detailed breakdown of internal RAM consumption during boot process.

| Component | ESP8266 | ESP32 | % of RAM | Notes |
|-----------|---------|-------|----------|-------|
| Bootloader Code | 8,192 | 8,192 | ~10% | Core logic |
| Hardware Detection | 2,048 | 2,048 | ~2.5% | SPI, devices |
| MCU Detection | 512 | 512 | ~0.6% | Platform ID |
| SMP Detection | 256 | 512 | ~0.3-0.6% | ESP32 only |
| SPI Bus Init | 1,024 | 1,024 | ~1.3% | Pins, speed |
| SPI Device Enum | 2,048 | 2,048 | ~2.5% | Device list |
| **RAM Length Test** | **3,072** | **3,072** | **~4%** | **Pattern buffers** |
| &nbsp;&nbsp;&nbsp;&nbsp;- Test patterns | 1,024 | 1,024 | | 0xAA, 0x55 |
| &nbsp;&nbsp;&nbsp;&nbsp;- Progress tracking | 512 | 512 | | TFT display |
| &nbsp;&nbsp;&nbsp;&nbsp;- Result storage | 512 | 512 | | RamTestInfo |
| &nbsp;&nbsp;&nbsp;&nbsp;- Binary search | 1,024 | 1,024 | | Address tracking |
| RAM Initialization | 1,024 | 1,024 | ~1.3% | Internal+external |
| SD Card Init | 1,024 | 1,024 | ~1.3% | SdFat library |
| Autostart Config | 2,048 | 2,048 | ~2.5% | INI parser |
| TFT Init | 2,048 | 2,048 | ~2.5% | ST7789 driver |
| Kernel Check | 1,024 | 1,024 | ~1.3% | Size validation |
| Memory Strategy | 2,048 | 2,048 | ~2.5% | Config structs |
| Memory Strategy Test | 1,024 | 1,024 | ~1.3% | GUI size testing |
| Display Results | 1,024 | 1,024 | ~1.3% | TFT summary |
| Error Handling | 512 | 512 | ~0.6% | Messages, codes |
| **TOTAL BOOTLOADER** | **~25KB** | **~25KB** | **~31%** | **Peak usage** |
| **Stack Usage** | 4,096 | 4,096 | ~5% | Call stack |
| **Heap Usage** | ~21KB | ~21KB | ~26% | Dynamic alloc |

---

## 🖥️ Kernel RAM Usage

Detailed breakdown of internal RAM consumption by the kernel at runtime.

| Component | ESP8266 | ESP32 | % of RAM | Notes |
|-----------|---------|-------|----------|-------|
| Kernel Code | 32,768 | 32,768 | ~40% | Main logic |
| GUIKit Core | 16,384 | 16,384 | ~20% | Widget system |
| Widget Definitions | 4,096 | 4,096 | ~5% | Type structures |
| Rendering Engine | 8,192 | 8,192 | ~10% | TFT drawing |
| Touch Handling | 2,048 | 2,048 | ~2.5% | XPT2046 driver |
| **WebDAV Server** | **8,192** | **8,192** | **~10%** | **HTTP + WebDAV** |
| &nbsp;&nbsp;&nbsp;&nbsp;- Connection mgmt | 2,048 | 2,048 | | Client state |
| &nbsp;&nbsp;&nbsp;&nbsp;- Request parsing | 2,048 | 2,048 | | Buffer |
| &nbsp;&nbsp;&nbsp;&nbsp;- File I/O | 2,048 | 2,048 | | SdFat operations |
| &nbsp;&nbsp;&nbsp;&nbsp;- Auth system | 2,048 | 2,048 | | User management |
| HTTP Server | 4,096 | 4,096 | ~5% | Web interface |
| mDNS Service | 2,048 | 2,048 | ~2.5% | Bonjour/Zeroconf |
| **File Manager** | **4,096** | **4,096** | **~5%** | **SD operations** |
| &nbsp;&nbsp;&nbsp;&nbsp;- Path handling | 1,024 | 1,024 | | String ops |
| &nbsp;&nbsp;&nbsp;&nbsp;- File caching | 2,048 | 2,048 | | Recent files |
| &nbsp;&nbsp;&nbsp;&nbsp;- Quota system | 1,024 | 1,024 | | User limits |
| JSON Parser | 2,048 | 2,048 | ~2.5% | Streaming parser |
| GUI Loader | 4,096 | 4,096 | ~5% | JSON to widgets |
| **Memory Strategy** | **4,096** | **4,096** | **~5%** | **Strategy engine** |
| &nbsp;&nbsp;&nbsp;&nbsp;- Strategy config | 1,024 | 1,024 | | Thresholds |
| &nbsp;&nbsp;&nbsp;&nbsp;- Bank management | 2,048 | 2,048 | | RAM banks |
| &nbsp;&nbsp;&nbsp;&nbsp;- Loading logic | 1,024 | 1,024 | | Strategy select |
| **Task Switcher** | **2,048** | **2,048** | **~2.5%** | **Freeze/thaw** |
| &nbsp;&nbsp;&nbsp;&nbsp;- Task A state | 1,024 | 1,024 | | Saved context |
| &nbsp;&nbsp;&nbsp;&nbsp;- Task B state | 1,024 | 1,024 | | Active context |
| Task Progress | 512 | 512 | ~0.6% | Minimal display |
| **PNG Converter** | **1,024** | **1,024** | **~1.3%** | **Decode state** |
| &nbsp;&nbsp;&nbsp;&nbsp;- Decode config | 512 | 512 | | Mode settings |
| **RAM Freeze System** | **2,048** | **2,048** | **~2.5%** | **Save/restore** |
| &nbsp;&nbsp;&nbsp;&nbsp;- Compression buf | 1,024 | 1,024 | | GZIP state |
| &nbsp;&nbsp;&nbsp;&nbsp;- SD I/O | 1,024 | 1,024 | | File operations |
| Network Stack | 8,192 | 8,192 | ~10% | WiFi, TCP/IP |
| System Utilities | 4,096 | 4,096 | ~5% | Time, logging |
| **TOTAL KERNEL** | **~110KB** | **~110KB** | **~138%*** | **Without external RAM** |
| **With External RAM** | **~20KB** | **~20KB** | **~25%** | **Actual internal usage** |
| **Stack Usage** | 8,192 | 8,192 | ~10% | Call stack |
| **Heap Usage** | ~12KB | ~12KB | ~15% | Dynamic alloc |

> *** Note: 138% of ESP8266's 80KB free RAM means the kernel **requires external RAM** or **SD Card Swap** to function.

---

## 📈 Per-Feature RAM Breakdown

| Feature | ESP8266 | ESP32 | Notes |
|---------|---------|-------|-------|
| **Core System** | | | |
| - Bootloader | 25,600 | 25,600 | With all features |
| - Kernel base | 40,960 | 40,960 | Minimal |
| - Network stack | 8,192 | 8,192 | WiFi + TCP/IP |
| **Total Core** | **74,752** | **74,752** | ~93% of ESP8266 |
| **GUI & Rendering** | | | |
| - GUIKit framework | 16,384 | 16,384 | Widget system |
| - TFT renderer | 8,192 | 8,192 | Drawing engine |
| - Touch handler | 2,048 | 2,048 | Input system |
| **Total GUI** | **26,624** | **26,624** | ~33% of ESP8266 |
| **Web Services** | | | |
| - WebDAV server | 8,192 | 8,192 | Full server |
| - HTTP server | 4,096 | 4,096 | Web interface |
| - mDNS service | 2,048 | 2,048 | Discovery |
| **Total Web** | **14,336** | **14,336** | ~18% of ESP8266 |
| **File System** | | | |
| - File manager | 4,096 | 4,096 | SD operations |
| - JSON parser | 2,048 | 2,048 | Streaming |
| - GUI loader | 4,096 | 4,096 | JSON to widgets |
| **Total FS** | **10,240** | **10,240** | ~13% of ESP8266 |
| **Advanced Features** | | | |
| - Task switcher | 2,048 | 2,048 | Freeze/thaw |
| - Task progress | 512 | 512 | Minimal display |
| - PNG converter | 1,024 | 1,024 | Decode state |
| - RAM freeze | 2,048 | 2,048 | Save/restore |
| - RAM length test | 3,072 | 3,072 | Bootloader only |
| **Total Advanced** | **8,704** | **8,704** | ~11% of ESP8266 |
| **GRAND TOTAL** | **134,656** | **134,656** | **~168% of ESP8266** / **~42% of ESP32** |

---

## 🎯 RAM Optimization Notes

### ESP8266 (80KB free internal RAM)
- **Bootloader:** ~25KB (31% of free RAM) - ✅ Fits comfortably
- **Kernel:** ~110KB - ❌ **WON'T FIT in internal RAM alone**
- **Solution:** Use external RAM (128KB SRAM) + SD Card Swap
- **Typical internal usage:** ~25KB during boot, ~20KB for kernel data (when using external RAM)
- **Memory Strategy:** STOP-at-first-success (External RAM → SD Swap → Internal RAM)

### ESP32 (320KB free internal RAM)
- **Bootloader:** ~25KB (8% of free RAM) - ✅ Very lightweight
- **Kernel:** ~110KB (34% of free RAM) - ✅ Fits with room to spare
- **Solution:** Can run entirely in internal RAM, or use PSRAM for larger GUIs
- **Typical internal usage:** ~25KB boot + ~110KB kernel = ~135KB (42% of free RAM)
- **Memory Strategy:** Same STOP-at-first-success logic, but more headroom

---

## 📊 Memory Strategy Impact

How GUI sizes are handled based on available RAM:

| GUI Size | ESP8266 (Internal) | ESP32 (Internal) | Strategy Used |
|----------|-------------------|------------------|----------------|
| 1KB | ✅ 80KB | ✅ 320KB | INTERNAL_RAM |
| 4KB | ✅ 80KB | ✅ 320KB | INTERNAL_RAM |
| 8KB | ❌ 80KB | ✅ 320KB | EXTERNAL_RAM |
| 16KB | ❌ 80KB | ✅ 320KB | EXTERNAL_RAM |
| 32KB | ❌ 80KB | ✅ 320KB | EXTERNAL_RAM |
| 64KB | ❌ 80KB | ✅ 320KB | EXTERNAL_RAM |
| 128KB | ❌ 80KB | ✅ 320KB | EXTERNAL_RAM |
| 256KB | ❌ 80KB | ❌ 320KB | SD_CARD_SWAP |
| 512KB+ | ❌ 80KB | ❌ 320KB | SD_CARD_SWAP |

> ✅ = Fits in internal RAM  
> ❌ = Does not fit, uses external RAM or SD swap

---

## 🔄 Bootloader vs Kernel Comparison

| Aspect | Bootloader | Kernel | Notes |
|--------|------------|--------|-------|
| **Size (Flash)** | 8-16KB | 256KB-2MB | Bootloader is minimal |
| **Internal RAM Usage** | ~25KB | ~20KB | Kernel is optimized |
| **External RAM Usage** | Minimal | Variable | GUI data stored externally |
| **SD Card Usage** | Config only | Kernel bin | Kernel loaded from SD |
| **TFT Usage** | Yes | Yes | Progress display |
| **Duration** | ~1-2 sec | Runtime | Boot time only |
| **Critical Features** | Detection | Services | Different roles |

---

## 💡 Key Takeaways

1. **ESP8266 requires external RAM** - The kernel (~110KB) exceeds the 80KB free internal RAM
2. **Bootloader is lightweight** - Only ~25KB RAM usage, fits on both platforms
3. **RAM Length Test adds ~3KB** - Small overhead for big reliability benefits
4. **Memory Strategy enables flexibility** - Automatically uses best available option
5. **ESP32 has more headroom** - Can run kernel in internal RAM with ~185KB free for GUI data

---

## 📚 Related Documentation

- [Memory Strategy Configuration](memory_strategy_config.md) - Detailed configuration
- [Bootloader Documentation](../src/boot/README.md) - Complete bootloader info
- [Bootloader Simulations](BOOTLOADER_SIMULATION.md) - Example boot outputs
- [HARDWARE.md](HARDWARE.md) - Hardware requirements
