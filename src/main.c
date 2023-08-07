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
#include "leds/leds.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

bool timer_callback(repeating_timer_t *rt){
    static bool status = true;
    gpio_put(27, status);
    status ^= true;
    return true;
}

mutex_t mutex_v;

led_array_t led_array;


void core1_entry(){
    multicore_lockout_victim_init();
    //Waiting for usb connection, somehow it breaks the PWM (probs interrupts)

    uint64_t delay = 500;
    while(1){
        mutex_enter_blocking(&mutex_v);
        led_array_update_values(&led_array, &delay);
        mutex_exit(&mutex_v);
        busy_wait_us(delay);
    }
    /*

    gpio_init(18);
    gpio_set_dir(18, GPIO_OUT);

    gpio_init(19);
    gpio_set_dir(19, GPIO_OUT);

    gpio_init(20);
    gpio_set_dir(20, GPIO_OUT);

    repeating_timer_t timer;

    add_repeating_timer_ms(250, timer_callback, NULL, &timer);

    uint8_t status = 0, new_status = 0;
    while(1){
        //No need to block since writing to flash blocks this core :D
        new_status = flash_target_contents[0];
        
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
    */
}


/*------------- MAIN -------------*/
int main(void){
    //Don't put delays (interrups) before core1    
    multicore_launch_core1(core1_entry);
    led_array = led_array_init();
    mutex_init(&mutex_v);

    tusb_init();
    board_init();

    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);

    keyboard_t kbd = keyboard_new();
    keyboard_init(&kbd);  

    bool write = false;

    //TODO MAKE IT SINGLE CORE SINCE SECOND CORE REFUSES TO COOPERATE
    // TRY MUTEX TO UPDATE VALUES IN SECOND CORE AND JUST ASSIGN THEM IN FIRST CORE
    // Assign to pwm

    mutex_enter_blocking(&mutex_v);
    if (flash_target_contents[0] >= 48 ){
        led_array_update_mode(&led_array, flash_target_contents[0] - 48);
    }
    else {
        led_array_update_mode(&led_array, flash_target_contents[0]);
    }
    mutex_exit(&mutex_v);

    keyboard_update_key(&kbd);

    while (1){
        uint8_t write_data[FLASH_PAGE_SIZE] = { 0 };
        tud_task(); // tinyusb device task
        led_blinking_task();

        hid_task(&kbd);
        cdc_task(&kbd, watchdog_caused_reboot(), &write, write_data);
        if ( write ){
            save_flash(write_data);
            write = false;
            keyboard_update_key(&kbd);
        }

        mutex_enter_blocking(&mutex_v);
        if (flash_target_contents[0] >= 48 ){
            led_array_update_mode(&led_array, flash_target_contents[0] - 48);
        }
        else {
            led_array_update_mode(&led_array, flash_target_contents[0]);
        }
        led_array_set_levels(&led_array);
        mutex_exit(&mutex_v);
    }
    //flash_target_contents[0]
}