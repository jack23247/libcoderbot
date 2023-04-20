/**
 * @file motor.c
 * @author Jacopo Maltagliati
 * @date 20 Apr 2023
 * @brief Library for interfacing with the CoderBot mobile platform.
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

#include "motor.h"

static const int MAX_DUTY_CYC = 255; //<< Paperino

void cbMotorGPIOinit(const cbMotor_t* motor) {
    gpioSetPWMrange(motor->pin_fw, MAX_DUTY_CYC);
    gpioSetPWMfrequency(motor->pin_fw, 100);
    gpioSetPWMrange(motor->pin_bw, MAX_DUTY_CYC);
    gpioSetPWMfrequency(motor->pin_bw, 100);
}

int cbMotorMove(cbMotor_t* motor, cbDir_t direction, float duty_cycle) {
    if(duty_cycle < .0f || duty_cycle > 1.0f)
        return 66; // Scaling out of range
    if(direction == 0) {
        direction = motor->direction; // Use same direction as before
    } else {
        motor->direction = direction;
    }
    int pwm = (int) (MAX_DUTY_CYC * duty_cycle);
    if(motor->direction == forward) {
        gpioPWM(motor->pin_fw, pwm);
    } else if(motor->direction == backward) {
        gpioPWM(motor->pin_bw, pwm);
    } else {
        return motor->direction; // No direction given
    }
    return 0;
}

void cbMotorReset(cbMotor_t* motor) {
    cbMotorMove(motor, forward, .0f);
}
