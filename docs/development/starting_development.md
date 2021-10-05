---
title: Development for NRF51
page_id: starting_development 
---

This page aims at documenting how to start developing with the Crazyflie 2.X NRF51 chip


### nRF51

Clone the crazyflie2-nrf-firmware project.

Then make the firmware:
```
make clean
make
```

To flash, first install the cflib and cfclient, and put the CF2.X in bootloader mode.

To flash using the radio bootloader:

```
make cload
```

To flash using a programming cable:
```
make flash
```


## Debugging

Debugging the nRF51 chip requires a different configuration, but is otherwise identical to [debugging the STM32](https://www.bitcraze.io/documentation/repository/crazyflie-firmware/master/development/openocd_gdb_debugging/). The alternative configuration can be found below.

> **_NOTE:_**
> Make sure your executable (cf2\_nrf.elf) is identical to the one running on your Crazyflie.


### Debugging in VS Code

The Cortex Debug Configuration for nRF51 debugging is replaced with:

    {
        // Use IntelliSense to learn about possible attributes.
        // Hover to view descriptions of existing attributes.
        // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
        "version": "0.2.0",
        "configurations": [
            
                "name": "nRF51 Debug",
                "cwd": "${workspaceRoot}",
                "executable": "./cf2_nrf.elf",
                "request": "launch",
                "type": "cortex-debug",
                "device": "nrf51822",
                "servertype": "openocd",
                "interface": "swd",
                "configFiles": [
                    "interface/stlink-v2.cfg",
                    "target/nrf51.cfg"
                ],
                "runToMain": true,
                "preLaunchCommands": [
                    "set mem inaccessible-by-default off",
                    "enable breakpoint",
                    "monitor reset"
                ]
            }
        ]
    }


### Debugging in Eclipse

Replace cf2.elf with cf2\_rbf.elf
Replace target/stm32f4x.cfg with target/nrf51.cfg

## Note on programming cables

For debugging and flashing without a radio an SWD cable is required. By default any cable supported by OpenODC can be used. The STLink-V2 is used by the Bitcraze team. Other cables like JLink can also be used but will require to manually enter the flashing command line and to modify your (VS Code/Eclipse) debugging configuration accordingly.

### J-LINK

The SEGGER J-LINK can be used for debugging and flashing.

-   Download and install a recent version of
    [OpenOCD](http://openocd.org/), you will need at least version 0.9.
-   (Optional) Download and install (or extract) the [J-Link
    Software](https://www.segger.com/jlink-software.html). The following
    was tested with version 5.10u.W


     * If there is an error, you can execute the following once and type connect in the command prompt. This fixes issues in case openOCD left the J-Link in an invalid state.

    ./JLinkExe -device STM32F405RG -if swd -speed 4000
