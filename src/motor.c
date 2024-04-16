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

/**
 * The frequency of the soft-PWM thread. 
 * @link https://abyz.me.uk/rpi/pigpio/cif.html#gpioSetPWMfrequency
 */
#define PWM_FREQ 100 

/**
 * The scaled range of the PWM's Duty Cycle.
 * @link https://abyz.me.uk/rpi/pigpio/cif.html#gpioSetPWMrange
 */
#define MAX_DUTY_CYC 255

/**
 * @brief Initializes the GPIO pins used for driving a motor.
 * @param motor A pointer to the handle of the motor.
 */
void cbMotorGPIOinit(const cbMotor_t* motor) {
    // Fw
    gpioSetMode(motor->pin_fw, PI_OUTPUT);
    gpioSetPWMrange(motor->pin_fw, MAX_DUTY_CYC);
    gpioSetPWMfrequency(motor->pin_fw, PWM_FREQ);
    // Bw
    gpioSetMode(motor->pin_bw, PI_OUTPUT);
    gpioSetPWMrange(motor->pin_bw, MAX_DUTY_CYC);
    gpioSetPWMfrequency(motor->pin_bw, PWM_FREQ);
}

/**
 * @brief Moves a motor.
 * @param motor A pointer to the handle of the motor.
 * @param direction The direction in which to move the motor. If zero the 
 *                  direction of the motion is unchanged.
 * @param duty_cycle The duty cycle expressed in percentage in the range (0,1].
 * @return A condition code.
 */
int cbMotorMove(cbMotor_t* motor, cbDir_t direction, float duty_cycle) {
    // Check for the range of the Duty Cycle
    if(duty_cycle <= .0f || duty_cycle > 1.0f) return CB_ERANGE;
    int pwm = (int) (MAX_DUTY_CYC * duty_cycle);
    if(direction) motor->direction = direction;
    switch(motor->direction) {
        /* In order to move the motor you need to set one pin to 0 (Ground) and
         * apply a certain PWM signal to the other pin. 
         */
        case forward:
            gpioPWM(motor->pin_fw, pwm);
            gpioWrite(motor->pin_bw, 0);
            break;
        case backward:
            gpioWrite(motor->pin_fw, 0);
            gpioPWM(motor->pin_bw, pwm);
            break;
        default:
            /* - The specified direction is wrong;
             * - You chose not to change the direction but the previous 
             *   direction was not specified yet.
             */
            return CB_ENOMODE;
    }
    return CB_SUCCESS;
}

void cbMotorReset(cbMotor_t* motor) {
    gpioWrite(motor->pin_fw, 0);
    gpioWrite(motor->pin_bw, 0);
}
