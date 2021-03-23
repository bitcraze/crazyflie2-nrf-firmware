# Crazyflie 2.0 NRF51 firmware [![CI](https://github.com/bitcraze/crazyflie2-nrf-firmware/workflows/CI/badge.svg)](https://github.com/bitcraze/crazyflie2-nrf-firmware/actions?query=workflow%3ACI)

Source code of the firmware running in the Crazyflie 2.0 nRF51822.
This microcontroller have a couple of roles:
 - Power management (ON/OFF logic and battery handling)
 - Radio communication
   - Enhanced Shockburst compatible with Crazyradio (PA)
   - Bluetooth low energy using the Nordic Semiconductor S110 stack
 - One-wire memory access

## Installation instructions

See the [installation instructions](docs/build/build.md) in the github docs folder.

## Documentation

Check out the [Bitcraze documentation](https://www.bitcraze.io/documentation/repository/crazyflie2-nrf-firmware/master/) on our website.

## License

Most of the code is licensed under LGPL-3.0.

Some files under src/ble/ are modified from Nordic semiconductor examples.