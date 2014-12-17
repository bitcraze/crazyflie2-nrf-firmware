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
 * DS28E05 OW memory driver
 */
#include "ds28e05.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <ownet.h>

#define DS28E05_READ_MEMORY   0xf0
#define DS28E05_WRITE_MEMORY  0x55
#define PAGE_MASK             0b01110000
#define SEG_MASK              0b00001110
#define SEG_SIZE              8
#define BYTES_PER_SEG         2
#define BYTES_PER_PAGE        (SEG_SIZE * BYTES_PER_SEG)
#define NUM_PAGES             7

int ds28e05ReadMemory(int bus, uint16_t t, unsigned char *buffer, int length)
{
  int i;

  if (owAccess(bus) == 0)
    goto error;

  owWriteByte(bus, DS28E05_READ_MEMORY);

  owWriteByte(bus, t);
  owWriteByte(bus, t>>8);


  //memset(buffer, 0xff, length);
  //owBlock(bus, 0, buffer, length);

  for (i=0; i<length; i++)
    buffer[i] = owReadByte(bus);

  return 1;
  error:
  return 0;
}

int ds28e05WriteMemory(int bus, uint8_t address, char *buffer, int length)
{
  char a,b;
  char sf,ef;
  uint8_t sb[2]={0};
  uint8_t eb[2]={0};
  uint8_t spage, epage, npage, wpage;
  uint8_t nseg, wseg=0;
  uint8_t wBytes=0, rBytes=0, wAddr = 0;

  // Calculate pages
  spage = (address & PAGE_MASK) >> 4;
  epage = ((address + length) & PAGE_MASK) >> 4;
  if (epage == NUM_PAGES) epage = NUM_PAGES - 1;
  npage = epage - spage;

  //This memory must be written respecting 16bits boundaries
  sf = (address&0x01) != 0;
  ef = ((address+length)&0x01) != 0;

  if (ef)
  {
    ds28e05ReadMemory(bus, address+length, &eb[1], 1);
    eb[0] = buffer[length-1];
    length++;
  }
  if (sf)
  {
    ds28e05ReadMemory(bus, address-1, &sb[0], 1);
    sb[1] = buffer[0];
    length++;
    address--;
  }

  // Write pages
  for (wpage = 0; wpage <= npage; wpage++)
  {
    wAddr = address + wBytes;
    // Calculate segments to write
    if ((length - wBytes) > (BYTES_PER_PAGE))
      // Will we pass a page boudary
      if (wAddr % (SEG_SIZE * BYTES_PER_SEG) == 0)
        nseg = SEG_SIZE;
      else
        nseg = (BYTES_PER_PAGE - (wAddr % (BYTES_PER_PAGE))) >> 1;
    else
      nseg = ((length - wBytes) & SEG_MASK) >> 1;

    if (owAccess(bus) == 0)
      goto error;

    owWriteByte(bus, DS28E05_WRITE_MEMORY);
    owWriteByte(bus, wAddr);
    owWriteByte(bus, 0xff);
    // Write segments within page
    for (wseg = 0; wseg < nseg; wseg++)
    {
      if (sf)
      {
        owWriteByte(bus, sb[0]);
        owWriteByte(bus, sb[1]);
        wBytes += 2;
        rBytes++;
        sf = 0;
      }
      else if (ef && (length - wBytes) <= 2)
      {
        owWriteByte(bus, eb[0]);
        owWriteByte(bus, eb[1]);
        wBytes += 2;
        rBytes++;
        ef = 0;
      }
      else
      {
        owWriteByte(bus, buffer[rBytes]);
        owWriteByte(bus, buffer[rBytes+1]);
        wBytes += 2;
        rBytes += 2;
      }

      a = owReadByte(bus);
      b = owReadByte(bus);

      printf("Readback: %02x %02x (%c%c)\n", a, b, a, b);

      owWriteByte(bus, 0xff);

      msDelay(16);

      a = owReadByte(bus);

      printf("Verification byte: %02x\n", a);

      if (a != 0xAAU)
        goto error;
    }

    owTouchReset(0);
  }

  return 1;
error:
  owTouchReset(0);
  return 0;
}
