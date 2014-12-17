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
#ifndef __BLE_CRAZYFLIES_H__
#define __BLE_CRAZYFLIES_H__

#include <stdbool.h>
#include "esb.h"
#include <ble.h>

uint32_t ble_crazyflies_init(uint8_t uuidType);

void ble_crazyflies_on_ble_evt(ble_evt_t * p_ble_evt);


bool bleCrazyfliesIsPacketReceived();
EsbPacket* bleCrazyfliesGetRxPacket();
void bleCrazyfliesReleaseRxPacket(EsbPacket* packet);
void bleCrazyfliesSendPacket(EsbPacket* packet);

#endif
