#pragma once

#include "nrf.h"

#define LEDS_NUMBER    1

#define LED_START      13
#define LED_1          13

#define LEDS_ACTIVE_STATE 1

#define LEDS_LIST { LED_1 }

#define BUTTONS_NUMBER 1

#define BUTTON_START   17
#define BUTTON_1       17
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1 }

#define RX_PIN_NUMBER  39
#define TX_PIN_NUMBER  29
#define CTS_PIN_NUMBER NRF_UART_PSEL_DISCONNECTED
#define RTS_PIN_NUMBER 14
#define HWFC           NRF_UART_HWFC_ENABLED

// #define BSP_BUTTON_0   BUTTON_1

#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_SYNTH,            \
                                 .rc_ctiv       = 0,                                \
                                 .rc_temp_ctiv  = 0,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}
