#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "tud_utils.h"
#include "pico/stdlib.h"

#include "../kbd/kbd.h"
#include "../flash/flash.h"

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
};


static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void){
    blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void){
    blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en){
    (void) remote_wakeup_en;
    blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void){
    blink_interval_ms = BLINK_MOUNTED;
}

void hid_task(keyboard_t* kbd){
    const uint32_t interval_ms = POLLING_INTERVAL;
    static uint32_t start_ms = 0;
    if ( board_millis() - start_ms < interval_ms ) return; // not enough time
    start_ms += interval_ms;
    /*
    const uint64_t interval_us = POLLING_INTERVAL * 1000u;
    static uint64_t start_us = 0;
    if ( to_us_since_boot(get_absolute_time()) - start_us < interval_us ) return;
    start_us += interval_us;
    */

    //uint32_t const btn = board_button_read();

    // Remote wakeup
    if ( tud_suspended() ){
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        if ( keyboard_update_status(kbd) ){
            tud_remote_wakeup();
        }
    }

    /*------------- keyboard_t -------------*/
    if ( !tud_hid_n_ready(0) ) return;
    
    uint8_t buffer[keycode_buffer] = {0};
    keyboard_update_buffer(kbd, buffer, keycode_buffer);
    tud_hid_nkro_keyboard_report(0, buffer);
    
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

bool tud_hid_nkro_keyboard_report(uint8_t report_id, uint8_t keycode[keycode_buffer]){

    hid_nkro_keyboard_report_t report = {
        .modifier   = 0,
        .reserved   = 0,
        .boot_keys  = {0},
        .key_bitmap = {0},
    };

    uint8_t key_count = 0;
    bool is_modifier;

    for(uint8_t key = 0; key < keycode_buffer; key++){

        //Boot key support
        if ( key_count < 6 ){
            is_modifier = boot_key_modifier(&report, keycode[key], &key_count);
        }

        //NKRO key support
        //                keycode[key] // 8           keycode[key] % 8
        if ( !is_modifier ){
            report.key_bitmap[keycode[key] >> 3] |= 1 << (keycode[key] & 0x7);
        }
    }
    
    return tud_hid_n_report(0, report_id, &report, sizeof(report));
}

bool boot_key_modifier(hid_nkro_keyboard_report_t* report, uint8_t key, uint8_t* counter){
    //If modifier key
    if ( key >= 0xE0 ){
        report->modifier |= (1 << (key - 0xE0) );
        return true; 
    }
    else {
        report->boot_keys[*counter] = key;
        (*counter)++;
        return false;
    } 
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen){
    // TODO not Implemented
    (void) itf;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize){
    // TODO set LED based on CAPLOCK, NUMLOCK etc...
    (void) itf;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void){
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  //board_led_write(led_state);
  gpio_put(0, led_state);
  led_state = 1 - led_state; // toggle
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+

void cdc_write_char_dec(uint8_t val){
    uint8_t u = val % 10;
    uint8_t d = (val / 10) % 10;
    uint8_t c = (val / 100) % 10;
    tud_cdc_write_char(c + 48);
    tud_cdc_write_char(d + 48);
    tud_cdc_write_char(u + 48);
    tud_cdc_write_char(' ');
}

void cdc_task(keyboard_t* kbd, bool reboot, bool* write, uint8_t* write_buff){
    // connected and there are data available
    if ( tud_cdc_available() ){
        // read datas
        char buf[64];
        uint32_t count = tud_cdc_read(buf, sizeof(buf));
        tud_cdc_write(buf, count);
        tud_cdc_write_flush();
        cdc_write_char_dec(buf[0]);

        if ( buf[0] == '1' || buf[0] == 1){
            for(uint8_t i = 0; i < 9; i++){
                tud_cdc_write_char(kbd->pins[i].pin + 48);
            }
        }

        if ( buf[0] == '2' || buf[0] == 2 ){
            tud_cdc_write_char('\n');
            tud_cdc_write_char('\n');
            for(uint8_t i = 0; i < 9; i++){
                for(uint8_t j = 0; j < kbd->pins[i].key_count; j++){
                    cdc_write_char_dec(kbd->pins[i].keys[j]);
                }
                tud_cdc_write_char('\n');
            }
        }

        /*
        if ( buf[0] == '2' || buf[0] == 2 ){
            keyboard_update_status(kbd);
            for(uint8_t i = 0; i < 9; i++){
                tud_cdc_write_char(((kbd->status & (1 << i)) >> i) + 48);
                tud_cdc_write_char(' ');
            }
        }
        */

        if ( buf[0] == '3' || buf[0] == 3 ){
            tud_cdc_write_char((char) reboot + 48);
        }

        (void) count;

        if ( buf[0] == '4' || buf[0] == 4 ){
            tud_cdc_write_char(flash_target_contents[0]);
            tud_cdc_write_char(' ');
            for(uint8_t i = 0; i < 10; i++){
                cdc_write_char_dec(flash_target_contents[i]);
                
            }
        }

        if ( buf[0] == '5' || buf[0] == 5 ){
            *write = true;
            memcpy(write_buff, buf + 1, count - 1);
        }
    }
}
