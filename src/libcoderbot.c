/**
 * @file libcoderbot.c
 * @author Jacopo Maltagliati
 * @date 8 Apr 2023
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

#include "libcoderbot.h"

#include "pigpio.h"
#include "stdlib.h"

void cbInit() {
    if (gpioInitialise() < 0) exit(EXIT_FAILURE);
    cbEncoderGPIOinit(cbEncoderLeft, {PIN_ENCODER_LEFT_A, PIN_ENCODER_LEFT_B}, 50);
    cbEncoderGPIOinit(cbEncoderRight, {PIN_ENCODER_RIGHT_A, PIN_ENCODER_RIGHT_B}, 50);
};

void cbCleanup() {
    gpioTerminate();
    exit(EXIT_SUCCESS);
}