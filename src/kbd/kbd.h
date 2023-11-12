#ifndef _KBD_H_
#define _KBD_H_

#define PIN_COUNT 9

#define DEBOUNCE_TIME 0x3FF // 10 ms

/*
#define K0 0
#define K1 1
#define K2 2
#define K3 3
#define K4 7
#define K5 8
#define K6 4
#define K7 5
#define K8 6
/*/
#define K0 1
#define K1 3
#define K2 5
#define K3 12
#define K4 14
#define K5 16
#define K6 20
#define K7 22
#define K8 24

#define KN {K0, K1, K2, K3, K6, K7, K8, K4, K5}

#define kc_s(...) __VA_ARGS__, 0


//Max 4 keys, else unexpected behavior
//Adding a 0 at the end to mark key binding endings
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
                 K6_keys, K7_keys, K8_keys, K4_keys, \
                 K5_keys}

typedef struct {
    uint8_t pin; 
    uint8_t key_count;
    uint8_t keys[4];
    //uint8_t alternate_keys[1];
    uint16_t debounce;
} keyboard_pin_t;

typedef struct {
    keyboard_pin_t pins[PIN_COUNT];
    uint16_t status;
    //bool alternate_key;
    uint16_t counter;
    bool start_counter;
} keyboard_t;

keyboard_t* keyboard_new(void);

void keyboard_init(keyboard_t* kbd);

bool keyboard_update_status(keyboard_t* kbd);

bool keyboard_update_buffer(keyboard_t* kbd, uint8_t* buffer, uint8_t buflength);

bool keyboard_update_key(keyboard_t* kbd);

#endif /* _KBD_H_ */