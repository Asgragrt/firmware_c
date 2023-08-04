#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "tusb/tud_utils.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+


/*------------- MAIN -------------*/
int main(void){
    tusb_init();
    board_init();

    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);

    while (1){
        tud_task(); // tinyusb device task
        led_blinking_task();

        hid_task();
        cdc_task();
    }
}