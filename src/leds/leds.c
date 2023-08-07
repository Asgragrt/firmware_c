#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "leds.h"
#include "leds_utils.h"

void set_duty(led_t* led, uint16_t duty){
    led->duty = duty;
    pwm_set_gpio_level(led->pin, duty);
}

led_array_t led_array_init(void){
    uint8_t leds[LED_COUNT] = LEDN;

    led_array_t led_array = {
        .mode = 0,
        .increasing = true,
        .time_counter = 0,
        .led_counter = 0,
        .duty_assigned = false,
    };

    uint8_t pwm_slice_num;
    for(uint8_t i = 0; i < LED_COUNT; i++){
        led_t led = {
            .pin = leds[i],
            .duty = INIT_DUTY,
        };
        gpio_set_function(led.pin, GPIO_FUNC_PWM);
        pwm_slice_num = pwm_gpio_to_slice_num(led.pin);
        pwm_set_enabled(pwm_slice_num, true);
        
        //set_duty(&led, led.duty);
        led_array.leds[i] = led;        
    }

    return led_array;
}

void led_array_set_levels(led_array_t* led_array){
    if ( led_array->duty_assigned ) return;

    for(uint8_t i = 0; i < LED_COUNT; i++){
        pwm_set_gpio_level(led_array->leds[i].pin, led_array->leds[i].duty);
    }

    led_array->duty_assigned = true;
}

bool led_array_set_mode(led_array_t* led_array, uint8_t mode){
    uint8_t original_mode = led_array->mode;
    //default
    led_array->mode = 0;
    if ( mode < MODE_COUNT ){
        led_array->mode = mode;
    }

    return original_mode == led_array->mode;
}

uint16_t wave_led_pos(led_array_t* led_array, uint8_t current_led){
    int16_t dif = abs_m((int16_t)led_array->led_counter - (int16_t)current_led);
    
    // TODO add different cases depending on the mode
    return (uint16_t) MIN(dif, abs_m((LED_COUNT - dif)) );
}

void increase_timer(led_array_t* led_array){
    led_array->time_counter += 1;
    if ( led_array->time_counter == SPEED ){
        led_array->time_counter = 0;
        led_array->led_counter = (led_array->led_counter + 1) % LED_COUNT;
    }
}

//Updates the led_duty
void wave_duty(led_array_t* led_array, uint8_t current_led, uint16_t* led_duty){
    uint16_t led_pos = wave_led_pos(led_array, current_led);
    uint16_t target_duty = (uint16_t) ((float)HIGH * breath_approx(led_pos) );
    *led_duty = linear(*led_duty, target_duty, SPEED - led_array->time_counter);
}

uint64_t simple_wave(led_array_t* led_array){
    for(uint8_t i = 0; i < LED_COUNT; i++){
        wave_duty(led_array, i, &led_array->leds[i].duty);
    }
    increase_timer(led_array); 

    //Delay time in us
    return 400UL;    
}

uint64_t breathing(led_array_t* led_array){
    uint16_t duty = led_array->leds[0].duty;
    uint16_t diff = ( ( (uint16_t) ( ((float) duty) / 1500.0) ) << 1) + 1;

    led_array->increasing = led_array->increasing ^ (duty > HIGH || duty < LOW + 1);
    duty = duty + diff * led_array->increasing - diff * !led_array->increasing;

    for (uint8_t i = 0; i < LED_COUNT; i++){
        led_array->leds[i].duty = duty;
    }

    return 500ULL;
}

uint64_t on_off(led_array_t* led_array){
    led_array->increasing ^= true;
    uint16_t duty = HIGH * led_array->increasing;

    for (uint8_t i = 0; i < LED_COUNT; i++){
        led_array->leds[i].duty = duty;
    }

    return 500000ULL;
}

void led_array_update_mode(led_array_t* led_array, uint8_t mode){
    if ( led_array_set_mode(led_array, mode) ) return;

    // TODO update all needed variables
    switch (led_array->mode){
        //Breathing & on_off
        case _breathing: case _on_off:
            led_array->increasing = true;
            for(uint8_t i = 0; i < LED_COUNT; i++){
                led_array->leds[i].duty = INIT_DUTY;
            }
            break;
            
        //Simple wave
        case _simple_wave: default:
            led_array->led_counter = 0;
            led_array->time_counter = 0;
            break;
    } 
}

void led_array_update_values(led_array_t* led_array, uint64_t* delay_value){
    if ( !led_array->duty_assigned ) return;

    switch(led_array->mode){
        case _breathing:
            *delay_value = breathing(led_array);
            break;

        case _on_off:
            *delay_value = on_off(led_array);
            break;

        _simple_wave: default:
            *delay_value = simple_wave(led_array);
            break;

    }

    led_array->duty_assigned = false;
}

//pwm_set_gpio_level