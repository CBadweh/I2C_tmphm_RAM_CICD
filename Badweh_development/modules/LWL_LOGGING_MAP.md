# LWL Logging Map - Where Flight Recorder Data Comes From

## **Visual Flow with LWL Points**

### **Complete Measurement Cycle (1 second period):**

```
TIME 0ms: TIMER FIRES
          ↓
     [LWL: TMPHM_TMR_TRIG]  ← Timer callback sets state = RESERVE_I2C
          ↓
          
TIME ~1ms: RESERVE I2C
          ↓
     [LWL: TMPHM_RESERVED]  ← Got I2C bus #3
          ↓
     [LWL: TMPHM_WR_CMD]    ← Starting write of 0x2c 0x06
          ↓
          
TIME ~2ms: I2C INTERRUPT - Write START
          ↓
     [LWL: I2C_WR_START]    ← I2C hardware generated START condition
          ↓
     (I2C sends address 0x44 with W bit = 0)
          ↓
          
TIME ~3ms: I2C INTERRUPT - Address ACK
          ↓
     [LWL: I2C_WR_ADDR_ACK] ← Sensor acknowledged address
          ↓
     (I2C sends data bytes: 0x2c, 0x06)
          ↓
          
TIME ~4ms: I2C INTERRUPT - Write Complete
          ↓
     [LWL: I2C_WR_DONE]     ← Both bytes transmitted
          ↓
          
TIME ~5ms: TMPHM Detects Write Complete
          ↓
     [LWL: TMPHM_WR_OK]     ← TMPHM knows write succeeded
          ↓
     (Wait 15ms for sensor to measure)
          ↓
          
TIME ~20ms: TMPHM Starts Read
          ↓
     [LWL: TMPHM_RD_START]  ← Starting read of 6 bytes
          ↓
          
TIME ~21ms: I2C INTERRUPT - Read START
          ↓
     [LWL: I2C_RD_START]    ← I2C hardware generated START condition
          ↓
     (I2C sends address 0x44 with R bit = 1)
          ↓
          
TIME ~22ms: I2C INTERRUPT - Address ACK
          ↓
     [LWL: I2C_RD_ADDR_ACK] ← Sensor ready to send data
          ↓
     (I2C receives 6 bytes: temp MSB/LSB/CRC, hum MSB/LSB/CRC)
          ↓
          
TIME ~24ms: I2C INTERRUPT - Read Complete
          ↓
     [LWL: I2C_RD_DONE]     ← All 6 bytes received
          ↓
          
TIME ~25ms: TMPHM Processes Data
          ↓
     (Validate CRC)
          ↓
     (Convert to temp/humidity)
          ↓
     [LWL: TMPHM_MEAS]      ← Valid measurement: temp=235, hum=450
          ↓
     (Release I2C bus)
          ↓
     (Return to STATE_IDLE)
          ↓
          
TIME 1000ms: NEXT TIMER FIRES → REPEAT
```

---

## **Code Locations:**

### **I2C Driver (`i2c.c`):**

| Line | LWL Call | Function | Context |
|------|----------|----------|---------|
| 536 | I2C_WR_START | `i2c_interrupt()` | START condition sent for write |
| 546 | I2C_WR_ADDR_ACK | `i2c_interrupt()` | Address ACKed in write |
| 572 | I2C_WR_DONE | `i2c_interrupt()` | All write bytes sent |
| 584 | I2C_RD_START | `i2c_interrupt()` | START condition sent for read |
| 594 | I2C_RD_ADDR_ACK | `i2c_interrupt()` | Address ACKed in read |
| 621 | I2C_RD_DONE | `i2c_interrupt()` | All read bytes received |
| 653 | I2C_ERROR | `i2c_interrupt()` | Any error condition |

### **TMPHM Module (`tmphm.c`):**

| Line | LWL Call | Function | Context |
|------|----------|----------|---------|
| 154 | TMPHM_RESERVED | `tmphm_run()` | I2C bus acquired |
| 165 | TMPHM_WR_CMD | `tmphm_run()` | Write command starting |
| 184 | TMPHM_WR_OK | `tmphm_run()` | Write completed |
| 201 | TMPHM_RD_START | `tmphm_run()` | Read starting after 15ms wait |
| 256 | TMPHM_MEAS | `tmphm_run()` | Valid measurement captured |
| 261 | TMPHM_CRC_ERR | `tmphm_run()` | CRC validation failed |
| 308 | TMPHM_TMR_TRIG | `tmr_callback()` | Timer triggered new cycle |

---

## **LWL Macro Usage Examples:**

### **No Data (0 bytes):**
```c
LWL("TMPHM_TMR_TRIG", 0);
// Logs just the event ID, no additional data
```

### **1 Byte:**
```c
LWL("TMPHM_RESERVED", 1, LWL_1(st.cfg.i2c_instance_id));
//                    │   └─ LWL_1() extracts lowest byte
//                    └─ Total bytes = 1
```

### **2 Bytes:**
```c
LWL("I2C_WR_START", 2, LWL_1(st->dest_addr), LWL_1(st->msg_len));
//                  │   └─ First byte        └─ Second byte
//                  └─ Total bytes = 2
```

### **4 Bytes (2-byte values):**
```c
LWL("TMPHM_MEAS", 4, LWL_2(temp), LWL_2(hum));
//                │   └─ 2 bytes  └─ 2 bytes
//                └─ Total bytes = 4
```

**Macro Breakdown:**
- `LWL_1(x)` = 1 byte (bits 0-7)
- `LWL_2(x)` = 2 bytes (bits 0-15)
- `LWL_3(x)` = 3 bytes (bits 0-23)
- `LWL_4(x)` = 4 bytes (bits 0-31)

---

## **Circular Buffer Behavior:**

```
Buffer: [1008 bytes]
        ↓
   [----write pointer moves--->]
        ↓
   [Entry1][Entry2][Entry3]...[Entry84]
        ↓
   When full, wraps to start (overwrites oldest data)
        ↓
   Always have last ~84 entries (at current logging rate)
```

**At current logging rate:**
- ~12 LWL entries per measurement cycle
- ~7 cycles captured in buffer
- ~7 seconds of activity preserved

---

## **Debugging Scenarios:**

### **Scenario 1: System Crashes**
```
Before crash:
1. TMPHM_TMR_TRIG
2. TMPHM_RESERVED
3. TMPHM_WR_CMD
4. I2C_WR_START
5. [CRASH]

LWL buffer shows: Last event was I2C_WR_START
→ Conclusion: Crash happened during I2C write
→ Check: Interrupt priorities, I2C hardware state
```

### **Scenario 2: No Sensor Readings**
```
LWL dump shows:
1. TMPHM_TMR_TRIG
2. TMPHM_RESERVED
3. TMPHM_WR_CMD
4. I2C_WR_START
5. I2C_ERROR (error=ACK_FAIL)
6. (cycle repeats)

→ Conclusion: Sensor not responding to address
→ Check: Sensor power, I2C pullups, address jumper
```

### **Scenario 3: Timer Overruns**
```
LWL dump shows:
1. TMPHM_TMR_TRIG
2. TMPHM_RESERVED
3. TMPHM_WR_CMD
4. (Timer fires again before cycle complete!)
5. (State machine still in WRITE state)

→ Conclusion: Measurement taking > 1 second
→ Check: I2C guard timer timeout, sensor response time
```

---

## **Memory Map:**

```
RAM (96KB total):
├── Stack: ~4KB (top of RAM)
├── Heap: Minimal (~1KB)
├── BSS (globals): ~7KB
│   ├── LWL buffer: 1008 bytes ← Flight recorder here!
│   ├── Module states: ~2KB
│   └── Other globals: ~4KB
└── Data: ~800 bytes (initialized)

Flash (512KB total):
├── Code: 44.92 KB
│   ├── I2C driver: ~4KB
│   ├── TMPHM module: ~2KB
│   ├── LWL module: ~1.5KB
│   └── Other modules: ~37KB
└── Available: 467 KB (91% free!)
```

---

## **Console Commands Quick Reference:**

```bash
# LWL Commands
> lwl status          # Show recording status and buffer index
> lwl dump            # Hex dump of circular buffer
> lwl enable 0        # Stop recording
> lwl enable 1        # Start recording
> lwl test            # Add test entries

# I2C Commands
> i2c test reserve 0           # Reserve bus
> i2c test write 0 44 2c 06    # Write to sensor
> i2c test read 0 44 6         # Read from sensor
> i2c test status              # Check operation status
> i2c test msg                 # View message buffer
> i2c test release 0           # Release bus

# Button Test
> [Press USER button]          # Runs automated I2C test
```

---

**Your Day 3 is COMPLETE! Time to test on hardware!** 🎯

Connect your serial console and watch the magic happen! ✨

