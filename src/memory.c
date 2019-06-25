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
 *
 * OW memory interface for syslink
 */
#include "memory.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "syslink.h"

#include "ow.h"
#include "ownet.h"
#include "ds2431.h"
#include "ds28e05.h"

extern int ble_init(void);
extern void ble_deinit(void);

static bool isInit;
static int nMemory;

static bool selectMemory(int n)
{
  if (owFirst(0, 1, 0))
  {
    n--;
    while (n>=0 && owNext(0, 1, 0))
    {
      n--;
    }
  }

  if (n<0)
  {
    return true;
  }

  return false;
}

int owScan()
{
  int nMem = 0;
  owTouchReset(0);

  msDelay(10);

  if (owFirst(0, 1, 0))
  {
    nMem++;
    while(owNext(0, 1, 0))
    {
      nMem++;
    }
  }
  return nMem;
}

void memoryInit()
{
  owInit();
  msDelay(10);

  nMemory = owScan();

  isInit = true;
}



// Returns true if the packet has been mofified and is good for sending back to the SMT
bool memorySyslink(struct syslinkPacket *pk) {
  bool tx = false;
  struct memoryCommand_s *command = (void*)pk->data;

  #ifdef BLE
  // Disable BLE stack to have real-time control during OW access
  ble_deinit();
  #endif

  if (isInit == false)
    return false;

  switch (pk->type) {
    case SYSLINK_OW_SCAN:
      nMemory = owScan();
      pk->data[0] = nMemory;
      pk->length = 1;
      tx = true;
      break;
    case SYSLINK_OW_GETINFO:
      if (selectMemory(command->nmem))
      {
        owSerialNum(0, command->info.memId, 1);
        pk->length = 1+8;
        tx = true;
      } else {
        //Cannot select the memory
        pk->data[0] = -1;
        pk->length = 1;
        tx=true;
      }
      break;

    case SYSLINK_OW_READ:
      if (selectMemory(command->nmem) &&
          ds28e05ReadMemory(0, command->read.address, command->read.data, 29))
      {
        pk->length = 32;
        tx=true;
      } else {
        //Cannot select the memory
        pk->data[0] = -1;
        pk->length = 1;
        tx=true;
      }

      break;
    case SYSLINK_OW_WRITE:
      if (selectMemory(command->nmem) &&
          ds28e05WriteMemory(0, command->write.address, command->write.data, command->write.length))
      {
        tx=true;
      } else {
        //Cannot select the memory
        pk->data[0] = -1;
        pk->length = 1;
        tx=true;
      }
      break;
  }

  #ifdef ble
  ble_init();
  #endif

  return tx;
}
