# Embedded Systems Documentation - Beginner's Guide

**Goal: Get 80% of documentation value with 20% of the effort**

**Time commitment: 30 minutes for your first project documentation**

---

## Why Document?

3 months from now, you'll ask yourself:
- "Which pin is the I2C clock?"
- "How do I build this again?"
- "What I2C address did I use for the SHT31?"

Documentation answers these questions in 10 seconds instead of 10 minutes of code searching.

---

## The 3 Essential Documents

### 1. README.md (15 minutes)

**Location:** Project root folder

**Purpose:** Anyone (including future you) can understand what this project does and how to use it

**Template:**

```markdown
# [Your Project Name]

One sentence: What does this do?
Example: "Temperature and humidity monitor using SHT31 sensor on STM32F4"

## Hardware
- **MCU**: STM32F407VG / ESP32 / [your chip]
- **Sensors**: SHT31 (temp/humidity), MPU6050 (IMU)
- **Communication**: I2C, UART
- **Power**: 3.3V, ~50mA

## Quick Start
```bash
# Clone and build
git clone [repo-url]
cd firmware
make

# Flash to device
make flash

# View output
screen /dev/ttyUSB0 115200
```

## Pin Connections
See `docs/pinout.md` for detailed connections

## Current Status
- ✅ Working: I2C communication, SHT31 temperature reading
- 🚧 In Progress: MPU6050 integration, FreeRTOS tasks
- ⏳ Planned: Data logging to SD card

## Documentation
- Hardware pinout: `docs/pinout.md`
- Build guide: `docs/build.md` (optional for now)

## License
MIT / Your choice
```

**Tips:**
- Keep hardware list SHORT - just main components
- Status section helps you track progress
- Use emojis (✅ 🚧 ⏳) for quick visual scanning

---

### 2. Pinout Table (10 minutes)

**Location:** `docs/pinout.md`

**Purpose:** Never guess which wire goes where again

**Template:**

```markdown
# Hardware Pinout

## Pin Connections

| MCU Pin | Function | Connected To | Notes |
|---------|----------|--------------|-------|
| PB6 | I2C1 SCL | SHT31, MPU6050 | 4.7kΩ pull-up |
| PB7 | I2C1 SDA | SHT31, MPU6050 | 4.7kΩ pull-up |
| PA9 | UART1 TX | USB-UART adapter | Debug output |
| PA10 | UART1 RX | USB-UART adapter | Debug input |
| PC13 | GPIO OUT | LED | Active low |
| PA0 | ADC1 CH0 | Battery voltage | Voltage divider |

## I2C Device Addresses

| Device | Address | Purpose |
|--------|---------|---------|
| SHT31 | 0x44 | Temperature & humidity sensor |
| MPU6050 | 0x68 | 6-axis IMU (accelerometer + gyroscope) |

## Power Supply
- **Input**: 5V USB or 7-12V external
- **MCU voltage**: 3.3V (regulated)
- **Current draw**: ~50mA typical, ~100mA max
```

**Tips:**
- Include pull-up resistor values (you'll forget them)
- Note I2C addresses (0x44, 0x68, etc.)
- Add "Active low" or "Active high" for GPIO
- If you have a voltage divider, mention it

---

### 3. Build Instructions (5 minutes)

**Location:** Add to README.md OR create `docs/build.md`

**Purpose:** Build the project 6 months from now without frustration

**Template (add to README):**

```markdown
## Build Instructions

### Prerequisites
- **Compiler**: ARM GCC (`arm-none-eabi-gcc`)
- **Debugger**: ST-Link or J-Link
- **Tools**: Make

### Installation (Ubuntu/Debian)
```bash
sudo apt-get install gcc-arm-none-eabi make openocd
```

### Installation (macOS)
```bash
brew install armmbed/formulae/arm-none-eabi-gcc
brew install openocd
```

### Build
```bash
cd firmware
make clean
make
```

Output files in `build/`:
- `firmware.elf` - Debug symbols
- `firmware.bin` - Binary for flashing
- `firmware.hex` - Intel HEX format

### Flash to Device
```bash
make flash
```

Or manually with OpenOCD:
```bash
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
  -c "program build/firmware.bin 0x08000000 verify reset exit"
```

### Verify
Connect serial terminal:
```bash
screen /dev/ttyUSB0 115200
```

Expected output:
```
[INFO] System initialized
[INFO] I2C1 initialized at 400kHz
[INFO] SHT31 detected at 0x44
[INFO] MPU6050 detected at 0x68
```
```

**Tips:**
- Include installation commands for your OS
- Show expected output so you know it worked
- Note baud rate for serial terminal

---

## Done! You're 80% There

With these 3 documents, you have:
- ✅ Project overview (README)
- ✅ Hardware connections (pinout)
- ✅ How to build (instructions)

**This covers 80% of documentation needs for personal/learning projects.**

---

## When You're Ready for More

### Add These ONLY When You Actually Need Them:

**Week 2-4:**
- Add a simple block diagram (if people ask "how does this work?")
- Add troubleshooting section (after you fix the same bug twice)

**Month 2:**
- Add schematic PDF (if sharing hardware with others)
- Add architecture diagram (if project >10 files)

**Month 3+:**
- See `embedded-docs-advanced.md` for comprehensive documentation

---

## Quick Reference: Documentation Priorities

| Document | Time | Priority | When to Create |
|----------|------|----------|----------------|
| README | 15 min | **MUST** | Day 1 |
| Pinout table | 10 min | **MUST** | Before connecting hardware |
| Build instructions | 5 min | **MUST** | Before sharing or archiving |
| Block diagram | 20 min | Should | When project has >3 modules |
| Schematic PDF | 10 min | Should | When sharing hardware |
| Troubleshooting | 15 min | Nice | After fixing common bugs |
| Architecture doc | 30 min | Nice | When project >10 files |
| Test plan | 45 min | Nice | When writing tests |

---

## Real Examples from Your Projects

### Example: SHT31 I2C Driver Project

**README.md:**
```markdown
# SHT31 Temperature & Humidity Sensor Driver

I2C driver for SHT31 sensor on STM32F4

## Hardware
- **MCU**: STM32F407VG
- **Sensor**: SHT31-DIS
- **Interface**: I2C1 at 400kHz

## Quick Start
```bash
make
make flash
screen /dev/ttyUSB0 115200
```

## Status
- ✅ Single-shot measurement
- ✅ CRC validation
- 🚧 Periodic mode
```

**Pinout:**
```markdown
| Pin | Function | Connected To |
|-----|----------|--------------|
| PB6 | I2C1 SCL | SHT31 SCL (pin 4) |
| PB7 | I2C1 SDA | SHT31 SDA (pin 3) |

I2C Address: 0x44
```

**That's it. You're done.**

---

## Common Beginner Mistakes

### ❌ Don't Do This:
- Writing 50-page documentation before writing code
- Documenting every internal function
- Making perfect diagrams with professional tools
- Documenting things that are obvious from code

### ✅ Do This Instead:
- Document ONLY what future-you will forget
- Focus on public APIs and hardware connections
- Use simple markdown tables (no fancy tools needed)
- Document "why" decisions, not "what" the code does

---

## Tools You Need (All Free)

**For documentation:**
- Text editor (VS Code, Vim, whatever you use)
- Markdown (built into GitHub)

**For diagrams (optional):**
- ASCII art (zero learning curve)
- Excalidraw (draw.io) for simple diagrams
- Your phone camera (schematic on paper? Just photograph it)

**That's it.** No expensive tools needed.

---

## FAQ

**Q: Should I document everything?**
A: No. Document what you'll forget. Hardware connections, build steps, I2C addresses, weird bugs you fixed.

**Q: When should I write documentation?**
A: 
- README: Day 1 (15 minutes)
- Pinout: Before wiring hardware (10 minutes)
- Build instructions: Before you commit/share (5 minutes)

**Q: What about Doxygen comments in code?**
A: Start with the beginner Doxygen rule (separate file). Add function comments for public APIs only.

**Q: My project is just for learning. Do I need docs?**
A: YES. Future-you (3 months from now) will thank present-you. Plus, it builds good habits for professional work.

**Q: Should I wait until my project is done?**
A: No. Document as you go:
- New hardware? Update pinout table (2 minutes)
- Changed build process? Update instructions (3 minutes)
- Fixed annoying bug? Add to troubleshooting (5 minutes)

---

## Checklist: Minimum Viable Documentation

Before you consider a project "documented":

- [ ] README.md exists with project description
- [ ] Hardware list in README (MCU, sensors, interfaces)
- [ ] Quick start commands in README
- [ ] Pinout table created with pin numbers and connections
- [ ] I2C/SPI addresses documented
- [ ] Build instructions show how to compile
- [ ] Flash instructions show how to program device
- [ ] Expected output documented (serial console, LED behavior, etc.)

**8 checkboxes. 30 minutes total. That's all you need to start.**

---

## Next Steps

1. **Right now**: Create README.md with hardware list and quick start (15 min)
2. **Before wiring**: Create pinout.md with your connections (10 min)
3. **Before committing code**: Add build instructions (5 min)
4. **Next week**: Add one troubleshooting tip when you fix a bug
5. **Next month**: Review `embedded-docs-advanced.md` for next steps

---

## Remember

> "Perfect documentation tomorrow is worse than good-enough documentation today."

Start simple. Build the habit. Expand as needed.

**You got this! 🚀**