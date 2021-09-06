  /**
   *    ||          ____  _ __
   * +------+      / __ )(_) /_______________ _____  ___
   * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
   * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
   *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
   *
   * Crazyflie 2.1 NRF Firmware
   * Copyright (c) 2021, Bitcraze AB, All rights reserved.
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

#include "pm.h"
#include "systick.h"

#define TIMEOUT_TICKS 1000 // 100 ms

static enum {
  nothingTodo = 0,
  shutdownRequested,
  shutdownApproved,
} state;

static unsigned int requestSentTicks;

void shutdownSendRequest()
{
  struct syslinkPacket slTxPacket = {
    .type = SYSLINK_PM_SHUTDOWN_REQUEST,
  };

  if (state == shutdownRequested) {
    return;
  }

  syslinkSend(&slTxPacket);

  state = shutdownRequested;
  requestSentTicks = systickGetTick();
}

void shutdownReceivedAck()
{
  if (state != shutdownRequested) {
    return;
  }

  state = shutdownApproved;
}

void shutdownProcess()
{
  switch (state)
  {
    case nothingTodo: break;

    case shutdownApproved:
      pmSetState(pmAllOff);
      state = nothingTodo;
      break;

    case shutdownRequested:
      if ((systickGetTick() - requestSentTicks) > TIMEOUT_TICKS) {
        pmSetState(pmAllOff);
        state = nothingTodo;
      }
      break;
  }
}
