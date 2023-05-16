#include <pigpio.h>
#include <stdlib.h>

#include "../include/cbdef.h"
#include "../include/encoder.h"

#include "timespec.h"

cbEncoder_t cbEncoderLeft = {PIN_ENCODER_LEFT_A, PIN_ENCODER_LEFT_B, GPIO_PIN_NC, 0, 0, 0};
cbEncoder_t cbEncoderRight = {PIN_ENCODER_RIGHT_A, PIN_ENCODER_RIGHT_B, GPIO_PIN_NC, 0, 0, 0};

void init() {
    if (gpioInitialise() < 0) exit(EXIT_FAILURE);
    // Left
    cbEncoderGPIOinit(&cbEncoderLeft);
    cbEncoderRegisterISRs(&cbEncoderLeft, 50);
    // Right
    cbEncoderGPIOinit(&cbEncoderRight);
    cbEncoderRegisterISRs(&cbEncoderRight, 50);
}

void terminate() {
    cbEncoderCancelISRs(&cbEncoderLeft);
    cbEncoderCancelISRs(&cbEncoderRight);
    gpioTerminate();
}

void sleep(int ms) {
    timespec_t clock;
    nsec_t delta = 0;
    tsSet(clock);
    while(delta < (ms * NSEC_PER_MSEC)) {
        delta += tsTickNs(clock);
    }
}

void printEncoderData(const cbEncoder_t* l, const cbEncoder_t* r) {
    printf("          L         R\nD %10d%10d\nT %10d%10d\nE %10d%10d\n\n",
        l->direction, r->direction,
        l->ticks, r->ticks,
        l->bad_ticks, r->bad_ticks
    );
}

/*
    L       R
D   1       
T
E

*/

int main(void) {
    init();
    atexit(terminate);
    int delta_ms = 500;
    printf("Every %dms:\n", delta_ms);
    for(int i = 0; i < 20; i++) {
        printEncoderData(&cbEncoderLeft, &cbEncoderRight);
        sleep(delta_ms);
    }
    exit(EXIT_SUCCESS);
}
