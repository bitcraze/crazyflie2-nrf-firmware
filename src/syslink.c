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
#include "syslink.h"

#include <nrf.h>
#include "sdk_common.h"
#include "nrf_nvic.h"
#include "boards.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nrf_drv_uart.h"
#include "atomic.h"

#define NRF_LOG_MODULE_NAME "SYSLINK"
#include "nrf_log.h"

/* Frame format:
 * +----+-----+------+-----+=============+-----+-----+
 * |  START   | TYPE | LEN | DATA        |   CKSUM   |
 * +----+-----+------+-----+=============+-----+-----+
 *
 * - Start is 2 bytes constant, defined bellow
 * - Length and type are uint8_t
 * - Length define the data length
 * - CKSUM is 2 bytes Fletcher 8 bit checksum. See rfc1146.
 *   Checksum is calculated with TYPE, LEN and DATA
 */


#define START "\xbc\xcf"
#define START_BYTE1 0xBC
#define START_BYTE2 0xCF

static nrf_drv_uart_t m_uart = NRF_DRV_UART_INSTANCE(0);

static uint8_t m_tx_buffer[SYSLINK_MTU + 6];

static enum {state_first_start, state_second_start, state_length, state_type, state_data, state_cksum1, state_cksum2, state_done} state = state_first_start;

static uint8_t m_syslinkRxCheckSum1ErrorCnt;
static uint8_t m_syslinkRxCheckSum2ErrorCnt;

static struct syslinkPacket m_received_packet;
static bool m_received_packet_valid = false;
static uint8_t m_rx_buffer[SYSLINK_MTU];

void syslinkReset() {
  state = state_first_start;
}

static void uart_on_receive(uint8_t *data, int length) {
  int index = 0;
  static uint8_t cksum_a;
  static uint8_t cksum_b;
  int to_fetch_next = 1;
  static int step;

  while (index < length) {
    NRF_LOG_DEBUG("state: %d, index: %d, data: %02x\n", state, index, data[index]);
    switch(state)
    {
      case state_first_start:
        state = (data[index++] == START_BYTE1) ? state_second_start : state_first_start;
        break;
      case state_second_start:
        state = (data[index++] == START_BYTE2) ? state_type : state_first_start;
        break;
      case state_type:
        m_received_packet.type = data[index++];
        cksum_a = m_received_packet.type;
        cksum_b = cksum_a;
        state = state_length;
        break;
      case state_length:
        m_received_packet.length = data[index++];
        cksum_a += m_received_packet.length;
        cksum_b += cksum_a;
        to_fetch_next = m_received_packet.length + 2;
        step = 0;
        if (m_received_packet.length > 0 && m_received_packet.length <= SYSLINK_MTU)
          state = state_data;
        else if (m_received_packet.length > SYSLINK_MTU)
          state = state_first_start;
        else
          state = state_cksum1;
        break;
      case state_data:
        if (step < SYSLINK_MTU)
        {
          m_received_packet.data[step] = data[index++];
          cksum_a += m_received_packet.data[step];
          cksum_b += cksum_a;
        }
        step++;
        to_fetch_next = m_received_packet.length - step + 2;
        if(step >= m_received_packet.length) {
          state = state_cksum1;
        }
        break;
      case state_cksum1:
        if (data[index++] == cksum_a)
        {
          state = state_cksum2;
          to_fetch_next = 1;
        }
        else
        {  // Wrong checksum
          m_syslinkRxCheckSum1ErrorCnt++;
          state = state_first_start;
          to_fetch_next = 5;
#ifdef SYSLINK_CKSUM_MON
          if (NRF_GPIO->OUT & (1<<LED_PIN))
            NRF_GPIO->OUTCLR = 1<<LED_PIN;
          else
            NRF_GPIO->OUTSET = 1<<LED_PIN;
#endif
        }
        break;
      case state_cksum2:
        if (data[index++] == cksum_b)
        {
          state = state_done;
        }
        else
        {  // Wrong checksum
          m_syslinkRxCheckSum2ErrorCnt++;
          state = state_first_start;
          step = 0;
          to_fetch_next = 4;
#ifdef SYSLINK_CKSUM_MON
          if (NRF_GPIO->OUT & (1<<LED_PIN))
            NRF_GPIO->OUTCLR = 1<<LED_PIN;
          else
            NRF_GPIO->OUTSET = 1<<LED_PIN;
#endif
        }
        break;
      case state_done:
        state = state_first_start;
        m_received_packet_valid = true;
        break;
    }
  }

  // Setup transfer of next packet
  nrf_drv_uart_rx(&m_uart, m_rx_buffer, to_fetch_next);
}

uint32_t syslinkSend(struct syslinkPacket *packet)
{
  uint8_t cksum_a=0;
  uint8_t cksum_b=0;
  int i;

  if (!nrf_drv_uart_tx_in_progress(&m_uart))
  {
    // construct packet in m_tx_buffer
    memcpy(m_tx_buffer, START, 2);
    m_tx_buffer[2] = packet->type;
    cksum_a += packet->type;
    cksum_b += cksum_a;
    m_tx_buffer[3] = packet->length;
    cksum_a += packet->length;
    cksum_b += cksum_a;
    memcpy(&m_tx_buffer[4], packet->data, packet->length);
    for (i=0; i < packet->length; i++)
    {
      cksum_a += packet->data[i];
      cksum_b += cksum_a;
    }
    m_tx_buffer[4+packet->length] = cksum_a;
    m_tx_buffer[5+packet->length] = cksum_b;
    
    // Send buffer
    return nrf_drv_uart_tx(&m_uart, m_tx_buffer, 6+packet->length);
  } else {
    return NRF_ERROR_BUSY;
  }
}

uint32_t syslinkSendBlocking(struct syslinkPacket *packet) {
  while (syslink_is_tx_busy()) {
    // Wait for previous packet to be sent
  }
  return syslinkSend(packet);
}

uint8_t syslinkGetRxCheckSum1ErrorCnt() {
  return m_syslinkRxCheckSum1ErrorCnt;
}

uint8_t syslinkGetRxCheckSum2ErrorCnt() {
  return m_syslinkRxCheckSum2ErrorCnt;
}

static void uart_evt_handler(nrf_drv_uart_event_t * p_event, void * p_context) {
  switch (p_event->type) {
    case NRF_DRV_UART_EVT_RX_DONE:
      uart_on_receive(p_event->data.rxtx.p_data, p_event->data.rxtx.bytes);
      break;
    case NRF_DRV_UART_EVT_TX_DONE:
      break;
    case NRF_DRV_UART_EVT_ERROR:
      // Relaunch the pump ...
      int mask = p_event->data.error.error_mask;
      NRF_LOG_ERROR("UART error: 0x%02X\n", mask);
      break;
    default:
      break;
  }
}

uint32_t syslinkInit() {
  uint32_t err_code;
  static nrf_drv_uart_config_t config = {
      .baudrate = UART_BAUDRATE_BAUDRATE_Baud1M,
      .hwfc = HWFC,
      .interrupt_priority = 1,
      .parity = NRF_UART_PARITY_EXCLUDED,
      .pseltxd = TX_PIN_NUMBER,
      .pselrxd = RX_PIN_NUMBER,
      .pselcts = CTS_PIN_NUMBER,
      .pselrts = RTS_PIN_NUMBER, 
  };

  err_code = nrf_drv_uart_init(&m_uart, &config, uart_evt_handler);
  VERIFY_SUCCESS(err_code);

  // Configure RX pin as input with pull-up, make sure we will not se a break condition if the STM32 is not powered
  nrf_gpio_cfg_input(config.pselrxd, NRF_GPIO_PIN_PULLUP);

  // Packet reception, start with receiving next header (2 start + type + length)
  // If we are not yet at the start of a packet, the receive state machine will synchronize itself ...
  nrf_drv_uart_rx_enable(&m_uart);
  nrf_drv_uart_rx(&m_uart, m_rx_buffer, 4);

  return NRF_SUCCESS;
}

bool syslinkReceive(struct syslinkPacket *packet) {
  if (m_received_packet_valid) {
    memcpy(packet, &m_received_packet, sizeof(struct syslinkPacket));
    m_received_packet_valid = false;

    // Restart packet reception, start with receiving next header (2 start + type + length)
    nrf_drv_uart_rx(&m_uart, m_rx_buffer, 4);

    return true;
  } else {
    return false;
  }
}

bool syslink_is_tx_busy() {
  return nrf_drv_uart_tx_in_progress(&m_uart) == true;
}
