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

int main(void) {
    init();
    atexit(terminate);
    printf("Killing the motors.\n");
    exit(EXIT_SUCCESS);
}
