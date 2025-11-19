# Badweh_Development - I2C Temperature/Humidity Monitor

Main implementation project demonstrating production-quality bare-metal embedded systems development on STM32F401RE.

## Project Architecture

### System Overview

```
Super Loop Architecture (Bare-Metal, No RTOS)

┌─────────────────────────────────────────────────────────┐
│                     app_main.c                          │
│                  (Application Entry)                     │
│                                                          │
│  INIT Phase:                                            │
│  ├─ Initialize UART (console communication)             │
│  ├─ Initialize I2C3 (sensor bus)                        │
│  ├─ Initialize TMPHM (sensor module)                    │
│  ├─ Initialize Timer (periodic callbacks)               │
│  └─ Initialize Console (command interface)              │
│                                                          │
│  START Phase:                                           │
│  ├─ Enable interrupts                                   │
│  ├─ Start timers                                        │
│  └─ Register console commands                           │
│                                                          │
│  SUPER LOOP:                                            │
│  ├─ console_run()  → Process user commands              │
│  ├─ tmr_run()      → Fire timer callbacks               │
│  ├─ tmphm_run()    → Process sensor state machine       │
│  └─ i2c_run()      → Process I2C state machine          │
└─────────────────────────────────────────────────────────┘
         ↓ calls modules (all non-blocking)
┌──────────────────┬──────────────────┬──────────────────┐
│   I2C Driver     │  TMPHM Module    │  Support Modules │
│  (modules/i2c)   │ (modules/tmphm)  │                  │
├──────────────────┼──────────────────┼──────────────────┤
│ • Interrupt-     │ • Measurement    │ • Console        │
│   driven FSM     │   state machine  │ • Timer service  │
│ • Non-blocking   │ • CRC validation │ • UART driver    │
│   API            │ • Integer math   │ • Command infra  │
│ • Guard timers   │ • Periodic       │ • Logging        │
│ • Error recovery │   sampling       │                  │
└──────────────────┴──────────────────┴──────────────────┘
         ↓ hardware access
┌─────────────────────────────────────────────────────────┐
│              STM32 LL Drivers (Core/)                   │
│  • I2C3 peripheral configuration                        │
│  • UART2 configuration                                  │
│  • Clock setup (84 MHz)                                 │
│  • Interrupt vector table                               │
└─────────────────────────────────────────────────────────┘
```

## Module Documentation

### I2C Driver (`modules/i2c/`)

**Purpose:** Interrupt-driven I2C master driver for sensor communication

**Key Features:**
- 7-state finite state machine (FSM)
- Non-blocking API design
- Guard timer protection (100ms timeout per operation)
- Comprehensive error detection and recovery
- Bus reservation system for shared resource management
- Performance monitoring and diagnostics

**State Machine:**
```
IDLE → WR_GEN_START → WR_SENDING_ADDR → WR_SENDING_DATA → IDLE
IDLE → RD_GEN_START → RD_SENDING_ADDR → RD_READING_DATA → IDLE
```

**API Examples:**
```c
// Reserve bus
i2c_reserve(I2C_INSTANCE_3);

// Initiate write (returns immediately)
i2c_write(I2C_INSTANCE_3, 0x44, cmd_buffer, 2);

// Poll for completion
while (i2c_get_op_status(I2C_INSTANCE_3) == MOD_ERR_OP_IN_PROG) {
    // Continue other work in super loop
}

// Check result
if (i2c_get_error(I2C_INSTANCE_3) == I2C_ERR_NONE) {
    // Success!
}

// Release bus
i2c_release(I2C_INSTANCE_3);
```

**Documentation:**
- [Simplified Explanation](modules/i2c/SIMPLIFIED_EXPLANATION.md) - Overview for learners
- [What Was Removed](modules/i2c/WHAT_WAS_REMOVED.md) - Simplification details

### TMPHM Module (`modules/tmphm/`)

**Purpose:** Temperature/Humidity sensor interface (SHT31-D)

**Key Features:**
- State machine for measurement cycle
- CRC-8 validation of sensor data
- Integer-only arithmetic (no floating point)
- Timer-based periodic sampling
- Console commands for testing

**State Machine:**
```
IDLE → RESERVED → WRITE_CMD → WAIT → READ_DATA → VALIDATE → CONVERT → IDLE
```

**Measurement Cycle:**
1. Reserve I2C bus
2. Send measurement command (0x2C06 - high repeatability)
3. Wait 15ms for sensor to complete
4. Read 6 bytes (temp MSB/LSB/CRC, humidity MSB/LSB/CRC)
5. Validate CRC-8
6. Convert to engineering units (integer math)
7. Release I2C bus

**Documentation:**
- [TMPHM Learning Guide](modules/tmphm/TMPHM_LEARNING_GUIDE.md)

### Console Module (`modules/console/`)

**Purpose:** Command-line interface for testing and debugging

**Features:**
- Command registration and dispatch
- Parameter parsing (hex, decimal, string)
- Help system
- Non-blocking processing

**Available Commands:**
```
i2c reserve       - Reserve I2C bus
i2c release       - Release I2C bus
i2c test write    - Write data to I2C device
i2c test read     - Read data from I2C device
i2c status        - Show I2C error status
i2c pm            - Show performance counters
tmphm test        - Trigger single measurement
tmphm auto        - Toggle automatic sampling
```

### Timer Module (`modules/tmr/`)

**Purpose:** Timer service for periodic callbacks

**Features:**
- Multiple timer instances
- Millisecond resolution
- One-shot and periodic modes
- Callback-based notification

### UART Module (`modules/ttys/`)

**Purpose:** Serial communication for console

**Features:**
- Interrupt-driven RX
- Polling-based TX
- Configurable baud rate (115200 default)
- Buffer management

## Build System

### Quick Build
```bash
cd ci-cd-tools
build.bat
```

The build script:
1. Sets up ARM GCC toolchain paths
2. Cleans previous build artifacts
3. Compiles Debug configuration
4. Flashes binary to STM32F401RE via ST-Link

### Build Outputs
Located in `Debug/`:
- `Badweh_Development.elf` - Executable with debug symbols
- `Badweh_Development.bin` - Flash binary
- `Badweh_Development.map` - Memory map
- `Badweh_Development.list` - Disassembly listing

### Memory Usage
```
Flash:  41.43 KB / 512 KB (8.1%)
RAM:    Minimal static allocation
Stack:  Adequate headroom
```

## Configuration

### Hardware Configuration (`Core/Src/main.c`)
Generated by STM32CubeMX:
- **Clock:** 84 MHz system clock
- **I2C3:** PB4 (SDA), PA8 (SCL), 100 kHz
- **UART2:** PA2 (TX), PA3 (RX), 115200 baud
- **TIM2:** 1 ms tick for timer service

### Module Configuration (`modules/include/config.h`)
```c
#define CONFIG_I2C_HAVE_INSTANCE_3 1
#define CONFIG_I2C_DFLT_TRANS_GUARD_TIME_MS 100
#define CONFIG_TMPHM_HAVE_INSTANCE_1 1
```

## Testing

### Console Testing
1. Connect serial terminal (115200 baud)
2. Reset board
3. Use console commands to test functionality

### Fault Injection Testing
Software-only fault simulation for error path testing:
```
>> i2c test wrong_addr    # Toggle wrong address fault
>> i2c test nack          # Toggle NACK fault
>> i2c test timeout       # Toggle timeout fault
>> i2c test auto          # Run with fault active
```

See [Fault Injection Guide](../docs/fault-injection.md) for details.

### Manual Testing
```
>> i2c reserve 0          # Reserve I2C3
>> i2c test write 0 0x44 0x2C 0x06  # Send measurement command
>> i2c test read 0 0x44 6 # Read result
>> i2c status             # Check for errors
>> i2c release 0          # Release bus
```

## Troubleshooting

### Common Issues

**Build fails:**
- Verify STM32CubeIDE installed
- Check toolchain paths in `build.bat`
- Ensure Make is available

**Flash fails:**
- Check ST-Link connection
- Verify board serial number in flash command
- Try manual reset

**No console output:**
- Verify UART2 connection (PA2/PA3)
- Check baud rate (115200)
- Ensure USB cable supports data (not charge-only)

**I2C errors:**
- Verify sensor connections (PB4=SDA, PA8=SCL)
- Check pull-up resistors (sensor board has built-in)
- Use `i2c status` command to diagnose
- Check sensor I2C address (0x44 or 0x45)

### Debug Commands
```
>> i2c pm              # Performance counters
>> i2c status          # Error status
>> i2c get_op_status 0 # Current operation status
```

## Development Notes

### Design Patterns Used
- **State Machines:** For I2C driver and TMPHM module
- **Non-blocking APIs:** All operations return immediately
- **Resource Reservation:** Honor-based bus sharing
- **Guard Timers:** Timeout protection for all operations
- **Defensive Programming:** Validate inputs, check states
- **Performance Monitoring:** Counters with saturation math

### Coding Standards
- C99 standard
- Consistent naming conventions
- Doxygen-style comments
- Module-based architecture
- Clear separation of concerns

### Future Enhancements
- [ ] Add RAM features (fault handlers, watchdog, logging)
- [ ] Implement CI/CD pipeline
- [ ] Add unit tests
- [ ] Support multiple sensor instances
- [ ] Add data logging to non-volatile storage
- [ ] Implement sensor calibration

## References

- [STM32F401RE Reference Manual](https://www.st.com/resource/en/reference_manual/dm00096844-stm32f401-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) - Chapter 18: I2C
- [SHT31-D Datasheet](https://cdn-shop.adafruit.com/product-files/2857/Sensirion_Humidity_SHT3x_Datasheet_digital-767294.pdf)
- [Project Documentation](../docs/) - Technical guides and learning materials

