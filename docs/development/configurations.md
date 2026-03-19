---
title: Configurations
page_id: configurations 
---

Here we present some compile time options that can be enabled/disabled.

## Make: compile time options

These options are passed directly on the `make` command line.

#### PLATFORM
Select which platform to build for. The default is `cf2`. Available platforms
are: `cf2`, `bolt`, `tag`, `flapper`, `cf21bl`.
```
make PLATFORM=bolt
```

#### BLE
The BLE stack can be disabled by building with 
```
make BLE=0
```
This is useful when BLE isn't wanted or when debugging and code stepping as
this is not possible when BLE stack is running. Defaults to `1` (enabled) for
most platforms. Platforms `bolt` and `tag` default to `0`.

#### EXT_ANTENNA
Enable the external antenna output, the u.fl. connector with
```
make EXT_ANTENNA=1
```
This will switch the antenna output from the chip antenna to the u.fl connector.

#### RADIOTEST
Enable radio test mode. When enabled the radio will continuously transmit
instead of operating normally. BLE still functions. Used for RF testing.
```
make RADIOTEST=1
```

#### RECEIVE_RADIOTEST
Enable the receive side of radio testing. When enabled, BLE is automatically
disabled.
```
make RECEIVE_RADIOTEST=1
```

## config.mk: compile time defines

Add these to your `config.mk` file in the root using:
```
CFLAGS += -DNAME_OF_THE_DEFINE
```

For defines that take a value, use:
```
CFLAGS += -DNAME_OF_THE_DEFINE=value
```

### Power management

#### ENABLE_FAST_CHARGE_1A
This will put the charging chip on 1A power budget mode. The charging current will
be 1A - system current, which usually is around 100-200mA. Make sure the battery can
handle this type of charging current. Also the charging chip will get quite hot.

#### PM_WAKEUP_NRF_ON_PGOOD
This will enable wake-up from system off when VUSB is powered. Can be useful when
you want to wakeup a swarm that has the Qi-deck or Crazyflie 2.1 brushless on charging
pads. A side effect is that it can not be switched off if it has VUSB power.

#### DISABLE_CHARGE_TEMP_CONTROL
This will disable the charge temp control. Make sure the battery can handle it.

#### PM_SYSLINK_INCLUDE_TEMP
Include temperature readings in the syslink power management data sent to
the STM32.

### Radio

#### RFX2411N_BYPASS_MODE
Mainly a testing mode but it will put the radio power amplifier in bypass mode,
effectively disabling it.

#### DISABLE_PA
Disable the radio power amplifier entirely. Both the PA enable pin and the
PA TX disable pin are driven low.

#### DEFAULT_RADIO_RATE
Override the default radio data rate. If not defined, defaults to `esbDatarate2M`.
```
CFLAGS += -DDEFAULT_RADIO_RATE=esbDatarate250K
```

#### DEFAULT_RADIO_CHANNEL
Override the default radio channel. If not defined, defaults to `80`.
```
CFLAGS += -DDEFAULT_RADIO_CHANNEL=100
```

#### CONT_WAVE_TEST
Enable continuous wave transmission test mode. When defined, the normal ESB
packet reception loop is bypassed and the radio outputs a continuous carrier
wave for RF certification and testing.

#### RSSI_ACK_PACKET
This will embed RSSI in the ACK packet.

#### RSSI_VBAT_ACK_PACKET
This will embed RSSI and voltage measurement in the ACK packet. Mutually
exclusive with `RSSI_ACK_PACKET`.

### Debug

#### DEBUG_PRINT_ON_SEGGER_RTT
Use SEGGER RTT for debugging. It outputs debug prints to the RTT server.

#### SEMIHOSTING
Enable ARM semihosting support. When defined, `printf` output is routed to
the debugger console. Requires the `semihosting` make target to enable it in
OpenOCD.

#### DEBUG_UART
Reduce the UART baud rate from 1 Mbaud to 460800 baud for debugging
purposes.

#### SYSLINK_CKSUM_MON
Enable syslink checksum error monitoring. When a checksum error is detected
the LED is toggled, providing a visual indicator of communication errors.

#### DEBUG_TIMESLOT
Enable debug output for BLE timeslot scheduling.