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
 * esb.c - Implementation of the Nordic ESB protocol in PRX mode for nRF51822
 */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "esb.h"
#include "pm.h"

#include <nrf.h>

#ifdef BLE
#include <ble_gap.h>
#include <nrf_soc.h>
#endif

#define RXQ_LEN 16
#define TXQ_LEN 16

static bool isInit = true;

static int channel = 2;
static int datarate = esbDatarate2M;
static int txpower = RADIO_TXPOWER_TXPOWER_0dBm;
static bool contwave = false;
static uint64_t address = 0xE7E7E7E7E7ULL;

static enum {doTx, doRx} rs;      //Radio state

static EsbPacket rxPackets[TXQ_LEN];
static int rxq_head = 0;
static int rxq_tail = 0;

static EsbPacket txPackets[TXQ_LEN];
static int txq_head = 0;
static int txq_tail = 0;

// 1bit packet counters
static int curr_down = 1;
static int curr_up = 1;

static bool has_safelink;

static EsbPacket ackPacket;     // Empty ack packet
static EsbPacket servicePacket; // Packet sent to answer a low level request

/* helper functions */

static uint32_t swap_bits(uint32_t inp)
{
  uint32_t i;
  uint32_t retval = 0;

  inp = (inp & 0x000000FFUL);

  for(i = 0; i < 8; i++)
  {
    retval |= ((inp >> i) & 0x01) << (7 - i);
  }

  return retval;
}

static uint32_t bytewise_bitswap(uint32_t inp)
{
  return (swap_bits(inp >> 24) << 24)
       | (swap_bits(inp >> 16) << 16)
       | (swap_bits(inp >> 8) << 8)
       | (swap_bits(inp));
}

/* Radio protocol implementation */

static bool isRetry(EsbPacket *pk)
{
  static int prevPid;
  static int prevCrc;

  bool retry = false;

  if ((prevPid == pk->pid) && (prevCrc == pk->crc)) {
    retry = true;
  }

  prevPid = pk->pid;
  prevCrc = pk->crc;

  return retry;
}

// Handles the queue
static void setupTx(bool retry)
{
  static EsbPacket * lastSentPacket;

  if (retry) {
    NRF_RADIO->PACKETPTR = (uint32_t)lastSentPacket;
  } else {
    if (lastSentPacket != &ackPacket) {
      //No retry, TX payload has been sent!
      if (txq_head != txq_tail) {
        txq_tail = ((txq_tail+1)%TXQ_LEN);
      }
    }
    if (lastSentPacket == &servicePacket) {
      servicePacket.size = 0;
    }

    if (servicePacket.size) {
      NRF_RADIO->PACKETPTR = (uint32_t)&servicePacket;
      lastSentPacket = &servicePacket;
    } else if (txq_tail != txq_head) {
      // Send next TX packet
      NRF_RADIO->PACKETPTR = (uint32_t)&txPackets[txq_tail];
      if (has_safelink) {
        txPackets[txq_tail].data[0] = (txPackets[txq_tail].data[0]&0xf3) | curr_down<<2;
      }
      lastSentPacket = &txPackets[txq_tail];
    } else {
      // Send empty ACK
#ifdef RSSI_ACK_PACKET
      ackPacket.size = 3;
      ackPacket.data[0] = 0xf3 | curr_down<<2;
      ackPacket.data[1] = 0x01;
      ackPacket.data[2] = NRF_RADIO->RSSISAMPLE;
#elif defined RSSI_VBAT_ACK_PACKET
      ackPacket.size = 4 + sizeof(uint32_t);
      ackPacket.data[0] = 0xf3 | curr_down<<2;
      ackPacket.data[1] = 0x01;
      ackPacket.data[2] = NRF_RADIO->RSSISAMPLE;
      ackPacket.data[3] = 0x02;
      uint32_t vBat = (uint32_t)(pmGetVBAT() * 1000); // Send voltage in mV
      memcpy(&ackPacket.data[4], &vBat, sizeof(uint32_t));
#else
      ackPacket.size = 1;
      ackPacket.data[0] = 0xf3 | curr_down<<2;
#endif
      NRF_RADIO->PACKETPTR = (uint32_t)&ackPacket;
      lastSentPacket = &ackPacket;
    }
  }

  //After being disabled the radio will automatically send the ACK
  NRF_RADIO->SHORTS &= ~RADIO_SHORTS_DISABLED_RXEN_Msk;
  NRF_RADIO->SHORTS |= RADIO_SHORTS_DISABLED_TXEN_Msk;
  rs = doTx;
  NRF_RADIO->TASKS_DISABLE = 1UL;
}

static void setupRx()
{
  NRF_RADIO->PACKETPTR = (uint32_t)&rxPackets[rxq_head];

  NRF_RADIO->SHORTS &= ~RADIO_SHORTS_DISABLED_TXEN_Msk;
  NRF_RADIO->SHORTS |= RADIO_SHORTS_DISABLED_RXEN_Msk;
  rs = doRx;
  NRF_RADIO->TASKS_DISABLE = 1UL;
}

void RADIO_IRQHandler()
{
  esbInterruptHandler();
}

void esbInterruptHandler()
{
  EsbPacket *pk;

  if (NRF_RADIO->EVENTS_END) {
	  NRF_RADIO->EVENTS_END = 0UL;

    switch (rs){
    case doRx:
      //Wrong CRC packet are dropped
      if (!NRF_RADIO->CRCSTATUS) {
        NRF_RADIO->TASKS_START = 1UL;
        return;
      }

      pk = &rxPackets[rxq_head];
      pk->rssi = (uint8_t) NRF_RADIO->RSSISAMPLE;
      pk->crc = NRF_RADIO->RXCRC;
      pk->match = NRF_RADIO->RXMATCH;

      // If no more space available on RX queue, drop packet!
      if (((rxq_head+1)%RXQ_LEN) == rxq_tail) {
        NRF_RADIO->TASKS_START = 1UL;
        return;
      }

      // If this packet is a retry, send the same ACK again
      if ((pk->match == ESB_UNICAST_ADDRESS_MATCH) && isRetry(pk)) {
        setupTx(true);
        return;
      }

      if ((pk->match == ESB_UNICAST_ADDRESS_MATCH))
      {
        // Match safeLink packet and answer it
        if (pk->size == 3 && (pk->data[0]&0xf3) == 0xf3 && pk->data[1] == 0x05) {
          has_safelink = pk->data[2];
          memcpy(servicePacket.data, pk->data, 3);
          servicePacket.size = 3;
          setupTx(false);

          // Reset packet counters
          curr_down = 1;
          curr_up = 1;
          return;
        }

        // Good packet received, yea!
        if (!has_safelink || (pk->data[0] & 0x08) != curr_up<<3) {
          // Push the queue head to push this packet and prepare the next
          rxq_head = ((rxq_head+1)%RXQ_LEN);
          curr_up = 1-curr_up;
        }

        if (!has_safelink || (pk->data[0]&0x04) != curr_down<<2) {
          curr_down = 1-curr_down;
          setupTx(false);
        } else {
          setupTx(true);
        }
      } else
      {
        // Push the queue head to push this packet and prepare the next
        rxq_head = ((rxq_head+1)%RXQ_LEN);
        // broadcast => no ack
        NRF_RADIO->PACKETPTR = (uint32_t)&rxPackets[rxq_head];
        NRF_RADIO->TASKS_START = 1UL;
      }


      break;
    case doTx:
      //Setup RX for next packet
      setupRx();
      break;
    }
  }
}


/* Public API */

// S1 is used for compatibility with NRF24L0+. These three bits are used
// to store the PID and NO_ACK.
#define PACKET0_S1_SIZE                  (3UL)
// S0 is not used
#define PACKET0_S0_SIZE                  (0UL)
// The size of the packet length field is 6 bits
#define PACKET0_PAYLOAD_SIZE             (6UL)
// The size of the base address field is 4 bytes
#define PACKET1_BASE_ADDRESS_LENGTH      (4UL)
// Don't use any extra added length besides the length field when sending
#define PACKET1_STATIC_LENGTH            (0UL)
// Max payload allowed in a packet
#define PACKET1_PAYLOAD_SIZE             (32UL)

void esbInit()
{
  NRF_RADIO->POWER = 1;
  // Enable Radio interrupts
#ifndef BLE
  NVIC_SetPriority(RADIO_IRQn, 3);
  NVIC_EnableIRQ(RADIO_IRQn);
#else
  NVIC_EnableIRQ(RADIO_IRQn);
#endif


  NRF_RADIO->TXPOWER = (txpower << RADIO_TXPOWER_TXPOWER_Pos);

  switch (datarate) {
  case esbDatarate250K:
      NRF_RADIO->MODE = (RADIO_MODE_MODE_Nrf_250Kbit << RADIO_MODE_MODE_Pos);
      break;
  case esbDatarate1M:
      NRF_RADIO->MODE = (RADIO_MODE_MODE_Nrf_1Mbit << RADIO_MODE_MODE_Pos);
      break;
  case esbDatarate2M:
      NRF_RADIO->MODE = (RADIO_MODE_MODE_Nrf_2Mbit << RADIO_MODE_MODE_Pos);
      break;
  }

  NRF_RADIO->FREQUENCY = channel;

  if (contwave) {
    NRF_RADIO->TEST = 3;
    NRF_RADIO->TASKS_RXEN = 1U;
    return;
  }

  // Radio address config
  // We use local addresses 0 and 1
  //  * local address 0 is the unique address of the Crazyflie, used for 1-to-1 communication.
  //    This can be set dynamically and the current address is stored in EEPROM.
  //  * local address 1 is used for broadcasts
  //    This is currently 0xFFE7E7E7E7.
  NRF_RADIO->PREFIX0 = 0xC4C3FF00UL | (bytewise_bitswap(address >> 32) & 0xFF);  // Prefix byte of addresses 3 to 0
  NRF_RADIO->PREFIX1 = 0xC5C6C7C8UL;  // Prefix byte of addresses 7 to 4
  NRF_RADIO->BASE0   = bytewise_bitswap((uint32_t)address);  // Base address for prefix 0
  NRF_RADIO->BASE1   = 0xE7E7E7E7UL;  // Base address for prefix 1-7
  NRF_RADIO->TXADDRESS = 0x00UL;      // Set device address 0 to use when transmitting
  NRF_RADIO->RXADDRESSES = (1<<0) | (1<<1);    // Enable device address 0 and 1 to use which receiving

  // Packet configuration
  NRF_RADIO->PCNF0 = (PACKET0_S1_SIZE << RADIO_PCNF0_S1LEN_Pos) |
                     (PACKET0_S0_SIZE << RADIO_PCNF0_S0LEN_Pos) |
                     (PACKET0_PAYLOAD_SIZE << RADIO_PCNF0_LFLEN_Pos);

  // Packet configuration
   NRF_RADIO->PCNF1 = (RADIO_PCNF1_WHITEEN_Disabled << RADIO_PCNF1_WHITEEN_Pos)    |
                      (RADIO_PCNF1_ENDIAN_Big << RADIO_PCNF1_ENDIAN_Pos)           |
                      (PACKET1_BASE_ADDRESS_LENGTH << RADIO_PCNF1_BALEN_Pos)       |
                      (PACKET1_STATIC_LENGTH << RADIO_PCNF1_STATLEN_Pos)           |
                      (PACKET1_PAYLOAD_SIZE << RADIO_PCNF1_MAXLEN_Pos);

  // CRC Config
  NRF_RADIO->CRCCNF = (RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos); // Number of checksum bits
  NRF_RADIO->CRCINIT = 0xFFFFUL;      // Initial value
  NRF_RADIO->CRCPOLY = 0x11021UL;     // CRC poly: x^16+x^12^x^5+1

  // Enable interrupt for end event
  NRF_RADIO->INTENSET = RADIO_INTENSET_END_Msk;

  // Set all shorts so that RSSI is measured and only END is required interrupt
  NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk;
  NRF_RADIO->SHORTS |= RADIO_SHORTS_ADDRESS_RSSISTART_Msk;
  NRF_RADIO->SHORTS |= RADIO_SHORTS_DISABLED_TXEN_Msk;
  NRF_RADIO->SHORTS |= RADIO_SHORTS_DISABLED_RSSISTOP_Enabled;

  // Set RX buffer and start RX
  rs = doRx;
	NRF_RADIO->PACKETPTR = (uint32_t)&rxPackets[rxq_head];
  NRF_RADIO->TASKS_RXEN = 1U;

  isInit = true;
}

void esbReset()
{
  if (!isInit) return;
#ifndef BLE
  __disable_irq();
#endif
  NRF_RADIO->TASKS_DISABLE = 1;
  NRF_RADIO->POWER = 0;

  NVIC_GetPendingIRQ(RADIO_IRQn);
  __enable_irq();
#ifndef BLE
  esbInit();
#endif
}

void esbDeinit()
{
#ifndef BLE
  NVIC_DisableIRQ(RADIO_IRQn);
#endif

  NRF_RADIO->INTENCLR = RADIO_INTENSET_END_Msk;
  NRF_RADIO->SHORTS = 0;
  NRF_RADIO->TASKS_DISABLE = 1;
  NRF_RADIO->POWER = 0;
}

bool esbIsRxPacket()
{
  return (rxq_head != rxq_tail);
}

EsbPacket * esbGetRxPacket()
{
  EsbPacket *pk = NULL;

  if (esbIsRxPacket()) {
    pk = &rxPackets[rxq_tail];
  }

  return pk;
}

void esbReleaseRxPacket()
{
  rxq_tail = (rxq_tail+1)%RXQ_LEN;
}

bool esbCanTxPacket()
{
  return ((txq_head+1)%TXQ_LEN)!=txq_tail;
}

EsbPacket * esbGetTxPacket()
{
  EsbPacket *pk = NULL;

  if (esbCanTxPacket()) {
    pk = &txPackets[txq_head];
  }

  return pk;
}

void esbSendTxPacket()
{
  txq_head = (txq_head+1)%TXQ_LEN;
}

void esbSetDatarate(EsbDatarate dr)
{
  datarate = dr;

  esbReset();
}

void ble_advertising_stop(void);
void advertising_start(void);
void ble_sd_stop(void);

void esbSetContwave(bool enable)
{
  contwave = enable;

#ifdef BLE
  if (enable)
    ble_advertising_stop();
  else
    advertising_start();
#endif


  esbReset();
}

void esbSetChannel(unsigned int ch)
{
  if (channel < 126) {
	  channel = ch;
	}

  esbReset();
}

void esbSetTxPower(int power)
{
  txpower = power;

  esbReset();
}

void esbSetTxPowerDbm(int8_t powerDbm)
{
  if      (powerDbm <= -30) { txpower = RADIO_TXPOWER_TXPOWER_Neg30dBm; }
  else if (powerDbm <= -20) { txpower = RADIO_TXPOWER_TXPOWER_Neg20dBm; }
  else if (powerDbm <= -16) { txpower = RADIO_TXPOWER_TXPOWER_Neg16dBm; }
  else if (powerDbm <= -12) { txpower = RADIO_TXPOWER_TXPOWER_Neg12dBm; }
  else if (powerDbm <= -8)  { txpower = RADIO_TXPOWER_TXPOWER_Neg8dBm; }
  else if (powerDbm <= -4)  { txpower = RADIO_TXPOWER_TXPOWER_Neg4dBm; }
  else if (powerDbm <=  0)  { txpower = RADIO_TXPOWER_TXPOWER_0dBm; }
  else if (powerDbm >=  4)  { txpower = RADIO_TXPOWER_TXPOWER_Pos4dBm; }

  esbReset();
}

void esbSetAddress(uint64_t addr)
{
  address = addr;

  esbReset();
}
