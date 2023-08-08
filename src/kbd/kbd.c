#include "tusb.h"
#include "pico/stdlib.h"

#include "kbd.h"
#include "flash/flash.h"

keyboard_t keyboard_new(void){
    uint8_t pins[PIN_COUNT] = KN;

    uint8_t keys[] = KN_keys;
    uint8_t keys_size = sizeof(keys) / sizeof(uint8_t);
    uint8_t current_idx = 0;

    keyboard_t kbd = {
        .status = 0,
    };

    for(uint8_t i = 0; i < PIN_COUNT; i++){
        keyboard_pin_t kp = {
            .pin = pins[i],
            .keys = {0},
        };
        //Maybe j < 4
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

uint8_t kbd_get_pin(keyboard_t* kbd, uint8_t idx){
    return kbd->pins[idx].pin;
}

uint8_t* kbd_get_keys(keyboard_t* kbd, uint8_t idx){
    return kbd->pins[idx].keys;
}

uint8_t kbd_get_key_count(keyboard_t* kbd, uint8_t idx){
    return kbd->pins[idx].key_count;
}

void keyboard_init(keyboard_t* kbd){
    uint8_t pin;
    for(uint8_t i = 0; i < PIN_COUNT; i++){
        pin = kbd_get_pin(kbd, i);
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);
    }
}

bool keyboard_update_status(keyboard_t* kbd){
    uint16_t status = kbd->status;
    uint16_t new_status = 0;
    for(uint8_t i = 0; i < PIN_COUNT; i++){
        if( !gpio_get(kbd_get_pin(kbd, i)) ){
            new_status |= 1 << i;
        }
    }

    kbd->status = new_status;
    return status != new_status;
}

bool keyboard_update_buffer(keyboard_t* kbd, uint8_t* buffer, uint8_t buflength){
    uint16_t status = kbd->status;
    uint16_t new_status = 0;

    uint8_t offset = 0;
    for (uint8_t i = 0; i < PIN_COUNT; i++){
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
// 05 01 69 01 00 23 26 FF
//    ^^ aquí empieza la escritura
// MODO LED 01
// MODIFICAR TECLAS 69
// MODIFICAR 01 teclas
// Tecla índice 00
// Código HID 23 (6)
// Código HID 26 (9)
// FIN de modificación de tecla

//Flipped keys 05 00 69 09 00 0F FF 01 0E FF 02 0D FF 03 3A FF 04 12 E0 FF 05 2C FF 06 09 FF 07 07 FF 08 16 FF
//                ^^
//Se ocupa mandar todo el código cuando se modifique una sola cosa
//Estructura: 05_ModoLed_69_CantidadDeTeclasAModificar_PosiciónTecla_CódigoHID0_CódigoHID1.._CódigoHID3_FF_...
bool keyboard_update_key(keyboard_t* kbd){
    if ( flash_target_contents[1] != 0x69 ) return false;

    uint8_t key_modification_count = flash_target_contents[2];

    
    uint8_t current_idx = 3;
    uint8_t key_pin;

    for ( uint8_t i = 0; i < key_modification_count; i++ ){
        key_pin = flash_target_contents[current_idx];
        if ( key_pin >= PIN_COUNT ) return false;

        for ( uint8_t j = current_idx + 1; ; j++ ){
            if ( flash_target_contents[j] == 0xFF ){
                kbd->pins[key_pin].key_count = j - current_idx - 1;
                current_idx = j + 1;
                break;
            }

            kbd->pins[key_pin].keys[j - current_idx - 1] = flash_target_contents[j];
        }
    }

    return true;
}