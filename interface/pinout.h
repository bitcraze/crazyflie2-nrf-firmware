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
#ifndef __PINOUT_H__
#define __PINOUT_H__

#ifndef DEBUG_UART
#define UART_RX_PIN 30
#define UART_TX_PIN 29
#define UART_RTS_PIN 14
#else
#define UART_RX_PIN 12
#define UART_TX_PIN 11
#endif

/* Rev. B prototype is patched to switch PM_CHG and PM_ISET
 * so that PM_ISET can be measured. For other prototypes PM_ISET
 * and VBAT will use the same AIN to measure.
 */
#ifndef REV_A
#define REV_B
#endif

#ifdef REV_A
#warning This is a REV_A build!
#endif

#ifdef REV_B
  #define PM_VCCEN_PIN 10
  #define BUTTON_PIN 17

  #define PM_VBAT_PIN 1
  #define PM_VBAT_SINK_PIN 2
  #define PM_PGOOD_PIN 4

  #define USE_OW_IO_PULLUP
  #define OW_PULLUP_PIN 9
  #define OW_PIN 8
  #define WKUP_PIN 11

  #define LED_PIN 13

  #define PM_CHG_PIN 0
  #define AIN_VBAT ADC_CONFIG_PSEL_AnalogInput2
  #define AIN_ISET ADC_CONFIG_PSEL_AnalogInput4
  #define AIN_VBAT_DIVIDER ADC_CONFIG_INPSEL_AnalogInputTwoThirdsPrescaling
  #define ADC_SCALER 3
  //#define ADC_DIVIDER 4.54545454
  #define ADC_DIVIDER (3.0/2.0)

  //#define NO_UART_FC
  //#define SYSLINK_CKSUM_MON
#else
  #define PM_VCCEN_PIN 9
  #define BUTTON_PIN 17

  #define PM_VBAT_PIN 1
  #define PM_VBAT_SINK_PIN 2
  #define PM_PGOOD_PIN 4

  #define USE_OW_IO_PULLUP
  #define OW_PULLUP_PIN 12
  #define OW_PIN 8
  #define WKUP_PIN 11

  #define LED_PIN 10

  #define PM_CHG_PIN 3
  #define AIN_VBAT ADC_CONFIG_PSEL_AnalogInput2
  #define AIN_ISET ADC_CONFIG_PSEL_AnalogInput2
  #define AIN_VBAT_DIVIDER ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling
  #define ADC_SCALER 3
  #define ADC_DIVIDER 3.0
#endif

// EN1/2 are switched for Rev. A
#define PM_EN1 5
#define PM_EN2 6
#define PM_CHG_EN 7

#define RADIO_PAEN_PIN 19
#define RADIO_PATX_DIS_PIN 20

#define STM_BOOT0_PIN 22
#define STM_NRST_PIN 21

/* SWD from nRF51 to STM32 using SPI */
#define STM_SWD_SDIO_MISO 24
#define STM_SWD_SDIO_MOSI 25
#define STM_SWD_CLK_CLK 28

#endif //__PINOUT_H__

