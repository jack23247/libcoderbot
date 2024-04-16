#include <pigpio.h>
#include <stdlib.h>

#include "../include/cbdef.h"
#include "../include/motor.h"

#include "timespec.h"

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

void sleep(int ms) {
    timespec_t clock;
    nsec_t delta = 0;
    tsSet(&clock);
    while(delta < (ms * NSEC_PER_MSEC)) {
        delta += tsTickNs(&clock);
    }
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
    int pat_idx = 0;
    for(int i = 0; i < 4; i++) {
        printf("%d:%d:%d\n", pat_idx,
                             cbMotorMove(&cbMotorLeft, patterns[pat_idx].left, 0.5),
                             cbMotorMove(&cbMotorRight, patterns[pat_idx].right, 0.5));
        pat_idx = (++pat_idx) % 4;
        sleep(delta_ms);
    }
    exit(EXIT_SUCCESS);
}
