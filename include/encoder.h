/**
 * @file encoder.h
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

#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

#include "gpio.h"

typedef enum { backward = -1, forward = 1 } dir_t;

struct cbEncoder {
    CODERBOT_GPIO pin_a, pin_b;
    int last_gpio;
    uint16_t level_a, level_b;
    dir_t direction;
    int64_t ticks;
    uint32_t bad_ticks;
};

typedef struct cbEncoder cbEncoder_t;

void cbEncoderGPIOinit(int pin_a, int pin_b);
void cbEncoderRegisterISRs(const cbEncoder_t* enc, int pin_a, int pin_b, int timeout);
void cbEncoderCancelISRs(int pin_a, int pin_b);

void cbEncoderISRa(int gpio, int level, uint32_t tick, void *userdata);
void cbEncoderISRb(int gpio, int level, uint32_t tick, void *userdata);

#endif // ENCODER_H
