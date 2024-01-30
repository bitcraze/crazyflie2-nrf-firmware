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

#pragma once

#include "ble.h"
#include "ble_srv_common.h"
#include <stdint.h>
#include <stdbool.h>

#define BLE_CRAZYFLIE_SERVICE_UUID "00000201-1C7F-4F9E-947B-43B7C00A9A08"

typedef struct ble_crazyflie_s ble_crazyflie_t;
typedef void (*ble_crazyflie_data_handler_t)(ble_crazyflie_t *p_crazyflie, uint8_t *p_data, uint16_t length);

struct ble_crazyflie_s {
    uint8_t uuid_type;
    uint16_t service_handle;
    uint16_t conn_handle; 
    ble_gatts_char_handles_t crtp_char_handles;
    ble_gatts_char_handles_t crtpup_char_handles;
    ble_gatts_char_handles_t crtpdown_char_handles;
    bool crtpdown_notification_enabled;
    uint8_t packet_buffer[32];
    uint8_t packet_length;
    uint8_t packet_index;
    ble_crazyflie_data_handler_t data_handler;

    uint8_t tx_pk_free;
};

typedef struct {
    ble_crazyflie_data_handler_t data_handler;
} ble_crazyflie_init_t;

uint32_t ble_crazyflie_init(ble_crazyflie_t *p_crazyflie, const ble_crazyflie_init_t *p_crazyflie_init);

void ble_crazyflie_on_ble_evt(ble_crazyflie_t *p_crazyflie, ble_evt_t const *p_ble_evt);

uint32_t ble_crazyflie_send_packet(ble_crazyflie_t *p_crazyflie, uint8_t *p_data, uint16_t length);
