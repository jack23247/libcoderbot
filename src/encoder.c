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

#include <pigpio.h>

#include "encoder.h"

/**
 * @brief Initializes PiGPIO to service the Pulses from an Encoder.
 * @param pin_a The PIN connected to Channel A.
 * @param pin_a The PIN connected to Channel B.
 */
void cbEncoderGPIOinit(int pin_a, int pin_b) {
    // Channel A
    gpioSetMode(pin_a, PI_INPUT);
    gpioSetPullUpDown(pin_a, PI_PUD_UP);
    // Channel B
    gpioSetMode(pin_b, PI_INPUT);
    gpioSetPullUpDown(pin_b, PI_PUD_UP);
}

/**
 * @brief Registers the ISRs for the Encoder's Channels.
 * @param enc A pointer to the structure containing the parameters for the
 *            initialization. 
 * @param pin_a The PIN connected to Channel A.
 * @param pin_a The PIN connected to Channel B.
 * @param timeout A time in milliseconds after which the ISR is terminated.
 * @link  https://abyz.me.uk/rpi/pigpio/cif.html#gpioSetISRFunc
 */
void cbEncoderRegisterISRs(const cbEncoder_t* enc, int pin_a, int pin_b, int timeout) {
    // Channel A
    gpioSetISRFuncEx(pin_a, EITHER_EDGE, timeout, cbEncoderISRa, (void*)enc);
    // Channel B
    gpioSetISRFuncEx(pin_b, EITHER_EDGE, timeout, cbEncoderISRb, (void*)enc);
}

/**
 * @brief Unregisters the ISRs for the Encoder's Channels.
 * @param pin_a The PIN connected to Channel A.
 * @param pin_a The PIN connected to Channel B.
 */
void cbEncoderCancelISRs(int pin_a, int pin_b) {
    // Channel A
    gpioSetISRFunc(pin_a, EITHER_EDGE, 0, NULL);
    // Channel B
    gpioSetISRFunc(pin_b, EITHER_EDGE, 0, NULL);
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