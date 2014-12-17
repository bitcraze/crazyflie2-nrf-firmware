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
 * 1ms systick timer
 */
#include <nrf.h>

static unsigned int tick = 0;

//void RTC1_IRQHandler()
void TIMER2_IRQHandler()
{
  tick++;
  //NRF_RTC1->EVENTS_TICK = 0;
  NRF_TIMER2->EVENTS_COMPARE[0] = 0;
}

void systickInit()
{
//  NRF_RTC1->PRESCALER = 327; //100Hz systick
//  NRF_RTC1->EVTENSET = RTC_EVTENSET_TICK_Msk;
//  NRF_RTC1->INTENSET = RTC_INTENSET_TICK_Msk;
//  NVIC_EnableIRQ(RTC1_IRQn);

//  NRF_RTC1->TASKS_START = 1UL;

  NRF_TIMER2->TASKS_CLEAR = 1;

  NRF_TIMER2->PRESCALER = 4;
  NRF_TIMER2->CC[0] = 1000;
  NRF_TIMER2->SHORTS = 1UL<<TIMER_SHORTS_COMPARE0_CLEAR_Pos;
  NRF_TIMER2->INTENSET = (1UL << TIMER_INTENSET_COMPARE0_Pos);
  NVIC_SetPriority(TIMER2_IRQn, 3);
  NVIC_EnableIRQ(TIMER2_IRQn);

  NRF_TIMER2->TASKS_START = 1;
}

unsigned int systickGetTick()
{
  return tick;
}

