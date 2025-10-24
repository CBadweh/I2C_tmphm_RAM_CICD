# Critical Path Guide - I2C Write Operation

This guide traces **exactly** what happens during a 2-byte I2C write operation.
Follow this to understand how the non-blocking, interrupt-driven state machine works.

---

## Example: Write 2 bytes (0x2C, 0x06) to address 0x44 (SHT31-D sensor)

---

### **STEP 1: User Code (Main Loop) - Time: 0μs**

```c
// User calls from main loop
i2c_reserve(I2C_INSTANCE_3);
rc = i2c_write(I2C_INSTANCE_3, 0x44, buffer, 2);
// rc = 0 (success: operation STARTED, not finished!)
```

**What happens in `i2c_write()`:**
- Calls `start_op(instance_id, 0x44, buffer, 2, STATE_MSTR_WR_GEN_START)`

**What happens in `start_op()`:**
```c
// Line ~200
st->state = STATE_MSTR_WR_GEN_START;    // Set initial state
LL_I2C_Enable(st->i2c_reg_base);        // Turn on I2C peripheral
LL_I2C_GenerateStartCondition(I2C3);    // Tell hardware: send START
ENABLE_ALL_INTERRUPTS(st);              // Enable interrupts
return 0;                                // RETURN IMMEDIATELY!
```

**State after:** `STATE_MSTR_WR_GEN_START`, waiting for START to finish

**Time elapsed:** ~2μs ✅ **Non-blocking!**

---

### **STEP 2: Hardware Sends START - Time: ~10μs**

Hardware sends START condition on the bus:
```
SDA: ‾‾‾‾‾\______
SCL: ‾‾‾‾‾‾‾‾‾‾‾‾
```

When complete, hardware sets **SB flag** in SR1 register and triggers **interrupt**.

---

### **STEP 3: Interrupt #1 - SB Flag - Time: ~11μs**

Hardware calls: `I2C3_EV_IRQHandler()`

**Interrupt handler code:**
```c
void I2C3_EV_IRQHandler(void) {
    i2c_interrupt(I2C_INSTANCE_3, INTER_TYPE_EVT, I2C3_EV_IRQn);
}
```

**Inside `i2c_interrupt()`:**
```c
sr1 = st->i2c_reg_base->SR1;  // Read status: SB flag set

switch (st->state) {
    case STATE_MSTR_WR_GEN_START:
        if (sr1 & LL_I2C_SR1_SB) {
            // START sent! Write address
            st->i2c_reg_base->DR = st->dest_addr << 1;  // 0x44 << 1 = 0x88
            st->state = STATE_MSTR_WR_SENDING_ADDR;
        }
        break;
}
// Return from interrupt
```

**State after:** `STATE_MSTR_WR_SENDING_ADDR`

**Time elapsed:** ~12μs (1μs in interrupt handler)

---

### **STEP 4: Hardware Sends Address - Time: ~12-100μs**

Hardware sends address byte 0x88 (0x44 with W bit = 0):
```
Bits: 0 1 0 0 0 1 0 0 (0x44) + W(0) = 0x88
```

Slave (SHT31-D) recognizes its address and sends **ACK**.

Hardware sets **ADDR flag** and triggers **interrupt**.

---

### **STEP 5: Interrupt #2 - ADDR Flag - Time: ~101μs**

**Inside `i2c_interrupt()`:**
```c
sr1 = st->i2c_reg_base->SR1;  // Read status: ADDR flag set

switch (st->state) {
    case STATE_MSTR_WR_SENDING_ADDR:
        if (sr1 & LL_I2C_SR1_ADDR) {
            // Address ACKed! Clear flag
            (void)st->i2c_reg_base->SR2;  // Reading SR2 clears ADDR
            
            // Send first data byte
            st->state = STATE_MSTR_WR_SENDING_DATA;
            if (sr1 & LL_I2C_SR1_TXE) {
                st->i2c_reg_base->DR = st->msg_bfr[0];  // Send 0x2C
                st->msg_bytes_xferred = 1;
            }
        }
        break;
}
```

**State after:** `STATE_MSTR_WR_SENDING_DATA`

**Data sent:** First byte (0x2C) loaded into hardware

---

### **STEP 6: Hardware Sends First Data Byte - Time: ~101-190μs**

Hardware shifts out byte 0x2C:
```
Bits: 0 0 1 0 1 1 0 0 (0x2C)
```

Slave ACKs. Hardware sets **TXE flag** (Transmit Empty) and triggers **interrupt**.

---

### **STEP 7: Interrupt #3 - TXE Flag - Time: ~191μs**

**Inside `i2c_interrupt()`:**
```c
switch (st->state) {
    case STATE_MSTR_WR_SENDING_DATA:
        if (sr1 & LL_I2C_SR1_TXE) {
            if (st->msg_bytes_xferred < st->msg_len) {
                // Send next byte
                st->i2c_reg_base->DR = st->msg_bfr[1];  // Send 0x06
                st->msg_bytes_xferred = 2;
            }
        }
        break;
}
```

**Data sent:** Second byte (0x06) loaded into hardware

---

### **STEP 8: Hardware Sends Second Data Byte - Time: ~191-280μs**

Hardware shifts out byte 0x06. Slave ACKs.

Both bytes transmitted! Hardware sets **BTF flag** (Byte Transfer Finished).

---

### **STEP 9: Interrupt #4 - BTF Flag - Time: ~281μs**

**Inside `i2c_interrupt()`:**
```c
switch (st->state) {
    case STATE_MSTR_WR_SENDING_DATA:
        if (sr1 & LL_I2C_SR1_BTF) {
            // All bytes sent!
            if (st->msg_bytes_xferred >= st->msg_len) {
                op_stop_success(st, true);
            }
        }
        break;
}
```

**Inside `op_stop_success()`:**
```c
DISABLE_ALL_INTERRUPTS(st);
LL_I2C_GenerateStopCondition(st->i2c_reg_base);  // Send STOP
tmr_inst_start(st->guard_tmr_id, 0);             // Cancel timer
LL_I2C_Disable(st->i2c_reg_base);
st->state = STATE_IDLE;                          // Back to IDLE!
st->last_op_error = I2C_ERR_NONE;
```

**State after:** `STATE_IDLE` ✅ **Operation complete!**

---

### **STEP 10: User Code Polls Status - Time: 282μs+**

```c
// User polling in main loop
rc = i2c_get_op_status(I2C_INSTANCE_3);
// rc = 0 (SUCCESS! Operation complete)

i2c_release(I2C_INSTANCE_3);  // Free the bus
```

---

## 📊 Summary

| Event | Time | State | What Happened |
|-------|------|-------|---------------|
| User calls i2c_write() | 0μs | IDLE → WR_GEN_START | Started, returned |
| Interrupt #1 (SB) | 11μs | → WR_SENDING_ADDR | START sent, address loaded |
| Interrupt #2 (ADDR) | 101μs | → WR_SENDING_DATA | Address ACKed, byte 1 loaded |
| Interrupt #3 (TXE) | 191μs | WR_SENDING_DATA | Byte 1 sent, byte 2 loaded |
| Interrupt #4 (BTF) | 281μs | → IDLE | Byte 2 sent, STOP, done! |
| User polls status | 282μs+ | IDLE | Gets success result |

**Total time:** ~281μs in background, user code blocked ~2μs

**Key insight:** User's `i2c_write()` returned in 2μs, but operation took 281μs to complete in the background via 4 separate interrupts!

---

## 🎯 The Critical Path (Function Call Stack)

```
User Code:
  i2c_write()
    → start_op()
      → LL_I2C_GenerateStartCondition()  // Hardware starts
      → ENABLE_ALL_INTERRUPTS()
      → return 0 ✅ NON-BLOCKING!

[Hardware does work...]

Interrupt #1:
  I2C3_EV_IRQHandler()
    → i2c_interrupt(INTER_TYPE_EVT)
      → Read SR1 (SB flag)
      → switch(STATE_MSTR_WR_GEN_START)
        → Write address to DR
        → state = STATE_MSTR_WR_SENDING_ADDR

Interrupt #2:
  I2C3_EV_IRQHandler()
    → i2c_interrupt(INTER_TYPE_EVT)
      → Read SR1 (ADDR flag)
      → switch(STATE_MSTR_WR_SENDING_ADDR)
        → Read SR2 (clear ADDR)
        → Write byte 1 to DR
        → state = STATE_MSTR_WR_SENDING_DATA

Interrupt #3:
  I2C3_EV_IRQHandler()
    → i2c_interrupt(INTER_TYPE_EVT)
      → Read SR1 (TXE flag)
      → switch(STATE_MSTR_WR_SENDING_DATA)
        → Write byte 2 to DR

Interrupt #4:
  I2C3_EV_IRQHandler()
    → i2c_interrupt(INTER_TYPE_EVT)
      → Read SR1 (BTF flag)
      → switch(STATE_MSTR_WR_SENDING_DATA)
        → op_stop_success()
          → Generate STOP
          → Disable interrupts
          → state = STATE_IDLE ✅ DONE!

User Code:
  i2c_get_op_status()
    → state == IDLE && error == NONE
    → return 0 ✅ SUCCESS!
```

---

## 🔍 Where to Set Breakpoints (Debugging)

1. **`start_op()`** - Line ~200 - See operation start
2. **`i2c_interrupt()`** - Line ~280 - See each interrupt
3. **`STATE_MSTR_WR_GEN_START`** - See first state
4. **`STATE_MSTR_WR_SENDING_ADDR`** - See address handling
5. **`STATE_MSTR_WR_SENDING_DATA`** - See data sending
6. **`op_stop_success()`** - Line ~490 - See completion

---

## 💡 Key Takeaways

1. **User function returns in microseconds** - doesn't wait
2. **Hardware does the work** - generates signals, shifts bits
3. **Interrupts advance the state** - one step per interrupt
4. **State machine is simple** - one switch statement
5. **Each state handles one event** - SB, ADDR, TXE, BTF
6. **Completion is async** - poll `i2c_get_op_status()` to check

This is **event-driven programming** for embedded systems!

