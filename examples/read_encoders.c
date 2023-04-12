#include <pigpio.h>
#include <stdlib.h>

#include "../include/gpio.h"
#include "../include/encoder.h"

#include "h_time.h"

cbEncoder_t cbEncoderLeft = {GPIO_PIN_NC, 0, 0, 0};
cbEncoder_t cbEncoderRight = {GPIO_PIN_NC, 0, 0, 0};

void init() {
    if (gpioInitialise() < 0) exit(EXIT_FAILURE);
    // Left
    cbEncoderGPIOinit(PIN_ENCODER_LEFT_A, PIN_ENCODER_LEFT_B);
    cbEncoderRegisterISRs(&cbEncoderLeft, PIN_ENCODER_LEFT_A, PIN_ENCODER_LEFT_B, 50);
    // Right
    cbEncoderGPIOinit(PIN_ENCODER_RIGHT_A, PIN_ENCODER_RIGHT_B);
    cbEncoderRegisterISRs(&cbEncoderRight, PIN_ENCODER_RIGHT_A, PIN_ENCODER_RIGHT_B, 50);
}

void terminate() {
    cbEncoderCancelISRs(PIN_ENCODER_LEFT_A, PIN_ENCODER_LEFT_B);
    cbEncoderCancelISRs(PIN_ENCODER_RIGHT_A, PIN_ENCODER_RIGHT_B);
    gpioTerminate();
    exit(EXIT_SUCCESS);
}
int main(void) {
    init();
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
    terminate();
    exit(EXIT_SUCCESS);
}