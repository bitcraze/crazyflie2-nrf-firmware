#include "sdk_common.h"
#include "ble_crazyflie.h"

#define NRF_LOG_MODULE_NAME "CRAZYFLIE_SRV"
#include "nrf_log.h"

#define CRAZYFLIE_BASE_UUID {{0x08, 0x9A, 0x0A, 0xC0, 0xB7, 0x43, 0x7B, 0x94, 0x9E, 0x4F, 0x7F, 0x1C, 0x00, 0x00, 0x00, 0x00}}


static void on_write(ble_crazyflie_t* p_crazyflie, ble_evt_t const* p_ble_evt) {
    ble_gatts_evt_write_t * p_evt_write = (void*) &p_ble_evt->evt.gatts_evt.params.write;

    if (p_evt_write->handle == p_crazyflie->crtp_char_handles.value_handle) {
        p_crazyflie->data_handler(p_crazyflie, p_evt_write->data, p_evt_write->len);
    } else if (p_evt_write->handle == p_crazyflie->crtpup_char_handles.value_handle) {
        if (p_evt_write->len > 0 && ((p_evt_write->data[0] & 0x80) == 0x80)) {
            p_crazyflie->packet_length = p_evt_write->data[0] & 0x1F;
            p_crazyflie->packet_index = p_evt_write->len - 1;
            memcpy(p_crazyflie->packet_buffer, p_evt_write->data+1, p_evt_write->len-1);
        } else {
            memcpy(p_crazyflie->packet_buffer+p_crazyflie->packet_index, p_evt_write->data+1, p_evt_write->len-1);
            p_crazyflie->data_handler(p_crazyflie, p_crazyflie->packet_buffer, p_crazyflie->packet_length);
        }
    } else if (p_evt_write->handle == p_crazyflie->crtpdown_char_handles.cccd_handle) {
        if (p_evt_write->len == 2) {
            if (ble_srv_is_notification_enabled(p_evt_write->data)) {
                p_crazyflie->crtpdown_notification_enabled = true;
            } else {
                p_crazyflie->crtpdown_notification_enabled = false;
            }
        }
    }
}

static void on_connect(ble_crazyflie_t* p_crazyflie, ble_evt_t const* p_ble_evt) {
    p_crazyflie->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

static void on_disconnect(ble_crazyflie_t* p_crazyflie, ble_evt_t const* p_ble_evt) {
    UNUSED_PARAMETER(p_ble_evt);
    p_crazyflie->conn_handle = BLE_CONN_HANDLE_INVALID;
}

static uint32_t crtp_char_add(ble_crazyflie_t* p_crazyflie, const ble_crazyflie_init_t* p_crazyflie_init) {
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t attr_char_value;
    ble_uuid_t ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read = 1;
    char_md.char_props.notify = 1;
    char_md.char_props.write = 1;
    char_md.char_props.write_wo_resp = 1;
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf = NULL;
    char_md.p_user_desc_md = NULL;
    char_md.p_cccd_md = &cccd_md;
    char_md.p_sccd_md = NULL;

    ble_uuid.type = p_crazyflie->uuid_type;
    ble_uuid.uuid = 0x0202;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len = 20;

    return sd_ble_gatts_characteristic_add(p_crazyflie->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_crazyflie->crtp_char_handles);

}

static uint32_t crtpup_char_add(ble_crazyflie_t* p_crazyflie, const ble_crazyflie_init_t* p_crazyflie_init) {
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t attr_char_value;
    ble_uuid_t ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write = 1;
    char_md.char_props.write_wo_resp = 1;
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf = NULL;
    char_md.p_user_desc_md = NULL;
    char_md.p_cccd_md = &cccd_md;
    char_md.p_sccd_md = NULL;

    ble_uuid.type = p_crazyflie->uuid_type;
    ble_uuid.uuid = 0x0203;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len = 20;

    return sd_ble_gatts_characteristic_add(p_crazyflie->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_crazyflie->crtpup_char_handles);

}

uint32_t crtpdown_char_add(ble_crazyflie_t* p_crazyflie, const ble_crazyflie_init_t* p_crazyflie_init) {
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t attr_char_value;
    ble_uuid_t ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify = 1;
    char_md.char_props.read = 1;
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf = NULL;
    char_md.p_user_desc_md = NULL;
    char_md.p_cccd_md = &cccd_md;
    char_md.p_sccd_md = NULL;

    ble_uuid.type = p_crazyflie->uuid_type;
    ble_uuid.uuid = 0x0204;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len = 0;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len = 20;

    return sd_ble_gatts_characteristic_add(p_crazyflie->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_crazyflie->crtpdown_char_handles);
}

uint32_t ble_crazyflie_init(ble_crazyflie_t *p_crazyflie, const ble_crazyflie_init_t *p_crazyflie_init) {
    uint32_t err_code;
    ble_uuid_t ble_uuid;
    ble_uuid128_t crazyflie_base_uuid = CRAZYFLIE_BASE_UUID;

    VERIFY_PARAM_NOT_NULL(p_crazyflie);
    VERIFY_PARAM_NOT_NULL(p_crazyflie_init);
    VERIFY_PARAM_NOT_NULL(p_crazyflie_init->data_handler);

    // Add Crazyflie serive UUID
    err_code = sd_ble_uuid_vs_add(&crazyflie_base_uuid, &p_crazyflie->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_crazyflie->uuid_type;
    ble_uuid.uuid = 0x0201;

    // Add Crazyflie service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, 
                                        &ble_uuid,
                                        &p_crazyflie->service_handle);
    VERIFY_SUCCESS(err_code);

    err_code = crtp_char_add(p_crazyflie, p_crazyflie_init);
    VERIFY_SUCCESS(err_code);

    err_code = crtpup_char_add(p_crazyflie, p_crazyflie_init);
    VERIFY_SUCCESS(err_code);

    err_code = crtpdown_char_add(p_crazyflie, p_crazyflie_init);
    VERIFY_SUCCESS(err_code);

    p_crazyflie->conn_handle = BLE_CONN_HANDLE_INVALID;
    p_crazyflie->data_handler = p_crazyflie_init->data_handler;
    p_crazyflie->crtpdown_notification_enabled = false;

    return NRF_SUCCESS;
}

void ble_crazyflie_on_ble_evt(ble_crazyflie_t *p_crazyflie, ble_evt_t const *p_ble_evt) {
    if (p_crazyflie == NULL || p_ble_evt == NULL) {
        return;
    }

    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_crazyflie, p_ble_evt);
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_crazyflie, p_ble_evt);
            break;
        case BLE_GATTS_EVT_WRITE:
            on_write(p_crazyflie, p_ble_evt);
            break;
        default:
            break;
    }
}

static uint32_t send_crtpdown_notification(ble_crazyflie_t *p_crazyflie, uint8_t *p_data, uint16_t length) {
    uint32_t err_code;
    ble_gatts_hvx_params_t hvx_params;

    VERIFY_PARAM_NOT_NULL(p_crazyflie);

    if ((p_crazyflie->conn_handle == BLE_CONN_HANDLE_INVALID) || (!p_crazyflie->crtpdown_notification_enabled)) {
        return NRF_ERROR_INVALID_STATE;
    }

    if (p_crazyflie->crtpdown_char_handles.value_handle == BLE_GATT_HANDLE_INVALID) {
        return NRF_ERROR_INVALID_STATE;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_crazyflie->crtpdown_char_handles.value_handle;
    hvx_params.type = BLE_GATT_HVX_NOTIFICATION;
    hvx_params.offset = 0;
    hvx_params.p_len = &length;
    hvx_params.p_data = p_data;

    err_code = sd_ble_gatts_hvx(p_crazyflie->conn_handle, &hvx_params);
    if (err_code == NRF_SUCCESS) {
        NRF_LOG_INFO("Sent %d bytes\n", length);
    } else {
        NRF_LOG_ERROR("Failed to send %d bytes: %d\n", length, err_code);
    }

    return err_code;
}

uint32_t ble_crazyflie_send_packet(ble_crazyflie_t *p_crazyflie, uint8_t *p_data, uint16_t length) {
    uint32_t err_code;
    uint8_t packet[32];

    if (length > 31) {
        return NRF_ERROR_INVALID_PARAM;
    }

    packet[0] = 0x80 | length;

    if (length > GATT_MTU_SIZE_DEFAULT - 3 - 1) {
        memcpy(packet+1, p_data, GATT_MTU_SIZE_DEFAULT - 3 - 1);
        err_code = send_crtpdown_notification(p_crazyflie, packet, GATT_MTU_SIZE_DEFAULT - 3 - 1 + 1);
        VERIFY_SUCCESS(err_code);
        memcpy(packet+1, p_data+GATT_MTU_SIZE_DEFAULT - 3, length - (GATT_MTU_SIZE_DEFAULT - 3 - 1));
        err_code = send_crtpdown_notification(p_crazyflie, packet, length - (GATT_MTU_SIZE_DEFAULT - 3 - 1) + 1);
        VERIFY_SUCCESS(err_code);
    } else {
        memcpy(packet+1, p_data, length);
        err_code = send_crtpdown_notification(p_crazyflie, packet, length+1);
        VERIFY_SUCCESS(err_code);
    }

    return NRF_SUCCESS;
}