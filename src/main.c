#include "descriptor/XboxDescriptors.h"
#include "ble_hid_device.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <string.h>
#include "btstack.h"

// Example: Map 2 buttons and a dpad to GPIOs
#define BUTTON_A_GPIO 2
#define BUTTON_B_GPIO 3
#define DPAD_UP_GPIO 4
#define DPAD_DOWN_GPIO 5

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

static void poll_inputs(void) {
	report.buttons = 0;
	report.hat = XBOX_HAT_NOTHING;
	if (!gpio_get(BUTTON_A_GPIO)) report.buttons |= XBOX_BUTTON_A;
	if (!gpio_get(BUTTON_B_GPIO)) report.buttons |= XBOX_BUTTON_B;
	if (!gpio_get(DPAD_UP_GPIO)) report.hat = XBOX_HAT_UP;
	if (!gpio_get(DPAD_DOWN_GPIO)) report.hat = XBOX_HAT_DOWN;
}

static void button_poll_timer_handler(btstack_timer_source_t *ts) {
	UNUSED(ts);
	poll_inputs();
	if (memcmp(&report, &last_sent_report, sizeof(XboxReport)) != 0) {
		ble_send_hid_report(&report);
		last_sent_report = report;
	}
	btstack_run_loop_set_timer(ts, 2);
	btstack_run_loop_add_timer(ts);
}

int main() {
	stdio_init_all();
	gpio_init(BUTTON_A_GPIO); gpio_set_dir(BUTTON_A_GPIO, GPIO_IN); gpio_pull_up(BUTTON_A_GPIO);
	gpio_init(BUTTON_B_GPIO); gpio_set_dir(BUTTON_B_GPIO, GPIO_IN); gpio_pull_up(BUTTON_B_GPIO);
	gpio_init(DPAD_UP_GPIO);  gpio_set_dir(DPAD_UP_GPIO, GPIO_IN);  gpio_pull_up(DPAD_UP_GPIO);
	gpio_init(DPAD_DOWN_GPIO);gpio_set_dir(DPAD_DOWN_GPIO, GPIO_IN);gpio_pull_up(DPAD_DOWN_GPIO);
	last_sent_report = report;
	btstack_hid();
	static btstack_timer_source_t button_poll_timer;
	button_poll_timer.process = &button_poll_timer_handler;
	btstack_run_loop_set_timer(&button_poll_timer, 2);
	btstack_run_loop_add_timer(&button_poll_timer);
	btstack_run_loop_execute();
	return 0;
}