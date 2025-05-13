#ifndef GPIO_H
#define GPIO_H

#include "driver/gpio.h"

#define BUTTON GPIO_NUM_19
#define LED GPIO_NUM_23
#define M0 GPIO_NUM_18
#define M1 GPIO_NUM_5
#define AUX GPIO_NUM_4
#define PININ_MAP ((1ULL<<BUTTON) | (1ULL<<AUX))
#define PINOUT_MAP ((1ULL<<M0) | (1ULL<<M1) | (1ULL<<LED))

void gpio_init(void);

#endif // GPIO_H