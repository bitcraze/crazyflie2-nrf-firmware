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
#ifndef __MEMORY_H__
#define __MEMORY_H__
#include <stdint.h>
#include <stdbool.h>

#include "syslink.h"

void memoryInit();

bool memorySyslink(struct syslinkPacket *pk);

struct memoryCommand_s {
  uint8_t nmem;
  union {
    struct {
      uint8_t memId[8];
    } __attribute__((packed)) info;
    struct  {
      uint16_t address;
      uint8_t data[29];
    } __attribute__((packed)) read;
    struct write {
      uint16_t address;
      uint16_t length;
      char data[27];
    } __attribute__((packed)) write;
  };
} __attribute__((packed));

#endif //__MEMORY_H__
