#include "sdk_common.h"
#include "ble_crazyflie.h"

#define CRAZYFLIE_BASE_UUID {{0x08, 0x9A, 0x0A, 0xC0, 0xB7, 0x43, 0x7B, 0x94, 0x9E, 0x4F, 0x7F, 0x1C, 0x00, 0x00, 0x00, 0x00}}


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
                                           &p_crazyflie->crtp_char_handles);

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
                                           &p_crazyflie->crtp_char_handles);
}

uint32_t ble_crazyflie_init(ble_crazyflie_t *p_crazyflie, const ble_crazyflie_init_t *p_crazyflie_init) {
    uint32_t err_code;
    ble_uuid_t ble_uuid;
    ble_uuid128_t crazyflie_base_uuid = CRAZYFLIE_BASE_UUID;

    VERIFY_PARAM_NOT_NULL(p_crazyflie);
    VERIFY_PARAM_NOT_NULL(p_crazyflie_init);

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

    return NRF_SUCCESS;
}

void ble_crazyflie_on_ble_evt(ble_crazyflie_t *p_crazyflie, ble_evt_t const *p_ble_evt);