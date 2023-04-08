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

#include "gpio.h"
#include "stdint.h"

/*
//levels: xxxxxxba
#define LEVELS_NONE(l) (!l)
#define LEVELS_ONE_OF(l) (l && l < 0b11)
#define LEVELS_BOTH(l) (l == 0b11)

#define READ_A(l) (l & 0b01)
#define READ_B(l) ((l & 0b10) >> 1)
#define WRITE_A(l,d) ((l &= 0b10) |= d)
#define WRITE_B(l,d) ((l &= 0b01) |= (d << 1))
*/

struct cbEncoder {
    gpio last_gpio;
    uint16_t a, b;
    int direction;
};

typedef struct cbEncoder cbEncoder_t;

void cbEncoderGPIOInit(const cbEncoder_t* enc, gpio pins[2], int timeout);
void cbEncoderISRa(int gpio, int level, uint32_t tick, void *userdata);
void cbEncoderISRb(int gpio, int level, uint32_t tick, void *userdata);

#endif // ENCODER_H