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
 * DS2431 OW memory driver
 */
#include <stdint.h>
#include <string.h>

#include <ownet.h>

#define DS2431_READ_SCRATCHPAD 0xaa
#define DS2431_WRITE_SCRATCHPAD 0x0f
#define DS2431_READ_MEMORY 0xf0
#define DS2431_COPY_SCRATCHPAD 0x55

int ds2431ReadScratchpad(int bus, uint16_t *t, uint8_t *es,
                         unsigned char *buffer, int length, uint16_t *crc16)
{
  unsigned int i;

  if (owAccess(bus) == 0)
    goto error;

  owWriteByte(bus, DS2431_READ_SCRATCHPAD);

  i = owReadByte(bus) & 0x00ff;
  i |= (owReadByte(bus)<<8) & 0x0ff00;
  if (t)
    *t = i;

  i = owReadByte(bus);
  if (es)
    *es = i;

  memset(buffer, 0xff, length);

  owBlock(bus, 0, buffer, length);

  if (length == 8) {
    i = (owReadByte(bus)<<8) & 0x0ff00;
    i |= owReadByte(bus);
    if (crc16)
      *crc16 = i;
  }

  return 1;
  error:
  return 0;
}

int ds2431WriteScratchpad(int bus, uint16_t t, unsigned char *buffer, int length)
{
  if (owAccess(bus) == 0)
    goto error;

  owWriteByte(bus, DS2431_WRITE_SCRATCHPAD);

  owWriteByte(bus, t);
  owWriteByte(bus, t>>8);


  owBlock(bus, 0, buffer, length);

  return 1;
  error:
  return 0;
}

int ds2431ReadMemory(int bus, uint16_t t, unsigned char *buffer, int length)
{
  if (owAccess(bus) == 0)
    goto error;

  owWriteByte(bus, DS2431_READ_MEMORY);

  owWriteByte(bus, t);
  owWriteByte(bus, t>>8);


  memset(buffer, 0xff, length);
  owBlock(bus, 0, buffer, length);

  return 1;
  error:
  return 0;
}

int ds2431CopyScratchpad(int bus, uint16_t t, uint8_t es)
{
  if (owAccess(bus) == 0)
    goto error;

  owWriteByte(bus, DS2431_COPY_SCRATCHPAD);

  owWriteByte(bus, t);
  owWriteByte(bus, t>>8);
  owWriteByte(bus, es);

  msDelay(15);

  if (owTouchBit(0, 1) == 0) {
    return 1;
  } else {
    OWERROR(OWERROR_COPY_SCRATCHPAD_FAILED);
    return 0;
  }
  error:
  return 0;
}

int ds2431WriteMemory(int bus, uint16_t t, unsigned char *buffer, int length)
{
  int i=0, j, k;
  unsigned char scratch[8];
  unsigned char readScratch[8];
  uint16_t readT;
  uint8_t readES;


  while(i < length) {
    for (j=0; j<8; j++) {
      if (length>0)
        scratch[j] = buffer[i++];
      else {
        scratch[j] = 0;
        i++;
      }
    }

    if (ds2431WriteScratchpad(bus, i-8, scratch, 8) == 0)
      goto error;

    for(k=0; k<2; k++) {
      if (ds2431ReadScratchpad(bus, &readT, &readES, readScratch, 8, NULL) == 0)
        goto error;

      if (!memcmp(scratch, readScratch, 8))
        break;

      if (ds2431WriteScratchpad(bus, i-8, scratch, 8) == 0)
        goto error;
    }
    if (k==2) {
      OWERROR(OWERROR_WRITE_SCRATCHPAD_FAILED);
    }

    if (ds2431CopyScratchpad(bus, readT, readES) == 0)
      goto error;
  }

  return 1;
  error:
  return 0;
}

