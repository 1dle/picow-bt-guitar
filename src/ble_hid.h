#ifndef BLUE_HID_H
#define BLUE_HID_H

#include "descriptor/XboxDescriptors.h"

int btstack_hid(void);
void ble_send_hid_report(const XboxReport *report);

#endif