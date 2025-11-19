# Build Instructions

**Purpose:** Step-by-step guide to build and flash this project

**The 6-month test:** If you come back to this project in 6 months, this document should get you from "blank slate" to "running code" without any confusion.

---

## Quick Start (If You Just Want It to Work)

```bash
# Navigate to build tools
cd Badweh_Development/ci-cd-tools

# Build and flash (one command does everything)
build.bat
```

**That's it.** The script handles everything: clean, compile, flash, reset.

If it worked, skip to [Verify Success](#verify-success).

If it failed, continue reading for detailed instructions.

---

## Prerequisites

### Required Software

**1. STM32CubeIDE** (includes everything you need)
- **What it includes:** ARM GCC compiler, Make, ST-Link tools, Flash programmer
- **Where to get:** https://www.st.com/en/development-tools/stm32cubeide.html
- **Version used:** 1.17.0 (but any recent version works)

**2. ST-Link Drivers** (usually installed with STM32CubeIDE)
- **Why you need it:** Communicates with the Nucleo board
- **Check if installed:** Plug in Nucleo board, should appear in Device Manager as "STMicroelectronics STLink Virtual COM Port"

### Optional Software

**3. Serial Terminal** (to view console output)
- **Options:**
  - PuTTY (Windows)
  - TeraTerm (Windows)
  - screen (Linux/Mac)
  - STM32CubeIDE built-in terminal
- **Settings:** 115200 baud, 8N1 (8 data bits, no parity, 1 stop bit)

---

## Installation Verification

**Check if everything is installed:**

```bash
# Check ARM GCC compiler
arm-none-eabi-gcc --version

# Expected output:
# arm-none-eabi-gcc (GNU Tools for STM32 12.3.rel1) 12.3.1 20231003
```

If you get "command not found", STM32CubeIDE might not be in your PATH. The build script handles this automatically.

---

## Build Configuration

This project has two build configurations:

### Debug Build (Default)

**Features:**
- ✅ Fault injection enabled (`ENABLE_FAULT_INJECTION` defined)
- ✅ Debug symbols included (for GDB debugging)
- ✅ No optimization (`-O0`)
- ✅ All test commands available

**When to use:**
- Development
- Testing
- Debugging
- CI/CD automation

**Compiler flags:**
```
-g3 -DDEBUG -O0 -Wall
```

**Binary size:** ~40KB (includes all test code)

---

### Release Build (Production)

**Features:**
- ✅ Fault injection removed (zero overhead)
- ✅ Optimized for size/speed (`-Os` or `-O2`)
- ✅ No debug symbols
- ✅ Minimal binary size

**When to use:**
- Production deployment
- Final product
- When code size matters

**Compiler flags:**
```
-O2 -DNDEBUG -Wall
```

**Binary size:** ~35KB (test code removed)

**How to build Release:**
1. Open project in STM32CubeIDE
2. Right-click project → "Build Configurations" → "Set Active" → "Release"
3. Build project

**Note:** The `build.bat` script uses Debug configuration by default.

---

## Build Process Explained

### What build.bat Does (Step-by-Step)

```batch
1. Set up environment variables
   - Workspace root path
   - Build configuration (Debug)
   - Project name
   - ST-Link serial number (for your specific board)

2. Add ARM GCC toolchain to PATH
   - STM32CubeIDE bundled compiler
   - Make utility

3. Clean previous build
   cd Badweh_Development/Debug
   make clean

4. Compile project
   make -j4  # Use 4 parallel jobs for faster builds

5. Flash to device via ST-Link
   STM32_Programmer_CLI --connect port=SWD --download binary --reset
```

### Build Output Files

**Location:** `Badweh_Development/Debug/`

| File | Purpose | Size | When You Need It |
|------|---------|------|------------------|
| `Badweh_Development.elf` | Executable with debug symbols | ~689 KB | Debugging with GDB/IDE |
| `Badweh_Development.bin` | Raw binary for flashing | ~34 KB | Manual flash, bootloader |
| `Badweh_Development.map` | Memory layout | Text | Analyzing memory usage |
| `Badweh_Development.list` | Disassembly listing | Text | Low-level debugging |

**Note:** `.elf` is large because it includes debug symbols. `.bin` is what actually goes on the chip.

---

## Step-by-Step Build

### Method 1: Automated Build Script (Recommended)

**Location:** `Badweh_Development/ci-cd-tools/build.bat`

**Usage:**
```bash
cd Badweh_Development/ci-cd-tools
build.bat
```

**What it does:**
1. ✅ Cleans old build
2. ✅ Compiles source files
3. ✅ Links binary
4. ✅ Flashes to device via SWD
5. ✅ Resets device automatically

**Expected output:**
```
Cleaning Debug build...
Building target: Badweh_Development.elf
Finished building target: Badweh_Development.elf
   text	   data	    bss	    dec	    hex	filename
  33752	    428	   5540	  39720	   9b28	Badweh_Development.elf
Flashing Badweh_Development.bin...
ST-LINK SN  : 0670FF3632524B3043205426
Voltage     : 3.24V
Device ID   : 0x433
Device name : STM32F401xD/E
Flash size  : 512 KBytes
Download verified successfully
```

**If successful:** Device resets and starts running new code.

---

### Method 2: Manual Build (If Script Fails)

**Step 1: Clean**
```bash
cd Badweh_Development/Debug
make clean
```

**Step 2: Build**
```bash
make -j4
```

**Step 3: Flash (manual)**
```bash
"C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI" ^
  --connect port=SWD sn=0670FF3632524B3043205426 ^
  --download Badweh_Development.bin 0x08000000 ^
  -hardRst
```

**Note:** Replace the ST-Link serial number with yours (find it using STM32CubeProgrammer).

---

### Method 3: Using STM32CubeIDE (GUI)

**Step 1: Import project**
1. Open STM32CubeIDE
2. File → Import → Existing Projects into Workspace
3. Select `Badweh_Development` folder
4. Click Finish

**Step 2: Build**
1. Right-click project → "Build Project"
2. Or: Ctrl+B

**Step 3: Flash and debug**
1. Click "Run" (green play button)
2. Or: F11 for debug mode

---

## Flash to Device

### Automatic Flash (via build.bat)

The build script automatically flashes after successful compilation.

### Manual Flash Options

**Option 1: STM32_Programmer_CLI (command line)**
```bash
cd Badweh_Development/Debug

"C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI" ^
  --connect port=SWD sn=0670FF3632524B3043205426 ^
  --download Badweh_Development.bin 0x08000000 ^
  -hardRst
```

**Option 2: STM32CubeProgrammer (GUI)**
1. Open STM32CubeProgrammer
2. Connect via SWD (click "Connect" button)
3. Open file: `Badweh_Development/Debug/Badweh_Development.bin`
4. Start address: `0x08000000`
5. Click "Start Programming"

**Option 3: From STM32CubeIDE**
1. Click "Run" button (green play icon)
2. IDE builds and flashes automatically

---

## Verify Success

### Hardware Check

**1. LED indicator:**
- Nucleo board has built-in LED (usually green)
- Should blink or change state (project-specific)

**2. ST-Link connection:**
- ST-Link LED should be solid red/green (connected)
- If blinking red rapidly: connection problem

### Serial Console Check

**Step 1: Find COM port**
- **Windows:** Device Manager → Ports (COM & LPT)
- Look for "STMicroelectronics Virtual COM Port (COMX)"
- Note the COM port number (e.g., COM3, COM7)

**Step 2: Open serial terminal**

**Using PuTTY:**
1. Connection type: Serial
2. Serial line: COM3 (your port number)
3. Speed: 115200
4. Click "Open"

**Using STM32CubeIDE built-in terminal:**
1. Window → Show View → Terminal
2. Click "Open a Terminal" icon
3. Connection type: Serial Terminal
4. Port: COM3
5. Baud rate: 115200

**Step 3: Reset board**
- Press black "RESET" button on Nucleo board
- Should see initialization messages

**Expected output:**
```
========================================
  DAY 1: I2C Error Detection
========================================

[INIT] Initializing modules...
[START] Starting modules...
[READY] Entering super loop...
Use console commands to test I2C error detection...
```

**If you see this:** ✅ Success! Build and flash worked.

**If you see garbage characters:** ❌ Wrong baud rate (check it's 115200)

**If you see nothing:** ❌ Check COM port, check board is powered, press reset

---

## Troubleshooting

### Build Fails: "arm-none-eabi-gcc: command not found"

**Problem:** Compiler not in PATH

**Solution 1:** Run `build.bat` (automatically sets PATH)

**Solution 2:** Add STM32CubeIDE to PATH manually
```bash
set PATH=C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.12.3.rel1.win32_1.1.0.202410251130\tools\bin;%PATH%
```

**Solution 3:** Use STM32CubeIDE GUI instead

---

### Build Fails: "make: *** No rule to make target"

**Problem:** Makefile out of sync with source files

**Solution:**
1. Open project in STM32CubeIDE
2. Right-click project → "Properties"
3. C/C++ Build → Settings
4. Click "Restore Defaults" (regenerates makefile)
5. Rebuild project

---

### Flash Fails: "Error: No ST-Link detected"

**Problem:** ST-Link not connected or drivers not installed

**Solution:**
1. Check USB cable is connected (use USB port closest to Ethernet jack)
2. Try different USB cable (some cables are charge-only)
3. Install ST-Link drivers from STM32CubeProgrammer
4. Check Device Manager for "STMicroelectronics STLink"
5. Try unplugging and replugging board

---

### Flash Fails: "Error: Target voltage too low"

**Problem:** Board not powered properly

**Solution:**
1. Check jumper JP5 is on "U5V" position (USB power)
2. Try different USB port on your computer
3. Check 3.3V LED on Nucleo board is lit

---

### Flash Fails: "Error in final launch sequence"

**Problem:** Board in unknown state or debugger stuck

**Solution:**
1. Unplug Nucleo board
2. Wait 5 seconds
3. Plug back in
4. Press RESET button
5. Try flash again

---

### Code Compiles But Doesn't Run

**Problem:** Code crashes immediately or doesn't start

**Solution 1:** Check for stack overflow
- Look at `.map` file for stack/heap usage
- Default stack: 0x400 (1024 bytes)
- Increase in STM32CubeIDE startup file if needed

**Solution 2:** Verify linker script
- Should be `STM32F401RETX_FLASH.ld`
- Flash start: 0x08000000
- RAM start: 0x20000000

**Solution 3:** Check clock configuration
- System clock should be 84 MHz for STM32F401RE
- Verify in `main.c` → `SystemClock_Config()`

---

### Fault Injection Commands Don't Work

**Problem:** Commands like `i2c test wrong_addr` not found

**Solution:** You're running Release build (fault injection disabled)

**Fix:** Build with Debug configuration
```bash
cd Badweh_Development/ci-cd-tools
build.bat  # Always uses Debug
```

---

### Build is Very Slow

**Problem:** Compilation takes >2 minutes

**Solution 1:** Use parallel compilation
```bash
make -j4  # Use 4 parallel jobs (build.bat already does this)
```

**Solution 2:** Disable cyclomatic complexity analysis
- Edit makefile: Remove `-fcyclomatic-complexity` flag
- Saves ~20% build time

**Solution 3:** Use faster storage
- Move project to SSD instead of HDD
- Significant speedup for large projects

---

## Build System Architecture

### Project Structure

```
Badweh_Development/
├── Core/
│   ├── Src/                    # STM32 HAL startup code
│   │   ├── main.c
│   │   ├── stm32f4xx_it.c      # Interrupt handlers
│   │   └── system_stm32f4xx.c
│   └── Startup/
│       └── startup_stm32f401retx.s  # Assembly startup
│
├── modules/                    # Application modules
│   ├── i2c/                    # I2C driver (fault injection here)
│   ├── tmphm/                  # Temperature/humidity module
│   ├── console/                # Console interface
│   ├── cmd/                    # Command parser
│   ├── log/                    # Logging
│   └── tmr/                    # Timer module
│
├── Drivers/                    # ST HAL/LL drivers
│   ├── STM32F4xx_HAL_Driver/
│   └── CMSIS/
│
├── Debug/                      # Build output (Debug config)
│   ├── makefile                # Auto-generated by CubeIDE
│   ├── sources.mk
│   ├── objects.list
│   └── Badweh_Development.*    # Output files
│
└── ci-cd-tools/
    └── build.bat               # Automated build script
```

### Makefile Hierarchy

**Main makefile:** `Badweh_Development/Debug/makefile`
- Auto-generated by STM32CubeIDE
- Includes:
  - `sources.mk` (list of source files)
  - `modules/*/subdir.mk` (per-module rules)
  - `Core/*/subdir.mk` (STM32 HAL rules)

**Don't edit makefiles directly!** Regenerated by CubeIDE.

### Compiler Flags Explained

**Debug build flags:**
```
-mcpu=cortex-m4           # Target Cortex-M4 processor
-mthumb                   # Use Thumb instruction set (16-bit)
-mfpu=fpv4-sp-d16         # Hardware floating point unit
-mfloat-abi=hard          # Use FPU for floating point
-g3                       # Maximum debug info
-DDEBUG                   # Define DEBUG macro (enables fault injection)
-O0                       # No optimization (easier debugging)
-Wall                     # All warnings
-ffunction-sections       # Each function in own section (for linker GC)
-fdata-sections           # Each data in own section
```

### Memory Layout

**STM32F401RE memory:**
- Flash: 512 KB (0x08000000 - 0x0807FFFF)
- RAM: 96 KB (0x20000000 - 0x20017FFF)

**Current usage (Debug build):**
- Text (code): ~34 KB
- Data (initialized variables): ~0.4 KB
- BSS (uninitialized variables): ~5.5 KB
- Stack: 1 KB (default)
- Heap: Minimal (no dynamic allocation used)

**Total:** ~40 KB (8% of Flash, 6% of RAM)

**Room for growth:** Plenty! You can 10x this project before worrying about space.

---

## Build Performance Tips

### Speed Up Compilation

**1. Use parallel compilation** (already enabled in build.bat)
```bash
make -j4  # Use 4 cores
```

**2. Disable unused analysis**
```bash
# Remove from makefile flags:
-fstack-usage              # Generates .su files
-fcyclomatic-complexity    # Generates .cyclo files
```

**3. Use incremental builds**
```bash
make  # Instead of make clean && make
```
Only recompiles changed files.

### Reduce Binary Size

**1. Use Release build** (`-Os` optimization)
- Saves ~15% code size

**2. Enable link-time optimization**
```bash
-flto  # Link-time optimization
```

**3. Remove unused code**
```bash
-Wl,--gc-sections  # Garbage collect unused sections (already enabled)
```

---

## Advanced Build Topics

### Cross-Platform Considerations

**This project is Windows-specific** (uses `.bat` scripts)

**To port to Linux/Mac:**
1. Convert `build.bat` to `build.sh`
2. Change path separators (`\` → `/`)
3. Update tool paths:
   - ARM GCC: Use apt/brew installed version
   - OpenOCD instead of STM32_Programmer_CLI

**Example Linux build.sh:**
```bash
#!/bin/bash
cd ../Debug
make clean
make -j4
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
  -c "program Badweh_Development.elf verify reset exit"
```

### Custom Build Configurations

**To create new config (e.g., "Test"):**
1. STM32CubeIDE: Right-click project → "Build Configurations" → "Manage"
2. Click "New", base on "Debug"
3. Name: "Test"
4. Customize defines:
   ```
   -DTEST_MODE
   -DENABLE_FAULT_INJECTION
   ```

### Continuous Integration

**For CI/CD pipeline**, the build process is:
```bash
# Jenkins/GitHub Actions workflow
cd Badweh_Development/ci-cd-tools
build.bat  # Builds and flashes
# Python test script connects via UART
python test_i2c_faults.py  # Runs fault injection tests
```

---

## Quick Reference

**Build commands:**
```bash
# Automated build (recommended)
cd Badweh_Development/ci-cd-tools
build.bat

# Manual build
cd Badweh_Development/Debug
make clean
make -j4

# Manual flash
STM32_Programmer_CLI --connect port=SWD --download Badweh_Development.bin 0x08000000 -hardRst
```

**Serial console:**
```
Port: COMX (find in Device Manager)
Baud: 115200
Data: 8 bits
Parity: None
Stop bits: 1
```

**Build outputs:**
```
Badweh_Development/Debug/
├── Badweh_Development.elf   # Debug symbols
├── Badweh_Development.bin   # Flash binary
└── Badweh_Development.map   # Memory layout
```

---

## Related Documentation

- [README.md](../README.md) - Project overview and quick start
- [docs/pinout.md](pinout.md) - Hardware pin connections
- [docs/fault-injection.md](fault-injection.md) - Fault injection testing guide

---

**Documentation maintained:** 2025-11-19
**Build system:** Make + ARM GCC (STM32CubeIDE toolchain)
**Target:** STM32F401RE (Nucleo-F401RE board)
