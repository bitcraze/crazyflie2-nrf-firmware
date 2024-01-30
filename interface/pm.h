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
#ifndef __PM_H__
#define __PM_H__
#include <stdbool.h>
#include <stdint.h>

typedef enum {pmAllOff=0, pmSysOff=1, pmSysPowered=2, pmSysBootSetup=3, pmSysRunning=4} PmState;

typedef enum {chgOff=0, chgCharging=1, chgCharged=2} ChgState;

void pmInit();

/* Return power flags that indicate if we're plugged in to USB, currently charging and if we can charge. */
uint8_t getPowerStatusFlags();

float pmGetVBAT(void);

float pmGetISET(void);

float pmGetTemp(void);

void pmProcess();

void pmSetState(PmState newState);

PmState pmGetState();

void pmSysBootloader(bool enable);

#endif //__PM_H__
