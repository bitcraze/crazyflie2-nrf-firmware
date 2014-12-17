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
 *
 * Bluetooth LE Crazyflie service
 *
 * Bitcraze UUID base: 00000000-1c7f-4f9e-947b-43b7c00a9a08
 */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ble.h>

#include "bitcraze_uuids.h"

#include "esb.h"
#include "ble_crazyflies.h"


static ble_gatts_char_handles_t crtp_handle;
static ble_gatts_char_handles_t crtpup_handle;
static ble_gatts_char_handles_t crtpdown_handle;
static uint16_t mConnHandle = 0xffffu;

static ble_gatts_char_md_t crtp_md = {
  .char_props = { .read = 1, .write = 1, .notify = 1 },
  //.char_ext_props = {},
  //.p_char_user_desc = (uint8_t *) "Led 1",
  //.char_user_desc_max_size = 32,
  //.char_user_desc_size = 5,

  .p_char_pf = NULL,
  .p_sccd_md = NULL,
};

static ble_gatts_char_md_t crtpup_md = {
  .char_props = { .write = 1, .write_wo_resp = 1 },

  .p_char_pf = NULL,
  .p_sccd_md = NULL,
};

static ble_gatts_char_md_t crtpdown_md = {
  .char_props = { .read = 1, .notify = 1 },

  .p_char_pf = NULL,
  .p_sccd_md = NULL,
};

static ble_gatts_attr_md_t crtp_attr_md = {
  .read_perm = {.sm = 1, .lv = 1},
  .write_perm = {.sm = 1, .lv = 1},
  .vloc = BLE_GATTS_VLOC_STACK,
  .vlen = 1,
};

static ble_gatts_attr_t crtp_attr = {
  .p_attr_md = &crtp_attr_md,
  .init_len = 1,
  .init_offs = 0,
  .max_len = 32,
};

static ble_gatts_attr_md_t crtpupdown_attr_md = {
  .read_perm = {.sm = 1, .lv = 1},
  .write_perm = {.sm = 1, .lv = 1},
  .vloc = BLE_GATTS_VLOC_STACK,
  .vlen = 1,
};

static ble_gatts_attr_t crtpupdown_attr = {
  .p_attr_md = &crtpupdown_attr_md,
  .init_len = 1,
  .init_offs = 0,
  .max_len = 32,
};



static bool crtpPacketReceived = false;
static EsbPacket rxPacket;

#define ERROR_CHECK(E) if (E != NRF_SUCCESS) { return err; }

void ble_crazyflies_on_ble_evt(ble_evt_t * p_ble_evt)
{
  ble_gatts_evt_write_t *p_write;

  switch(p_ble_evt->header.evt_id) {
    case BLE_GAP_EVT_CONNECTED:
      mConnHandle = p_ble_evt->evt.gap_evt.conn_handle;
      break;
    case BLE_GAP_EVT_DISCONNECTED:
      mConnHandle = 0xffffu;
      break;
    case BLE_GATTS_EVT_WRITE:
      p_write = &p_ble_evt->evt.gatts_evt.params.write;
      if (p_write->handle == crtp_handle.value_handle) {
        if (!crtpPacketReceived) {
          memcpy(rxPacket.data, p_write->data, p_write->len);
          rxPacket.size = p_write->len;
          crtpPacketReceived = true;
        }
      }
      if (p_write->handle == crtpup_handle.value_handle) {
        static unsigned char pkdata[32];
        static int length;
        static unsigned char pid = 0xff;
        bool received = false;

        if ((p_write->data[0] & 0x80) == 0x80) {
          pid = p_write->data[0] & 0x60;
          length = (p_write->data[0]&0x1f) + 1;

          if (length>19) {
            memcpy(pkdata, p_write->data+1, 19);
            received = false;
          } else {
            memcpy(pkdata, p_write->data+1, length);
            received = true;
          }
        } else if ((p_write->data[0]&0x60) == pid) {
          pid = 0xff;
          memcpy(pkdata+19, p_write->data+1, length-19);
          received = true;
        }

        if(received && !crtpPacketReceived) {
          memcpy(rxPacket.data, pkdata, length);
          rxPacket.size = length;
          crtpPacketReceived = true;
        }
      }
      break;
    default:
      break;
  }
}


uint32_t ble_crazyflies_init(uint8_t uuidType)
{
  uint32_t err;
  ble_uuid_t  service_uuid;
  ble_uuid_t  crtp_uuid;
  uint16_t service_handle;
  uint8_t initial_value = 0xFF;
  ble_gatts_attr_md_t cccd_md;

  service_uuid.uuid = UUID_CRAZYFLIE_SERVICE;
  service_uuid.type = uuidType;

  err = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                 &service_uuid,
                                 &service_handle);
  ERROR_CHECK(err);

  /* Bidirectional full-length CRTP characteristic  */
  memset(&cccd_md, 0, sizeof(cccd_md));

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
  cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

  crtp_md.p_cccd_md  = &cccd_md;


  crtp_uuid.type = uuidType;
  crtp_uuid.uuid = UUID_CRAZYFLIE_CRTP;

  crtp_attr.p_uuid = &crtp_uuid;
  crtp_attr.p_value = &initial_value;
  crtp_attr.init_len = 1;

  err = sd_ble_gatts_characteristic_add(service_handle,
                                        &crtp_md,
                                        &crtp_attr,
                                        &crtp_handle);

  ERROR_CHECK(err);
  /* Uplink (nrf receives) segmented CRTP characteristic */
  memset(&cccd_md, 0, sizeof(cccd_md));

  //BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
  //BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
  //cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

  crtp_md.p_cccd_md  = NULL; //&cccd_md;


  crtp_uuid.type = uuidType;
  crtp_uuid.uuid = UUID_CRAZYFLIE_CRTP_UP;

  crtpupdown_attr.p_uuid = &crtp_uuid;
  crtpupdown_attr.p_value = &initial_value;
  crtpupdown_attr.init_len = 1;

  err = sd_ble_gatts_characteristic_add(service_handle,
                                        &crtpup_md,
                                        &crtpupdown_attr,
                                        &crtpup_handle);

  ERROR_CHECK(err);
  /* Downlink (nrf Sends) segmented CRTP characteristic */
  memset(&cccd_md, 0, sizeof(cccd_md));

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
  cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

  crtpdown_md.p_cccd_md  = &cccd_md;


  crtp_uuid.type = uuidType;
  crtp_uuid.uuid = UUID_CRAZYFLIE_CRTP_DOWN;

  crtpupdown_attr.p_uuid = &crtp_uuid;
  crtpupdown_attr.p_value = &initial_value;
  crtpupdown_attr.init_len = 1;

  err = sd_ble_gatts_characteristic_add(service_handle,
                                        &crtpdown_md,
                                        &crtpupdown_attr,
                                        &crtpdown_handle);

  ERROR_CHECK(err);

  return NRF_SUCCESS;
}

bool bleCrazyfliesIsPacketReceived()
{
  return crtpPacketReceived;
}

EsbPacket* bleCrazyfliesGetRxPacket()
{
  return &rxPacket;
}


void bleCrazyfliesReleaseRxPacket(EsbPacket* packet)
{
  crtpPacketReceived = false;
}

void bleCrazyfliesSendPacket(EsbPacket* packet)
{
  ble_gatts_hvx_params_t params;
  uint16_t len = packet->size;
  static unsigned char buffer[20];
  static int pid = 0;

  if (mConnHandle == 0xffffu)
    return;


  if (len>20)
    len = 20;

  memset(&params, 0, sizeof(params));
  params.type = BLE_GATT_HVX_NOTIFICATION;
  params.handle = crtp_handle.value_handle;
  params.p_data = packet->data;
  params.p_len = &len;

  sd_ble_gatts_hvx(mConnHandle, &params);

  // Send segmented packet
  len = (packet->size>19)?20:packet->size+1;

  buffer[0] = 0x80u | ((pid<<5)&0x60) | ((packet->size-1) & 0x1f);
  memcpy(&buffer[1], packet->data, len);

  memset(&params, 0, sizeof(params));
  params.type = BLE_GATT_HVX_NOTIFICATION;
  params.handle = crtpdown_handle.value_handle;
  params.p_data = buffer;
  params.p_len = &len;
  sd_ble_gatts_hvx(mConnHandle, &params);

  if (packet->size > 19) {
    len = (packet->size-19)+1;

    // Continuation contains only the PID
    buffer[0] = ((pid<<5)&0x60);
    memcpy(&buffer[1], &packet->data[19], len);

    memset(&params, 0, sizeof(params));
    params.type = BLE_GATT_HVX_NOTIFICATION;
    params.handle = crtpdown_handle.value_handle;
    params.p_data = buffer;
    params.p_len = &len;
    sd_ble_gatts_hvx(mConnHandle, &params);
  }

  pid++;
}
