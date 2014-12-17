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
#include <stdbool.h>

#include <nrf.h>
#include <nrf_gpio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pinout.h"
#include "swd.h"
#include "systick.h"

void swdInit(void)
{
  nrf_gpio_cfg_output(STM_SWD_CLK_CLK);
  nrf_gpio_cfg_output(STM_SWD_SDIO_MOSI);
  nrf_gpio_cfg_input(STM_SWD_SDIO_MISO, NRF_GPIO_PIN_NOPULL);

  NRF_SPI0->PSELSCK  = STM_SWD_CLK_CLK;
  NRF_SPI0->PSELMOSI = STM_SWD_SDIO_MOSI;
  NRF_SPI0->PSELMISO = STM_SWD_SDIO_MISO;

  NRF_SPI0->FREQUENCY = (uint32_t) 0x02000000;

  NRF_SPI0->CONFIG = (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos) | (SPI_CONFIG_ORDER_MsbFirst << SPI_CONFIG_ORDER_Pos);
  NRF_SPI0->EVENTS_READY = 0U;

  NRF_SPI0->ENABLE = (SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);
}

#define TIMEOUT_COUNTER          0x3000UL

uint8_t tx_data[35];
uint8_t rx_data[35];

bool swdTest(void) {
  // Start = 1
  // DP access = 0
  // Read access = 1
  // DP reg = 00 (IDCODE)
  // Parity = odd bits -> 1
  // Stop = 0
  // Park = 1 (not 0!)
  // 1 0 1 ?? (LSB) PARITY 0 0

  // Should read out (and does):0x2BA01477 (according to OpenOCD and http://www.st.com/web/en/resource/technical/document/reference_manual/DM00031020.pdf)
  //T ACK |------------DATA-----------------|P
  //? 100 1 11011100010 100000000 101110 10100

  uint8_t header = 0xA5;
  uint8_t idx = 0;
  tx_data[idx++] = 0xFF;
  tx_data[idx++] = 0xFF;
  tx_data[idx++] = 0xFF;
  tx_data[idx++] = 0xFF;
  tx_data[idx++] = 0xFF;
  tx_data[idx++] = 0xFF;
  tx_data[idx++] = 0xFF;
  tx_data[idx++] = 0xFF;
#if 0
  tx_data[idx++] = 0x9E;
  tx_data[idx++] = 0xE7;
#else
  tx_data[idx++] = 0x79;
  tx_data[idx++] = 0xE7;
#endif
  tx_data[idx++] = 0xFF;
  tx_data[idx++] = 0xFF;
  tx_data[idx++] = 0xFF;
  tx_data[idx++] = 0xFF;
  tx_data[idx++] = 0xFF;
  tx_data[idx++] = 0xFF;
  tx_data[idx++] = 0xFF;
  tx_data[idx++] = 0xFC;
  tx_data[idx++] = header;
  // Rest is 0x00

  uint32_t counter = 0;
  uint16_t number_of_txd_bytes = 0;
  uint32_t transfer_size = 35;

  while(number_of_txd_bytes < transfer_size)
  {
    NRF_SPI0->TXD = (uint32_t)(tx_data[number_of_txd_bytes]);

    while ((NRF_SPI0->EVENTS_READY == 0U) && (counter < TIMEOUT_COUNTER))
    {
      counter++;
    }

    if (counter == TIMEOUT_COUNTER)
    {
      return false;
    }
    else
    {   /* clear the event to be ready to receive next messages */
      NRF_SPI0->EVENTS_READY = 0U;
    }
    rx_data[number_of_txd_bytes] = (uint8_t)NRF_SPI0->RXD;
    number_of_txd_bytes++;
  }

  return true;
}
