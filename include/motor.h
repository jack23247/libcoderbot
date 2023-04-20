/**
 * @file motor.h
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

#ifndef MOTOR_H
#define MOTOR_H

#include "gpio.h"

#include <stdbool.h>

typedef enum cbDir { backward = -1, forward = 1 } cbDir_t;

struct cbMotor {
    cbGPIO_t pin_fw, pin_bw;
    cbDir_t direction;
};

typedef struct cbMotor cbMotor_t;

void cbMotorGPIOinit(const cbMotor_t* motor);
int cbMotorMove(cbMotor_t* motor, cbDir_t direction, float duty_cycle);
void cbMotorReset(cbMotor_t* motor);

#endif // MOTOR_H
