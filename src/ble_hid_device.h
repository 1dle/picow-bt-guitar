#ifndef BLE_HID_DEVICE_H
#define BLE_HID_DEVICE_H

#include "descriptor/XboxDescriptors.h"

int btstack_hid(void);
void ble_send_hid_report(const XboxReport *report);

#endif