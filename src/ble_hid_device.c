#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "btstack_config.h"
#include "btstack.h"
#include "ble/gatt-service/hids_device.h"
#include "ble/gatt-service/battery_service_server.h"
#include "ble/gatt-service/device_information_service_server.h"

#include "descriptor/XboxDescriptors.h"
#include "ble_hid_device.h"
#include <ble_hid.h>

static hci_con_handle_t le_connection_handle = HCI_CON_HANDLE_INVALID;
static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_packet_callback_registration_t l2cap_event_callback_registration;
static btstack_packet_callback_registration_t sm_event_callback_registration;

#define BLE_CONN_INTERVAL_MIN 6   // 7.5 ms (units of 1.25 ms)
#define BLE_CONN_INTERVAL_MAX 6   // request fixed 7.5 ms
#define BLE_CONN_LATENCY      0   // no skipped events
#define BLE_SUPERVISION_TO    200 // 2.0 s (units of 10 ms)

// Advertisement data
static const uint8_t adv_data[] = {
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,
    0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE & 0xff, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE >> 8,
    0x03, BLUETOOTH_DATA_TYPE_APPEARANCE, 0xC3, 0x03,
    0x11, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME,
    't', 'i', 't', 'k', 'o', 's', ' ', 'm', 'i', 'k', 'k', 'e', 'n', 't', 'y', 'u'
};
static const uint8_t adv_data_len = sizeof(adv_data);

// Packet handler for HCI, SM, L2CAP, and HIDS events
static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    (void) channel;
    (void) size;

    if (packet_type != HCI_EVENT_PACKET) return;

    uint8_t event_type = hci_event_packet_get_type(packet);

    switch (event_type) {
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            le_connection_handle = HCI_CON_HANDLE_INVALID;
            printf("Disconnected\n");
            break;

        case SM_EVENT_JUST_WORKS_REQUEST:
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            break;

        case SM_EVENT_NUMERIC_COMPARISON_REQUEST:
            sm_numeric_comparison_confirm(sm_event_numeric_comparison_request_get_handle(packet));
            break;

        case HCI_EVENT_LE_META:
            switch (hci_event_le_meta_get_subevent_code(packet)) {
                case HCI_SUBEVENT_LE_CONNECTION_COMPLETE: {
                    uint8_t status = hci_subevent_le_connection_complete_get_status(packet);
                    if (status == 0) {
                        le_connection_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                        printf("Connected, handle: 0x%04x\n", le_connection_handle);
                        gap_request_connection_parameter_update(le_connection_handle,
                                                                BLE_CONN_INTERVAL_MIN,
                                                                BLE_CONN_INTERVAL_MAX,
                                                                BLE_CONN_LATENCY,
                                                                BLE_SUPERVISION_TO);
                    } else {
                        printf("Connection failed, status: 0x%02x\n", status);
                    }
                    break;
                }
                case HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE: {
                    uint16_t interval_units = hci_subevent_le_connection_update_complete_get_conn_interval(packet);
                    uint16_t latency = hci_subevent_le_connection_update_complete_get_conn_latency(packet);
                    printf("BLE conn params: interval=%.2f ms latency=%u\n", interval_units * 1.25f, latency);
                    break;
                }
                default:
                    break;
            }
            break;

        case HCI_EVENT_HIDS_META:
            switch (hci_event_hids_meta_get_subevent_code(packet)) {
                case HIDS_SUBEVENT_INPUT_REPORT_ENABLE:
                    le_connection_handle = hids_subevent_input_report_enable_get_con_handle(packet);
                    printf("HID Input Report %s\n",
                           hids_subevent_input_report_enable_get_enable(packet) ? "enabled" : "disabled");
                    break;
                case HIDS_SUBEVENT_PROTOCOL_MODE:
                    printf("HID Protocol: %s\n",
                        hids_subevent_protocol_mode_get_protocol_mode(packet) ? "Report" : "Boot");
                    break;
                case HIDS_SUBEVENT_SET_REPORT:
                    break;
                case HIDS_SUBEVENT_OUTPUT_REPORT_ENABLE:
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }
}

// Pack XboxReport into 16-byte Xbox Series X HID input report format
static void pack_xbox_report(const XboxReport *report, uint8_t *out) {
    // Left stick X, Y (16-bit LE unsigned)
    out[0] = report->left_stick_x & 0xFF;
    out[1] = (report->left_stick_x >> 8) & 0xFF;
    out[2] = report->left_stick_y & 0xFF;
    out[3] = (report->left_stick_y >> 8) & 0xFF;

    // Right stick X, Y (16-bit LE unsigned)
    out[4] = report->right_stick_x & 0xFF;
    out[5] = (report->right_stick_x >> 8) & 0xFF;
    out[6] = report->right_stick_y & 0xFF;
    out[7] = (report->right_stick_y >> 8) & 0xFF;

    // Left trigger (Brake) - 10-bit in 16-bit LE field (0-1023)
    out[8] = report->left_trigger & 0xFF;
    out[9] = (report->left_trigger >> 8) & 0x03;

    // Right trigger (Accelerator) - 10-bit in 16-bit LE field (0-1023)
    out[10] = report->right_trigger & 0xFF;
    out[11] = (report->right_trigger >> 8) & 0x03;

    // Hat switch (4-bit) + padding
    out[12] = report->hat & 0x0F;

    // Buttons 1-15 (15 bits) + 1-bit padding
    uint16_t btns = report->buttons & 0x7FFF;
    out[13] = btns & 0xFF;
    out[14] = (btns >> 8) & 0x7F;

    // Record button (1 bit) + 7-bit padding
    out[15] = 0;
}

// Send HID input report to connected host
void ble_send_hid_report(const XboxReport *report) {
    if (le_connection_handle == HCI_CON_HANDLE_INVALID) return;

    uint8_t hid_report[XBOX_INPUT_REPORT_SIZE];
    pack_xbox_report(report, hid_report);

    hids_device_send_input_report(le_connection_handle, hid_report, XBOX_INPUT_REPORT_SIZE);
}

// Initialize BLE HID device
int btstack_hid(void) {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("ERROR: cyw43 init failed\n");
        return -1;
    }

    l2cap_init();

    // Security Manager - bonding + MITM + Secure Connections
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    sm_set_authentication_requirements(SM_AUTHREQ_BONDING | SM_AUTHREQ_MITM_PROTECTION | SM_AUTHREQ_SECURE_CONNECTION);

    att_server_init(profile_data, NULL, NULL);

    battery_service_server_init(100);

    // Device information - Xbox Series X identity
    device_information_service_server_init();
    device_information_service_server_set_manufacturer_name("Microsoft");
    device_information_service_server_set_model_number("Xbox Series X");
    device_information_service_server_set_serial_number("309710067104021");
    device_information_service_server_set_hardware_revision("01.00");
    device_information_service_server_set_firmware_revision("5.09");
    device_information_service_server_set_software_revision("1.0");
    device_information_service_server_set_pnp_id(2, 0x045E, 0x0B13, 0x0509);

    // HID service
    hids_device_init(0x00, xbox_hid_descriptor, sizeof(xbox_hid_descriptor));

    // Register packet handlers
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    l2cap_event_callback_registration.callback = &packet_handler;
    l2cap_add_event_handler(&l2cap_event_callback_registration);

    sm_event_callback_registration.callback = &packet_handler;
    sm_add_event_handler(&sm_event_callback_registration);

    hids_device_register_packet_handler(packet_handler);

    // Advertising
    gap_set_local_name("titkos mikkentyu");
    gap_advertisements_set_data(adv_data_len, (uint8_t *)adv_data);

    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(0x0030, 0x0030, 0, 0, null_addr, 0x07, 0x00);
    gap_advertisements_enable(1);

    hci_power_control(HCI_POWER_ON);

    printf("BLE HID initialized\n");
    return 1;
}
