# I2C Temperature/Humidity Monitor

Temperature and humidity monitor using SHT31-D sensor on STM32F401RE.

## Hardware

- **MCU**: STM32F401RE (Nucleo board)
- **Sensor**: SHT31-D (temperature & humidity sensor from Adafruit)
- **Communication**: I2C3, UART2
- **Power**: 5V USB (via Nucleo board), 3.3V regulated, ~50mA typical

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

## Documentation

- Hardware pinout: `docs/pinout.md`
- I2C driver documentation: `Badweh_Development/modules/i2c/`
- Temperature/Humidity module: `Badweh_Development/modules/tmphm/`

## License

MIT / Your choice

