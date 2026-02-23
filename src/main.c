#include "descriptor/XboxDescriptors.h"
#include "ble_hid_device.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <string.h>
#include "hardware/adc.h"
#include "btstack.h"

#define LED 18
#define WHAMMY 27
#define WHAMMY_CH 1
//#define SLIDER 28;

                          //frets    select strum
const int gp_buttons[] = {0,1,2,3,4, 19,20, 21,22,};
const int num_buttons = 9;

// GPIO button index to Xbox button bitmask
const uint16_t xbox_button_map[] = {
    XBOX_BUTTON_A,          // GPIO 0 - A
    XBOX_BUTTON_B,          // GPIO 1 - B
    XBOX_BUTTON_X,          // GPIO 2 - X
    XBOX_BUTTON_Y,          // GPIO 3 - Y
    XBOX_BUTTON_LB,         // GPIO 4 - LB
    XBOX_BUTTON_RB,         // GPIO 19 - RB
    XBOX_BUTTON_SELECT,     // GPIO 20 - SELECT
    XBOX_BUTTON_START,      // GPIO 21 - START
    XBOX_BUTTON_HOME,       // GPIO 22 - HOME
};

const int gp_dpad[] = {6, 9, 7, 8}; // up right down left

#define INPUT_POLL_MS 1

static XboxReport report =
        {
                .buttons = 0,
                .hat = XBOX_HAT_NOTHING,
                .left_stick_x = XBOX_AXIS_CENTER,
                .left_stick_y = XBOX_AXIS_CENTER,
                .right_stick_x = XBOX_AXIS_CENTER,
                .right_stick_y = XBOX_AXIS_CENTER,
                .left_trigger = 0,
                .right_trigger = 0,
        };

        static XboxReport last_sent_report =
            {
                .buttons = 0,
                .hat = XBOX_HAT_NOTHING,
                .left_stick_x = XBOX_AXIS_CENTER,
                .left_stick_y = XBOX_AXIS_CENTER,
                .right_stick_x = XBOX_AXIS_CENTER,
                .right_stick_y = XBOX_AXIS_CENTER,
                .left_trigger = 0,
                .right_trigger = 0,
            };


long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static void button_poll_timer_handler(btstack_timer_source_t *ts) {
    UNUSED(ts);

    // Read buttons
    report.buttons = 0U;
    unsigned int j = 0U;

    for(j = 0U; j < num_buttons; j++){
        if(!gpio_get(gp_buttons[j])){
            report.buttons |= xbox_button_map[j];
        }
    }

    // Read dpad
    unsigned int dpad_mask = 0;
    for (j = 0U; j < 4; j++) {
        if (!gpio_get(gp_dpad[j])) {
            dpad_mask |= (1 << j);
        }
    }
    switch (dpad_mask) {
        case 1: report.hat = XBOX_HAT_UP; break;
        case 2: report.hat = XBOX_HAT_RIGHT; break;
        case 4: report.hat = XBOX_HAT_DOWN; break;
        case 8: report.hat = XBOX_HAT_LEFT; break;
        default: report.hat = XBOX_HAT_NOTHING; break;
    }

    // Read whammy bar (maps to left trigger, 0-1023)
    uint16_t i_whammy = adc_read();
    if (i_whammy < 25) i_whammy = 0;
    if (i_whammy > 2050) i_whammy = 2050;
    report.left_trigger = map(i_whammy, 0, 2050, 0, 1023);

    if (memcmp(&report, &last_sent_report, sizeof(XboxReport)) != 0) {
        ble_send_hid_report(&report);
        last_sent_report = report;
    }

    btstack_run_loop_set_timer(ts, INPUT_POLL_MS);
    btstack_run_loop_add_timer(ts);
}

int main() {
    // ADC for whammy bar
    adc_init();
    adc_gpio_init(WHAMMY);
    adc_select_input(WHAMMY_CH);

    // GPIO buttons and dpad
    for (int i = 0; i < num_buttons; i++) {
        gpio_init(gp_buttons[i]);
        gpio_set_dir(gp_buttons[i], GPIO_IN);
        gpio_pull_up(gp_buttons[i]);
        if (i < 4) {
            gpio_init(gp_dpad[i]);
            gpio_set_dir(gp_dpad[i], GPIO_IN);
            gpio_pull_up(gp_dpad[i]);
        }
    }

    btstack_hid();

    // Low-latency polling timer for inputs
    static btstack_timer_source_t button_poll_timer;
    button_poll_timer.process = &button_poll_timer_handler;
    btstack_run_loop_set_timer(&button_poll_timer, INPUT_POLL_MS);
    btstack_run_loop_add_timer(&button_poll_timer);

    btstack_run_loop_execute();
    return 0;
}