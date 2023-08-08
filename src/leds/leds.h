#ifndef _LEDS_H_
#define _LEDS_H_

#define LOW 0
#define HIGH 55000


#define LED_COUNT 9

#define LED0 18
#define LED1 19
#define LED2 20
#define LED3 21
#define LED4 22
#define LED5 26
#define LED6 27
#define LED7 28
#define LED8 15

#define LEDN {LED0, LED1, LED2, LED3, LED4,\
              LED5, LED6, LED7, LED8}

/*
#define LEDN {LED0, LED1, LED2, LED3, LED4,\
              LED5}
*/
/*
//Real leds
#define LED_COUNT 10

#define LED0 2
#define LED1 4
#define LED2 6
#define LED3 13
#define LED4 15
#define LED5 17
#define LED6 19
#define LED7 21
#define LED8 23
#define LED9 25
#define LEDN {LED0, LED1, LED2, LED3, LED4,\
              LED5, LED6, LED7, LED8, LED9}
*/

enum {
    _simple_wave = 0,
    _breathing = 1,
    _on_off = 2,
    _double_wave = 3,
    E_MODE_COUNT,
};

#define MODE_COUNT E_MODE_COUNT
#define SPEED 400
#define INIT_DUTY 1

typedef struct {
    uint8_t pin;
    uint16_t duty;    
} led_t;

typedef struct {
    led_t leds[LED_COUNT];
    uint8_t mode;
    bool increasing;
    uint16_t time_counter;
    uint16_t led_counter;
    bool duty_assigned;
} led_array_t;

led_array_t led_array_init(void);

void led_array_set_levels(led_array_t* led_array);

void led_array_update_mode(led_array_t* led_array, uint8_t mode);

void led_array_update_values(led_array_t* led_array, uint64_t* delay_value);

#endif /* _LEDS_H_ */