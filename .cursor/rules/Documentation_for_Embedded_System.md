# Complete Documentation Guide for Embedded Systems Projects

---

## 🚀 BEGINNER'S FAST START (Start Here!)

**New to embedded documentation? Do ONLY these 3 things first. Everything else in this guide can wait.**

### Your First 30 Minutes of Documentation

#### 1. Create README.md (15 minutes)

Save this as `README.md` in your project root:

```markdown
# [Your Project Name]

Brief description: What does this project do? (1-2 sentences)

## Hardware
- **MCU**: STM32F4 / ESP32 / [your chip]
- **Sensors**: SHT31 (I2C: 0x44), MPU6050 (I2C: 0x68)
- **Power**: 3.3V

## Quick Start
```bash
# Build
make

# Flash to device
make flash
```

## Pin Connections
See `docs/pinout.md`

## Status
- ✅ Working: I2C communication with SHT31
- 🚧 In progress: MPU6050 integration
- ⏳ Planned: Data logging
```

**That's it for README. Move on.**

#### 2. Create docs/pinout.md (10 minutes)

Make a `docs` folder and create `docs/pinout.md`:

```markdown
# Pin Connections

| Pin | Connected To | Notes |
|-----|--------------|-------|
| PB6 | I2C SCL | SHT31 + MPU6050 |
| PB7 | I2C SDA | SHT31 + MPU6050 |
| PA9 | UART TX | Debug output |
| PC13 | LED | Status indicator |

## I2C Devices
- SHT31: Address 0x44 (temp/humidity sensor)
- MPU6050: Address 0x68 (IMU)
```

**Done. That's all the hardware docs you need right now.**

#### 3. Add Build Instructions to README (5 minutes)

If your build process is more complex than `make`, add a section:

```markdown
## Build Instructions

### Requirements
- ARM GCC toolchain
- Make

### Build and Flash
```bash
cd firmware
make clean
make
make flash
```

### Verify
Connect serial terminal at 115200 baud:
```bash
screen /dev/ttyUSB0 115200
```
Expected output: "System initialized"
```

---

### ✋ STOP HERE

**You now have:**
- ✅ README with quick start
- ✅ Hardware pinout
- ✅ Build instructions

**This is 80% of the value for 20% of the effort.**

### When to Come Back to This Guide

Use the rest of this document as a **reference manual** when you need:
- Section 2: Schematics and detailed hardware docs → When sharing hardware
- Section 4: Architecture diagrams → When project gets complex (>10 files)
- Section 5: Testing docs → When writing tests
- Section 6: Troubleshooting → When common issues emerge
- Section 7-10: Advanced topics → When relevant

**Don't try to do everything at once.** Document incrementally as your project grows.

---

## Documentation Hierarchy (80/20 Priority)

### Tier 1: Critical (Must Have) - 20% effort, 80% value
1. **README.md** - Project entry point
2. **Hardware Documentation** - Schematics and pinout
3. **Build Instructions** - Getting code running
4. **API Documentation** - Doxygen-generated

### Tier 2: Important (Should Have)
5. **Architecture Overview** - System design
6. **Setup Guide** - Development environment
7. **Testing Documentation** - How to verify functionality

### Tier 3: Nice to Have
8. **Troubleshooting Guide**
9. **Performance Metrics**
10. **Change Log**

---

## 1. README.md (Project Entry Point)

Your README is the **first impression**. Keep it scannable and actionable.

### Essential Sections

```markdown
# Project Name

Brief one-paragraph description of what this does.

## Hardware
- MCU: STM32F4 / ESP32 / etc.
- Sensors: SHT31 (I2C: 0x44), MPU6050 (I2C: 0x68)
- Communication: UART, I2C, SPI
- Power: 3.3V, current draw: ~50mA

## Features
- ✅ I2C communication with SHT31 and MPU6050
- ✅ FreeRTOS task scheduling
- ✅ UART debug output
- 🚧 CI/CD pipeline (in progress)
- ⏳ CAN bus interface (planned)

## Quick Start
```bash
# Clone repository
git clone <repo-url>

# Build firmware
cd firmware
make

# Flash to device
make flash
```

## Project Structure
```
├── src/              # Source files
├── include/          # Header files
├── drivers/          # Hardware drivers
├── test/             # Unit tests
├── docs/             # Documentation
├── hardware/         # Schematics and PCB files
└── tools/            # Build scripts and utilities
```

## Documentation
- [Hardware Setup](docs/hardware-setup.md)
- [Build Guide](docs/build-guide.md)
- [API Reference](docs/api/html/index.html)
- [Architecture](docs/architecture.md)

## License
MIT / Apache 2.0 / Proprietary

## Contact
Your Name - email@example.com
```

---

## 2. Hardware Documentation

### A. Pinout Table (`docs/hardware/pinout.md`)

```markdown
# Hardware Pinout

## MCU Pin Assignments

| Pin | Function | Peripheral | Notes |
|-----|----------|------------|-------|
| PA9 | UART TX | USART1 | Debug output |
| PA10 | UART RX | USART1 | Debug input |
| PB6 | I2C SCL | I2C1 | 400kHz, 4.7kΩ pull-up |
| PB7 | I2C SDA | I2C1 | 400kHz, 4.7kΩ pull-up |
| PC13 | GPIO | LED | Active low |
| PA0 | ADC | ADC1_CH0 | Battery voltage monitor |

## Sensor Connections

### SHT31 Temperature/Humidity Sensor
- **I2C Address**: 0x44
- **VCC**: 3.3V
- **Pull-ups**: 4.7kΩ on SCL/SDA
- **Power consumption**: 1.5µA (sleep), 800µA (measuring)

### MPU6050 IMU
- **I2C Address**: 0x68 (AD0 low) / 0x69 (AD0 high)
- **VCC**: 3.3V
- **INT Pin**: PB0 (optional, for motion detection)
- **Power consumption**: 3.9mA (active), 10µA (sleep)

## Power Supply
- **Input**: 5V via USB or external
- **Regulation**: LDO to 3.3V
- **Current budget**: 
  - MCU: ~30mA
  - SHT31: ~1mA
  - MPU6050: ~4mA
  - **Total**: ~50mA typical
```

### B. Schematic Files

**Location**: `hardware/schematics/`

Include:
- PDF exports (for easy viewing)
- Native CAD files (KiCad, Eagle, Altium)
- Bill of Materials (BOM)
- PCB layout files

```
hardware/
├── schematics/
│   ├── main-board-v1.0.pdf
│   ├── main-board-v1.0.kicad_sch
│   └── BOM.csv
├── pcb/
│   ├── gerbers/
│   └── main-board-v1.0.kicad_pcb
└── 3d-models/
    └── enclosure.step
```

### C. Block Diagram (`docs/hardware/block-diagram.md`)

Use ASCII art or tools like draw.io:

```
┌─────────────┐
│   STM32F4   │
│     MCU     │
└──────┬──────┘
       │
       ├─── I2C1 ───┬─── SHT31 (Temp/Humidity)
       │            │
       │            └─── MPU6050 (IMU)
       │
       ├─── UART1 ──── Debug Console
       │
       ├─── GPIO ────── Status LED
       │
       └─── ADC1 ────── Battery Monitor
```

---

## 3. Build and Setup Documentation

### A. Development Environment Setup (`docs/setup-guide.md`)

```markdown
# Development Environment Setup

## Prerequisites
- **Compiler**: ARM GCC 10.3+ or IAR
- **Build System**: Make / CMake
- **Debugger**: OpenOCD + ST-Link / J-Link
- **IDE**: VS Code (recommended) / Eclipse / STM32CubeIDE

## Installation Steps

### Linux (Ubuntu/Debian)
```bash
# Install ARM toolchain
sudo apt-get install gcc-arm-none-eabi gdb-multiarch

# Install OpenOCD
sudo apt-get install openocd

# Install build tools
sudo apt-get install make cmake
```

### macOS
```bash
brew install armmbed/formulae/arm-none-eabi-gcc
brew install open-ocd
```

### Windows
1. Download [ARM GCC](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)
2. Install [OpenOCD](https://gnutoolchains.com/arm-eabi/openocd/)
3. Add to PATH

## VS Code Setup
Install extensions:
- C/C++ (Microsoft)
- Cortex-Debug
- Makefile Tools

`.vscode/settings.json`:
```json
{
  "cortex-debug.armToolchainPath": "/usr/bin",
  "C_Cpp.default.compilerPath": "/usr/bin/arm-none-eabi-gcc"
}
```
```

### B. Build Instructions (`docs/build-guide.md`)

```markdown
# Build Guide

## Standard Build
```bash
cd firmware
make clean
make -j8
```

**Output**: `build/firmware.elf`, `build/firmware.bin`, `build/firmware.hex`

## Build Configurations

### Debug Build
```bash
make BUILD=debug
```
- Optimization: -O0
- Debug symbols: -g3
- Assertions: Enabled

### Release Build
```bash
make BUILD=release
```
- Optimization: -O2
- Debug symbols: Minimal
- Assertions: Disabled

## Flashing Firmware

### Using ST-Link
```bash
make flash
```

### Using OpenOCD
```bash
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
  -c "program build/firmware.elf verify reset exit"
```

### Using J-Link
```bash
JLinkExe -device STM32F407VG -if SWD -speed 4000 \
  -CommandFile flash.jlink
```

## Verifying Installation
```bash
# Connect serial terminal
screen /dev/ttyUSB0 115200

# Expected output:
# [INFO] System initialized
# [INFO] I2C1 ready
# [INFO] Sensors detected: SHT31, MPU6050
```
```

---

## 4. Architecture Documentation

### System Architecture (`docs/architecture.md`)

```markdown
# System Architecture

## Software Layers

```
┌─────────────────────────────────────┐
│     Application Layer               │
│  (Sensor reading, data processing)  │
└──────────────┬──────────────────────┘
               │
┌──────────────┴──────────────────────┐
│     Middleware Layer                │
│  (FreeRTOS, Communication Protocol) │
└──────────────┬──────────────────────┘
               │
┌──────────────┴──────────────────────┐
│     Hardware Abstraction Layer      │
│  (I2C, UART, GPIO drivers)          │
└──────────────┬──────────────────────┘
               │
┌──────────────┴──────────────────────┐
│     Hardware Layer                  │
│  (STM32 HAL / Register access)      │
└─────────────────────────────────────┘
```

## Task Structure (FreeRTOS)

| Task Name | Priority | Stack Size | Period | Function |
|-----------|----------|------------|--------|----------|
| sensor_task | 3 | 512 bytes | 100ms | Read SHT31/MPU6050 |
| processing_task | 2 | 1024 bytes | 50ms | Process sensor data |
| uart_task | 1 | 256 bytes | Event | Handle UART comms |
| idle_task | 0 | 128 bytes | - | System idle |

## Data Flow

```
SHT31/MPU6050 → I2C Driver → Sensor Task → Data Queue 
                                               ↓
                                        Processing Task
                                               ↓
                                        UART Output
```

## Memory Map

```
Flash (512KB):
├── 0x08000000: Bootloader (16KB)
├── 0x08004000: Application (480KB)
└── 0x0807F000: Configuration (4KB)

RAM (128KB):
├── 0x20000000: .data + .bss (32KB)
├── 0x20008000: Heap (64KB)
└── 0x20018000: Stack (32KB)
```

## Key Design Decisions

### Why FreeRTOS?
- **RAM**: Lightweight (~10KB overhead)
- **Scheduling**: Preemptive priority-based
- **Ecosystem**: Wide hardware support

### Why I2C for Sensors?
- **Wiring**: Only 2 wires (vs 4+ for SPI)
- **Multi-device**: Can daisy-chain sensors
- **Speed**: 400kHz sufficient for 10Hz sampling

### Error Handling Strategy
- **I2C errors**: Retry 3x, then flag sensor offline
- **RTOS errors**: Assert + system reset
- **Watchdog**: 5-second timeout
```

---

## 5. Testing Documentation

### Test Plan (`docs/testing/test-plan.md`)

```markdown
# Test Plan

## Unit Tests

### I2C Driver Tests
- [x] Initialize I2C peripheral
- [x] Write single byte
- [x] Read single byte
- [x] Write multiple bytes
- [x] Handle NACK error
- [x] Handle timeout error

### SHT31 Driver Tests
- [x] Sensor detection
- [x] Temperature reading (±0.5°C accuracy)
- [x] Humidity reading (±2% accuracy)
- [x] CRC validation
- [x] I2C error recovery

### MPU6050 Driver Tests
- [x] Sensor detection
- [x] Accelerometer reading
- [x] Gyroscope reading
- [x] Self-test
- [x] FIFO operation

## Integration Tests

### System Boot Test
```
1. Power on device
2. Verify boot message on UART
3. Check all sensors detected
4. Verify LED blinks
Expected: No errors, <500ms boot time
```

### Sensor Data Flow Test
```
1. Request sensor reading
2. Verify I2C transaction
3. Check data in queue
4. Verify UART output
Expected: Data within specs, <10ms latency
```

## Hardware Tests

### I2C Bus Test
- **Pull-up resistors**: Verify 4.7kΩ ±5%
- **Signal integrity**: Check rise time <1µs
- **Addressing**: Scan for 0x44, 0x68

### Power Consumption Test
- **Active**: ~50mA @ 3.3V
- **Idle**: ~15mA @ 3.3V
- **Sleep**: <1mA @ 3.3V

## Performance Benchmarks

| Metric | Target | Actual |
|--------|--------|--------|
| Boot time | <500ms | 320ms |
| I2C read latency | <5ms | 2.8ms |
| Task switch time | <50µs | 12µs |
| Memory usage | <80KB RAM | 54KB RAM |

## Test Equipment
- Logic analyzer (Saleae)
- Oscilloscope
- Multimeter
- USB-UART adapter
```

---

## 6. Additional Documentation

### A. Troubleshooting Guide (`docs/troubleshooting.md`)

```markdown
# Troubleshooting Guide

## I2C Issues

### Sensor Not Detected
**Symptoms**: Error "Sensor not found at 0x44"

**Check**:
1. Pull-up resistors present (4.7kΩ)?
2. Sensor powered (3.3V)?
3. Correct I2C address?
4. SDA/SCL not swapped?

**Debug**:
```bash
# Run I2C scanner
make flash-debug
# Expected: "Found device at 0x44, 0x68"
```

### I2C Bus Lockup
**Symptoms**: All I2C transactions timeout

**Solution**:
1. Power cycle device
2. Check for short circuits
3. Generate 9 clock pulses to clear bus

## Build Issues

### Linker Error: "Region RAM overflowed"
**Cause**: Stack/heap too large

**Solution**:
```c
// In linker script (STM32F4.ld)
_Min_Heap_Size = 0x8000;  /* Reduce from 64KB */
_Min_Stack_Size = 0x4000; /* Reduce from 32KB */
```

### "arm-none-eabi-gcc: command not found"
**Solution**:
```bash
# Linux
export PATH=$PATH:/usr/bin/arm-none-eabi/bin

# Add to ~/.bashrc for persistence
```
```

### B. Configuration Guide (`docs/configuration.md`)

```markdown
# Configuration Guide

## Compile-Time Configuration

Edit `include/config.h`:

```c
/* I2C Configuration */
#define I2C_CLOCK_SPEED    400000  // 400kHz
#define I2C_TIMEOUT_MS     100

/* Sensor Configuration */
#define SHT31_ADDR         0x44
#define MPU6050_ADDR       0x68
#define SENSOR_READ_RATE   100     // ms

/* FreeRTOS Configuration */
#define configTICK_RATE_HZ          1000
#define configMAX_PRIORITIES        5
#define configMINIMAL_STACK_SIZE    128

/* Debug Configuration */
#define ENABLE_DEBUG_UART   1
#define DEBUG_UART_BAUD     115200
```

## Runtime Configuration

Via UART commands:
```
> set sensor_rate 50    # Set to 50ms
> set debug_level 2     # Verbose logging
> save_config           # Persist to flash
> reboot
```
```

### C. Change Log (`CHANGELOG.md`)

```markdown
# Changelog

## [1.2.0] - 2025-11-18

### Added
- MPU6050 gyroscope support
- FreeRTOS task monitoring
- Watchdog timer

### Changed
- I2C timeout increased to 100ms
- Sensor read rate now configurable

### Fixed
- I2C bus lockup on sensor disconnect
- Memory leak in UART handler

## [1.1.0] - 2025-10-15

### Added
- SHT31 temperature/humidity sensor driver
- UART debug interface

## [1.0.0] - 2025-09-01

### Added
- Initial release
- Basic I2C communication
- GPIO control
```

---

## 7. Documentation Tools & Automation

### Recommended Tools

| Purpose | Tool | Why |
|---------|------|-----|
| Diagrams | draw.io / Excalidraw | Free, version-controllable |
| API docs | Doxygen | Industry standard |
| README | Markdown | Universal, GitHub-friendly |
| Schematics | KiCad / Eagle | Open-source EDA |
| Timing | WaveDrom | Generate timing diagrams |
| State machines | PlantUML | Text-based UML |

### Automating Documentation

#### Auto-generate API docs in CI/CD

```yaml
# .github/workflows/docs.yml
name: Generate Documentation

on: [push]

jobs:
  docs:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      
      - name: Install Doxygen
        run: sudo apt-get install doxygen graphviz
      
      - name: Generate docs
        run: doxygen Doxyfile
      
      - name: Deploy to GitHub Pages
        uses: peaceiris/actions-gh-pages@v3
        with:
          github_token: ${{ secrets.GITHUB_TOKEN }}
          publish_dir: ./docs/html
```

#### Auto-update pinout table from code

```python
# tools/generate_pinout.py
# Parses pin definitions from code and generates markdown table
```

---

## 8. Documentation Checklist

Before releasing or sharing your project:

### Essential
- [ ] README.md with quick start
- [ ] Pinout table or diagram
- [ ] Build instructions
- [ ] Schematic PDF
- [ ] Doxygen comments on public APIs

### Important
- [ ] Architecture overview
- [ ] Development environment setup
- [ ] Test plan/results
- [ ] BOM (Bill of Materials)

### Nice to Have
- [ ] Troubleshooting guide
- [ ] Performance benchmarks
- [ ] Change log
- [ ] Contributing guidelines
- [ ] License file

---

## 9. Best Practices

### Keep Documentation Close to Code
```
src/drivers/sht31/
├── sht31.c
├── sht31.h
└── README.md        # Driver-specific docs
```

### Use Templates
Create templates for common docs:
- `docs/templates/driver-readme-template.md`
- `docs/templates/test-plan-template.md`

### Document as You Go
Don't wait until the end:
- New driver → Add README with usage example
- New hardware → Update pinout table
- API change → Update Doxygen comments

### Make It Searchable
- Use clear section headers
- Add keywords to descriptions
- Create a `docs/README.md` index

### Version Your Documentation
Tag documentation with releases:
```bash
git tag -a v1.0.0 -m "Release 1.0.0 with docs"
```

---

## 10. Example Project Structure

```
embedded-project/
├── README.md                    # Main entry point
├── CHANGELOG.md
├── LICENSE
│
├── src/                         # Source code
├── include/                     # Headers
├── drivers/                     # Hardware drivers
│   ├── sht31/
│   │   ├── sht31.c
│   │   ├── sht31.h
│   │   └── README.md           # Driver docs
│   └── mpu6050/
│
├── test/                        # Unit tests
│   └── README.md
│
├── docs/                        # Documentation hub
│   ├── README.md               # Documentation index
│   ├── setup-guide.md
│   ├── build-guide.md
│   ├── architecture.md
│   ├── api/                    # Doxygen output
│   │   └── html/
│   ├── hardware/
│   │   ├── pinout.md
│   │   └── block-diagram.png
│   ├── testing/
│   │   └── test-plan.md
│   └── troubleshooting.md
│
├── hardware/                    # Hardware design files
│   ├── schematics/
│   │   ├── main-board.pdf
│   │   └── main-board.kicad_sch
│   ├── pcb/
│   └── BOM.csv
│
├── tools/                       # Build scripts, utilities
│   └── flash.sh
│
└── Doxyfile                     # Doxygen config
```

---

## Summary: 80/20 Documentation Strategy

**20% effort that gives 80% value:**

1. **README.md** - 30 minutes
   - What, why, quick start
   
2. **Pinout table** - 20 minutes
   - Pin assignments, I2C addresses
   
3. **Build instructions** - 20 minutes
   - How to compile and flash
   
4. **Schematic PDF** - 10 minutes
   - Export from CAD tool
   
5. **Doxygen comments** - Ongoing
   - Document as you code

**Total time to minimum viable documentation: ~90 minutes**

Everything else can be added incrementally as the project grows!