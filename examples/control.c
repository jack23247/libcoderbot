#include <math.h>
#include <pigpio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../include/cbdef.h"
#include "../include/motor.h"
#include "../include/encoder.h"
#include "timespec.h"

#define PID_RESPONSE_MEDIUM

#ifdef PID_RESPONSE_SOFT
#define KP 0.04   // proportional coefficient
#define KD 0.02   // derivative coefficient
#define KI 0.005  // integral coefficient
#endif

#ifdef PID_RESPONSE_MEDIUM
#define KP 0.4   // proportional coefficient
#define KD 0.1   // derivative coefficient
#define KI 0.02  // integral coefficient
#endif

#ifdef PID_RESPONSE_STRONG
#define KP 0.9   // proportional coefficient
#define KD 0.05  // derivative coefficient
#define KI 0.03  // integral coefficient
#endif

// PID parameters
#define PI_INTERVAL_MSEC 20.0f // 50Hz
#define ENC_DIST_PER_TICK_MM 0.06

cbMotor_t cbMotorLeft = {PIN_LEFT_FORWARD, PIN_LEFT_BACKWARD, forward};
cbMotor_t cbMotorRight = {PIN_RIGHT_FORWARD, PIN_RIGHT_BACKWARD, forward};
cbEncoder_t cbEncoderLeft = {
    PIN_ENCODER_LEFT_A, PIN_ENCODER_LEFT_B, GPIO_PIN_NC, 0, 0, 0};
cbEncoder_t cbEncoderRight = {
    PIN_ENCODER_RIGHT_A, PIN_ENCODER_RIGHT_B, GPIO_PIN_NC, 0, 0, 0};

void init() {
    if (gpioInitialise() < 0) exit(EXIT_FAILURE);
    // Left
    cbMotorGPIOinit(&cbMotorLeft);
    cbEncoderGPIOinit(&cbEncoderLeft);
    cbEncoderRegisterISRs(&cbEncoderLeft, 50);
    // Right
    cbMotorGPIOinit(&cbMotorRight);
    cbEncoderGPIOinit(&cbEncoderRight);
    cbEncoderRegisterISRs(&cbEncoderRight, 50);
}

void terminate() {
    cbMotorReset(&cbMotorLeft);
    cbMotorReset(&cbMotorRight);
    cbEncoderCancelISRs(&cbEncoderLeft);
    cbEncoderCancelISRs(&cbEncoderRight);
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

void printEncoderData(const cbEncoder_t* l, const cbEncoder_t* r) {
    printf("          L         R\nD %10d%10d\nT %10d%10d\nE %10d%10d\n\n",
           l->direction, r->direction, l->ticks, r->ticks, l->bad_ticks,
           r->bad_ticks);
}

void control() {
    int distFromGoal_mm = 115;
    int prevLeftTicks = 0;
    int prevRightTicks = 0;
    int leftError, rightError, leftIntegralError = 0, rightIntegralError = 0;

    float targetSpeedLeft_mm_s = 5.0;
    float targetSpeedRight_mm_s = 5.0;

    // Assume that the initial duty cycles are equal and that coderbot
    // moves at 11.5mm/s at full PWM duty cycle, it should reach its goal after
    // 10 sec
    float leftDutyCyc = 1.0;
    float rightDutyCyc = 1.0;

    cbMotorMove(&cbMotorLeft, forward, leftDutyCyc);
    cbMotorMove(&cbMotorRight, forward, rightDutyCyc);

    while (distFromGoal_mm > 0) {
        sleep(PI_INTERVAL_MSEC);
        int leftTicks = cbEncoderLeft.ticks;
        int rightTicks = cbEncoderRight.ticks;
        float leftCorr, rightCorr;
        int distLeft_mm = ENC_DIST_PER_TICK_MM * (leftTicks - prevLeftTicks);
        {
            prevLeftTicks = leftTicks;
            float speedLeft_mm_s = distLeft_mm / (PI_INTERVAL_MSEC * MSEC_PER_SEC);
            leftError = (targetSpeedLeft_mm_s - speedLeft_mm_s) / targetSpeedLeft_mm_s * 100.0;
            leftCorr = (leftErr * KP) + (leftIntegralError * KI);
            leftIntegralError += leftError;
        }
        int distRight_mm = ENC_DIST_PER_TICK_MM * (rightTicks - prevRightTicks);
        {
            prevRightTicks = rightTicks;
            float speedRight_mm_s = distRight_mm / (PI_INTERVAL_MSEC * MSEC_PER_SEC);
            leftError = (targetSpeedRight_mm_s - speedRight_mm_s) / targetSpeedRight_mm_s * 100.0;
            rightCorr = (rightErr * KP) + (rightIntegralError * KI);
            rightIntegralError += rightError;
        }
        distFromGoal_mm -= (distLeft_mm + distRight_mm) / 2;
        cbMotorMove(&cbMotorLeft, forward, leftDutyCyc + leftCorr - rightCorr);
        cbMotorMove(&cbMotorLeft, forward, rightDutyCyc + rightCorr - leftCorr);
    }
}

int main(void) {
    init();
    atexit(terminate);
    control();
    exit(EXIT_SUCCESS);
}
