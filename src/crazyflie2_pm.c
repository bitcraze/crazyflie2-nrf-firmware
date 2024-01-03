#include "crazyflie2_pm.h"

#include "nrf.h"
#include "nrf_gpio.h"
#include "boards.h"

static pm_state_t m_state = PM_STATE_OFF;

void crazyflie2_pm_init(void) {
    // Initialize GPIO pins
    nrf_gpio_cfg(PM_CHG, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_cfg(PM_VBAT, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_cfg(PM_VBAT_SYNC, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_cfg(PM_ISET, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_cfg(PM_PGOOD, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_cfg(PM_EN1, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_cfg(PM_EN2, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_cfg(PM_CHG_EN, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_cfg(VEN_D, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);

    // Set default charge state
    nrf_gpio_pin_clear(PM_EN1);
    nrf_gpio_pin_clear(PM_EN2);
    nrf_gpio_pin_clear(PM_CHG_EN);

    // Setup battery and charge current measurement
    nrf_gpio_pin_clear(PM_VBAT_SYNC);
    // todo: setup analog measurement

    crazyflie2_pm_set_state(PM_STATE_SYSTEM_OFF);
}

pm_state_t crazyflie2_pm_get_state(void) {
    return m_state;
}

void crazyflie2_pm_set_state(pm_state_t state) {
    m_state = state;

    switch (state) {
        case PM_STATE_OFF:
            nrf_gpio_pin_clear(VEN_D);
            // Todo: switch OFF nRF51 enabling wakeup by button
            break;
        case PM_STATE_SYSTEM_OFF:
            nrf_gpio_pin_clear(VEN_D);
            break;
        case PM_STATE_SYSTEM_ON:
            nrf_gpio_pin_set(VEN_D);
            break;
    }
    // Todo: set GPIOs for requested state
}