#include <pigpio.h>
#include <stdlib.h>

#include "../include/gpio.h"
#include "../include/motor.h"

#include "h_time.h"

cbMotor_t cbMotorLeft = {PIN_LEFT_FORWARD, PIN_LEFT_BACKWARD, forward};
cbMotor_t cbMotorRight = {PIN_RIGHT_FORWARD, PIN_RIGHT_BACKWARD, forward};

void init() {
    if (gpioInitialise() < 0) exit(EXIT_FAILURE);
    cbMotorGPIOinit(&cbMotorLeft);
    cbMotorGPIOinit(&cbMotorRight);
}

void terminate() {
    gpioTerminate();
}

typedef struct {
    cbDir_t left, right;
} pattern_t

pattern_t patterns[4] = {{forward, forward}, {forward, backward}, {backward, backward}, {backward, forward}};

int main(void) {
    init();
    atexit(terminate);
    int delta_ms = 500;
    printf("Every %dms:\n", delta_ms);
    timespec_t tick;
    HTime_InitBase();
    HTime_GetNs(&tick);
    int pat_idx = 0;
    for(int i = 0; i < 100; i++) {
        while(HTime_GetNsDelta(&tick) < (500 * NSEC_PER_MSEC)) {
            HTime_GetNs(&tick);
        }
        cbMotorMove(&cbMotorLeft, patterns[pat_idx]->left, 100-i);
        cbMotorMove(&cbMotorRight, patterns[pat_idx]->right, 100-i);
        ++pat_idx % 4;
        HTime_InitBase();
    }
    exit(EXIT_SUCCESS);
}
