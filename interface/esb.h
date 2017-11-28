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
 * Implementation of the Nordic ESB protocol in PRX mode for nRF51822
 */
#ifndef __ESB_H__
#define __ESB_H__

#include <stdbool.h>
#include <stdint.h>

/* ESB Radio packet */
typedef struct esbPacket_s {
  /* Part that is written by the radio DMA */
  struct {
    uint8_t size;
    union {
      uint8_t s1;
      struct {
        uint8_t ack :1;
        uint8_t pid :2;
      };
    };
    uint8_t data[32];
  } __attribute__((packed));
  /* Written by the radio interrupt routine */
  /* Since the value of the RSSI sample can only be between ~[0, 100] we down cast
   * from uint32_t to uint8_t without lose of precision */
  uint8_t rssi;
  unsigned int crc;
  uint8_t match;
} EsbPacket;

typedef enum esbDatarate_e { esbDatarate250K=0,
                             esbDatarate1M=1,
                             esbDatarate2M=2 } EsbDatarate;

/*** For compatibility ***/
#define RADIO_RATE_250K esbDatarate250K
#define RADIO_RATE_1M esbDatarate1M
#define RADIO_RATE_2M esbDatarate2M

#define ESB_UNICAST_ADDRESS_MATCH 0
#define ESB_MULTICAST_ADDRESS_MATCH 1

/* Initialize the radio for ESB */
void esbInit();

/* Stop ESB and free the radio */
void esbDeinit();

void esbInterruptHandler();

/* Return true is a packet has been received and is ready to read */
bool esbIsRxPacket();

/* Return the packet received of NULL if none*/
EsbPacket * esbGetRxPacket();

/* Release the RX packet so that it can be used again by the radio */
void esbReleaseRxPacket();

/* Return true if a packet can be pushed in the TX queue */
bool esbCanTxPacket();

/* Return the address of the next TX packet in the TX queue */
EsbPacket * esbGetTxPacket();

/* Release and set for sending the buffer returned by getTxPacket */
void esbSendTxPacket();

/* Set datarate */
void esbSetDatarate(EsbDatarate datarate);

/* Set channel */
void esbSetChannel(unsigned int channel);

/* Set output power (according to nRF51 defines)*/
void esbSetTxPower(int power);

/* Set output power in Dbm*/
void esbSetTxPowerDbm(int8_t powerDbm);

/* Set of disable radio continuous wave */
void esbSetContwave(bool enable);

/* Set the address of the radio */
void esbSetAddress(uint64_t address);

#endif //__ESB_H__
