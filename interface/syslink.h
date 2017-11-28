/**
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie 2.0 NRF Firmware
 * Copyright (c) 2014, Bitcraze AB, All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */
#ifndef __PACKET_H__
#define __PACKET_H__

#include <stdbool.h>
#include <stdint.h>

#define SYSLINK_STARTUP_DELAY_TIME_MS 1000
#define SYSLINK_SEND_PERIOD_MS 10

#define SYSLINK_MTU 32

struct syslinkPacket {
  uint8_t type;
  uint8_t length;
  char data[SYSLINK_MTU];
};

bool syslinkReceive(struct syslinkPacket *packet);

bool syslinkSend(struct syslinkPacket *packet);


// Defined packet types
#define SYSLINK_RADIO_RAW           0x00
#define SYSLINK_RADIO_CHANNEL       0x01
#define SYSLINK_RADIO_DATARATE      0x02
#define SYSLINK_RADIO_CONTWAVE      0x03
#define SYSLINK_RADIO_RSSI          0x04
#define SYSLINK_RADIO_ADDRESS       0x05
#define SYSLINK_RADIO_RAW_BROADCAST 0x06
#define SYSLINK_RADIO_POWER         0x07


#define SYSLINK_PM_SOURCE 0x10

#define SYSLINK_PM_ONOFF_SWITCHOFF 0x11

#define SYSLINK_PM_BATTERY_VOLTAGE 0x12
#define SYSLINK_PM_BATTERY_STATE   0x13
#define SYSLINK_PM_BATTERY_AUTOUPDATE 0x14

#define SYSLINK_OW_SCAN 0x20
#define SYSLINK_OW_GETINFO 0x21
#define SYSLINK_OW_READ 0x22
#define SYSLINK_OW_WRITE 0x23

#endif
