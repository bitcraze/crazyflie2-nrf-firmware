---
title: Syslink protocol
page_id: syslink
---


Syslink is the protocol used between the STM32 and NRF51 in Crazyflie
2.X. It handles low level communication on the serial port.

Physical format
---------------

In Crazyflie 2.X syslink is transmitted on serial port at 1MBaud. It is
a packet-based protocol.

      +-----------+------+-----+=============+-----+-----+
      |   START   | TYPE | LEN | DATA        |   CKSUM   |
      +-----------+------+-----+=============+-----+-----+

-   **START** is 2 bytes constant, 0xBC 0xCF.
-   **TYPE** defines the type of packet
-   **LENGTH** and type are uint8\_t, length defines the data length.
-   **CKSUM** is 2 bytes Fletcher 8 bit checksum. See
    [rfc1146](https://tools.ietf.org/html/rfc1146). calculated with
    TYPE, LEN and DATA.

**Note**: Unless otherwise specified, all numbers are encoded in
low-endian form.

Packet types
------------

Packet types are defined in the syslink.h file (in the
[stm32](https://github.com/bitcraze/crazyflie-firmware/blob/crazyflie2/hal/interface/syslink.h)
and
[nrf51](https://github.com/bitcraze/crazyflie2-nrf-firmware/blob/master/interface/syslink.h)
firmware).

Packets are organized in groups to ease routing in firmwares:

|  Group  | Name                             | Description|
|  -------| ---------------------------------| ---------------------------------------------------|
|  0x00   | [RADIO](#radio-packets)          | Radio related packets. For data and configuration|
|  0x10   | [PM](#power-management-packets)  | Power management|
|  0x20   | [OW](#one-wire-packets)          | One wire memory access|
|  0x30   | [SYS](#system-packets)           | System information|
|  0xF0   | [DEBUG](#debug-packets)          | Debug and diagnostics|

A packet type has its group in the high nibble and the type in the low
nibble. In the rest of the page packet type are written with group.

Radio packets
-------------

### SYSLINK\_RADIO\_RAW

-   **Type**: 0x00
-   **Data format**: Raw radio packet as sent in the air

This packet carries the raw radio packet. The NRF51 acts as a radio
bridge. Because the NRF51 does not have much memory and the STM32 is
capable of bursting a lot of data a flow control rules has been made:
The STM32 is allowed to send a SYSLINK\_RADIO\_RAW packet only when one
SYSLINK\_RADIO\_RAW packet has been received.

The NRF51 is regularly sending CRTP NULL packet or empty packets to the
STM32 to get the communication working both ways.

**Note** So far SYSLINK\_RADIO\_RAW is the only syslink
packet that has flow control constrain, all other packets can be sent
full duplex at any moment.

### SYSLINK\_RADIO\_CHANNEL

-   **Type**: 0x01
-   **Data format**: One uint8\_t indicating the radio channel

This packet is used only in ESB mode.

Packet sent to the NRF51 to set the radio channel to use. The NRF51 then
send back the same packet to confirm that the setting has been done.

NRF51 radio channel are spaced by 1MHz from 2400MHz to 2525MHz.

### SYSLINK\_RADIO\_DATARATE

-   **Type**: 0x02
-   **Data format**: One uint8\_t indicating the radio datarate.

This packet is used only in ESB mode.

Packet sent to the NRF51 to set the radio datarate to use. The NRF51
then send back the same packet to confirm that the setting has been
done.

Possible datarate:

 | Value  | Datarate|
 | -------| ----------|
 | 0      | 250Kbps|
 | 1      | 1Mbps|
 | 2      | 2Mbps|

### SYSLINK\_RADIO\_CONTWAVE

-   **Type**: 0x03
-   **Data format**: One uint8\_t at 0 for disable, \>0 for enable.

Allows to put the nRF51 in continuous wave mode. If enabled the nRF51
will disable Bluetooth advertising and set its radio to emit a
continuous sinus wave at the currently set channel frequency.

**Warning** _Continuous wave is a test
mode used, among other thing, during manufacturing test. It will affect
other wireless communication like Wifi and should be used with care in a
test environment_

### SYSLINK\_RADIO\_RSSI

-   **Type**: 0x04
-   **Data format**: One uint8\_t indicating the last received packet
    RSSI

This packet is meaningful only in ESB mode.

Packet sent 100 times per second to the STM32. Contains the power value
of the latest received packet. The value is the same as reported by the
NRF51: between 40 and 100 which means measurement of -40dBm to -100dBm.

### SYSLINK\_RADIO\_ADDRESS

-   **Type**: 0x05
-   **Data format**: 5 bytes representing the radio address (little-endian)

This packet is used only in ESB mode.

Packet sent to the NRF51 to set the 5-byte radio address to use. The
NRF51 then sends back the same packet to confirm that the setting has
been applied.

### SYSLINK\_RADIO\_RAW\_BROADCAST

-   **Type**: 0x06
-   **Data format**: Raw radio packet received as a broadcast

Similar to SYSLINK\_RADIO\_RAW but indicates the packet was received on the
multicast address (local address 1) rather than the unicast address.
Sent from NRF51 to STM32.

### SYSLINK\_RADIO\_POWER

-   **Type**: 0x07
-   **Data format**: One int8\_t indicating the radio TX power in dBm

Packet sent to the NRF51 to set the radio transmit power. The NRF51
then sends back the same packet to confirm that the setting has been
applied.

### SYSLINK\_RADIO\_P2P

-   **Type**: 0x08
-   **Data format**:

        +------+------+==========+
        | PORT | RSSI | DATA     |
        +------+------+==========+

-   **PORT**: uint8\_t, the P2P port number (low nibble)
-   **RSSI**: uint8\_t, RSSI of the received P2P packet
-   **DATA**: Payload from the P2P packet

Sent from NRF51 to STM32 when a unicast peer-to-peer packet is received.

### SYSLINK\_RADIO\_P2P\_ACK

-   **Type**: 0x09

Reserved for P2P acknowledgments.

### SYSLINK\_RADIO\_P2P\_BROADCAST

-   **Type**: 0x0A
-   **Data format**:

    When sent **STM32 → NRF51** (to transmit):

        +------+==========+
        | PORT | DATA     |
        +------+==========+

    -   **PORT**: uint8\_t, the P2P port number
    -   **DATA**: Payload to send

    When sent **NRF51 → STM32** (on reception):

        +------+------+==========+
        | PORT | RSSI | DATA     |
        +------+------+==========+

    Same format as [SYSLINK\_RADIO\_P2P](#radio_p2p).

Sent from STM32 to NRF51 to transmit a P2P broadcast packet. The
packet is sent immediately without buffering. If BLE is active it will be
disabled automatically.

When a P2P broadcast is received by the NRF51, it is forwarded to the
STM32 using this same type with the received format.

### SYSLINK\_RADIO\_READY

-   **Type**: 0x0B
-   **Data format**: No data

Sent from STM32 to NRF51 to signal that the STM32 is ready for radio
communication. The NRF51 responds with the same packet type as an
acknowledgment. On boot the NRF51 delays enabling radio reception until
either this command is received or a 3 second timeout expires.

Power management packets
------------------------

### SYSLINK\_PM\_SOURCE

-   **Type**: 0x10

Power source information. Defined in the protocol but currently unused
in the NRF51 firmware.

### SYSLINK\_PM\_ONOFF\_SWITCHOFF

-   **Type**: 0x11
-   **Format**: No data

When sent by the STM32 to the NRF51, the NRF51 switches OFF the system and
goes in deep sleep.

### SYSLINK\_PM\_BATTERY\_VOLTAGE

-   **Type**: 0x12

Battery voltage report. Defined in the protocol but currently unused
in the NRF51 firmware (battery voltage is sent as part of
SYSLINK\_PM\_BATTERY\_STATE instead).

### SYSLINK\_PM\_BATTERY\_STATE

-   **Type**: 0x13
-   **Format**:

        +-------+------+------+------+
        | FLAGS | VBAT | ISET | TEMP |
        +-------+------+------+------+
         1 byte  4 bytes 4 bytes 4 bytes
                                (optional)

-   **FLAGS**: Bit0: Charging. Bit1: USB Powered. Bit2: Can charge.
-   **VBAT**: IEEE single float. Battery voltage.
-   **ISET**: IEEE single float. Charge current in milli-Amperes.
-   **TEMP** (optional): IEEE single float. Temperature. Only present
    when compiled with `PM_SYSLINK_INCLUDE_TEMP`, which adds 4 bytes
    to the packet.

The SYSLINK\_PM\_BATTERY\_STATE packet is sent 100 times per second to
the STM32 (every 10ms). Sending is activated after the STM32 sends a
SYSLINK\_PM\_BATTERY\_AUTOUPDATE packet.

### SYSLINK\_PM\_BATTERY\_AUTOUPDATE

-   **Type**: 0x14
-   **Format**: No data

Sent by the STM32 to the NRF51 to enable periodic battery state and
RSSI reporting. Until this packet is received, the NRF51 does not send
SYSLINK\_PM\_BATTERY\_STATE or SYSLINK\_SYSLINK\_RADIO\_RSSI packets.

### SYSLINK\_PM\_SHUTDOWN\_REQUEST

-   **Type**: 0x15
-   **Format**: No data

Sent by the NRF51 to the STM32 to request a graceful shutdown (e.g.
when the button is short-pressed while running). The STM32 should
respond with a SYSLINK\_PM\_SHUTDOWN\_ACK. If no acknowledgment is
received within 100ms, the NRF51 will force a shutdown.

### SYSLINK\_PM\_SHUTDOWN\_ACK

-   **Type**: 0x16
-   **Format**: No data

Sent by the STM32 to the NRF51 to acknowledge a shutdown request. Upon
receipt the NRF51 proceeds to power off the system.

### SYSLINK\_PM\_LED\_ON

-   **Type**: 0x17
-   **Format**: No data

Sent by the STM32 to the NRF51 to turn on the NRF51 LED.

### SYSLINK\_PM\_LED\_OFF

-   **Type**: 0x18
-   **Format**: No data

Sent by the STM32 to the NRF51 to turn off the NRF51 LED.

### SYSLINK\_PM\_DECKCTRL\_DFU

-   **Type**: 0x19
-   **Data format**: One uint8\_t. Non-zero to enter DFU mode, zero for
    normal mode.

Sent by the STM32 to the NRF51 to put the deck controller into DFU
(Device Firmware Update) mode. The NRF51 sets the WKUP pin accordingly
and power-cycles the system to reset the deck controller.

One Wire packets
----------------

One Wire packets provide access to the 1-Wire memories on attached
expansion decks. The NRF51 handles the low-level 1-Wire protocol and
exposes scan, info, read, and write operations over syslink.

### SYSLINK\_OW\_SCAN

-   **Type**: 0x20
-   **Request format**: No data
-   **Response format**: One uint8\_t indicating the number of 1-Wire
    memories found

Sent by the STM32 to scan the 1-Wire bus. The NRF51 responds with the
number of detected memories.

### SYSLINK\_OW\_GETINFO

-   **Type**: 0x21
-   **Request format**: One uint8\_t memory index
-   **Response format**: One uint8\_t memory index followed by 8 bytes
    of the 1-Wire serial number (ROM ID)

If the memory index is invalid, the response contains a single byte
set to 0xFF.

### SYSLINK\_OW\_READ

-   **Type**: 0x22
-   **Request format**:

        +------+---------+
        | NMEM | ADDRESS |
        +------+---------+

    -   **NMEM**: uint8\_t, memory index
    -   **ADDRESS**: uint8\_t, start address to read from

-   **Response format**: The request header followed by 29 bytes of data
    (total length 32 bytes)

Reads 29 bytes from the specified address of the selected 1-Wire memory.
If the memory index is invalid, the response contains a single byte
set to 0xFF.

### SYSLINK\_OW\_WRITE

-   **Type**: 0x23
-   **Request format**:

        +------+---------+--------+=======+
        | NMEM | ADDRESS | LENGTH | DATA  |
        +------+---------+--------+=======+

    -   **NMEM**: uint8\_t, memory index
    -   **ADDRESS**: uint8\_t, start address to write to
    -   **LENGTH**: uint8\_t, number of bytes to write
    -   **DATA**: bytes to write

-   **Response format**: The same packet is echoed back on success. On
    failure the response contains a single byte: 0xFF if the memory
    could not be selected, or 0xFE if writing is not supported (e.g.
    when BLE is active).

**Note**: Writing 1-Wire memory is not supported when BLE is enabled.

System packets
--------------

### SYSLINK\_SYS\_NRF\_VERSION

-   **Type**: 0x30
-   **Request format**: No data
-   **Response format**: Null-terminated string containing the NRF51
    firmware version tag

Sent by the STM32 to request the NRF51 firmware version. The response
string includes the version tag, a `*` suffix if the build was from
modified sources, and a platform identifier in parentheses
(e.g. `"2024.02 (cf2)"`).

Debug packets
-------------

### SYSLINK\_DEBUG\_PROBE

-   **Type**: 0xF0
-   **Request format**: No data
-   **Response format**: 8 bytes of diagnostic data

        +-------+------+------+---------+----------+----------+---------+---------+
        | ADDR  | CHAN | RATE | DROPPED | UART_ERR | UART_CNT | CKSUM1  | CKSUM2  |
        +-------+------+------+---------+----------+----------+---------+---------+

-   **ADDR**: uint8\_t, 1 if a SYSLINK\_RADIO\_ADDRESS command has been received
-   **CHAN**: uint8\_t, 1 if a SYSLINK\_RADIO\_CHANNEL command has been received
-   **RATE**: uint8\_t, 1 if a SYSLINK\_RADIO\_DATARATE command has been received
-   **DROPPED**: uint8\_t, 1 if UART data has been dropped
-   **UART\_ERR**: uint8\_t, UART error flags
-   **UART\_CNT**: uint8\_t, UART error count
-   **CKSUM1**: uint8\_t, syslink RX checksum 1 error count
-   **CKSUM2**: uint8\_t, syslink RX checksum 2 error count

Sent by the STM32 to request NRF51 diagnostic information. Useful for
debugging communication issues between the STM32 and NRF51.
