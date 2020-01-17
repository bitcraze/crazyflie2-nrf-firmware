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
#ifdef BLE
#include <nrf_soc.h>
#endif

#include "pm.h"
#include "button.h"
#include "led.h"
#include "systick.h"
#include "uart.h"
#include "platform.h"

#define TICK_BETWEEN_STATE 2

//#define ENABLE_FAST_CHARGE_1A
//#define RFX2411N_BYPASS_MODE

static PmConfig const *pmConfig;
static PmState state;
static PmState targetState;
static bool systemBootloader=false;

typedef enum {adcVBAT, adcISET} ADCState;

static ADCState adcState = adcVBAT;

static float vBat;
static float iSet;
static float temp;

void pmInit()
{
  state = pmSysOff; //When NRF starts, the system is OFF
  targetState = state;

  /* PM-side IOs */
  nrf_gpio_cfg_output(PM_VCCEN_PIN);
  nrf_gpio_pin_clear(PM_VCCEN_PIN);

  nrf_gpio_cfg_input(PM_PGOOD_PIN, NRF_GPIO_PIN_PULLUP);
  //NRF_GPIO->PIN_CNF[PM_PGOOD_PIN] |= (GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos);

  nrf_gpio_cfg_input(PM_CHG_PIN, NRF_GPIO_PIN_PULLUP);

  pmConfig = platformGetPmConfig();
}

bool pmUSBPower(void) {
	return nrf_gpio_pin_read(PM_PGOOD_PIN) == 0;
}

bool pmIsCharging(void) {
  return nrf_gpio_pin_read(PM_CHG_PIN) == 0;
}

/*ChgState chgState(void) {

	int pgood = nrf_gpio_pin_read(PM_PGOOD_PIN);
	int chg = nrf_gpio_pin_read(PM_CHG_PIN);

	if (pgood)
}*/

static void pmStartAdc(ADCState state)
{
	if (state == adcVBAT) {
	    NRF_ADC->CONFIG = AIN_VBAT << ADC_CONFIG_PSEL_Pos |
	        ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos |
	        ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos |
	        pmConfig->adcPrescalingSetup << ADC_CONFIG_INPSEL_Pos;
	}
	if (state == adcISET) {
	    NRF_ADC->CONFIG = AIN_ISET << ADC_CONFIG_PSEL_Pos |
	        ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos |
	        ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos |
	        ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos;

	}
	adcState = state;
	NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;
  NRF_ADC->TASKS_START = 0x01;
}

static void pmNrfPower(bool enable)
{
  if (!enable) {
    //stop NRF
    LED_OFF();
    // Turn off PA
    if (platformHasRfx2411n()) {
      nrf_gpio_pin_clear(RADIO_PA_RX_EN);
      nrf_gpio_pin_clear(RADIO_PA_MODE);
      nrf_gpio_pin_clear(RADIO_PA_ANT_SW);
    } else {
      nrf_gpio_pin_clear(RADIO_PAEN_PIN);
    }
    // Disable 1-wire pull-up
    nrf_gpio_pin_clear(OW_PULLUP_PIN);
    // CE, EN1 and EN2 externally pulled low. Put low to not draw any current.
    nrf_gpio_pin_clear(PM_EN1);
    nrf_gpio_pin_clear(PM_EN2);
    nrf_gpio_pin_clear(PM_CHG_EN);

    nrf_gpio_cfg_input(PM_VBAT_SINK_PIN, NRF_GPIO_PIN_NOPULL);
    NRF_POWER->GPREGRET |= 0x01; // Workaround for not being able to determine reset reason...
#ifdef BLE
    sd_power_system_off();
#else
    NRF_POWER->SYSTEMOFF = 1UL;
#endif
    while(1);
  }
}

static void pmDummy(bool enable) {
  ;
}

static void pmPowerSystem(bool enable)
{
  if (enable) {
    NRF_GPIO->PIN_CNF[STM_NRST_PIN] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                                      | (GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos)
                                      | (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
                                      | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                                      | (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
    nrf_gpio_pin_clear(STM_NRST_PIN); //Hold STM reset
    nrf_gpio_pin_set(PM_VCCEN_PIN);
  } else {
    nrf_gpio_cfg_input(STM_NRST_PIN, NRF_GPIO_PIN_PULLDOWN);
    nrf_gpio_pin_clear(PM_VCCEN_PIN);
  }
}

static void pmSysBoot(bool enable)
{
  if (enable) {
    nrf_gpio_cfg_output(STM_BOOT0_PIN);

    if (systemBootloader) {
      nrf_gpio_pin_set(STM_BOOT0_PIN);
    } else {
      nrf_gpio_pin_clear(STM_BOOT0_PIN);
    }

    uartInit();
  } else {
    nrf_gpio_cfg_input(STM_BOOT0_PIN, NRF_GPIO_PIN_PULLDOWN);

    uartDeinit();
  }
}

static void pmRunSystem(bool enable)
{
  if (enable) {
    // Release the reset pin!
    nrf_gpio_pin_set(STM_NRST_PIN);

    //Activate UART TX pin
    nrf_gpio_cfg_output(UART_TX_PIN);
    nrf_gpio_pin_set(UART_TX_PIN);

    if (pmConfig->hasCharger) {
      nrf_gpio_cfg_output(PM_EN1);
      nrf_gpio_cfg_output(PM_EN2);
    #ifdef ENABLE_FAST_CHARGE_1A
      // 980mA current
      nrf_gpio_pin_clear(PM_EN1);
      nrf_gpio_pin_set(PM_EN2);
    #else
      // Set 500mA current
      nrf_gpio_pin_set(PM_EN1);
      nrf_gpio_pin_clear(PM_EN2);
    #endif
      // Enable charging
      nrf_gpio_cfg_output(PM_CHG_EN);
      nrf_gpio_pin_clear(PM_CHG_EN);
    }

    if (platformHasRfx2411n()) {
      // Enable RF power amplifier
      nrf_gpio_cfg_output(RADIO_PA_RX_EN);
      nrf_gpio_cfg_output(RADIO_PA_MODE);
      nrf_gpio_cfg_output(RADIO_PA_ANT_SW);
    #ifdef USE_EXT_ANTENNA
      // Select u.FL antenna
      nrf_gpio_pin_clear(RADIO_PA_ANT_SW);
    #else
      // Select chip antenna
      nrf_gpio_pin_set(RADIO_PA_ANT_SW);
    #endif

    #ifdef RFX2411N_BYPASS_MODE
        nrf_gpio_pin_set(RADIO_PA_MODE);
    #else
        nrf_gpio_pin_set(RADIO_PA_RX_EN);
        nrf_gpio_pin_clear(RADIO_PA_MODE);
    #endif
    } else {
        // Enable RF power amplifier
        nrf_gpio_cfg_output(RADIO_PAEN_PIN);

    #ifdef DISABLE_PA
        nrf_gpio_pin_clear(RADIO_PAEN_PIN);
        nrf_gpio_cfg_output(RADIO_PATX_DIS_PIN);
        nrf_gpio_pin_clear(RADIO_PATX_DIS_PIN);
    #else
        nrf_gpio_pin_set(RADIO_PAEN_PIN);
    #endif
    }

    if (pmConfig->hasVbatSink) {
      // Sink battery divider
      nrf_gpio_cfg_output(PM_VBAT_SINK_PIN);
      nrf_gpio_pin_clear(PM_VBAT_SINK_PIN);
    }

    pmStartAdc(adcVBAT);

  } else {
    //Disable UART
    nrf_gpio_cfg_input(UART_TX_PIN, NRF_GPIO_PIN_PULLDOWN);

    //Hold reset
    nrf_gpio_pin_clear(STM_NRST_PIN);
  }
}

/* User API to set and get power state */
void pmSetState(PmState newState)
{
  targetState = newState;
}

PmState pmGetState()
{
  return state;
}

float pmGetVBAT(void) {
	return vBat;
}

float pmGetISET(void) {
  return iSet;
}

float pmGetTemp(void) {
  return temp;
}

void pmSysBootloader(bool enable)
{
  systemBootloader = enable;
}

/* Defines all the power states for easy-usage by a generic state machine */
const struct {
  void (*call)(bool enable);
} statesFunctions[] = {
  [pmAllOff] =       { .call = pmDummy },
  [pmSysOff] =       { .call = pmNrfPower },
  [pmSysPowered] =   { .call = pmPowerSystem },
  [pmSysBootSetup] = { .call = pmSysBoot },
  [pmSysRunning] =   { .call = pmRunSystem },
};

void pmProcess() {
  static int lastTick = 0;
  static int lastAdcTick = 0;

  if (systickGetTick() - lastTick > TICK_BETWEEN_STATE) {
    lastTick = systickGetTick();

    if (targetState > state) {
      state++;
      if (statesFunctions[state].call) {
        statesFunctions[state].call(true);
      }
    } else if (targetState < state) {
      if (statesFunctions[state].call) {
        statesFunctions[state].call(false);
      }
      state--;
    }
  }

  // VBAT sampling can't be done to often or it will affect the reading, ~100Hz is OK.
  if (systickGetTick() - lastAdcTick > pmConfig->ticksBetweenAdcMeasurement && !NRF_ADC->BUSY)
  {
    uint16_t rawValue = NRF_ADC->RESULT;
    lastAdcTick = systickGetTick();

    // Start temp read
    NRF_TEMP->TASKS_START = 1;

	  if (adcState == adcVBAT) {
		  vBat = (float) (rawValue / 1023.0) * 1.2 * pmConfig->vbatFactor;
		  if (pmConfig->hasCharger) {
		    pmStartAdc(adcISET);
		  } else {
	      pmStartAdc(adcVBAT);
	    }
	  } else if (adcState == adcISET) {
		  // V_ISET = I_CHARGE / 400 × R_ISET
		  float v = (float) (rawValue / 1023.0) * 1.2 * 3;
		  iSet = (v * 400.0) / 1000.0;
		  pmStartAdc(adcVBAT);
	  }
	  //TODO: Handle the battery charging...
  }

  // Check that environmental temp is OK for charging
  if (NRF_TEMP->EVENTS_DATARDY)
  {
    temp = (float)(NRF_TEMP->TEMP / 4.0);
    if (temp < 15.0 || temp > 45.0)
    {
      // Disable charging
      nrf_gpio_pin_set(PM_CHG_EN);
      LED_OFF();
    }
    else
    {
      // Enable charging
      nrf_gpio_pin_clear(PM_CHG_EN);
      LED_ON();
    }
  }
}

