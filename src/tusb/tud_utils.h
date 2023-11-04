#ifndef _TUD_UTILS_H_
#define _TUD_UTILS_H_

#include "../kbd/kbd.h"

#define BITMAP_BYTE_SIZE 20

#define POLLING_INTERVAL 1 //ms

#define TUD_HID_REPORT_DESC_NKRO_KEYBOARD() \
    HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP                         ),\
    HID_USAGE      ( HID_USAGE_DESKTOP_KEYBOARD                     ),\
    HID_COLLECTION ( HID_COLLECTION_APPLICATION                     ),\
    HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD                        ),\
        HID_USAGE_MIN    ( 224                                      ),\
        HID_USAGE_MAX    ( 231                                      ),\
        HID_LOGICAL_MIN  ( 0                                        ),\
        HID_LOGICAL_MAX  ( 1                                        ),\
        HID_REPORT_COUNT ( 8                                        ),\
        HID_REPORT_SIZE  ( 1                                        ),\
        HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE   ),\
    /* 7 bytes of padding for boot support (1 Reserved - 6 for 6kro) */ \
    HID_REPORT_SIZE        ( 8                                      ),\
    HID_REPORT_COUNT       ( 1 + 6                                  ),\
    HID_INPUT              ( HID_CONSTANT                           ),\
    /*LED output report*/ \
    HID_USAGE_PAGE ( HID_USAGE_PAGE_LED                             ),\
        HID_REPORT_SIZE    ( 1                                      ),\
        HID_REPORT_COUNT   ( 5                                      ),\
        HID_USAGE_MIN      ( 1                                      ),\
        HID_USAGE_MAX      ( 5                                      ),\
        HID_OUTPUT         ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),\
    /*LED 3 bit padding*/ \
    HID_REPORT_SIZE        ( 3                                      ),\
    HID_REPORT_COUNT       ( 1                                      ),\
    HID_OUTPUT             ( HID_CONSTANT                           ),\
    /*bitmap of keys (32 bytes)*/\
    HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD                        ),\
        HID_REPORT_SIZE    ( 1                                      ),\
        HID_REPORT_COUNT_N ( BITMAP_BYTE_SIZE * 8, 2                ),\
        HID_LOGICAL_MIN    ( 0                                      ),\
        HID_LOGICAL_MAX    ( 1                                      ),\
        HID_USAGE_MIN      ( 0                                      ),\
        HID_USAGE_MAX      ( BITMAP_BYTE_SIZE * 8 - 1               ),\
        HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),\
    HID_COLLECTION_END\

typedef struct TU_ATTR_PACKED{
    uint8_t modifier;   /**< Keyboard modifier (KEYBOARD_MODIFIER_* masks). */
    uint8_t reserved;   /**< Reserved for OEM use, always set to 0. */
    uint8_t boot_keys[6];
    uint8_t key_bitmap[BITMAP_BYTE_SIZE];
} hid_nkro_keyboard_report_t;

void tud_mount_cb(void);
void tud_umount_cb(void);
void tud_suspend_cb(bool remote_wakeup_en);
void tud_resume_cb(void);

#define keycode_buffer 20

void led_blinking_task(void);
void hid_task(keyboard_t* kbd);
bool tud_hid_nkro_keyboard_report(uint8_t report_id, uint8_t keycode[keycode_buffer]);
bool boot_key_modifier(hid_nkro_keyboard_report_t* report, uint8_t key, uint8_t* counter);
void cdc_task(keyboard_t* kbd, bool reboot, bool* write, uint8_t* write_buff);

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen);
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize);

#endif /* _TUD_UTILS_H_ */