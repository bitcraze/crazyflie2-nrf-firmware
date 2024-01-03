#pragma once

typedef enum {
    PM_STATE_OFF,
    PM_STATE_SYSTEM_OFF,
    PM_STATE_SYSTEM_ON,
} pm_state_t;

/**
 * Initialize the power management module.
 */
void crazyflie2_pm_init(void);

/**
 * Get the current power management state.
 */
pm_state_t crazyflie2_pm_get_state(void);

/**
 * Set the current power management state.
 */
void crazyflie2_pm_set_state(pm_state_t state);
