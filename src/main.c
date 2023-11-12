#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tusb.h"

#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "hardware/watchdog.h"

#include "tusb/tud_utils.h"
#include "kbd/kbd.h"
#include "flash/flash.h"
#include "leds/leds.h"

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
}


/*------------- MAIN -------------*/
int main(void){
    //Don't put delays (interrups) before core1    
    multicore_launch_core1(core1_entry);
    led_array = led_array_init();
    mutex_init(&mutex_v);

    tusb_init();
    //board_init();
    gpio_init(0);
    gpio_set_dir(0, GPIO_OUT);

    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);

    keyboard_t* kbd = keyboard_new();
    keyboard_init(kbd);  

    bool write = false;

    //TODO MAKE IT SINGLE CORE SINCE SECOND CORE REFUSES TO COOPERATE
    // TRY MUTEX TO UPDATE VALUES IN SECOND CORE AND JUST ASSIGN THEM IN FIRST CORE
    // Assign to pwm

    mutex_enter_blocking(&mutex_v);
        led_array_update_mode(
            &led_array, flash_target_contents[0] 
            - 48 * (flash_target_contents[0] >= 48)
            );
    mutex_exit(&mutex_v);

    //keyboard_update_key(kbd);

    while (1){
        uint8_t write_data[FLASH_PAGE_SIZE] = { 0 };
        tud_task(); // tinyusb device task
        led_blinking_task();

        hid_task(kbd);
        cdc_task(kbd, watchdog_caused_reboot(), &write, write_data);
        if ( write ){
            save_flash(write_data);
            write = false;
            //keyboard_update_key(&kbd);
        }

        mutex_enter_blocking(&mutex_v);

            led_array_update_mode(
                &led_array, flash_target_contents[0] 
                - 48 * (flash_target_contents[0] >= 48)
                );
            if ( tud_suspended() ) {
                led_array_update_mode(&led_array, _led_off);
            }
            led_array_set_levels(&led_array);

        mutex_exit(&mutex_v);
    }
    //flash_target_contents[0]
}