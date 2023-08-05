#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"

#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "hardware/watchdog.h"

#include "tusb/tud_utils.h"
#include "kbd/kbd.h"
#include "flash/flash.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

const int FIFO_LENGTH = 1;

queue_t multicore_fifo;


//Debug lock 
void core1_entry(){
    multicore_lockout_victim_init();

    gpio_init(18);
    gpio_set_dir(18, GPIO_OUT);

    gpio_init(19);
    gpio_set_dir(19, GPIO_OUT);

    bool status = false;

    bool read;

    uint8_t fifo_read;

    while(1){
        read = queue_try_remove(&multicore_fifo, &fifo_read);
        if ( read ){
            if ( fifo_read == '0' ){
                gpio_put(18, true);
                gpio_put(19, false);
            }
            else {
                gpio_put(19, true);
                gpio_put(18, false);
            }
        }    
    }
}




/*------------- MAIN -------------*/
int main(void){
    multicore_launch_core1(core1_entry);

    tusb_init();
    board_init();

    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);

    keyboard kbd = kbd_new();
    kbd_init(&kbd);

    queue_init(&multicore_fifo, sizeof(uint8_t), FIFO_LENGTH);

    //multicore_lockout_start_blocking();

    //FLASH READING AND WRITING

    //multicore_lockout_end_blocking();

    bool write = false;

    uint8_t val = 0;

    while (1){
        uint8_t write_data[FLASH_PAGE_SIZE] = { 0 };
        tud_task(); // tinyusb device task
        led_blinking_task();

        hid_task(&kbd);
        cdc_task(&kbd, watchdog_caused_reboot(), &write, write_data);
        if ( write ){
            val = write_data[0];
            queue_try_add(&multicore_fifo, &val);
            save_flash(write_data);
            write = false;
        }
    }
}