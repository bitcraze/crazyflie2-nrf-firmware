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

#include "crc32_calc.h"
#include "ow.h"
#include "ownet.h"
#include "ds2431.h"
#include "ds28e05.h"

static bool isInit;
static int nMemory;

extern int bleEnabled;

#define OW_MAX_CACHED 4
static struct {unsigned char address[8]; unsigned char data[122];} owCache[OW_MAX_CACHED];

#define DECK_INFO_HEADER_ID       0xEB
#define DECK_INFO_HEADER_SIZE     7
#define DECK_INFO_NAME            1
#define DECK_INFO_TLV_VERSION     0
#define DECK_INFO_TLV_VERSION_POS 8
#define DECK_INFO_TLV_LENGTH_POS  9
#define DECK_INFO_TLV_DATA_POS    10

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

#pragma pack(push, 1)
typedef struct deckInfo_s {
    union {
        struct {
            uint8_t header;
            uint32_t usedPins;
            uint8_t vid;
            uint8_t pid;
            uint8_t crc;          // CRC of the header (LSB of the 32-bit CRC)
            uint8_t rawTlv[104];
        };
        uint8_t raw[112];
    };

    struct {
        uint8_t* data; // pointer to the TLV data start
        int length;    // length of the TLV data
    } tlv;
} DeckInfo;
#pragma pack(pop)


/*
 * TLV scanning helpers
 */

static int findType(const uint8_t *tlvData, int tlvLength, int type)
{
    int pos = 0;
    while (pos < tlvLength) {
        int currentType = tlvData[pos];
        int lengthField = tlvData[pos + 1];
        if (currentType == type) {
            return pos;
        }
        // move past this TLV: 1 byte for type, 1 for length, plus payload
        pos += (2 + lengthField);
    }
    return -1;
}

static bool deckTlvHasElement(const uint8_t *tlvData, int tlvLength, int type)
{
    return (findType(tlvData, tlvLength, type) >= 0);
}

static int deckTlvGetString(const uint8_t *tlvData, int tlvLength, int type,
                            char *stringOut, int maxLen)
{
    int pos = findType(tlvData, tlvLength, type);
    if (pos < 0) {
        if (maxLen > 0) {
            stringOut[0] = '\0';
        }
        return -1;
    }

    int strLen = tlvData[pos + 1];
    if (strLen >= maxLen) {
        strLen = maxLen - 1;
    }
    memcpy(stringOut, &tlvData[pos + 2], strLen);
    stringOut[strLen] = '\0';

    return strLen;
}

// Returns true if a deck with the given vid/pid and board name is found in memory
bool memoryHasDeck(uint8_t vid, uint8_t pid, const char *boardName)
{
    for (int i = 0; i < nMemory; i++) {
        DeckInfo info;
        // Copy up to 112 bytes from owCache[i].data
        memcpy(info.raw, owCache[i].data, sizeof(info.raw));

        // 1) Check the header
        if (info.header != DECK_INFO_HEADER_ID) {
            continue; // Not a valid deck
        }

        // 2) Compute the full 32-bit CRC for the deck info header, then compare its LSB
        uint32_t fullCrc = crc32CalculateBuffer(info.raw, DECK_INFO_HEADER_SIZE);
        uint8_t  computed = (uint8_t)(fullCrc & 0xFF);
        if (info.crc != computed) {
            // Mismatch
            continue;
        }

        // 3) Check vid/pid
        if (info.vid != vid || info.pid != pid) {
            continue;
        }

        // 4) Check the TLV version
        uint8_t version = info.raw[DECK_INFO_TLV_VERSION_POS];
        if (version != DECK_INFO_TLV_VERSION) {
            continue;
        }

        // 5) Get the length of the TLV block
        uint8_t tlvLen = info.raw[DECK_INFO_TLV_LENGTH_POS];

        // 6) Setup TLV area
        info.tlv.data   = &info.raw[DECK_INFO_TLV_DATA_POS];
        info.tlv.length = tlvLen;

        // 7) Check the TLV CRC
        //    (Compute a CRC from offset 8 (version) through offset 9+tlvLen,
        //     then compare its LSB to the byte right after the TLV data)
        {
          // Ensure we don’t go out of bounds accessing info.raw[DECK_INFO_TLV_DATA_POS + tlvLen]
          if ((DECK_INFO_TLV_DATA_POS + tlvLen) < sizeof(info.raw)) {
              uint8_t storedTlvCrc = info.raw[DECK_INFO_TLV_DATA_POS + tlvLen];

              // Compute a 32-bit CRC on [ version..(version+tlvLen+1) ],
              // i.e. info.raw[8..(9+tlvLen)], which is (tlvLen + 2) bytes
              uint32_t computedTlvCrc = crc32CalculateBuffer(
                  &info.raw[DECK_INFO_TLV_VERSION_POS],
                  (tlvLen + 2)
              );

              if ((computedTlvCrc & 0xFF) != storedTlvCrc) {
                  // TLV region corrupt
                  continue;
              }
          } else {
              // If there's no room for the stored TLV CRC byte, it's invalid
              continue;
          }
      }

        // 8) Check for board name in TLV
        if (deckTlvHasElement(info.tlv.data, info.tlv.length, DECK_INFO_NAME)) {
            char foundName[64];
            deckTlvGetString(info.tlv.data, info.tlv.length,
                             DECK_INFO_NAME, foundName, sizeof(foundName));
            if (strcmp(foundName, boardName) == 0) {
                return true;
            }
        }
    }

    return false; // not found
}
