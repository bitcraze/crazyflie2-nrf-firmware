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

static bool isInit;
static int nMemory;

extern int bleEnabled;

#define OW_MAX_CACHED 4
static struct {unsigned char address[8]; unsigned char data[122];} owCache[OW_MAX_CACHED];

static void cacheMemory(int nMem)
{
  if (nMem<OW_MAX_CACHED) {
    owSerialNum(0, owCache[nMem].address, 1);
    ds28e05ReadMemory(0, 0, owCache[nMem].data, 112);
  }
}

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
    cacheMemory(0);
    nMem++;
    while(owNext(0, 1, 0))
    {
      if (nMem < OW_MAX_CACHED) {
        cacheMemory(nMem);
      }

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

  if (isInit == false)
    return false;

  switch (pk->type) {
    case SYSLINK_OW_SCAN:
      if (!bleEnabled) {
        nMemory = owScan();
      }

      pk->data[0] = nMemory;
      pk->length = 1;
      tx = true;
      break;
    case SYSLINK_OW_GETINFO:
      if (bleEnabled && command->nmem < nMemory) {
        memcpy(command->info.memId, owCache[command->nmem].address, 8);
        pk->length = 1+8;
        tx = true;
      } else if (!bleEnabled && selectMemory(command->nmem)) {
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
      if (bleEnabled && command->nmem<nMemory) {
        memcpy(command->read.data, &owCache[command->nmem].data[command->read.address], 29);
        pk->length = 32;
        tx=true;

      } else if (!bleEnabled && selectMemory(command->nmem) &&
          ds28e05ReadMemory(0, command->read.address, command->read.data, 29)) {
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
      if (bleEnabled) {
        //Cannot currently write the memory when compiled with BLE
        pk->data[0] = -2;
        pk->length = 1;
        tx=true;
      } else {
        if (selectMemory(command->nmem) &&
            ds28e05WriteMemory(0, command->write.address, command->write.data, command->write.length)) {
          tx=true;
        } else {
          //Cannot select the memory
          pk->data[0] = -1;
          pk->length = 1;
          tx=true;
        }
      }
      break;
  }

  return tx;
}


/**
 * Return true if any discovered deck has:
 *   - Header magic 0xEB at data[0]
 *   - Specified VID at data[5]
 *   - Specified PID at data[6]
 *   - boardName element matching the provided boardName in the key/value area
 */
bool memoryHasDeck(uint8_t vid, uint8_t pid, const char *boardName)
{
  for (int i = 0; i < nMemory; i++)
  {
    const unsigned char *d = owCache[i].data;

    // 1) Basic header sanity checks
    if (d[0] != 0xEB) {
      continue; // Not a valid Crazyflie deck memory header
    }
    if (d[5] != vid || d[6] != pid) {
      continue; // Not a deck with the specified VID/PID
    }

    // 2) key/value parse:
    //    Byte 9 = dataLength. Then from d[10..(10+dataLength-1)]
    //    we have a series of elements:
    //      Element format = { element_id (1 byte), length (1 byte), data[] }
    unsigned int dataLen = d[9];
    if (dataLen == 0) {
      // This deck has no key/value pairs
      continue;
    }

    // Start parsing at offset = 10
    unsigned int offset = 10;
    // We must not exceed the memory we read, so also guard with 10 + dataLen <= 122
    unsigned int endOffset = (10 + dataLen <= 122) ? (10 + dataLen) : 122;

    bool foundBoardName = false;
    while (offset + 2 <= endOffset)  // need at least element_id + length
    {
      uint8_t elementId = d[offset];
      uint8_t length    = d[offset + 1];
      offset += 2;  // move past id and length

      if (offset + length > endOffset) {
        break; // malformed or truncated
      }

      if (elementId == 1) {
        // Element ID = 1 => boardName (string)
        // Compare with provided boardName
        if (length == strlen(boardName) && memcmp(&d[offset], boardName, length) == 0) {
          foundBoardName = true;
          // We can break early since we only care about one match
          break;
        }
      }

      // Move to next element
      offset += length;
    }

    if (foundBoardName) {
      return true; // This deck matches the specified criteria
    }
  }

  // No matching deck found
  return false;
}
