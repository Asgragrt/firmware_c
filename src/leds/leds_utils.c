#include "pico/stdlib.h"

int16_t abs_m(int16_t val){
    return val * ((val > 0) - (val < 0));
}

uint16_t linear(uint16_t actual, uint16_t target, uint16_t dif){
    float m = (float)(target - actual) / dif;
    return (uint16_t)(m + actual);
}

// TODO verify datatype
float breath_approx(uint8_t x){
    if ( x == 0 ){
        return 1.0;
    }
    else if ( x == 1 ){
        return 0.3;
    }
    else if ( x == 2 ){
        return 0.05;
    }
    else {
        return 0.0;
    }
}