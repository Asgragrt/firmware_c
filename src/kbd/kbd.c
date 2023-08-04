#include "tusb.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "kbd.h"

keyboard kbd_new(void){
    uint8_t pins[pin_count] = KN;

    uint8_t keys[] = KN_keys;
    uint8_t keys_size = sizeof(keys) / sizeof(uint8_t);
    uint8_t current_idx = 0;

    keyboard kbd = {
        .status = 0,
    };

    for(uint8_t i = 0; i < pin_count; i++){
        keyboard_pin kp = {
            .pin = pins[i],
            .keys = {0},
        };
        for(uint8_t j = current_idx; j < keys_size; j++){
            if ( keys[j] == 0 ){
                kp.key_count = j - current_idx;
                current_idx = j + 1;
                break;
            }

            if ( j - current_idx > 3) break;

            kp.keys[j - current_idx] = keys[j];
        }

        kbd.pins[i] = kp;
    }

    return kbd;
}

uint8_t kbd_get_pin(keyboard* kbd, uint8_t idx){
    return kbd->pins[idx].pin;
}

uint8_t* kbd_get_keys(keyboard* kbd, uint8_t idx){
    return kbd->pins[idx].keys;
}

uint8_t kbd_get_key_count(keyboard* kbd, uint8_t idx){
    return kbd->pins[idx].key_count;
}

void kbd_init(keyboard* kbd){
    uint8_t pin;
    for(uint8_t i = 0; i < pin_count; i++){
        pin = kbd_get_pin(kbd, i);
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);
    }
}

bool update_status(keyboard* kbd){
    uint16_t status = kbd->status;
    uint16_t new_status = 0;
    for(uint8_t i = 0; i < pin_count; i++){
        if( !gpio_get(kbd_get_pin(kbd, i)) ){
            new_status |= 1 << i;
        }
    }

    kbd->status = new_status;
    return status != new_status;
}

bool update_buffer(keyboard* kbd, uint8_t* buffer, uint8_t buflength){
    uint16_t status = kbd->status;
    uint16_t new_status = 0;

    uint8_t offset = 0;
    for (uint8_t i = 0; i < pin_count; i++){
        if( !gpio_get(kbd_get_pin(kbd, i)) ){
            //if ( offset + kbd_get_key_count(kbd, i) >= buflength) break;
            memcpy(buffer + offset, kbd_get_keys(kbd, i), kbd_get_key_count(kbd, i));
            new_status |= 1 << i;
            offset += kbd->pins[i].key_count;
        }
    }

    kbd->status = new_status;
    return status != new_status;
}