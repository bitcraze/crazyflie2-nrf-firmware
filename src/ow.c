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
#include <errno.h>

#include <nrf.h>
#include <nrf_gpio.h>

#include "pinout.h"
#include "ow.h"
#include "systick.h"


#define USE_OW_IO_PULLUP

void owInit(void)
{
#ifdef USE_OW_IO_PULLUP
  NRF_GPIO->PIN_CNF[OW_PULLUP_PIN] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                                    | (GPIO_PIN_CNF_DRIVE_S0H1 << GPIO_PIN_CNF_DRIVE_Pos)
                                    | (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
                                    | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                                    | (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
  nrf_gpio_pin_set(OW_PULLUP_PIN);
  NRF_GPIO->PIN_CNF[OW_PIN] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                                      | (GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos)
                                      | (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
                                      | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                                      | (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
  nrf_gpio_pin_set(OW_PIN);
#else
  NRF_GPIO->PIN_CNF[OW_PIN] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                                      | (GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos)
                                      | (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos)
                                      | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                                      | (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);

#endif

  // oflnk.c expect:
  // CC0 sets OWPIN to 0
  // CC1 sets OWPIN to 1
  // CC2 Sampling of the GPIO
  // CC3 stops the timer and clear a flag (interrup routine defined in owlnk.c)

  NRF_TIMER1->PRESCALER = 0;
/*
  //Set OW to 0
  NRF_GPIOTE->CONFIG[0] =   (GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos)
                          | (OW_PIN << GPIOTE_CONFIG_PSEL_Pos)
                          | (GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos)
                          | (GPIOTE_CONFIG_OUTINIT_High << GPIOTE_CONFIG_OUTINIT_Pos);
  //Connected to Timer1 CC0
  NRF_PPI->CH[0].EEP = (int)&NRF_TIMER1->EVENTS_COMPARE[0];
  NRF_PPI->CH[0].TEP = (int)&NRF_GPIOTE->TASKS_OUT[0];
  NRF_PPI->CHENSET = PPI_CHENSET_CH0_Msk;

  //Set OW to 1
  NRF_GPIOTE->CONFIG[1] =   (GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos)
                          | (OW_PIN << GPIOTE_CONFIG_PSEL_Pos)
                          | (GPIOTE_CONFIG_POLARITY_LoToHi << GPIOTE_CONFIG_POLARITY_Pos)
                          | (GPIOTE_CONFIG_OUTINIT_High << GPIOTE_CONFIG_OUTINIT_Pos);
  //Connected to Timer1 CC1
  NRF_PPI->CH[1].EEP = (int)&NRF_TIMER1->EVENTS_COMPARE[1];
  NRF_PPI->CH[1].TEP = (int)&NRF_GPIOTE->TASKS_OUT[1];
  NRF_PPI->CHENSET = PPI_CHENSET_CH1_Msk;

  //Force the pin High
  NRF_GPIOTE->TASKS_OUT[1] = 1;
*/

  // Enable Interrupt for compare 2 and 3
  NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE0_Msk | TIMER_INTENSET_COMPARE1_Msk | TIMER_INTENSET_COMPARE2_Msk | TIMER_INTENSET_COMPARE3_Msk;
  NVIC_SetPriority(TIMER1_IRQn, 1);
  NVIC_EnableIRQ(TIMER1_IRQn);

  //Compare 3 shorts to stop
  NRF_TIMER1->SHORTS = TIMER_SHORTS_COMPARE3_STOP_Msk | TIMER_SHORTS_COMPARE3_CLEAR_Msk;
}

void owRaiseError(int error)
{
  errno = error;
}
