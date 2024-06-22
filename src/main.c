#include "descriptor/SwitchDescriptors.h"
#include "blue_hid.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include "hardware/adc.h"

#define LED 18
#define WHAMMY 27
#define WHAMMY_CH 1
//#define SLIDER 28;

                          //frets    select strum
const int gp_buttons[] = {0,1,2,3,4, 19,20, 21,22,};
const int num_buttons = 9;
const int gp_dpad[] = {6,9,7,8}; //up right down left
//const int num_dpad = 4;

//const int num_buttons = 9;
//const int first_btn_gpio = 7;

const uint delay = 500; // 1s delay

static SwitchReport report =
        {
                .buttons = 0,
                .hat = SWITCH_HAT_NOTHING,
                .lx = 0,
                .ly = 0,
                .rx = SWITCH_JOYSTICK_MID,
                .ry = SWITCH_JOYSTICK_MID,
                .vendor = 0,
        };


long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int main(){
    //stdio_init_all(); //115200 debug
    
    //battery or connection indicator
    /*const uint LED_PIN = LED;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);*/

    //init adc for whammy
    adc_init();
    adc_gpio_init(WHAMMY);
    adc_select_input(WHAMMY_CH);

    
    //init gpio
    int i = 0;
    for(i = 0; i < num_buttons; i++){
        gpio_init(gp_buttons[i]);
        gpio_set_dir(gp_buttons[i], GPIO_IN);
        gpio_pull_up(gp_buttons[i]);
        if(i < 4){ //init dpad in same loop
            gpio_init(gp_dpad[i]);
            gpio_set_dir(gp_dpad[i], GPIO_IN);
            gpio_pull_up(gp_dpad[i]);
        }
    }
    int connected = btstack_hid();

    while(1){

        //read buttons
        report.buttons = 0U;
        unsigned int j = 0U;

        for(j = 0U; j < num_buttons; j++){
            if(!gpio_get(gp_buttons[j])){
                report.buttons |= (1<<j);
            }
        }

        //read dpad
        unsigned int dpad_mask = 0;
        for(j=0U;j<4;j++){
            if(!gpio_get(gp_dpad[j])){
                dpad_mask |= (1<<j);
            }
        }
        switch(dpad_mask){
            case 1: //up
                report.hat = SWITCH_HAT_UP;
                break;
            case 2: //right
                report.hat = SWITCH_HAT_RIGHT;
                break;
            case 4: //down
                report.hat = SWITCH_HAT_DOWN;
                break;
            case 8: //left
                report.hat = SWITCH_HAT_LEFT;
                break;
            default:
                report.hat = SWITCH_HAT_NOTHING;
                break;
        }

        //read whammy
        
        //
        uint16_t i_whammy = adc_read();

        if(i_whammy < 25) i_whammy = 0;
        if(i_whammy > 2050) i_whammy = 2050;
        long axis_whammy_val = map(i_whammy, 0, 2050, SWITCH_JOYSTICK_MID, SWITCH_JOYSTICK_MAX);

        report.ly = axis_whammy_val;

        new_data(&report);
    }
    return 0;
}