#ifndef _LEDS_UTILS_H_
#define _LEDS_UTILS_H_

int16_t abs_m(int16_t val);

uint16_t linear(uint16_t actual, uint16_t target, uint16_t dif);

float breath_approx(uint8_t x);

#endif /* _LEDS_UTILS_H_ */