/**
 * @file control.c
 * @author Jacopo Maltagliati
 * @date 19 Mag 2023
 * @brief Trivial P-I controller for the CoderBot platform.
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

#include <pigpio.h>
#include <stdlib.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "../include/cbdef.h"
#include "../include/motor.h"
#include "../include/encoder.h"
#include "timespec.h"

/* PID PARAMETERS ---------------------------------------------------------- */

#define KP 0.005
#define KI 0.0005

#define PI_INTERVAL_MSEC 20 // 50Hz

#define LEFT_WHEEL_RAY_MM 33.f
#define RIGHT_WHEEL_RAY_MM 33.f
// TODO Oscilloscopio alla mano, come funziona sta roba?
#define TICKS_PER_REVOLUTION 16 //< Ticks per motor revolution
#define TRANSMISSION_RATIO 120

#define PWM_CLAMPING_EVENTS_MAX 10 //< Clamping events after which the
                                   //  controller should yield.

/* TYPEDEFS ---------------------------------------------------------------- */

/**
 * @brief A structure holding the parameters for the P-I controller.
 */
typedef struct {
    int ticks, //< The ticks read from the encoder.
        prevTicks; //< The ticks read the last time the structure was updated.
    float dutyCyc, //< The duty cycle of the motor in percentage.
          travel_mm, //< The distance the wheel has traveled in mm.
          speed_mm_s, //< The speed of the wheel in mm.
          targetSpeed_mm_s, //< The desired speed in mm/s.
          error_mm_s, //< The error in speed in mm/s.
          integralError_mm_s, //< The sum of the errors in mm/s.
          controlAction; //< The correction factor in duty cycle percentage.
    const float mmsPerTick; //< The distance traveled by the wheel per each
                            //  tick in mm.
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

/**
 * @brief Updates the Controller parameters for one of the sides.
 *
 * @param p The structure containing the parameters of the Controller at the
 *          current iteration.
 */
void update(ctrlParams_t* p) {
    p->travel_mm = (p->ticks - p->prevTicks) * p->mmsPerTick;
    p->prevTicks = p->ticks;
    p->speed_mm_s = (p->travel_mm / PI_INTERVAL_MSEC) * MSEC_PER_SEC;
    /* TODO
     * This only works if you go forward! The controller does not account for
     * the direction in which the wheel is supposed to rotate.
     */
    p->error_mm_s = (p->targetSpeed_mm_s - p->speed_mm_s);
    p->controlAction = (p->error_mm_s * KP) + (p->integralError_mm_s * KI);
    p->integralError_mm_s += p->error_mm_s;
}

/**
 * @brief Clamps the controlAction to a value in the range (0,1]. To avoid
 * overloading the motors in the event of a lockup, the control loop
 * is terminated after a number of Clamping events.
 *
 * @param p The structure containing the parameters of the Controller at the
 *          current iteration.
 */
int clamp(ctrlParams_t* p) {
    // Quando il motore è in STALLO, passa il massimo della corrente... questo
    // può essere problematico! Il cavo infatti si scalda, e gli avvolgimenti
    // sul motore si scaldano e la cosa può portare alla rottura dello smalto
    // e alla conseguente rottura del motore.
    static unsigned int events = 0;
    if(p->controlAction > 1.f) {
        p->controlAction = 1.f;
	    events++;
    } else if(p->controlAction <= 0.f) {
	    p->controlAction = 0.1f;
	    events++;
    }
    return(events > PWM_CLAMPING_EVENTS_MAX);
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
void control(float distFromGoal_mm, float targetSpeed_mm_s_L,
             float targetSpeed_mm_s_R, float dutyCyc_L, float dutyCyc_R) {

    ctrlParams_t left = {
        .ticks = 0,
        .prevTicks = 0,
        .dutyCyc = dutyCyc_L,
        .travel_mm = 0.f,
        .speed_mm_s = 0.f,
        .targetSpeed_mm_s = targetSpeed_mm_s_L,
        .error_mm_s = 0,
        .integralError_mm_s = 0,
        .controlAction = 0,
        .mmsPerTick = (LEFT_WHEEL_RAY_MM * 2 * M_PI) /
                      (TICKS_PER_REVOLUTION * TRANSMISSION_RATIO)
    };

    ctrlParams_t right = {
        .ticks = 0,
        .prevTicks = 0,
        .dutyCyc = dutyCyc_R,
        .travel_mm = 0.f,
        .speed_mm_s = 0.f,
        .targetSpeed_mm_s = targetSpeed_mm_s_R,
        .error_mm_s = 0,
        .integralError_mm_s = 0,
        .controlAction = 0,
        .mmsPerTick = (RIGHT_WHEEL_RAY_MM * 2 * M_PI) /
                      (TICKS_PER_REVOLUTION * TRANSMISSION_RATIO)
    };

    cbMotorMove(&cbMotorLeft, forward, left.dutyCyc);
    cbMotorMove(&cbMotorRight, forward, right.dutyCyc);

    while (distFromGoal_mm > 0.f) {
        /* Copying the ticks should be done as quickly as possible to avoid an
         * ISR interrupt from happening in between the copies. Consider
         * disabling the encoder callback temporarily if problems arise.
         */
        left.ticks = cbEncoderLeft.ticks;
        right.ticks = cbEncoderRight.ticks;
        //printf("dT_L: %d, dT_R: %d\n", left.ticks - left.prevTicks, right.ticks - right.prevTicks);
        update(&left);
        update(&right);
    	//printf("t_L: %f, t_R: %f\n", left.travel_mm, right.travel_mm);
        //printf("dFG: %f, cA_L: %f, cA_R: %f\n", distFromGoal_mm, left.controlAction, right.controlAction);
	    //printf("tS: %f, cS_L: %f, cS_L: %f\n", targetSpeed_mm_s_L, left.speed_mm_s, right.speed_mm_s);
        if(clamp(&left)) return;
        if(clamp(&right)) return;
        cbMotorMove(&cbMotorLeft, forward, left.controlAction);
       	cbMotorMove(&cbMotorRight, forward, right.controlAction);
        distFromGoal_mm -= (left.travel_mm + right.travel_mm) / 2;
        sleep(PI_INTERVAL_MSEC);
    }
}

int main(void) {
    init();
    atexit(terminate);
    control(500.f, 50.0f, 50.0f, .75f, .75f);
    exit(EXIT_SUCCESS);
}
