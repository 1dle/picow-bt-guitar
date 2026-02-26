#include "descriptor/XboxDescriptors.h"
#include "ble_hid_device.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <string.h>
#include "hardware/adc.h"
#include "btstack.h"

#define LED 18

// Whammy bar input GPIO and ADC channel
#define WHAMMY 27
#define WHAMMY_CH 1
//#define SLIDER 28;

typedef enum {
    INPUT_BUTTON,
    INPUT_HAT
} input_type_t;

typedef struct {
    int gpio;
    input_type_t type;
    uint16_t input;
} gpio_xbox_map_t;

// GPIO - Xbox input mapping list
static const gpio_xbox_map_t gpio_input_map[] = {
    {4,  INPUT_BUTTON, XBOX_BUTTON_A}, //Fret green
    {3,  INPUT_BUTTON, XBOX_BUTTON_B}, //Fret red
    {2,  INPUT_BUTTON, XBOX_BUTTON_Y}, //fret yellow
    {1,  INPUT_BUTTON, XBOX_BUTTON_X}, //fret blue
    {0,  INPUT_BUTTON, XBOX_BUTTON_LB}, //fret orange
    {20, INPUT_BUTTON, XBOX_BUTTON_START}, // start
    {19, INPUT_BUTTON, XBOX_BUTTON_SELECT}, //select
    {21, INPUT_HAT, XBOX_HAT_UP}, //strum up
    {22, INPUT_HAT, XBOX_HAT_DOWN}, //strum down
    //DPAD
    {6,  INPUT_HAT, XBOX_HAT_UP}, 
    {9,  INPUT_HAT, XBOX_HAT_RIGHT},
    {7,  INPUT_HAT, XBOX_HAT_DOWN},
    {8,  INPUT_HAT, XBOX_HAT_LEFT},
};

static const int gpio_input_count = (int)(sizeof(gpio_input_map) / sizeof(gpio_xbox_map_t));

#define INPUT_POLL_MS 1
#define WHAMMY_UPDATE_DIV 4
#define WHAMMY_SAMPLES 32
#define WHAMMY_DEADBAND 200

static XboxReport report = {
    .buttons = 0,
    .hat = XBOX_HAT_NOTHING,
    .left_stick_x = XBOX_AXIS_CENTER,
    .left_stick_y = XBOX_AXIS_CENTER,
    .right_stick_x = XBOX_AXIS_CENTER,
    .right_stick_y = XBOX_AXIS_CENTER,
    .left_trigger = 0,
    .right_trigger = 0,
};
static XboxReport last_sent_report;


long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static uint16_t read_whammy_filtered(void) {
        uint32_t sum = 0;
        for (int sample = 0; sample < WHAMMY_SAMPLES; sample++) {
                sum += adc_read();
        }

        uint16_t i_whammy = (uint16_t)(sum / WHAMMY_SAMPLES);
        if (i_whammy < 25) i_whammy = 0;
        if (i_whammy > 2080) i_whammy = 2080;

        //printf("Whammy raw: %u, filtered: %u\n", (unsigned)adc_read(), i_whammy);

        return (uint16_t)map(i_whammy, 0, 2080, XBOX_AXIS_MAX, XBOX_AXIS_CENTER);
}

static void button_poll_timer_handler(btstack_timer_source_t *ts) {
    UNUSED(ts);

    // Read all digital inputs from a single GPIO -> Xbox mapping list
    report.buttons = 0U;
    unsigned int dpad_mask = 0;
    for (int i = 0; i < gpio_input_count; i++) {
        const gpio_xbox_map_t *m = &gpio_input_map[i];
        if (gpio_get(m->gpio)) {
            continue;
        }

        if (m->type == INPUT_BUTTON) {
            report.buttons |= m->input;
        } else {
            // INPUT_HAT
            switch (m->input) {
                case XBOX_HAT_UP:    dpad_mask |= (1U << 0); break;
                case XBOX_HAT_RIGHT: dpad_mask |= (1U << 1); break;
                case XBOX_HAT_DOWN:  dpad_mask |= (1U << 2); break;
                case XBOX_HAT_LEFT:  dpad_mask |= (1U << 3); break;
                default: break;
            }
        }
    }
    switch (dpad_mask) {
        case 1: report.hat = XBOX_HAT_UP; break;
        case 2: report.hat = XBOX_HAT_RIGHT; break;
        case 4: report.hat = XBOX_HAT_DOWN; break;
        case 8: report.hat = XBOX_HAT_LEFT; break;
        default: report.hat = XBOX_HAT_NOTHING; break;
    }

    // Read whammy bar (maps to right stick X axis)
    static uint8_t whammy_div = 0;
    static uint16_t whammy_smoothed = 0;
    if (++whammy_div >= WHAMMY_UPDATE_DIV) {
        whammy_div = 0;
        uint16_t whammy_now = read_whammy_filtered();
        whammy_smoothed = (uint16_t)((3U * whammy_smoothed + whammy_now) / 4U);

        int whammy_delta = (int)whammy_smoothed - (int)report.right_stick_x;
        if (whammy_delta < 0) whammy_delta = -whammy_delta;
        if (whammy_delta >= WHAMMY_DEADBAND) {
            report.right_stick_x = whammy_smoothed;
        }
    }

    if(memcmp(&report, &last_sent_report, sizeof(XboxReport)) != 0) {
        ble_send_hid_report(&report);
        last_sent_report = report;
    }

    btstack_run_loop_set_timer(ts, INPUT_POLL_MS);
    btstack_run_loop_add_timer(ts);
}

int main() {

    // stdio
    //stdio_init_all();

    // ADC for whammy bar
    adc_init();
    adc_gpio_init(WHAMMY);
    adc_select_input(WHAMMY_CH);

    // GPIO inputs from single mapping list
    for (int i = 0; i < gpio_input_count; i++) {
        int gpio = gpio_input_map[i].gpio;
        gpio_init(gpio);
        gpio_set_dir(gpio, GPIO_IN);
        gpio_pull_up(gpio);
    }

    last_sent_report = report;

    btstack_hid();

    // Low-latency polling timer for inputs
    static btstack_timer_source_t button_poll_timer;
    button_poll_timer.process = &button_poll_timer_handler;
    btstack_run_loop_set_timer(&button_poll_timer, INPUT_POLL_MS);
    btstack_run_loop_add_timer(&button_poll_timer);

    btstack_run_loop_execute();
    return 0;
}