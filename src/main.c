#include "descriptor/SwitchDescriptors.h"
#include "blue_hid.h"

static SwitchReport switchReport =
        {
                .buttons = 0,
                .hat = SWITCH_HAT_NOTHING,
                .lx = SWITCH_JOYSTICK_MID,
                .ly = SWITCH_JOYSTICK_MID,
                .rx = SWITCH_JOYSTICK_MID,
                .ry = SWITCH_JOYSTICK_MID,
                .vendor = 0,
        };


int main(){
    switchReport.buttons |= SWITCH_MASK_B;
    switchReport.buttons |= SWITCH_MASK_X;
    btstack_hid(&switchReport);

    return 0;
}