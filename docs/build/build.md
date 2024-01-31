---
title: Building and Flashing
page_id: build
---

## Get Bluetooth dependencies

Compiling with bluetooth support requires the nRF51_SDK and S130 packages.

Just after cloning the repository:
``` bash
./tools/fetch_dependencies.sh
```

will download the zips and unpack them.
If you want to download manually from the Nordic semiconductor website, you
will find the details in nrf51_sdk/readme and s130/readme.

## Compiling

To compile arm-none-eabi- tools from https://launchpad.net/gcc-arm-embedded
should be in the path.
On Ubuntu, you can install the tools:

```
sudo apt-get install gcc-arm-none-eabi gdb-arm-none-eabi binutils-arm-none-eabi
```

Sofdivece flashing currently requires `nrfjprog`, can be found on [Nordic's website](https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools/download).

Flash the softdevice:
```bash
make flash_softdevice_jlink
```

Compilation options can be saved in config.mk. Main targets:

```
make                    # Make with BLE support
make BLE=0              # Make without BLE support

make cload              # Flash firmware over radio

make flash              # Flash firmware with jtag
make factory_reset      # Erase device and flash softdevice, bootloaders, and firmware for Crazyflie 2.0
make factory_reset_21   # Erase device and flash softdevice, bootloaders, and firmware for Crazyflie 2.1
                        # The Crazyflie 2.1 needs to be powered by battery only for this to work
```

## Platforms

The NRF firmware can be build for different platforms. The platform is passed in as an argument to make

`make PLATFORM=cf2`

Currently supported platforms are:
* `cf2` (default)
* `tag`
* `bolt`
* `flapper`
