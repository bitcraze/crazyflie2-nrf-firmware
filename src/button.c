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

#include "pinout.h"
#include "button.h"
#include "systick.h"

static ButtonEvent state;

void buttonInit(ButtonEvent initialEvent)
{
  nrf_gpio_cfg_input(BUTTON_PIN, NRF_GPIO_PIN_PULLUP);
  NRF_GPIO->PIN_CNF[BUTTON_PIN] |= (GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos);

  state = initialEvent;
}

void buttonProcess()
{
  static unsigned int lastTick;
  static unsigned int pressedTick;
  static bool pressed;

  if (lastTick != systickGetTick())
  {
    lastTick = systickGetTick();

    if (pressed==false && BUTTON_READ()==BUTTON_PRESSED)
    {
      pressed = true;
      pressedTick = systickGetTick();
    } else if (pressed==true && BUTTON_READ()==BUTTON_RELEASED) {
      pressed = false;
      if ((systickGetTick()-pressedTick)<BUTTON_LONGPRESS_TICK) {
        state = buttonShortPress;
      } else {
        state = buttonLongPress;
      }
    }
  }
}

ButtonEvent buttonGetState()
{
  ButtonEvent currentState = state;

  state = buttonIdle;

  return currentState;
}



