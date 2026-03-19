---
title: Home
page_id: home 
---

Source code of the firmware running in the Crazyflie 2.X nRF51822.
This microcontroller has a couple of roles:
 - Power management (ON/OFF logic and battery handling)
 - Radio communication
   - Enhanced Shockburst compatible with Crazyradio (PA)
   - Bluetooth low energy using the Nordic Semiconductor S130 stack
 - One-wire memory access
 - Output a 8mhz high precision clock to STM32 (only on brushless and without BLE).
