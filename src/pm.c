/**
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie 2.0 NRF Firmware
 * Copyright (c) 2024, Bitcraze AB, All rights reserved.
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
#include <nrf_soc.h>
#include <nrf_gpiote.h>

#include "pm.h"
#include "button.h"
#include "led.h"
#include "systick.h"
#include "uart.h"
#include "platform.h"

/**
 * The max and min temp to charge. Since we are using nrf inbuilt temp sensor and
 * the board heat it the reading is at least 10-15 deg too warm.
 */
#define PM_CHARGE_MIN_TEMP  (15.0)
#define PM_CHARGE_MAX_TEMP  (60.0)
#define PM_CHARGE_HYSTERESIS (1.0)

//#define ENABLE_FAST_CHARGE_1A
//#define RFX2411N_BYPASS_MODE

extern int bleEnabled;

static PmConfig const *pmConfig;
static PmState state;
static PmState targetState;
static bool systemBootloader=false;

typedef struct {
  uint8_t isCharging   : 1;
  uint8_t usbPluggedIn : 1;
  uint8_t canCharge    : 1;
  uint8_t unused       : 5;
} power_flag_t;

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

uint8_t pmPGood() {
  return nrf_gpio_pin_read(PM_PGOOD_PIN);
}

uint8_t pmIsCharging() {
  return nrf_gpio_pin_read(PM_CHG_PIN);
}

uint8_t getPowerStatusFlags() {
  power_flag_t powerFlags;
  uint8_t isCharging = pmIsCharging();
  uint8_t pGood      = pmPGood();

  if (pmConfig->hasCharger) {
    // On the Crazyflie 'pGood' means valid input voltage from USB.
    powerFlags.isCharging   = !isCharging;  // Active LOW
    powerFlags.usbPluggedIn = !pGood;       // Active LOW
    powerFlags.canCharge    = 1;            //
  } else {
    // Bolt doesn't have a battery charger and the nRF can't detect if
    // USB is pluggged in.
    powerFlags.usbPluggedIn   = 0;      // We don't know
    powerFlags.isCharging     = 0;      // Not used
    powerFlags.canCharge      = 0;      // No charger on bolt
  }

  return *( (uint8_t*) &powerFlags );
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

    if (bleEnabled) {
#ifdef BLE
      sd_power_system_off();
#endif
    } else {
      NRF_POWER->SYSTEMOFF = 1UL;
    }

    while(1);
  } else {
    nrf_gpio_pin_clear(PM_VCCEN_PIN);
  }
}

static void pmDummy(bool enable) {
  ;
}

#if 0
#ifndef BLE
#define OUTPUT_PIN_NUMBER   23
static void enable8MHzHLCKtoSTM(void)
{
  // Configure OUTPUT_PIN_NUMBER as an output.
  nrf_gpio_cfg_output(OUTPUT_PIN_NUMBER);

  // Configure GPIOTE channel 0 to toggle the pin state
  nrf_gpiote_task_config(0, OUTPUT_PIN_NUMBER,
                         NRF_GPIOTE_POLARITY_TOGGLE,
                         NRF_GPIOTE_INITIAL_VALUE_LOW);

  // Configure PPI channel 2 to toggle OUTPUT_PIN on every TIMER1 COMPARE[0] match.
  NRF_PPI->CH[2].EEP = (uint32_t)&NRF_TIMER1->EVENTS_COMPARE[0];
  NRF_PPI->CH[2].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[0];

  // Enable PPI channel 2
  NRF_PPI->CHEN = (PPI_CHEN_CH2_Enabled << PPI_CHEN_CH2_Pos);

  // Configure timer 2
  NRF_TIMER1->TASKS_STOP = 1;
  NRF_TIMER1->TASKS_CLEAR = 1;
  NRF_TIMER1->MODE      = TIMER_MODE_MODE_Timer;
  NRF_TIMER1->BITMODE   = TIMER_BITMODE_BITMODE_16Bit << TIMER_BITMODE_BITMODE_Pos;
  NRF_TIMER1->PRESCALER = 0;
  NRF_TIMER1->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Msk;
  NRF_TIMER1->INTENSET = 0;
  NVIC_DisableIRQ(TIMER1_IRQn);
  // Load the initial values to TIMER1 CC registers to get 8Mhz.
  NRF_TIMER1->CC[0] = 1;
  NRF_TIMER1->TASKS_START = 1;
}
#endif
#endif

static void pmPowerSystem(bool enable)
{
  if (enable) {
#ifndef BLE
//    enable8MHzHLCKtoSTM();
#endif
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
    #if defined(USE_EXT_ANTENNA) && (USE_EXT_ANTENNA == 1)
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
  int delay;
  int delayUp;
} statesFunctions[] = {
  [pmAllOff] =       { .call = pmDummy ,      .delay = 2, .delayUp = 2},
  [pmSysOff] =       { .call = pmNrfPower,    .delay = 2, .delayUp = 350},
  [pmSysPowered] =   { .call = pmPowerSystem, .delay = 2, .delayUp = 2},
  [pmSysBootSetup] = { .call = pmSysBoot,     .delay = 2, .delayUp = 2},
  [pmSysRunning] =   { .call = pmRunSystem,   .delay = 2, .delayUp = 2},
};

void pmProcess() {
  static int lastTick = 0;
  static int lastAdcTick = 0;

  int delay = (targetState>state)?statesFunctions[state].delayUp:statesFunctions[state].delay;

  if (systickGetTick() - lastTick > delay) {
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

#ifndef DISABLE_CHARGE_TEMP_CONTROL
  // Check that environmental temp is OK for charging
  if (pmConfig->hasCharger && NRF_TEMP->EVENTS_DATARDY)
  {
    temp = (float)(NRF_TEMP->TEMP / 4.0);
    if (temp < PM_CHARGE_MIN_TEMP || temp > PM_CHARGE_MAX_TEMP)
    {
      // Disable charging
      nrf_gpio_pin_set(PM_CHG_EN);
//      LED_OFF();
    }
    else if (temp > PM_CHARGE_MIN_TEMP + PM_CHARGE_HYSTERESIS  &&
             temp < PM_CHARGE_MAX_TEMP - PM_CHARGE_HYSTERESIS)
    {
      // Enable charging
      nrf_gpio_pin_clear(PM_CHG_EN);
//      LED_ON();
    }
  }
#endif
}
