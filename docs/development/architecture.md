---
title: Architecture
page_id: architecture 
---

When running without Softdevice (S110=0) the firmware is loaded at the
beginning of the flash and is running alone in the CPU.

When running with Softdevice (S110=1) independent of if BLE is activated
or not, the flash is filled as follow:
```
+--------------+ 256k
|     MBS      |    Write protected
+--------------+ 252k
|  Bootloader  |
+--------------+ 232k
|              |
|              |
|              |
|              |
|              |
|  Firmware    |
+--------------+ 88K
|              |
|              |
|              |
|              |
|              |
|              |
|  Softdevice  |
+--------------+ 4K
|     MBR      |    Write protected
+--------------+ 0
```

 - **MBR** Softdevice Master Boot Record.
 - **Softdevice** S110 Bluetooth stack
 - **Firmware** This firmware
 - **Bootloader** Bluetooth/Shockburst bootloader
 - **MBS** Master Boot Switch

Boot sequence:
```
 MBR ----> MBS ----> Bootloader ----> Firmware
```

The MBR is part of the Softdevice. It boots the CPU and jumps to MBS.
The MBR contains methods to start the Softdevice and can flash Softdevice
and bootloader.

The MBS handles the ON/OFF button and communicates the duration of the press to
the bootloader so that the bootloader knows what to boot. The reason for the
MBS is to allow updating the bootloader over the air while still having a
write-protected piece of software that can start the STM32 in USB DFU mode
for recovery (the STM32 has access to the NRF51 SWD programming port). The boot
switch is as follow:

| Press time      | Blue LED state | Program booted                               |
| --------------- | -------------- | -------------------------------------------- |
| Short           | Still          | Firmware                                     |
| Long (>3s)      | Slow blink     | Bootloader                                   |
| Very long (>5s) | Fast blink     | Stays in MBS and power STM32 in USB DFU mode |

The bootloader, if selected, starts the STM32 in bootloader mode and initializes
both BLE and Shockburst (ESB) radio. It can flash everything but MBR and MBS.
It also acts as a bridge to the STM32 bootloader.

If not selected, the bootloader jumps to the firmware.
