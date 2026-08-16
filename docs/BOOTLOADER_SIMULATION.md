# Bootloader Screen Output Simulations

This document contains simulated bootloader output for various hardware configurations, demonstrating the RAM length detection feature with TFT display output.

---

## Table of Contents
1. [ESP8266 with 128KB SRAM - Successful Boot](#esp8266-with-128kb-sram---successful-boot)
2. [ESP8266 with WTM Error (64K wired as 128K)](#esp8266-with-wtm-error-64k-wired-as-128k)
3. [ESP32 with 64MB PSRAM - Successful Boot](#esp32-with-64mb-psram---successful-boot)

---

## ESP8266 with 128KB SRAM - Successful Boot

### Boot Sequence Overview
```
Platform: ESP8266 (NodeMCU v3)
RAM: 208KB total (80KB internal + 128KB external SRAM)
External RAM: 23LC1024 at CS 16
SD Card: 32GB FAT32
TFT: ST7789, 320x240 at CS 15
Touch: XPT2046 at CS 4
Kernel: 256KB -> Load Strategy: SD_CARD_SWAP
RAM Test: PASSED (2 passes, size match)
```

### Key Features Demonstrated
- **Bootloader Header**: Shows version, date, and project info
- **MCU Detection**: Identifies ESP8266 platform
- **SMP Detection**: Single-core mode for ESP8266
- **SPI Enumeration**: Detects all 4 devices (SRAM, SD, TFT, Touch)
- **RAM Length Test**: 2-pass verification with progress display on TFT
- **RAM Initialization**: Uses verified size from RAM test
- **Memory Strategy**: Auto-configured with STOP-at-first-success behavior
- **TFT Display**: Shows progress bars during RAM test

### RAM Test TFT Output Sequence
```
+--------------------------------------+
| GUIKit Bootloader                    |
| 2026 Genose.org                      |
|                                      |
| [====      ] 25% RAM Test Pass 1     |
| Testing: 16KB...                     |
+--------------------------------------+

+--------------------------------------+
| GUIKit Bootloader                    |
| 2026 Genose.org                      |
|                                      |
| [========      ] 50% RAM Test Pass 1  |
| Testing: 32KB...                     |
+--------------------------------------+

... (progress continues to 100%) ...

+--------------------------------------+
| GUIKit Bootloader                    |
| 2026 Genose.org                      |
|                                      |
| RAM Test: PASSED                    |
| SRAM: 128KB (23LC1024)              |
+--------------------------------------+
```

### Final Boot Summary on TFT
```
+--------------------------------------+
| ******** GUIKit Bootloader *******   |
| 16/08/2026 - v1.0.0                 |
| 2026 Genose.org                      |
|                                      |
| ===== BOOT SUMMARY =====            |
|                                      |
| Platform: ESP8266                    |
| RAM: 208KB total                    |
|   Internal: 80KB                    |
|   External: 128KB (23LC1024)         |
| SD Card: 32GB FAT32                 |
| TFT: 320x240 (ST7789)               |
| Touch: XPT2046                      |
|                                      |
| Kernel: /kernel.bin                 |
| Size: 256KB                          |
| Strategy: SD_CARD_SWAP              |
|                                      |
| RAM Test: PASSED                   |
| All systems operational             |
|                                      |
| Loading GUI: /gui/chooser.GUIKIT    |
+--------------------------------------+
```

---

## ESP8266 with WTM Error (64K wired as 128K)

### Scenario
User has physically installed a 23LC512 (64KB) chip but the system is configured to expect a 23LC1024 (128KB). The RAM length detection catches this wiring error.

### Boot Sequence with WTM Detection
```
Platform: ESP8266 (NodeMCU v3)
RAM: 144KB total (80KB internal + 64KB external) [ADJUSTED]
External RAM: 64KB at CS 16 (WTM: 23LC512 wired as 23LC1024)
Kernel: 256KB -> Load Strategy: SD_CARD_SWAP (adjusted)
RAM Test: PASSED with WTM WARNING
Action: Continued boot with detected size (stop_on_failure=false)
```

### WTM Detection Process
```
[RAM_TEST] Pass 1/2: Testing with pattern 0xAA...
[RAM_TEST] Testing address range: 0x0000 - 0x0FFFF (64KB)
[RAM_TEST] Progress: 50% | Current: 32768 bytes
[RAM_TEST] ====> WRITE FAILURE at address 0x10000!
[RAM_TEST] ====> Address wraps around - RAM boundary detected
[RAM_TEST] Binary search refining...
[RAM_TEST] Testing lower range: 0x0000 - 0xFFFF (64KB)
[RAM_TEST] ✓ Pattern 0xAA verified across 65536 bytes
[RAM_TEST] Pass 2/2: Testing with pattern 0x55...
[RAM_TEST] ✓ Pattern 0x55 verified across 65536 bytes
[RAM_TEST] 
[RAM_TEST] ✗ DETECTED SIZE: 65536 bytes (64KB)
[RAM_TEST] ✗ EXPECTED SIZE: 131072 bytes (128KB)
[RAM_TEST] 
[RAM_TEST] *** WTM: 64K chip wired as 128K! ***
[RAM_TEST] *** WRONG THING MOUNTED! ***
[RAM_TEST] Actual chip: 23LC512 (64KB)
[RAM_TEST] Expected chip: 23LC1024 (128KB)
[RAM_TEST] stop_on_failure: false -> Continuing with detected size
```

### WTM Warning on TFT
```
+--------------------------------------+
| GUIKit Bootloader                    |
| 2026 Genose.org                      |
|                                      |
| *** WTM ALERT! ***                  |
| 64K chip wired as 128K!             |
| Actual: 23LC512 (64KB)               |
| Expected: 23LC1024 (128KB)           |
| Using: 64KB (detected)               |
+--------------------------------------+
```

### Final Boot Summary with WTM
```
+--------------------------------------+
| ******** GUIKit Bootloader *******   |
| 16/08/2026 - v1.0.0                 |
| 2026 Genose.org                      |
|                                      |
| ===== BOOT SUMMARY =====            |
|                                      |
| Platform: ESP8266                    |
| RAM: 144KB total *                  |
|   Internal: 80KB                    |
|   External: 64KB * (WTM)            |
| SD Card: 32GB FAT32                 |
| TFT: 320x240 (ST7789)               |
| Touch: XPT2046                      |
|                                      |
| Kernel: /kernel.bin                 |
| Size: 256KB                          |
| Strategy: SD_CARD_SWAP              |
|                                      |
| RAM Test: PASSED (WTM WARNING)      |
| ! 64K wired as 128K !                |
| Using detected: 64KB                |
|                                      |
| Loading GUI: /gui/chooser.GUIKIT    |
+--------------------------------------+
```

### Boot Halt Scenario (stop_on_failure=true)

If `stop_on_failure=true` in `/etc/GUIKIT_autostart.ini`:

```
[RAM_TEST] *** WTM: 64K chip wired as 128K! ***
[RAM_TEST] Actual: 23LC512 (64KB), Expected: 23LC1024 (128KB)
[RAM_TEST] stop_on_failure: true -> HALTING BOOT

[BOOT] ============================================================
[BOOT] !!! BOOT HALTED - RAM WTM ERROR !!!
[BOOT] ============================================================
```

TFT Display:
```
+--------------------------------------+
| ******** GUIKit Bootloader *******   |
| 16/08/2026 - v1.0.0                 |
| 2026 Genose.org                      |
|                                      |
| !!! BOOT ERROR !!!                   |
|                                      |
| WTM: Wrong Thing Mounted            |
| 64K chip wired as 128K              |
| Actual: 23LC512                     |
| Expected: 23LC1024                  |
|                                      |
| stop_on_failure: true               |
| Boot halted.                        |
|                                      |
| CHECK YOUR HARDWARE WIREING!        |
+--------------------------------------+
```

---

## ESP32 with 64MB PSRAM - Successful Boot

### Boot Sequence Overview
```
Platform: ESP32 (Dual-Core)
RAM: 64.32MB total (320KB internal + 64MB external PSRAM)
External RAM: 64MB PSRAM (ISSI IS66WVS5128ALL) at CS 255
SD Card: 64GB FAT32
TFT: ST7789, 320x240 at CS 5
Touch: XPT2046 at CS 21
SMP: Enabled (2 cores)
Kernel: 2MB -> Load Strategy: EXTERNAL_RAM
RAM Test: PASSED (2 passes, binary search for large RAM)
```

### Key Differences for ESP32
- **Dual-Core Detection**: SMP mode enabled
- **Quad SPI**: Faster 40MHz speed with quad mode
- **PSRAM Detection**: Identifies ISSI IS66WVS5128ALL (64MB)
- **Binary Search Optimization**: For large RAM, uses efficient binary search
- **Higher Thresholds**: External RAM min size = 16KB, SD swap min = 32KB

### RAM Test TFT Output (Large RAM)
```
+--------------------------------------+
| GUIKit Bootloader                    |
| 2026 Genose.org                      |
|                                      |
| [==        ] 12% RAM Test Pass 1    |
| Testing: 8MB...                      |
+--------------------------------------+

+--------------------------------------+
| GUIKit Bootloader                    |
| 2026 Genose.org                      |
|                                      |
| [======    ] 37% RAM Test Pass 1    |
| Testing: 24MB...                     |
+--------------------------------------+

+--------------------------------------+
| GUIKit Bootloader                    |
| 2026 Genose.org                      |
|                                      |
| [========= ] 62% RAM Test Pass 1    |
| Testing: 40MB...                     |
+--------------------------------------+

+--------------------------------------+
| GUIKit Bootloader                    |
| 2026 Genose.org                      |
|                                      |
| [==============  ] 87% RAM Test Pass1|
| Testing: 56MB...                     |
+--------------------------------------+

+--------------------------------------+
| GUIKit Bootloader                    |
| 2026 Genose.org                      |
|                                      |
| [================] 100% RAM Test Pass1|
| Pattern 0xAA: OK (64MB)               |
+--------------------------------------+
```

### Final Boot Summary on TFT (ESP32)
```
+--------------------------------------+
| ******** GUIKit Bootloader *******   |
| 16/08/2026 - v1.0.0                 |
| 2026 Genose.org                      |
|                                      |
| ===== BOOT SUMMARY =====            |
|                                      |
| Platform: ESP32 (Dual-Core)          |
| RAM: 64.32MB total                   |
|   Internal: 320KB                   |
|   External: 64MB PSRAM              |
| SD Card: 64GB FAT32                |
| TFT: 320x240 (ST7789)               |
| Touch: Yes (XPT2046)                |
| SMP: Enabled (2 cores)               |
|                                      |
| Kernel: /kernel.bin                 |
| Size: 2MB                            |
| Strategy: EXTERNAL_RAM              |
|                                      |
| RAM Test: PASSED                    |
| PSRAM: 64MB verified                 |
|                                      |
| Loading GUI: /gui/chooser.GUIKIT    |
+--------------------------------------+
```

### Post-Boot (ESP32 Specific)
```
[BOOT] Next: Loading kernel into external PSRAM...
[BOOT] Starting GUIKit system with SMP support...
[BOOT] Core 1 starting...
[BOOT] Both cores operational
```

---

## Configuration Reference

The RAM length detection is configured via `/etc/GUIKIT_autostart.ini`:

```ini
[ram_test]
; Enable RAM length detection test at boot
enabled = true

; Number of test passes (1 or 2)
; 1 = Single pattern test (faster)
; 2 = Double pattern test (more reliable, detects wiring errors)
test_passes = 2

; Timeout in milliseconds
timeout_ms = 5000

; Show progress on TFT during test
show_progress = true

; Stop boot if RAM test fails (mismatched size)
; If false, continue boot with detected size and show warning
stop_on_failure = false

; Expected RAM sizes for each bank (0 = auto-detect)
; If detected size doesn't match expected, a WTM warning is shown
bank_0 = 0
bank_1 = 0
```

---

## WTM (Wrong Thing Mounted) Error Codes

| WTM Message | Meaning | Solution |
|------------|---------|----------|
| `64K chip wired as 128K` | 23LC512 installed but 23LC1024 expected | Replace chip or update config |
| `128K chip wired as 512K` | 23LC1024 installed but LY68L6400 expected | Replace chip or update config |
| `Write failure at 0xXXXX` | RAM boundary detected during test | Check wiring/address lines |

---

## Boot Steps Summary

| Step | Description | Output |
|------|-------------|--------|
| 1 | MCU Platform Detection | `[CPU] ESP8266/ESP32 detected` |
| 2 | SMP Detection | `[SMP] Single-core/Dual-core` |
| 3 | SPI Bus Initialization | `[SPI] SCK, MOSI, MISO configured` |
| 4 | SPI Device Enumeration | `[SPI] Device list with types` |
| 5 | **RAM Length Detection** | `[RAM_TEST] Progress, patterns, results` |
| 6 | RAM Initialization | `[RAM] Internal + External with verified sizes` |
| 7 | SD Card Initialization | `[SD] Mount status` |
| 8 | Autostart Config Loading | `[CONFIG] INI file parsed` |
| 9 | TFT Initialization | `[TFT] Display ready` |
| 10 | Kernel Check | `[KERNEL] Size, fit check, strategy` |
| 11 | Memory Strategy Config | `[MEM_STRATEGY] Thresholds, flags` |
| 12 | Memory Strategy Test | `[MEM_TEST] GUI sizes vs strategies` |
| 13 | Display Results | `[TFT] Final summary screen` |

---

*See also: [src/boot/README.md](../src/boot/README.md) for complete bootloader documentation*
