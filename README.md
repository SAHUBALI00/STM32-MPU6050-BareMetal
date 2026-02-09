# STM32-MPU6050-BareMetal
This project is a deep-dive into embedded systems architecture. I developed a fully functional I2C communication driver for the STM32F103 (Cortex-M3) from the ground up, bypassing high-level libraries (HAL/LL). By manipulating memory-mapped registers, I achieved real-time data acquisition from an MPU6050 accelerometer and gyroscope.
Technical Highlights
Register-Level Architecture: Configured the RCC (Reset and Clock Control), GPIO, and I2C peripherals using direct pointer manipulation of hardware addresses.

Protocol Implementation: Manually managed the I2C state machine, including Start/Stop generation, 7-bit addressing, and Multi-byte burst reads.

Mathematical Processing: Converted raw 16-bit signed integers into human-readable G-force and rotation values using the sensor's sensitivity scale factors.

The "Engineering Moment": Debugging a Race Condition
The most significant learning phase of this project involved a "Heisenbug" where the I2C bus would hang unless a slow UART print command was present.

The Problem: The I2C hardware was entering a "Locked" state because the BTF (Byte Transfer Finished) flag wasn't being cleared according to the specific hardware sequence required by the STM32 silicon.

The Solution: I performed a deep-dive into the reference manual to understand the SR1 -> SR2 register reading sequence. By implementing a robust hardware-status polling loop and ensuring the ACK bit was pre-configured before the data phase, I stabilized the driver for high-speed operation without needing "magic" delays.

Key Skills Demonstrated
C Programming: Volatile pointers, bitwise operations, and memory mapping.

Debugging: Protocol analysis and real-time telemetry via UART(written to configure serialPort using UART of STM32, code :serialPort.h).

Hardware Knowledge: Understanding of Open-Drain configurations and internal pull-up logic.
