/**
 * @file gpio.h
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

#ifndef GPIO_H
#define GPIO_H

#define GPIO_PIN_NC -1

typedef enum {
    PIN_MOTOR_ENABLE = GPIO_PIN_NC,

    /* STMicroelectronics L293DD - Four channel H-Bridge driver
     *                  +---+_+---+
     *             Vcc  |1*     20|  Vcc   
     * Channel 1  GP17  |2      19|  GP23  Channel 4
     *    LF   ^   J61  |3   L  18|  J72   ^   RB
     *             GND  |4   2  17|  GND
     *             GND  |5   9  16|  GND
     *             GND  |6   3  15|  GND
     *             GND  |7   D  14|  GND
     * Channel 2   J62  |8   D  13|  J71   Channel 3
     *    LB   ^  GP18  |9      12|  GP22  ^   RF
     *                  |10     11|  Vcc
     *                  +---------+
     * 
     * J6 - Left Motor Header
     *    1   
     * +-+-+  1: Left Motor +/Forward
     * |.|.|  2: Left Motor -/Backwards
     * +-+-+
     * J6
     * 
     * J7 - Right Motor Header
     *    1   
     * +-+-+  1: Right Motor +/Forward
     * |.|.|  2: Right Motor -/Backwards
     * +-+-+
     * J7
     */
    PIN_LEFT_FORWARD = 17, // L293DD Pin 2, Ch.1
    PIN_LEFT_BACKWARD = 18, // L293DD Pin 9, Ch.2
    PIN_RIGHT_FORWARD = 22, // L293DD Pin 12, Ch.3
    PIN_RIGHT_BACKWARD = 23, // L293DD Pin 19, Ch.4

/* Unused
    PIN_PUSHBUTTON = 16, // 11
    
    // servo
    PIN_SERVO_1 = 19, // 9
    PIN_SERVO_2 = 26, // 10
    
    // sonar
    PIN_SONAR_1_TRIGGER = 5, // 18
    PIN_SONAR_1_ECHO = 27, // 7
    PIN_SONAR_2_TRIGGER = 5, // 18
    PIN_SONAR_2_ECHO = 6, // 8
    PIN_SONAR_3_TRIGGER = 5, // 18
    PIN_SONAR_3_ECHO = 12, // 23
    PIN_SONAR_4_TRIGGER = 5, // 18
    PIN_SONAR_4_ECHO = 13, // 23 
*/

    /* J11 - Left Encoder Header
     * +-+-+-+-+
     * |.|.|.|.| J11
     * +-+-+-+-+
     *  1
     *
     * 1: VCC +5V
     * 2: GND
     * 3: Channel B, IO15, Pin 10
     * 4: Channel A, IO14, Pin 8
     */ 
    PIN_ENCODER_LEFT_A = 14,
    PIN_ENCODER_LEFT_B = 15, 

    /* J12 - Right Encoder Header
     * +-+-+-+-+
     * |.|.|.|.| J12
     * +-+-+-+-+
     *  1
     *
     * 1: VCC +5V
     * 2: GND
     * 3: Channel A, IO24, Pin 18 
     * 4: Channel B, IO25, Pin 22
     */ 
    PIN_ENCODER_RIGHT_A = 24,
    PIN_ENCODER_RIGHT_B = 25
} CODERBOT_V5_GPIO;

#endif // GPIO_H