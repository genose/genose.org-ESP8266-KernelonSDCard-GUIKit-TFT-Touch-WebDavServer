# Kernel Functionality RAM Costs - ESP8266/ESP32 GUIKit

**Platform:** ESP8266 (80KB free internal RAM) / ESP32 (320KB free internal RAM)  
**Measurement:** Bytes (B) and Kilobytes (KB)  
**Scope:** Kernel runtime functionality costs only (excludes bootloader)

---

## 🎯 Kernel Functionality Cost Table

Complete breakdown of RAM consumption for each kernel functionality at runtime.

| **Functionality** | **Category** | **ESP8266** | **ESP32** | **% of ESP8266 Free RAM** | **% of ESP32 Free RAM** | **Notes** |
|------------------|-------------|--------------|-----------|---------------------------|-------------------------|-----------|
| **Core System** | | | | | | |
| Kernel Code | Core | 32,768 B | 32,768 B | ~40.96% | ~10.24% | Main kernel logic |
| System Utilities | Core | 4,096 B | 4,096 B | ~5.12% | ~1.28% | Time, logging, helpers |
| **Total Core System** | | **36,864 B** | **36,864 B** | **~46.08%** | **~11.52%** | |
|
| **GUI & Rendering** | | | | | | |
| GUIKit Core | GUI | 16,384 B | 16,384 B | ~20.48% | ~5.12% | Widget system framework - **PRIORITY: External RAM first** |
| Widget Definitions | GUI | 4,096 B | 4,096 B | ~5.12% | ~1.28% | Type structures, properties |
| Rendering Engine | GUI | 8,192 B | 8,192 B | ~10.24% | ~2.56% | TFT drawing, graphics |
| Touch Handling | GUI | 2,048 B | 2,048 B | ~2.56% | ~0.64% | XPT2046 touch driver |
| **Total GUI & Rendering** | | **30,720 B** | **30,720 B** | **~38.40%** | **~9.60%** | |
|
| **Web Services** | | | | | | |
| WebDAV Server | Web | 8,192 B | 8,192 B | ~10.24% | ~2.56% | Full WebDAV protocol |
| &nbsp;&nbsp;&nbsp;&nbsp;- Connection Management | Web | 2,048 B | 2,048 B | ~2.56% | ~0.64% | Client state tracking |
| &nbsp;&nbsp;&nbsp;&nbsp;- Request Parsing | Web | 2,048 B | 2,048 B | ~2.56% | ~0.64% | HTTP request buffer |
| &nbsp;&nbsp;&nbsp;&nbsp;- File I/O | Web | 2,048 B | 2,048 B | ~2.56% | ~0.64% | SdFat operations |
| &nbsp;&nbsp;&nbsp;&nbsp;- Authentication System | Web | 2,048 B | 2,048 B | ~2.56% | ~0.64% | User management |
| HTTP Server | Web | 4,096 B | 4,096 B | ~5.12% | ~1.28% | Web interface serving |
| mDNS Service | Web | 2,048 B | 2,048 B | ~2.56% | ~0.64% | Bonjour/Zeroconf discovery |
| **Total Web Services** | | **14,336 B** | **14,336 B** | **~17.92%** | **~4.48%** | |
|
| **File System** | | | | | | |
| File Manager | FS | 4,096 B | 4,096 B | ~5.12% | ~1.28% | SD card operations |
| &nbsp;&nbsp;&nbsp;&nbsp;- Path Handling | FS | 1,024 B | 1,024 B | ~1.28% | ~0.32% | String operations |
| &nbsp;&nbsp;&nbsp;&nbsp;- File Caching | FS | 2,048 B | 2,048 B | ~2.56% | ~0.64% | Recent files cache |
| &nbsp;&nbsp;&nbsp;&nbsp;- Quota System | FS | 1,024 B | 1,024 B | ~1.28% | ~0.32% | User storage limits |
| JSON Parser | FS | 2,048 B | 2,048 B | ~2.56% | ~0.64% | Streaming JSON parser |
| GUI Loader | FS | 4,096 B | 4,096 B | ~5.12% | ~1.28% | JSON to widget conversion |
| **Total File System** | | **10,240 B** | **10,240 B** | **~12.80%** | **~3.20%** | |
|
| **Memory Management** | | | | | | |
| Memory Strategy Engine | Memory | 4,096 B | 4,096 B | ~5.12% | ~1.28% | Strategy selection logic |
| &nbsp;&nbsp;&nbsp;&nbsp;- Strategy Configuration | Memory | 1,024 B | 1,024 B | ~1.28% | ~0.32% | Thresholds, settings |
| &nbsp;&nbsp;&nbsp;&nbsp;- Bank Management | Memory | 2,048 B | 2,048 B | ~2.56% | ~0.64% | RAM bank tracking |
| &nbsp;&nbsp;&nbsp;&nbsp;- Loading Logic | Memory | 1,024 B | 1,024 B | ~1.28% | ~0.32% | Strategy execution |
| **Total Memory Management** | | **4,096 B** | **4,096 B** | **~5.12%** | **~1.28%** | |
|
| **Advanced Features** | | | | | | |
| Task Switcher | Advanced | 2,048 B | 2,048 B | ~2.56% | ~0.64% | Freeze/thaw system |
| &nbsp;&nbsp;&nbsp;&nbsp;- Task A State | Advanced | 1,024 B | 1,024 B | ~1.28% | ~0.32% | Saved context |
| &nbsp;&nbsp;&nbsp;&nbsp;- Task B State | Advanced | 1,024 B | 1,024 B | ~1.28% | ~0.32% | Active context |
| Task Progress Tracking | Advanced | 512 B | 512 B | ~0.64% | ~0.16% | Minimal display state |
| PNG Converter | Advanced | 1,024 B | 1,024 B | ~1.28% | ~0.32% | Decode state machine |
| &nbsp;&nbsp;&nbsp;&nbsp;- Decode Configuration | Advanced | 512 B | 512 B | ~0.64% | ~0.16% | Mode settings |
| RAM Freeze System | Advanced | 2,048 B | 2,048 B | ~2.56% | ~0.64% | Save/restore mechanism |
| &nbsp;&nbsp;&nbsp;&nbsp;- Compression Buffer | Advanced | 1,024 B | 1,024 B | ~1.28% | ~0.32% | GZIP state |
| &nbsp;&nbsp;&nbsp;&nbsp;- SD I/O Buffer | Advanced | 1,024 B | 1,024 B | ~1.28% | ~0.32% | File operations |
| **Total Advanced Features** | | **8,192 B** | **8,192 B** | **~10.24%** | **~2.56%** | |
|
| **Network** | | | | | | |
| Network Stack | Network | 8,192 B | 8,192 B | ~10.24% | ~2.56% | WiFi + TCP/IP stack |
| **Total Network** | | **8,192 B** | **8,192 B** | **~10.24%** | **~2.56%** | |
|
| **Runtime Overhead** | | | | | | |
| Stack Usage | Runtime | 8,192 B | 8,192 B | ~10.24% | ~2.56% | Call stack reservation |
| Heap Usage | Runtime | 12,288 B | 12,288 B | ~15.36% | ~3.84% | Dynamic allocations |
| **Total Runtime Overhead** | | **20,480 B** | **20,480 B** | **~25.60%** | **~6.40%** | |
|
| **TOTAL KERNEL (Internal RAM Usage with External RAM)** | | **~20,480 B** | **~20,480 B** | **~25.60%** | **~6.40%** | Actual internal usage |
| **TOTAL KERNEL (Without External RAM)** | | **~110,592 B** | **~110,592 B** | **~138.24%** | **~34.56%** | All data in internal RAM |

---

## 📊 Summary by Category

| **Category** | **ESP8266** | **ESP32** | **% of ESP8266** | **% of ESP32** |
|-------------|--------------|-----------|------------------|----------------|
| Core System | 36,864 B | 36,864 B | ~46.08% | ~11.52% |
| GUI & Rendering | 30,720 B | 30,720 B | ~38.40% | ~9.60% |
| Web Services | 14,336 B | 14,336 B | ~17.92% | ~4.48% |
| File System | 10,240 B | 10,240 B | ~12.80% | ~3.20% |
| Memory Management | 4,096 B | 4,096 B | ~5.12% | ~1.28% |
| Advanced Features | 8,192 B | 8,192 B | ~10.24% | ~2.56% |
| Network | 8,192 B | 8,192 B | ~10.24% | ~2.56% |
| Runtime Overhead | 20,480 B | 20,480 B | ~25.60% | ~6.40% |
| **TOTAL** | **110,592 B** | **110,592 B** | **~138.24%** | **~34.56%** |

---

## 🎯 Functionality Cost Ranking (ESP8266 - Highest to Lowest)

| **Rank** | **Functionality** | **RAM Cost** | **% of Free RAM** | **Mandatory?** |
|----------|------------------|--------------|-------------------|----------------|
| 1 | Kernel Code | 32,768 B | ~40.96% | ✅ Yes |
| 2 | Rendering Engine | 8,192 B | ~10.24% | ✅ Yes (for GUI) |
| 3 | GUIKit Core | 8,192 B | ~10.24% | ✅ Yes |
| 4 | Network Stack | 8,192 B | ~10.24% | ✅ Yes (for networking) |
| 5 | WebDAV Server | 8,192 B | ~10.24% | ❌ Optional |
| 6 | Heap Usage | 8,192 B | ~10.24% | ✅ Yes |
| 7 | Stack Usage | 8,192 B | ~10.24% | ✅ Yes |
| 8 | System Utilities | 4,096 B | ~5.12% | ✅ Yes |
| 9 | Widget Definitions | 4,096 B | ~5.12% | ✅ Yes |
| 10 | File Manager | 4,096 B | ~5.12% | ✅ Yes (for projects) |
| 11 | GUI Loader | 4,096 B | ~5.12% | ✅ Yes |
| 12 | Memory Strategy Engine | 4,096 B | ~5.12% | ✅ Yes |
| 13 | HTTP Server | 4,096 B | ~5.12% | ❌ Optional |
| 14 | RAM Freeze System | 2,048 B | ~2.56% | ❌ Optional |
| 15 | Touch Handling | 2,048 B | ~2.56% | ✅ Yes (for touch) |
| 16 | JSON Parser | 2,048 B | ~2.56% | ✅ Yes |
| 17 | Task Switcher | 2,048 B | ~2.56% | ❌ Optional |
| 18 | mDNS Service | 2,048 B | ~2.56% | ❌ Optional |
| 19 | Task Progress | 512 B | ~0.64% | ❌ Optional |
| 20 | PNG Converter | 1,024 B | ~1.28% | ❌ Optional |

---

## 💡 Key Insights

### ESP8266 Memory Constraints
- **Total kernel functionality costs: ~110.6 KB** (exceeds 80KB free internal RAM)
- **Mandatory features alone: ~93 KB** (still exceeds 80KB)
- **Solution:** External RAM (PSRAM/SRAM) or SD Card Swap **required**
- **With external RAM:** Only ~20.5 KB used in internal RAM (~25.6%)

### ESP32 Memory Headroom
- **Total kernel functionality: ~110.6 KB** (34.56% of 320KB free RAM)
- **Remaining free RAM: ~209.4 KB** for GUI data, buffers, applications
- **Can run entirely in internal RAM** with excellent performance

### Cost Optimization Opportunities
1. **Web Services (22.4 KB):** Can be disabled if not needed (-28% RAM)
2. **Advanced Features (8.2 KB):** Task switcher, RAM freeze can be optional (-10% RAM)
3. **mDNS (2 KB):** Optional discovery service (-2.5% RAM)
4. **PNG Converter (1 KB):** Only needed for image conversion (-1.3% RAM)

### Minimal Kernel Configuration
If all optional features are disabled:
- **Mandatory core: ~93 KB** (ESP8266 still needs external RAM)
- **ESP32: ~93 KB** (29% of free RAM) - fits comfortably

---

## 🔄 Memory Strategy Impact

Based on the memory strategy (STOP-at-first-success):

| **Functionality** | **Can use External RAM?** | **Can use SD Swap?** | **Notes** |
|------------------|---------------------------|---------------------|-----------|
| GUIKit Core | ✅ Yes | ❌ No | Needs fast access |
| Widget Definitions | ✅ Yes | ❌ No | Type data |
| Rendering Engine | ✅ Yes | ❌ No | Frame buffers |
| Touch Handling | ✅ Yes | ❌ No | State data |
| WebDAV Server | ✅ Yes | ✅ Yes | File buffers |
| HTTP Server | ✅ Yes | ✅ Yes | Request buffers |
| File Manager | ✅ Yes | ✅ Yes | File operations |
| JSON Parser | ✅ Yes | ✅ Yes | Streaming possible |
| GUI Loader | ✅ Yes | ✅ Yes | JSON processing |
| Task Switcher | ✅ Yes | ✅ Yes | Freeze to SD |
| RAM Freeze System | ❌ No | ✅ Yes | Designed for SD |
| PNG Converter | ❌ No | ✅ Yes | Decode to file |

---

## 🎯 Component Loading Priority

**GUIKit Core is explicitly prioritized for External RAM first** due to performance requirements.

### Priority Matrix

| **Component** | **First Choice** | **Second Choice** | **Third Choice** | **Rationale** |
|---------------|------------------|-------------------|------------------|---------------|
| GUIKit Core | External RAM | N/A | Internal RAM | High access frequency, needs fast performance |
| Rendering Engine | External RAM | N/A | Internal RAM | Frame buffer access, performance critical |
| Widget Definitions | External RAM | N/A | Internal RAM | Type data, frequently accessed |
| Touch Handling | External RAM | N/A | Internal RAM | State data, low latency needed |
| WebDAV Server | External RAM | SD Swap | Internal RAM | Can tolerate swapping |
| HTTP Server | External RAM | SD Swap | Internal RAM | Can tolerate swapping |
| File Manager | External RAM | SD Swap | Internal RAM | File operations can stream |
| JSON Parser | External RAM | SD Swap | Internal RAM | Streaming possible |
| GUI Loader | External RAM | SD Swap | Internal RAM | JSON processing |
| Memory Strategy | Internal RAM | - | - | Small config, always internal |
| Task Switcher | External RAM | SD Swap | Internal RAM | Freeze state can be large |
| RAM Freeze System | SD Swap | - | - | Designed for SD Card storage |
| PNG Converter | SD Swap | - | - | Decode to file pattern |

### Key Rule
**GUIKit Core (16KB) + Rendering Engine (8KB) + Widget Definitions (4KB) + Touch Handling (2KB) = 30KB mandatory external RAM usage** for optimal performance on both ESP8266 and ESP32.

---

## 📈 Decode Strategy RAM Costs

For image processing (PNG/JPEG/TIFF):

| **Decode Strategy** | **ESP8266 Support** | **ESP32 Support** | **Internal RAM Buffer** | **External RAM Usage** | **SD Card Usage** |
|--------------------|---------------------|-------------------|------------------------|------------------------|------------------|
| DECODE_INTERNAL_TOFILE_NORMAL | ✅ Yes | ✅ Yes | Minimal | None | Full |
| DECODE_INTERNAL_TOFILE_FULL | ✅ Yes | ✅ Yes | Limited (~10KB) | None | Full |
| DECODE_EXTERNAL_TOFILE_NORMAL | ✅ Yes | ✅ Yes | Minimal | Yes | Full |
| DECODE_INTERNAL_FULL | ❌ No | ✅ Yes | Full image | None | None |

---

## 📚 Related Documentation

- [RAM Usage Table](RAM_USAGE_TABLE.md) - Complete bootloader + kernel breakdown
- [Memory Strategy Configuration](memory_strategy_config.md) - Strategy details
- [Bootloader Documentation](../src/boot/README.md) - Boot process
- [HARDWARE.md](HARDWARE.md) - Hardware requirements and RAM chips
