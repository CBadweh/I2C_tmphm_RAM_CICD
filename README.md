# Bare-Metal I2C Temperature/Humidity Monitor
## Production-Quality Embedded Systems Project

A professional bare-metal embedded systems project demonstrating interrupt-driven I2C communication, state machine design, and comprehensive error handling on the STM32F401RE microcontroller with SHT31-D sensor.

**Portfolio Highlights:**
- 🔧 Custom interrupt-driven I2C driver with 7-state FSM
- 🛡️ Comprehensive error detection and recovery
- 🧪 Professional fault injection testing framework
- 📊 Real-time temperature/humidity monitoring
- 🔄 Non-blocking super-loop architecture
- 📝 Extensive documentation and learning materials

---

## 🎯 Project Overview

This project implements a production-quality temperature and humidity monitoring system using professional embedded systems patterns:

- **Bare-metal development** (no RTOS) with super-loop architecture
- **Interrupt-driven I2C state machine** for non-blocking communication
- **Professional error handling** with guard timers and fault detection
- **Comprehensive testing infrastructure** including fault injection
- **Modular design** with clear separation of concerns
- **CI/CD ready** with automated build and test scripts

### Key Technical Achievements

✅ **Custom I2C Driver**: 7-state interrupt-driven FSM handling master read/write operations  
✅ **Error Recovery**: Robust timeout protection, bus error detection, and automatic recovery  
✅ **Sensor Integration**: SHT31-D with CRC-8 validation and integer-only math  
✅ **Fault Injection**: Professional testing framework for automated error path validation  
✅ **Performance Monitoring**: Diagnostic counters and console commands for debugging  
✅ **Zero-overhead Release Builds**: Debug-only instrumentation, production code stays lean

## Hardware

- **MCU**: STM32F401RE (Nucleo-64 board)
- **Sensor**: SHT31-D temperature & humidity sensor (Adafruit)
- **Communication**: I2C3 (100 kHz), UART2 (115200 baud)
- **Power**: 5V USB via Nucleo board, 3.3V regulated
- **Memory**: 41 KB Flash used, minimal RAM footprint

## Quick Start

```bash
# Navigate to project directory
cd Badweh_Development

# Build and flash to device
cd ci-cd-tools
build.bat
```

The build script automatically:
1. Cleans previous build
2. Compiles the project
3. Flashes the binary to the STM32F401RE via SWD

## Pin Connections

See `docs/pinout.md` for detailed pin connections and I2C device addresses.

## Current Status

- ✅ Working: I2C communication, SHT31 temperature/humidity reading
- ✅ Working: Console commands for testing and debugging
- ✅ Working: Timer-driven background sampling (1Hz)
- ✅ Working: CRC-8 validation of sensor data

## Build Instructions

### Prerequisites

- **Compiler**: ARM GCC (GNU Tools for STM32)
- **Build System**: Make
- **Debugger**: ST-Link (built into Nucleo board)
- **Flash Tool**: STM32_Programmer_CLI (part of STM32CubeIDE)

### Installation (Windows)

The project uses STM32CubeIDE toolchain. If you have STM32CubeIDE installed, the build script will automatically use the bundled toolchain.

**Required Software:**
- STM32CubeIDE (includes ARM GCC and STM32_Programmer_CLI)
- Make (included with STM32CubeIDE)

### Build

The project uses a custom build script located in `Badweh_Development/ci-cd-tools/`:

```bash
cd Badweh_Development/ci-cd-tools
build.bat
```

**What the build script does:**
1. Sets up the ARM GCC toolchain path
2. Cleans previous build artifacts
3. Compiles the project (Debug configuration)
4. Flashes the binary to the board via SWD

**Output files** (in `Badweh_Development/Debug/`):
- `Badweh_Development.elf` - Debug symbols
- `Badweh_Development.bin` - Binary for flashing
- `Badweh_Development.map` - Memory map

### Flash to Device

The `build.bat` script automatically flashes the binary after a successful build using STM32_Programmer_CLI via SWD connection.

**Manual flash** (if needed):
```bash
cd Badweh_Development/Debug
"C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI" --connect port=SWD sn=0670FF3632524B3043205426 --download Badweh_Development.bin 0x08000000 -hardRst
```

### Verify

Connect serial terminal to view console output:

**Settings:**
- **Port**: COM port assigned to Nucleo board (check Device Manager)
- **Baud Rate**: 115200
- **Data Bits**: 8
- **Stop Bits**: 1
- **Parity**: None

**Using PuTTY or similar:**
1. Open serial connection
2. Press reset button on Nucleo board
3. You should see initialization messages and temperature/humidity readings

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

## Fault Injection Testing

This project includes professional-grade fault injection capabilities for automated error path testing. Fault injection allows you to simulate real-world failures (sensor unplugged, wrong I2C address, etc.) without physical hardware changes.

**Key features:**
- ✅ Software-only fault simulation (no hardware changes needed)
- ✅ Toggle-based control for flexible CI/CD automation
- ✅ Zero production overhead (only enabled in Debug builds)
- ✅ Industry-standard testing approach (used in automotive/aerospace/medical)

**Quick usage:**
```
>> i2c test wrong_addr    # Toggle wrong address fault
>> i2c test nack          # Toggle NACK fault
>> i2c test timeout       # Toggle timeout fault
>> i2c test auto          # Run test with fault active
>> i2c test wrong_addr    # Disable fault (toggle again)
```

For complete documentation, see `docs/fault-injection.md`

**Note:** Fault injection is only available in Debug builds. Release builds have zero testing overhead.

## 📁 Project Structure

```
gene_Baremetal_I2CTmphm_RAM_CICD/
├── Badweh_Development/          # Main project (portfolio showcase)
│   ├── app1/                    # Application entry point
│   ├── modules/                 # Modular driver architecture
│   │   ├── i2c/                 # Custom I2C driver
│   │   ├── tmphm/               # Temperature/Humidity module
│   │   ├── console/             # Command-line interface
│   │   ├── cmd/                 # Command infrastructure
│   │   ├── tmr/                 # Timer service
│   │   └── ttys/                # Serial UART driver
│   ├── ci-cd-tools/             # Build automation
│   └── Core/                    # STM32 HAL/LL initialization
├── docs/                        # Technical documentation
│   ├── i2c_module.md            # I2C driver deep dive
│   ├── fault-injection.md       # Testing framework guide
│   ├── build.md                 # Build system details
│   ├── learning/                # Learning materials & lesson plans
│   └── reference/               # Reference documentation
└── reference/                   # Reference implementations from coursework
    ├── tmphm/                   # I2C/TMPHM reference code
    ├── ram-class-nucleo-f401re/ # RAM course reference
    └── ci-cd-class-1/           # CI/CD course reference
```

## 🛠️ Technologies & Tools

**Embedded Systems:**
- ARM Cortex-M4 microcontroller (STM32F401RE)
- Bare-metal C programming (no RTOS)
- STM32 Low-Level (LL) drivers for I2C
- Interrupt-driven architecture
- State machine design patterns

**Communication Protocols:**
- I2C (Inter-Integrated Circuit) master mode
- UART for console interface
- CRC-8 error detection

**Development Tools:**
- STM32CubeIDE (ARM GCC toolchain)
- ST-Link debugger/programmer
- Git version control
- Make build system
- Custom CI/CD scripts

**Testing & Quality:**
- Fault injection framework
- Static code analysis (cppcheck)
- Console-based diagnostics
- Performance monitoring

## 📚 Documentation

**Main Project:**
- [Hardware Pinout](docs/pinout.md) - Pin connections and I2C addresses
- [Fault Injection Guide](docs/fault-injection.md) - Testing framework documentation
- [Build System](docs/build.md) - Detailed build instructions
- [I2C Driver Deep Dive](docs/i2c_module.md) - Complete I2C implementation guide
- [Module Documentation](Badweh_Development/modules/) - Per-module README files

**Learning Materials:**
- [Lesson Plan](docs/learning/i2ctmphm_ram_cicd_lesson_plan.md) - Week-long learning progression
- [Progressive Reconstruction](docs/learning/Progressive_Reconstruction.md) - "Happy path first" approach
- [Day Summaries](docs/learning/) - Daily progress and lessons learned

**Reference Code:**
- [Reference README](reference/README.md) - Coursework reference implementations

## 🎓 Learning Journey

This project was developed following Gene Schrader's professional embedded systems courses, applying industry best practices:

1. **Day 1-2**: I2C driver design and interrupt-driven state machine implementation
2. **Day 3**: TMPHM sensor module with CRC validation and integer math
3. **Day 4**: Fault injection framework for comprehensive error testing
4. **Day 5**: Documentation and code refinement

The `reference/` directory contains complete coursework implementations that served as learning material. The main project (`Badweh_Development/`) demonstrates independent application of these patterns.

## 🚀 Key Features

### I2C Driver (`modules/i2c/`)
- 7-state interrupt-driven FSM (IDLE → START → ADDR → DATA → STOP)
- Non-blocking API design for super-loop integration
- Guard timer protection (100ms timeout)
- Comprehensive error detection (NACK, bus errors, arbitration loss)
- Bus reservation system for shared resource management
- Performance counters and diagnostic commands

### TMPHM Module (`modules/tmphm/`)
- State machine for measurement cycle
- CRC-8 validation of sensor data
- Integer-only temperature/humidity conversion
- Periodic sampling via timer callbacks
- Console commands for testing

### Fault Injection Framework
- Software-only fault simulation (no hardware changes)
- Toggle-based control for automation
- Zero production overhead (Debug build only)
- Industry-standard testing approach

## 🔍 Code Highlights

**Interrupt-Driven State Machine:**
```c
// I2C event interrupt advances state machine
void I2C3_EV_IRQHandler(void) {
    i2c_interrupt(I2C_INSTANCE_3, INTER_TYPE_EVT, 0);
}

// State machine handles one event per interrupt
static void i2c_interrupt(instance_id, inter_type, irq_type) {
    switch (state) {
        case STATE_MSTR_WR_SENDING_ADDR:
            // ADDR flag set → start sending data
            state = STATE_MSTR_WR_SENDING_DATA;
            break;
        // ... handle other states
    }
}
```

**Non-Blocking API Design:**
```c
// Initiate operation (returns immediately)
i2c_write(I2C_INSTANCE_3, 0x44, cmd, 2);

// Poll for completion in super loop
while (i2c_get_op_status(I2C_INSTANCE_3) == MOD_ERR_OP_IN_PROG) {
    // Continue other work
}
```

## 📊 Performance

- **Code size**: ~41 KB Flash (Debug build with instrumentation)
- **RAM usage**: Minimal static allocation, no dynamic memory
- **I2C speed**: 100 kHz (standard mode)
- **Measurement rate**: 1 Hz continuous sampling
- **Timeout protection**: 100ms guard timer per transaction

## License

MIT / Your choice

