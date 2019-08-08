---
title: Home
page_id: home 
---

Source code of the firmware running in the Crazyflie 2.0 nRF51822.
This microcontroller have a couple of roles:
 - Power management (ON/OFF logic and battery handling)
 - Radio communication
   - Enhanced Shockburst compatible with Crazyradio (PA)
   - Bluetooth low energy using the Nordic Semiconductor S110 stack
 - One-wire memory access

Compiling with bluetooth support requires the nRF51_SDK and S110 packages.

```
./tools/build/download_deps
```

will download the zips and unpack them.
If you want to download manually from the Nordic semiconductor website, you
will find the details in nrf51_sdk/readme and s110/readme.

License
-------

Most of the code is licensed under LGPL-3.0.

Some files under src/ble/ are modified from Nordic semiconductor examples.

