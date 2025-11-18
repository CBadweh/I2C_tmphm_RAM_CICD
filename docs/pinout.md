# Hardware Pinout

## Pin Connections

| MCU Pin | Function | Connected To | Notes |
|---------|----------|--------------|-------|
| PA8 | I2C3 SCL | SHT31-D sensor | 4.7kΩ pull-up |
| PB4 | I2C3 SDA | SHT31-D sensor | 4.7kΩ pull-up |
| PA2 | USART2 TX | USB-UART adapter | Debug output |
| PA3 | USART2 RX | USB-UART adapter | Debug input |
| PA5 | GPIO OUT | LD2 (Green LED) | Active high |
| PC13 | GPIO IN | B1 (Blue PushButton) | Active low, falling edge interrupt |

## I2C Device Addresses

| Device | Address | Purpose |
|--------|---------|---------|
| SHT31-D | 0x44 | Temperature & humidity sensor |

**Note:** The SHT31-D sensor address is determined by the ADDR pin. On the Adafruit board, the ADDR pin is pulled low, resulting in address 0x44. If ADDR is high, the address would be 0x45.

## Power Supply

- **Input**: 5V USB (via Nucleo board)
- **MCU voltage**: 3.3V (regulated by Nucleo board)
- **Current draw**: ~50mA typical, ~100mA max

## Communication Settings

- **I2C3**: 100kHz standard mode (I2C Fast Mode supported up to 400kHz)
- **UART2**: 115200 baud, 8N1 (8 data bits, no parity, 1 stop bit)

