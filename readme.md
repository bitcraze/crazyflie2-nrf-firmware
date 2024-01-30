# Crazyflie 2.X NRF51 firmware [![CI](https://github.com/bitcraze/crazyflie2-nrf-firmware/workflows/CI/badge.svg)](https://github.com/bitcraze/crazyflie2-nrf-firmware/actions?query=workflow%3ACI)

Source code of the firmware running in the Crazyflie 2.X nRF51822.
This microcontroller has a couple of roles:
 - Power management (ON/OFF logic and battery handling)
 - Radio communication
   - Enhanced Shockburst compatible with Crazyradio (PA)
   - Bluetooth low energy using the Nordic Semiconductor S130 stack
 - One-wire memory access

## Building and Flashing

See the [building and flashing instructions](docs/build/build.md) in the github docs folder.

## Official Documentation

Check out the [Bitcraze crazyflie2-nrf-firmware documentation](https://www.bitcraze.io/documentation/repository/crazyflie2-nrf-firmware/master/) on our website.

## Contribute
Go to the [contribute page](https://www.bitcraze.io/contribute/) on our website to learn more.

### Test code for contribution
Run the automated build locally to test your code

	./tools/build/build

## License

Most of the code is licensed under LGPL-3.0.

Some files under src/ble/ are modified from Nordic semiconductor examples.
