---
title: Crazyflie 2.X BLE Protocol
page_id: ble 
---


The current implementation of Bluetooth Low Energy (BLE) in Crazyflie
2.X is basically a CRTP bridge. It allows to send and receive CRTP
packets to and from the Crazyflie 2.0 main CPU, the STM32F4. This means
that to control the Crazyflie and send/receive information the classical
CRTP ports are used (ie. Commander, Log, Param). BLE services are
implemented in the NRF51, therefore it is possible to implement more
services.

Crazyflie service
-----------------

The crazyflie service has the UUID 00000201-1C7F-4F9E-947B-43B7C00A9A08

### Characteristics

#### CRTP

-   **UUID**: 00000202-1C7F-4F9E-947B-43B7C00A9A08
-   **Length**: 32
-   **Properties**: read, write, notify

This characteristics allows to directly send and received complete CRTP
packets. Though it is limited to 20 bytes only due to BLE stack
implementation limitation.

#### CRTPUP

-   **UUID**: 00000203-1C7F-4F9E-947B-43B7C00A9A08
-   **Length**: 20
-   **Properties**: write, write\_no\_response
Allows to send CRTP packet to Crazyflie. The data format is designed to
deal with the BLE 20Byte packets limitation VS the classical CRTP
32Bytes packets size. The first byte is a control byte and the rest is
raw data. The first byte format is:


|  **Bit** |  **Function**|
 | --------- |--------------|
|  7       |  Start|
|  5-6     |  PID|
|  0-4     |  Length|
|  ---------| --------------|

*PID* (packet identifier) is incremented for every CRTP packet. It permits to make sure no
corrupted packet will be generated in case of packet loss.

When sending a CRTP packet *Start* is 1 and *Length* is the packet
length-1 (ie. no point in sending empty packets). If the packet length
is higher than 19 the characteristic is written a second time with
*start* at 0, *Length* at 0, but *PID* unchanged and the rest of the
data.

    For example to send the CRTP packet: ff 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20

    The CRTPUP characteristics is written two times with the values (assuming PID=0):
    First write:  95 ff 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18
    Second write: 00 19 20

#### CRTPDOWN

-   **UUID**: 00000204-1C7F-4F9E-947B-43B7C00A9A08
-   **Length**: 20
-   **Properties**: read, notify

Allows to receive packets from the copter. The data format is identical
to CRTPUP.
