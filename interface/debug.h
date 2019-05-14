/*
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

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef DEBUG_PRINT_ON_SEGGER_RTT
  #include "SEGGER_RTT.h"
#endif

void debugInit(void);

#if defined(DEBUG_PRINT_ON_SEGGER_RTT)
  #define DEBUG_PRINT(fmt, ...) SEGGER_RTT_printf(0, fmt, ## __VA_ARGS__)
  #define DEBUG_PRINT_OS(fmt, ...) SEGGER_RTT_printf(0, fmt, ## __VA_ARGS__)
#else // No debug
  #define DEBUG_PRINT(fmt, ...)
  #define DEBUG_PRINT_OS(fmt, ...)
#endif

#endif //__DEBUG_H__