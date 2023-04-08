/**
 * @file libcoderbot.h
 * @author Jacopo Maltagliati
 * @date 23 Mar 2023
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

#ifndef LIBCODERBOT_H
#define LIBCODERBOT_H

#include "gpio.h"
#include "encoder.h"

// C99 compound literal
static cbEncoder_t* cbEncoderLeft = &cbEncoder_t{GPIO_PIN_NC, 0, 0, 0};
static cbEncoder_t* cbEncoderRight = &cbEncoder_t{GPIO_PIN_NC, 0, 0, 0};

void cbInit();
void cbCleanup();

#endif  // LIBCODERBOT_H