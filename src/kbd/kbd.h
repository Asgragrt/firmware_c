#ifndef _KBD_H_
#define _KBD_H_

#define pin_count 9

#define K0 0
#define K1 1
#define K2 2
#define K3 3
#define K4 7
#define K5 8
#define K6 4
#define K7 5
#define K8 6
#define KN {K0, K1, K2, K3, K4, K5, K6, K7, K8}

#define kc_s(...) __VA_ARGS__, 0


//Max 4 keys, else unexpected behavior
#define K0_keys kc_s(HID_KEY_S)
#define K1_keys kc_s(HID_KEY_D)
#define K2_keys kc_s(HID_KEY_F)
#define K3_keys kc_s(HID_KEY_SPACE)
#define K4_keys kc_s(HID_KEY_CONTROL_LEFT, HID_KEY_O)
#define K5_keys kc_s(HID_KEY_F1)
#define K6_keys kc_s(HID_KEY_J)
#define K7_keys kc_s(HID_KEY_K)
#define K8_keys kc_s(HID_KEY_L)
#define KN_keys {K0_keys, K1_keys, K2_keys, K3_keys, \
                 K4_keys, K5_keys, K6_keys, K7_keys, \
                 K8_keys}


typedef struct {
    uint8_t pin; 
    uint8_t key_count;
    uint8_t keys[4];
} keyboard_pin;

typedef struct {
    keyboard_pin pins[pin_count];
    uint16_t status;
} keyboard;

keyboard kbd_new(void);

void kbd_init(keyboard* kbd);

bool update_status(keyboard* kbd);

bool update_buffer(keyboard* kbd, uint8_t* buffer, uint8_t buflength);

#endif /* _KBD_H_ */