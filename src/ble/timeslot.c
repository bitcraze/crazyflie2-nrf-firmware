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
 * Nordic BLE stack timeslot management code
 */
#include <stdlib.h>
#include <stdint.h>
#include "timeslot.h"

#include <nrf.h>
#include <nrf_soc.h>

#include "esb.h"

#define TIMESLOT_LEN_US 1000

static nrf_radio_request_t timeslot_request = {
  .request_type = NRF_RADIO_REQ_TYPE_EARLIEST,
  .params.earliest.hfclk = NRF_RADIO_HFCLK_CFG_DEFAULT,
  .params.earliest.priority = NRF_RADIO_PRIORITY_NORMAL,
  .params.earliest.length_us = TIMESLOT_LEN_US,
  .params.earliest.timeout_us = 1000000,
};

static nrf_radio_signal_callback_return_param_t * timeslot_callback(uint8_t signal_type)
{
  static nrf_radio_signal_callback_return_param_t return_param;

  return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;

  switch (signal_type)
  {
    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_START:
      // Set up rescheduling
      NRF_TIMER0->INTENSET = (1UL << TIMER_INTENSET_COMPARE0_Pos);
      NRF_TIMER0->CC[0]    = TIMESLOT_LEN_US - 800;
      NVIC_EnableIRQ(TIMER0_IRQn);

      esbInit();

#ifdef DEBUG_TIMESLOT
      NRF_GPIOTE->TASKS_OUT[0] = 1;
#endif

      return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;
      break;
    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_RADIO:
      esbInterruptHandler();

      //NRF_GPIOTE->TASKS_OUT[0] = 1;

      return_param.callback_action = NRF_RADIO_SIGNAL_CALLBACK_ACTION_NONE;
      break;
    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_TIMER0:
      if (NRF_TIMER0->EVENTS_COMPARE[0] == 1)
      {
          NRF_TIMER0->EVENTS_COMPARE[0] = 0;

          return_param.params.extend.length_us = TIMESLOT_LEN_US;
          return_param.callback_action         = NRF_RADIO_SIGNAL_CALLBACK_ACTION_EXTEND;
      }

      break;
    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_EXTEND_FAILED:

      esbDeinit();

#ifdef DEBUG_TIMESLOT
      NRF_GPIOTE->TASKS_OUT[0] = 1;
#endif

      return_param.params.request.p_next   = &timeslot_request;
      return_param.callback_action         = NRF_RADIO_SIGNAL_CALLBACK_ACTION_REQUEST_AND_END;
      break;
    case NRF_RADIO_CALLBACK_SIGNAL_TYPE_EXTEND_SUCCEEDED:
      NRF_TIMER0->CC[0]    += TIMESLOT_LEN_US;
      break;
    default:
      break;
  }

  return &return_param;
}

void timeslot_sd_evt_signal(uint32_t sys_evt)
{
  switch (sys_evt) {
    case NRF_EVT_RADIO_BLOCKED:
    case NRF_EVT_RADIO_CANCELED:
      sd_radio_request(&timeslot_request);
      break;
    default:
      break;
  }
}

void timeslot_start(void)
{
  sd_radio_session_open(timeslot_callback);
  sd_radio_request(&timeslot_request);
}
