#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "tud_utils.h"

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

void hid_task(void){
    // Poll every 1ms
    const uint32_t interval_ms = 1;
    static uint32_t start_ms = 0;

    if ( board_millis() - start_ms < interval_ms ) return; // not enough time
    start_ms += interval_ms;

    uint32_t const btn = board_button_read();

    // Remote wakeup
    if ( tud_suspended() && btn ){
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    }

    /*------------- Keyboard -------------*/
    if ( !tud_hid_n_ready(0) ) return;
    // use to avoid send multiple consecutive zero report for keyboard
    static bool has_key = false;
    uint8_t buffer[20];
    for ( uint8_t i = 0; i < 20; i++ ) buffer[i] = 0;

    //Dealing with initial sends
    static uint8_t first_hundred_times = 100;
    if ( first_hundred_times ){
        tud_hid_nkro_keyboard_report(0, buffer);
        first_hundred_times -= 1;
        return;
    }
    
    if ( btn ){
        buffer[0] = HID_KEY_A;
        buffer[1] = HID_KEY_B;
        buffer[2] = HID_KEY_C;
        buffer[3] = HID_KEY_D;
        buffer[4] = HID_KEY_E;
        buffer[5] = HID_KEY_F;
        buffer[6] = HID_KEY_G;
        //buffer[0] = HID_KEY_CONTROL_LEFT;
        //buffer[1] = HID_KEY_O;
        

        tud_hid_nkro_keyboard_report(0, buffer);

        has_key = true;
    }
    else{
        // send empty key report if previously has key pressed
        if (has_key){ 
            tud_hid_nkro_keyboard_report(0, buffer);
        }
        
        has_key = false;
    }
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

bool tud_hid_nkro_keyboard_report(uint8_t report_id, uint8_t keycode[20]){

    hid_nkro_keyboard_report_t report;
    report.modifier = 0;
    report.reserved = 0;
    for(uint8_t i = 0; i < 6; i++) report.boot_keys[i] = 0;
    for(uint8_t i = 0; i < bitmap_byte_size; i++) report.key_bitmap[i] = 0;

    uint8_t key_count = 0;

    for(uint8_t key = 0; key < 20; key++){

        //Boot key support [UNTESTED]
        if ( key_count < 6 ){
            boot_key_modifier(&report, keycode[key], &key_count);
        }

        //NKRO key support
        //                keycode[key] // 8           keycode[key] % 8
        report.key_bitmap[keycode[key] >> 3] |= 1 << (keycode[key] & 0x7);
    }
    

    return tud_hid_n_report(0, report_id, &report, sizeof(report));
}

void boot_key_modifier(hid_nkro_keyboard_report_t* report, uint8_t key, uint8_t* counter){
    //If modifier key
    if ( key >= 0xE0 ){
        report->modifier |= (1 << (key - 0xE0) ); 
    }
    else {
        report->boot_keys[*counter] = key;
        *counter++;
    } 
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
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
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
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
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
void cdc_task(void)
{
    // connected and there are data available
    if ( tud_cdc_available() )
    {
        // read datas
        char buf[64];
        uint32_t count = tud_cdc_read(buf, sizeof(buf));
        //for(uint32_t i = 0; i < count; i++) tud_cdc_write_char(buf[i]);
        tud_cdc_write(buf, count);
        tud_cdc_write_flush();
        (void) count;
    }
}
