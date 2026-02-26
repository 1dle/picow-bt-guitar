#ifndef XBOX_DESCRIPTORS_H
#define XBOX_DESCRIPTORS_H

#include <stdint.h>

// Xbox Series X (Model 1914) BLE HID Report Descriptor
// Byte-for-byte match with real Xbox controller descriptor from ESP32 working project
// Source: https://github.com/DJm00n/ControllersInfo
//
// Input Report ID 0x01: 4 sticks (16-bit) + 2 triggers (10-bit) + hat (4-bit) + 15 buttons + record = 16 bytes
// Output Report ID 0x03: Haptic feedback (8 bytes)
static const uint8_t xbox_hid_descriptor[] = {
    0x05, 0x01,                     // Usage Page (Generic Desktop)
    0x09, 0x05,                     // Usage (Game Pad)
    0xA1, 0x01,                     // Collection (Application)
    0x85, 0x01,                     //   Report ID (0x01) - Input Report

    // ===== Left Stick (Physical Collection: Pointer) =====
    0x09, 0x01,                     //   Usage (Pointer)
    0xA1, 0x00,                     //   Collection (Physical)
    0x09, 0x30,                     //     Usage (X)
    0x09, 0x31,                     //     Usage (Y)
    0x15, 0x00,                     //     Logical Minimum (0)
    0x27, 0xFF, 0xFF, 0x00, 0x00,   //     Logical Maximum (65535)
    0x95, 0x02,                     //     Report Count (2)
    0x75, 0x10,                     //     Report Size (16)
    0x81, 0x02,                     //     Input (Data, Var, Abs)
    0xC0,                           //   End Collection (Physical)

    // ===== Right Stick (Physical Collection: Pointer) =====
    0x09, 0x01,                     //   Usage (Pointer)
    0xA1, 0x00,                     //   Collection (Physical)
    0x09, 0x32,                     //     Usage (Z)
    0x09, 0x35,                     //     Usage (Rz)
    0x15, 0x00,                     //     Logical Minimum (0)
    0x27, 0xFF, 0xFF, 0x00, 0x00,   //     Logical Maximum (65535)
    0x95, 0x02,                     //     Report Count (2)
    0x75, 0x10,                     //     Report Size (16)
    0x81, 0x02,                     //     Input (Data, Var, Abs)
    0xC0,                           //   End Collection (Physical)

    // ===== Left Trigger (Brake) - 10-bit =====
    0x05, 0x02,                     //   Usage Page (Simulation Controls)
    0x09, 0xC5,                     //   Usage (Brake)
    0x15, 0x00,                     //   Logical Minimum (0)
    0x26, 0xFF, 0x03,               //   Logical Maximum (1023)
    0x95, 0x01,                     //   Report Count (1)
    0x75, 0x0A,                     //   Report Size (10)
    0x81, 0x02,                     //   Input (Data, Var, Abs)
    // 6-bit padding
    0x15, 0x00,                     //   Logical Minimum (0)
    0x25, 0x00,                     //   Logical Maximum (0)
    0x75, 0x06,                     //   Report Size (6)
    0x95, 0x01,                     //   Report Count (1)
    0x81, 0x03,                     //   Input (Constant)

    // ===== Right Trigger (Accelerator) - 10-bit =====
    0x05, 0x02,                     //   Usage Page (Simulation Controls)
    0x09, 0xC4,                     //   Usage (Accelerator)
    0x15, 0x00,                     //   Logical Minimum (0)
    0x26, 0xFF, 0x03,               //   Logical Maximum (1023)
    0x95, 0x01,                     //   Report Count (1)
    0x75, 0x0A,                     //   Report Size (10)
    0x81, 0x02,                     //   Input (Data, Var, Abs)
    // 6-bit padding
    0x15, 0x00,                     //   Logical Minimum (0)
    0x25, 0x00,                     //   Logical Maximum (0)
    0x75, 0x06,                     //   Report Size (6)
    0x95, 0x01,                     //   Report Count (1)
    0x81, 0x03,                     //   Input (Constant)

    // ===== Hat Switch (D-Pad) - 4-bit with null state =====
    0x05, 0x01,                     //   Usage Page (Generic Desktop)
    0x09, 0x39,                     //   Usage (Hat switch)
    0x15, 0x01,                     //   Logical Minimum (1)
    0x25, 0x08,                     //   Logical Maximum (8)
    0x35, 0x00,                     //   Physical Minimum (0)
    0x46, 0x3B, 0x01,               //   Physical Maximum (315)
    0x66, 0x14, 0x00,               //   Unit (degrees)
    0x75, 0x04,                     //   Report Size (4)
    0x95, 0x01,                     //   Report Count (1)
    0x81, 0x42,                     //   Input (Data, Var, Abs, Null)
    // 4-bit padding (reset physical/unit)
    0x75, 0x04,                     //   Report Size (4)
    0x95, 0x01,                     //   Report Count (1)
    0x15, 0x00,                     //   Logical Minimum (0)
    0x25, 0x00,                     //   Logical Maximum (0)
    0x35, 0x00,                     //   Physical Minimum (0)
    0x45, 0x00,                     //   Physical Maximum (0)
    0x65, 0x00,                     //   Unit (none)
    0x81, 0x03,                     //   Input (Constant)

    // ===== Buttons 1-15 =====
    0x05, 0x09,                     //   Usage Page (Button)
    0x19, 0x01,                     //   Usage Minimum (Button 1)
    0x29, 0x0F,                     //   Usage Maximum (Button 15)
    0x15, 0x00,                     //   Logical Minimum (0)
    0x25, 0x01,                     //   Logical Maximum (1)
    0x75, 0x01,                     //   Report Size (1)
    0x95, 0x0F,                     //   Report Count (15)
    0x81, 0x02,                     //   Input (Data, Var, Abs)
    // 1-bit padding
    0x15, 0x00,                     //   Logical Minimum (0)
    0x25, 0x00,                     //   Logical Maximum (0)
    0x75, 0x01,                     //   Report Size (1)
    0x95, 0x01,                     //   Report Count (1)
    0x81, 0x03,                     //   Input (Constant)

    // ===== Record Button (Consumer Page) =====
    0x05, 0x0C,                     //   Usage Page (Consumer)
    0x0A, 0xB2, 0x00,               //   Usage (Record)
    0x15, 0x00,                     //   Logical Minimum (0)
    0x25, 0x01,                     //   Logical Maximum (1)
    0x95, 0x01,                     //   Report Count (1)
    0x75, 0x01,                     //   Report Size (1)
    0x81, 0x02,                     //   Input (Data, Var, Abs)
    // 7-bit padding
    0x15, 0x00,                     //   Logical Minimum (0)
    0x25, 0x00,                     //   Logical Maximum (0)
    0x75, 0x07,                     //   Report Size (7)
    0x95, 0x01,                     //   Report Count (1)
    0x81, 0x03,                     //   Input (Constant)

    // ===== Output Report ID 0x03 (Haptics / Force Feedback) =====
    0x05, 0x0F,                     //   Usage Page (Physical Interface Device)
    0x09, 0x21,                     //   Usage (Set Effect Report)
    0x85, 0x03,                     //   Report ID (0x03) - Output Report
    0xA1, 0x02,                     //   Collection (Logical)
    // DC Enable Actuators (4 bits)
    0x09, 0x97,                     //     Usage (DC Enable Actuators)
    0x15, 0x00,                     //     Logical Minimum (0)
    0x25, 0x01,                     //     Logical Maximum (1)
    0x75, 0x04,                     //     Report Size (4)
    0x95, 0x01,                     //     Report Count (1)
    0x91, 0x02,                     //     Output (Data, Var, Abs)
    // 4-bit padding
    0x15, 0x00,                     //     Logical Minimum (0)
    0x25, 0x00,                     //     Logical Maximum (0)
    0x75, 0x04,                     //     Report Size (4)
    0x95, 0x01,                     //     Report Count (1)
    0x91, 0x03,                     //     Output (Constant)
    // Magnitude (4x 8-bit, 0-100)
    0x09, 0x70,                     //     Usage (Magnitude)
    0x15, 0x00,                     //     Logical Minimum (0)
    0x25, 0x64,                     //     Logical Maximum (100)
    0x75, 0x08,                     //     Report Size (8)
    0x95, 0x04,                     //     Report Count (4)
    0x91, 0x02,                     //     Output (Data, Var, Abs)
    // Duration (8-bit, seconds x10^-2)
    0x09, 0x50,                     //     Usage (Duration)
    0x66, 0x01, 0x10,               //     Unit (seconds)
    0x55, 0x0E,                     //     Unit Exponent (-2)
    0x15, 0x00,                     //     Logical Minimum (0)
    0x26, 0xFF, 0x00,               //     Logical Maximum (255)
    0x75, 0x08,                     //     Report Size (8)
    0x95, 0x01,                     //     Report Count (1)
    0x91, 0x02,                     //     Output (Data, Var, Abs)
    // Start Delay (8-bit)
    0x09, 0xA7,                     //     Usage (Start Delay)
    0x15, 0x00,                     //     Logical Minimum (0)
    0x26, 0xFF, 0x00,               //     Logical Maximum (255)
    0x75, 0x08,                     //     Report Size (8)
    0x95, 0x01,                     //     Report Count (1)
    0x91, 0x02,                     //     Output (Data, Var, Abs)
    // Reset units
    0x65, 0x00,                     //     Unit (none)
    0x55, 0x00,                     //     Unit Exponent (0)
    // Loop Count (8-bit)
    0x09, 0x7C,                     //     Usage (Loop Count)
    0x15, 0x00,                     //     Logical Minimum (0)
    0x26, 0xFF, 0x00,               //     Logical Maximum (255)
    0x75, 0x08,                     //     Report Size (8)
    0x95, 0x01,                     //     Report Count (1)
    0x91, 0x02,                     //     Output (Data, Var, Abs)
    0xC0,                           //   End Collection (Logical)

    0xC0                            // End Collection (Application)
};

// Input Report data size (Report ID 0x01, without the report ID byte)
#define XBOX_INPUT_REPORT_SIZE 16

// Output Report data size (Report ID 0x03, without the report ID byte)
#define XBOX_OUTPUT_REPORT_SIZE 8

// Hat switch values (D-Pad) - 0 = neutral (null state outside 1-8 range)
#define XBOX_HAT_NEUTRAL           0x00
#define XBOX_HAT_NOTHING           0x00  // Legacy alias
#define XBOX_HAT_UP                0x01
#define XBOX_HAT_UP_RIGHT          0x02
#define XBOX_HAT_RIGHT             0x03
#define XBOX_HAT_DOWN_RIGHT        0x04
#define XBOX_HAT_DOWN              0x05
#define XBOX_HAT_DOWN_LEFT         0x06
#define XBOX_HAT_LEFT              0x07
#define XBOX_HAT_UP_LEFT           0x08

// Button bitmasks
#define XBOX_BUTTON_A              0x01
#define XBOX_BUTTON_B              0x02
#define XBOX_BUTTON_X              0x08 
#define XBOX_BUTTON_Y              0x10
#define XBOX_BUTTON_LB             0x40
#define XBOX_BUTTON_RB             0x80
#define XBOX_BUTTON_SELECT         0x400
#define XBOX_BUTTON_START          0x800
#define XBOX_BUTTON_HOME           0x1000
#define XBOX_BUTTON_LS             0x2000
#define XBOX_BUTTON_RS             0x4000

// Axis ranges (sticks: 16-bit unsigned, triggers: 10-bit unsigned)
#define XBOX_AXIS_MIN              0x0000
#define XBOX_AXIS_CENTER           0x8000
#define XBOX_AXIS_MAX              0xFFFF
#define XBOX_TRIGGER_MIN           0x0000
#define XBOX_TRIGGER_MAX           0x03FF  // 1023

// Xbox Series X BLE HID input report struct
// Field order matches report byte layout (see pack_xbox_report)
typedef struct {
    uint16_t left_stick_x;     // Usage X         - 0 to 65535, center 32768
    uint16_t left_stick_y;     // Usage Y         - 0 to 65535, center 32768
    uint16_t right_stick_x;    // Usage Z         - 0 to 65535, center 32768
    uint16_t right_stick_y;    // Usage Rz        - 0 to 65535, center 32768
    uint16_t left_trigger;     // Usage Brake     - 0 to 1023 (10-bit)
    uint16_t right_trigger;    // Usage Accel     - 0 to 1023 (10-bit)
    uint8_t  hat;              // Usage Hat       - 0=neutral, 1-8=directions
    uint16_t buttons;          // Usage Button    - 15 bits (1-15)
} XboxReport;

#endif // XBOX_DESCRIPTORS_H