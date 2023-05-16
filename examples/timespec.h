/**
 * @file timespec.h
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

#ifndef TIMESPEC_H
#define TIMESPEC_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

/** 
 * @brief Defines which clock to use. Refer to `man clock_gettime(3)` for a
 * list of valid clocks. `CLOCK_MONOTONIC_RAW` is used because it's fairly
 * reliable and unaffected by NTP adjustments. 
 */
#define USE_CLOCK CLOCK_MONOTONIC_RAW

// Constants used to convert the timespec values, as defined by the kernel's 
// internal time.h
#define MSEC_PER_SEC 1000L
#define USEC_PER_MSEC 1000L
#define NSEC_PER_USEC 1000L
#define NSEC_PER_MSEC 1000000L
#define USEC_PER_SEC 1000000L
#define NSEC_PER_SEC 1000000000L

typedef uint64_t nsec_t;

typedef struct timespec timespec_t;

/**
 * @brief Sets the value of a timespec using the timer defined in USE_CLOCK.
 * @param ts A pointer to the timespec to be set.
 */
static inline void tsSet(timespec_t* ts) {
    if (clock_gettime(USE_CLOCK, ts) == -1) {
        perror("tsSet:");
        exit(EXIT_FAILURE);
    }
}

/** 
  * @brief Converts a timespec to nanoseconds.
  * @param ts A pointer to the timespec to be converted.
  * @return The time held by the timespec in ns.
  */
static inline nsec_t tsToNs(const timespec_t* ts) {
    return ((nsec_t)ts->tv_sec * NSEC_PER_SEC) + ts->tv_nsec;
}

/**
 * @brief Updates a timespec and calculates the difference between now and the 
 *        last time the timespec was updated.
 * @param ts A pointer to the timespec to be updated.
 * @return The interval between the previous and the current update in ns.
 */
static inline nsec_t tsTickNs(timespec_t* ts) {
    nsec_t before = tsToNs(ts); 
    tsSet(ts);
    return tsToNs(ts) - before;
}

#endif  // TIMESPEC_H
