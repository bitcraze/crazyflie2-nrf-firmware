# Crazyflie 2.1 nRF firmware2

Firmware for the nRF51822 contained in the Crazyflie 2.1.

## Compile

Just after clonning the repository:
``` bash
./tools/fetch_dependencies.sh
```

Flashing currently requires `nrfjprog`, can be found on [Nordic's website](https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools/download).

Flash the softdevice:
```bash
make flash_softdevice
```

Then to complile and flash the crazyflie:
``` bash
make
make flash
```
