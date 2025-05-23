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

#include <pigpio.h>

void cbEncoderISRa(int gpio, int level, uint32_t tick, void* enc_gen);
void cbEncoderISRb(int gpio, int level, uint32_t tick, void* enc_gen);

/**
 * @brief Initializes PiGPIO to service the Pulses from an Encoder.
 * @param enc A pointer to a cbEncoder_t structure containing the parameters
 *            for the initialization.
 */
void cbEncoderGPIOinit(const cbEncoder_t* enc) {
    // Channel A
    gpioSetMode(enc->pin_a, PI_INPUT);
    gpioSetPullUpDown(enc->pin_a, PI_PUD_UP);
    // Channel B
    gpioSetMode(enc->pin_b, PI_INPUT);
    gpioSetPullUpDown(enc->pin_b, PI_PUD_UP);
}

/**
 * @brief Registers the ISRs for the Encoder's Channels.
 * @param enc A pointer to a cbEncoder_t structure containing the parameters
 *            of the encoder.
 * @param timeout A time in milliseconds after which the ISR is terminated.
 * @link  https://abyz.me.uk/rpi/pigpio/cif.html#gpioSetISRFunc
 */
void cbEncoderRegisterISRs(const cbEncoder_t* enc, int timeout) {
    // Channel A
    gpioSetISRFuncEx(enc->pin_a, EITHER_EDGE, timeout, cbEncoderISRa,
                     (void*)enc);
    // Channel B
    gpioSetISRFuncEx(enc->pin_b, EITHER_EDGE, timeout, cbEncoderISRb,
                     (void*)enc);
}

/**
 * @brief Registers custom ISRs for the Encoder's Channels.
 * @param enc A pointer to a cbEncoder_t structure containing the parameters
 *            of the encoder.
 * @param edge_a The edge on which to trigger for Channel A.
 * @param isr_a A function pointer to the ISR for Channel A.
 * @param edge_b The edge on which to trigger for Channel B.
 * @param isr_b A function pointer to the ISR for Channel B.
 * @param timeout A time in milliseconds after which the ISR is terminated.
 * @link  https://abyz.me.uk/rpi/pigpio/cif.html#gpioSetISRFunc
 */
void cbEncoderRegisterCustomISRs(const cbEncoder_t* enc, unsigned int edge_a,
                                 void (*isr_a)(int, int, uint32_t, void*),
                                 unsigned int edge_b,
                                 void (*isr_b)(int, int, uint32_t, void*),
                                 int timeout) {
    // Channel A
    gpioSetISRFuncEx(enc->pin_a, edge_a, timeout, isr_a, (void*)enc);
    // Channel B
    gpioSetISRFuncEx(enc->pin_b, edge_b, timeout, isr_b, (void*)enc);
}

/**
 * @brief Unregisters the ISRs for the Encoder's Channels.
 * @param enc A pointer to a cbEncoder_t structure containing the parameters
 *            of the encoder.
 */
void cbEncoderCancelISRs(const cbEncoder_t* enc) {
    // Channel A
    gpioSetISRFunc(enc->pin_a, EITHER_EDGE, 0, NULL);
    // Channel B
    gpioSetISRFunc(enc->pin_b, EITHER_EDGE, 0, NULL);
}

/**
 * @brief Service Routine for the Interrupt on Channel A.
 * @param gpio The GPIO Pin that triggered the Interrupt.
 * @param level The TTL level read from the Pin (0 or 1).
 * @param event_ts_us The timestamp at which the Interrupt happened expressed
 *        in microseconds elapsed since boot. WARNING: this wraps around from
 *        4294967295 to 0 roughly every 72 minutes.
 * @param enc_gen A generic pointer to the cbEncoder_t structure containing
 *                the encoder parameters.
 */
void cbEncoderISRa(int gpio, int level, uint32_t event_ts_us, void* enc_gen) {
    cbEncoder_t* enc = (cbEncoder_t*)enc_gen;
    if (gpio == enc->last_gpio) return;  // Debounce
    enc->last_gpio = gpio;
    enc->level_a = level;
    if (level ^ enc->level_b) {  // Either one of A or B is 1
        enc->direction = forward;
        enc->ticks += enc->direction;
    } else {
        enc->bad_ticks++;  // Self-diagnostics
    }
}

/**
 * @brief Service Routine for the Interrupt on Channel B.
 * @param gpio The GPIO Pin that triggered the Interrupt.
 * @param level The TTL level read from the Pin (0 or 1).
 * @param event_ts_us The timestamp at which the Interrupt happened expressed
 *        in microseconds elapsed since boot. WARNING: this wraps around from
 *        4294967295 to 0 roughly every 72 minutes.
 * @param enc_gen A generic pointer to the cbEncoder_t structure containing
 *                the encoder parameters.
 */
void cbEncoderISRb(int gpio, int level, uint32_t event_ts_us, void* enc_gen) {
    cbEncoder_t* enc = (cbEncoder_t*)enc_gen;
    if (gpio == enc->last_gpio) return;  // Debounce
    enc->last_gpio = gpio;
    enc->level_b = level;
    if (level ^ enc->level_a) {  // Either one of A or B is 1
        enc->direction = backward;
        enc->ticks += enc->direction;
    } else {
        enc->bad_ticks++;  // Self-diagnostics
    }
}
