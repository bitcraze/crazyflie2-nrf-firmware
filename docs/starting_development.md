---
title: Development for NRF51
page_id: starting_development 
---

This page aims at documenting how to start developing with Crazyflie 2.X NRF51 chip


### NRF51

### Note on programming cables

To be able to flash code without using the radio and to debug it a  SWD cable is needed.


By default any cable supported by openODC can be used. STLink-V2 is used
by the Bitcraze team. Other cables like JLink can also be used but will
require to manually enter the flashing command line and to modify
Eclipse configuration.

##### J-LINK

The SEGGER J-LINK can be used for debugging and flashing.

-   Download and install a recent version of
    [OpenOCD](http://openocd.org/), you will need at least version 0.9.
-   (Optional) Download and install (or extract) the [J-Link
    Software](https://www.segger.com/jlink-software.html). The following
    was tested with version 5.10u.W


     * If there is an error, you can execute the following once and type connect in the command prompt. This fixes issues in case openOCD left the J-Link in an invalid state.

    ./JLinkExe -device STM32F405RG -if swd -speed 4000

#### Command line

From command line the flash make target flashed the firmware using
programming cable

    make flash
