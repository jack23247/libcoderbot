/**
 * @file control.c
 * @author Jacopo Maltagliati
 * @date 19 Mag 2023
 * @brief Trivial P-D controller for the CoderBot platform.
 * @copyright Copyright (c) 2022-23, Jacopo Maltagliati.
 * 
 * This file is part of libcoderbot.
 * 
 * libcoderbot is free software: you can redistribute it and/or modify it under 
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 * 
 * libcoderbot is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for 
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * libcoderbot. If not, see <https://www.gnu.org/licenses/>.
 */

/* TODOs:
 * - Separare raggi da altri componenti costante
 * - Costanti separate LEFT e RIGHT
 * - ~~Togliere denominatore calcolo errore~~ Fatto
 */

#include <pigpio.h>
#include <stdlib.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "../include/cbdef.h"
#include "../include/motor.h"
#include "../include/encoder.h"
#include "timespec.h"

/* PID PARAMETERS ---------------------------------------------------------- */

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

#define PI_INTERVAL_MSEC 20 // 50Hz
#define ENC_DIST_PER_TICK_MM 0.14 //0.06
#define LEFT_WHEEL_RAY 32.f
#define RIGHT_WHEEL_RAY 32.f

/* TYPEDEFS ---------------------------------------------------------------- */

/**
 * @brief A structure holding the parameters for the P-I controller.
 */
typedef struct {
    int ticks, //< The ticks read from the encoder.
        prevTicks; //< The ticks read the previous time the structure was updated.
    float dutyCyc, //< The duty cycle of the motor in percentage. // MAYBE const?
          travel_mm, //< The distance the wheel has traveled in mm.
          speed_mm_s, //< The speed of the wheel in mm.
          targetSpeed_mm_s, //< The desired speed in mm/s.
          error_mm_s, //< The error in speed in mm/s.
          integralError_mm_s, //< The sum of the errors in mm/s.
          correction; //< The correction factor in duty cycle percentage.
    const float mmsPerTick; //< The distance traveled by the wheel per each tick in mm.
} ctrlParams_t;

/* GLOBALS ----------------------------------------------------------------- */

cbMotor_t cbMotorLeft = {PIN_LEFT_FORWARD, PIN_LEFT_BACKWARD, forward};
cbMotor_t cbMotorRight = {PIN_RIGHT_FORWARD, PIN_RIGHT_BACKWARD, forward};
cbEncoder_t cbEncoderLeft = {
    PIN_ENCODER_LEFT_A, PIN_ENCODER_LEFT_B, GPIO_PIN_NC, 0, 0, 0};
cbEncoder_t cbEncoderRight = {
    PIN_ENCODER_RIGHT_A, PIN_ENCODER_RIGHT_B, GPIO_PIN_NC, 0, 0, 0};

/* FUNCTIONS --------------------------------------------------------------- */

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

void update(ctrlParams_t* p) {
    //p->ticks = cbEncoderLeft.ticks;
    p->travel_mm = (p->ticks - p->prevTicks) * p->mmsPerTick;
    p->prevTicks = p->ticks;
    p->speed_mm_s = p->travel_mm / (PI_INTERVAL_MSEC * MSEC_PER_SEC);
    p->error_mm_s = (p->targetSpeed_mm_s - p->speed_mm_s); // / targetSpeed[L]_mm_s * 100.0;
    p->correction = (p->error_mm_s * KP) + (p->integralError_mm_s * KI);
    p->integralError_mm_s += p->error_mm_s;
}

/**
 * @brief Trivial Proportional-Integral controller for the CoderBot Platform.
 * 
 * @param distFromGoal_mm The distance from the goal in millimiters.
 * @param targetSpeed_mm_s_L The target speed of the left wheel in mm/s.
 * @param targetSpeed_mm_s_R The target speed of the right wheel in mm/s.
 * @param dutyCycL The initial duty cycle of the left motor.
 * @param dutyCycR The initial duty cycle of the right motor.
 */
void control(float distFromGoal_mm, float targetSpeed_mm_s_L, float targetSpeed_mm_s_R, float dutyCycL, float dutyCycR) {

    ctrlParams_t left = {
        .ticks = 0, 
        .prevTicks = 0, 
        .dutyCyc = dutyCycL, 
        .travel_mm = 0.f, 
        .speed_mm_s = 0.f,
        .targetSpeed_mm_s = targetSpeed_mm_s_L,
        .error_mm_s = 0,
        .integralError_mm_s = 0,
        .correction = 0,
        .mmsPerTick = (LEFT_WHEEL_RAY * 2 * M_PI) / 32 // TODO da rivedere
    };

    ctrlParams_t right = {
        .ticks = 0, 
        .prevTicks = 0, 
        .dutyCyc = dutyCycR, 
        .travel_mm = 0.f, 
        .speed_mm_s = 0.f,
        .targetSpeed_mm_s = targetSpeed_mm_s_R,
        .error_mm_s = 0,
        .integralError_mm_s = 0,
        .correction = 0,
        .mmsPerTick = (RIGHT_WHEEL_RAY * 2 * M_PI) / 32 // RIGHT_WHEEL_RAY * (360/16) // TODO da rivedere
    };

    cbMotorMove(&cbMotorLeft, forward, left.dutyCyc);
    cbMotorMove(&cbMotorRight, forward, right.dutyCyc);

    while (distFromGoal_mm > 0.f) {
        /* XXX
         * Copying the ticks should be done as quickly as possible to avoid an 
         * ISR interrupt from happening in between the copies. Consider 
         * disabling the encoder callback temporarily if problems arise. 
         */
        left.ticks = cbEncoderLeft.ticks;
        right.ticks = cbEncoderRight.ticks;
        update(&left);
        update(&right);
        //if(left.speed_mm_s > 1.f && right.speed_mm_s > 1.f) {
            cbMotorMove(&cbMotorLeft, forward, left.dutyCyc + left.correction - right.correction);
            cbMotorMove(&cbMotorLeft, forward, right.dutyCyc + right.correction - left.correction);
        //}
        distFromGoal_mm -= (left.travel_mm + right.travel_mm) / 2;
        sleep(PI_INTERVAL_MSEC);
    }
}

int main(void) {
    init();
    atexit(terminate);
    control(500.f, 10.0f, 10.0f, 1.0, 1.0);
    exit(EXIT_SUCCESS);
}
