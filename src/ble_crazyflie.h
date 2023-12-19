#pragma once

#include "ble.h"
#include "ble_srv_common.h"
#include <stdint.h>
#include <stdbool.h>

#define BLE_CRAZYFLIE_SERVICE_UUID "00000201-1C7F-4F9E-947B-43B7C00A9A08"

typedef struct ble_crazyflie_s ble_crazyflie_t;

struct ble_crazyflie_s {
    uint8_t uuid_type;
    uint16_t service_handle;
    ble_gatts_char_handles_t crtp_char_handles;
};

typedef struct {
    ; // ToDo: Packet receive handler
} ble_crazyflie_init_t;

uint32_t ble_crazyflie_init(ble_crazyflie_t *p_crazyflie, const ble_crazyflie_init_t *p_crazyflie_init);

void ble_crazyflie_on_ble_evt(ble_crazyflie_t *p_crazyflie, ble_evt_t const *p_ble_evt);

// ToDo: packet send function