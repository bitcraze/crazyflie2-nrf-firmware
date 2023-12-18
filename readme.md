# Crazyflie 2.1 nRF firmware2

Firmware for the nRF51822 contained in the Crazyflie 2.1.

Currently only supports the nRF51DK dev board.

## Compile

Just after clonning the repository:
``` bash
./tools/fetch_dependencies.sh
```

Then to complile and flash the nrf51dk:
``` bash
make
make flash
```