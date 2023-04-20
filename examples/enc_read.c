#include <pigpio.h>
#include <stdlib.h>

#include "../include/gpio.h"
#include "../include/encoder.h"

#include "h_time.h"

cbEncoder_t cbEncoderLeft = {PIN_ENCODER_LEFT_A, PIN_ENCODER_LEFT_B, GPIO_PIN_NC, 0, 0, 0};
cbEncoder_t cbEncoderRight = {PIN_ENCODER_RIGHT_A, PIN_ENCODER_RIGHT_B, GPIO_PIN_NC, 0, 0, 0};

void init() {
    if (gpioInitialise() < 0) exit(EXIT_FAILURE);
    // Left
    cbEncoderGPIOinit(&cbEncoderLeft);
    cbEncoderRegisterISRs(&cbEncoderLeft, PIN_ENCODER_LEFT_A, PIN_ENCODER_LEFT_B, 50);
    // Right
    cbEncoderGPIOinit(&cbEncoderRight);
    cbEncoderRegisterISRs(&cbEncoderRight, PIN_ENCODER_RIGHT_A, PIN_ENCODER_RIGHT_B, 50);
}

void terminate() {
    cbEncoderCancelISRs(&cbEncoderLeft);
    cbEncoderCancelISRs(&cbEncoderRight);
    gpioTerminate();
}

int main(void) {
    init();
    atexit(terminate);
    int delta_ms = 500;
    printf("Every %dms:\n", delta_ms);
    timespec_t tick;
    HTime_InitBase();
    HTime_GetNs(&tick);
    for(int i = 0; i < 100; i++) {
        while(HTime_GetNsDelta(&tick) < (500 * NSEC_PER_MSEC)) {
            HTime_GetNs(&tick);
        }
        printf("L:%d, R:%d\n", cbEncoderLeft.direction, cbEncoderRight.direction);
        HTime_InitBase();
    }
    exit(EXIT_SUCCESS);
}
