# Reference Code

This directory contains reference implementations from coursework that served as learning material for the main project.

## Contents

### tmphm/
Reference implementation from the **I2C Temperature/Humidity Monitor Course**.
- Complete I2C driver implementation
- TMPHM (Temperature/Humidity) module with SHT31-D sensor
- Console commands and testing infrastructure
- Module framework patterns

**Source:** Gene Schrader's I2C/Temperature/Humidity course

### ram-class-nucleo-f401re/
Reference implementation from the **Reliability, Availability, and Maintainability (RAM) Course**.
- Fault handling and recovery mechanisms
- Lightweight logging (LWL) module
- Watchdog timer implementation
- Assert and audit systems
- Stack overflow protection

**Source:** Gene Schrader's RAM course

### ci-cd-class-1/
Reference implementation from the **CI/CD with Hardware-in-the-Loop Testing Course**.
- Automated build scripts
- Static code analysis integration
- Hardware-in-the-loop (HIL) testing framework
- Jenkins CI/CD pipeline configuration
- Python-based test automation

**Source:** Gene Schrader's Embedded CI/CD course

## Purpose

These reference implementations demonstrate professional embedded systems patterns and practices:
- Interrupt-driven state machines
- Non-blocking APIs for bare-metal super loops
- Error handling and recovery strategies
- Resource management (reservation patterns)
- Performance monitoring and diagnostics
- Testing and continuous integration

The main project (`Badweh_Development/`) was built by learning from and adapting these patterns to create a production-quality I2C temperature/humidity monitoring system.

## Note

These are complete, working projects that can be built independently. They serve as educational references and demonstrate the evolution from learning to implementation in the main project.

