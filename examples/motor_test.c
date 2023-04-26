#include <pigpio.h>
#include <stdlib.h>

#include "../include/cbdef.h"
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
    cbMotorReset(&cbMotorLeft);
    cbMotorReset(&cbMotorRight);
    gpioTerminate();
}

void sleep(timespec_t* ts, int ms) {
    HTime_GetNs(ts);
    while(HTime_GetNsDelta(ts) < (ms * NSEC_PER_MSEC)) {
        HTime_GetNs(ts);
    }
    HTime_InitBase();
}

typedef struct {
    cbDir_t left, right;
} pattern_t;

pattern_t patterns[4] = {{forward, forward}, {forward, backward}, {backward, backward}, {backward, forward}};

int main(void) {
    init();
    atexit(terminate);
    int delta_ms = 5000;
    printf("Every %dms:\n", delta_ms);
    timespec_t tick;
    HTime_InitBase();
    int pat_idx = 0;
    for(int i = 0; i < 4; i++) {
        printf("%d:%d:%d\n", pat_idx, 
                             cbMotorMove(&cbMotorLeft, patterns[pat_idx].left, 0.5), 
                             cbMotorMove(&cbMotorRight, patterns[pat_idx].right, 0.5));
        pat_idx = (++pat_idx) % 4;
        sleep(&tick, delta_ms);
    }
    exit(EXIT_SUCCESS);
}
