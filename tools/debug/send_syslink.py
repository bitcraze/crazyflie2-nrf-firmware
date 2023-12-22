#!/usr/bin/env python

# Small script to stress the syslink RX state machine to see if it works well

import serial
import struct

nrf = serial.Serial("/dev/ttyACM0", 1000000, rtscts=True)

def send_packet(header: int, data: bytes):
    packet = bytearray()

    packet.extend(b"\xbc\xcf")
    packet.extend(struct.pack("<BB", header, len(data)))
    packet.extend(data)

    cksum_a = 0
    cksum_b = 0

    for b in packet:
        chsum_a = cksum_a + b
        cksum_b = cksum_b + cksum_a
    
    packet.append(cksum_a)
    packet.append(cksum_b)

    nrf.write(packet)

send_packet(0, b"hello")
