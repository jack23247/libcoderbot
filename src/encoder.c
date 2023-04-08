/**
 * @file encoder.c
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

#include "encoder.h"

/**
 * @brief Initializes PiGPIO to service the pulses from an encoder.
 * @param enc A pointer to the structure containing the parameters for the
 *            initialization.
 */
void cbEncoderGPIOInit(const cbEncoder_t* enc, gpio pins[2], int timeout) {
    for(int i = 0; i < 2; i++) {
        gpioSetMode(pins[i], PI_INPUT);
        gpioSetPullUpDown(pins[i], PI_PUD_DOWN);
        // https://abyz.me.uk/rpi/pigpio/cif.html#gpioSetISRFunc
    }
    gpioSetISRFuncEx(pins[0], EITHER_EDGE, timeout, cbEncoderISRa, (void*)enc);
    gpioSetISRFuncEx(pins[1], EITHER_EDGE, timeout, cbEncoderISRb, (void*)enc);
}

/**
 * @brief Service Routine for the Interrupt on Channel A.
 * @param gpio The GPIO Pin that triggered the Interrupt.
 * @param level The TTL level read from the Pin (0 or 1).
 * @param tick The tick at which the Interrupt happened expressed in the number 
 *        of microseconds elapsed since boot. WARNING: this wraps around from
 *        4294967295 to 0 roughly every 72 minutes.
 * @param userdata A generic pointer to the cbEncoder_t structure containing
 *                 the encoder parameters.
 */
void cbEncoderISRa(int gpio, int level, uint32_t tick, void *userdata) {
    cbEncoder_t* enc = (cbEncoder_t*)userdata;
    if(gpio == enc->last_gpio) return; // Debounce
    enc->last_gpio = gpio; 
    enc->a = level;
    if(level ^ enc->b) { // Either one of A or B is 1 
        enc->direction = 1;    
    }
    // else Direction unchanged
}

/**
 * @brief Service Routine for the Interrupt on Channel B.
 * @param gpio The GPIO Pin that triggered the Interrupt.
 * @param level The TTL level read from the Pin (0 or 1).
 * @param tick The tick at which the Interrupt happened expressed in the number 
 *        of microseconds elapsed since boot. WARNING: this wraps around from
 *        4294967295 to 0 roughly every 72 minutes.
 * @param userdata A generic pointer to the cbEncoder_t structure containing
 *                 the encoder parameters.
 */
void cbEncoderISRb(int gpio, int level, uint32_t tick, void *userdata) {
    cbEncoder_t* enc = (cbEncoder_t*)userdata;
    if(gpio == enc->last_gpio) return; // Debounce
    enc->last_gpio = gpio; 
    enc->b = level;
    if(level ^ enc->a) { // Either one of A or B is 1 
        enc->direction = -1;    
    }
    // else Direction unchanged
}