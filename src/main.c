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

mutex_t mutex_var;

bool timer_callback(repeating_timer_t *rt){
    static bool status = true;
    gpio_put(20, status);
    status ^= true;
    return true;
}

//Debug lock 
void core1_entry(){
    multicore_lockout_victim_init();

    gpio_init(18);
    gpio_set_dir(18, GPIO_OUT);

    gpio_init(19);
    gpio_set_dir(19, GPIO_OUT);

    gpio_init(20);
    gpio_set_dir(20, GPIO_OUT);

    repeating_timer_t timer;

    add_repeating_timer_ms(250, timer_callback, NULL, &timer);

    const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
    uint8_t status = 0, new_status = 0;
    while(1){
        mutex_enter_blocking(&mutex_var);
        new_status = flash_target_contents[0];
        mutex_exit(&mutex_var);
        
        if ( new_status != status ){
            switch ( new_status - 48){
                case 0:
                    gpio_put(18, false);
                    gpio_put(19, false);
                    break;

                case 1:
                    gpio_put(18, true);
                    gpio_put(19, false);
                    break;

                case 2:
                    gpio_put(18, false);
                    gpio_put(19, true);
                    break;

                default:
                    gpio_put(18, true);
                    gpio_put(19, true);
                    break;

            }
        }
        status = new_status;    
    }
}




/*------------- MAIN -------------*/
int main(void){
    //queue_init(&multicore_fifo, sizeof(uint8_t), FIFO_LENGTH);
    //init_q(&multicore_fifo);
    mutex_init(&mutex_var);
    
    multicore_launch_core1(core1_entry);

    tusb_init();
    board_init();

    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);

    keyboard kbd = kbd_new();
    kbd_init(&kbd);  

    bool write = false;

    uint8_t val = 0;

    while (1){
        uint8_t write_data[FLASH_PAGE_SIZE] = { 0 };
        tud_task(); // tinyusb device task
        led_blinking_task();

        hid_task(&kbd);
        cdc_task(&kbd, watchdog_caused_reboot(), &write, write_data);
        if ( write ){
            memcpy(&val, write_data, 1);

            //mutex_enter_timeout_ms(&mutex_var, 3);

            //mutex_exit(&mutex_var);

            save_flash(write_data);
            write = false;
        }
    }
}