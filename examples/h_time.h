/**
 * @file h_time.h
 * @author Jacopo Maltagliati (j.maltagliati@campus.unimib.it)
 * @brief An helper header containing various timing-related structures and
 * inlines.
 *
 * @copyright This file is part of a project released under the European Union
 * Public License, see LICENSE and LICENSE.it for details.
 *
 */

#ifndef H_TIME_H
#define H_TIME_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

/*
Importante, vedi http://projects.ira.disco.unimib.it/issues/1123
- Risoluzione Apparente (ad es. ms) vs. Risoluzione Effettiva (ad es. 55ms
DOS)
- Velocità della richiesta https://elinux.org/High_Resolution_Timers
dà una risposta sommaria, il timer è basato sullo "jiffy", che è un'unità
di misura di tempo interna del kernel che varia in base all'architettura.
Possiamo inoltre fare `cat /proc/timer_list` per vedere una lista dei
timer e delle loro capacità.
*/

/** @brief Defines which clock to use. Refer to `man clock_gettime(3)` for a
 * list of valid clocks. `CLOCK_MONOTONIC_RAW` is used because it's fairly
 * reliable and unaffected by NTP adjustments. */
#define USE_CLOCK CLOCK_MONOTONIC_RAW

/* Parameters used to convert the timespec values, retrieved from the kernel's
 * internal time.h
 */
#define MSEC_PER_SEC 1000L
#define USEC_PER_MSEC 1000L
#define NSEC_PER_USEC 1000L
#define NSEC_PER_MSEC 1000000L
#define USEC_PER_SEC 1000000L
#define NSEC_PER_SEC 1000000000L
#define FSEC_PER_SEC 1000000000000000L

typedef struct timespec timespec_t;

/** @brief A type used for time calculations and to keep deltas. It's also
 * used for the current timebase implementation. On an amd64 machine, it's 64
 * bits wide so it should keep just south of 2e10 seconds: we might reconsider
 * this choice in the future, but for now it's more than fine. */
typedef unsigned long long int nsec_t;

/** @brief A global variable that serves as a record of when the application
 * (or rather, the timing system) was started. It's used to calculate deltas
 * for performance measurements and keeping track of clock drifts. */
static nsec_t _timebase;  // HACK The implementation of timebase is yanky...
                          // it could use a refactor, as I'd like to
                          // initialize it just before launching the threads

/** @brief Convert a timespec to nanoseconds.
 *
 *  @param ts A pointer to the timespec to be converted.
 *
 *  @return The scalar nanosecond representation of the timespec.
 */
static inline nsec_t HTime_TsToNs(const timespec_t* ts) {
    return ((nsec_t)ts->tv_sec * NSEC_PER_SEC) + ts->tv_nsec;
}

/**
 * @brief Get the time from the clock specified by USE_CLOCK in nanoseconds.
 *
 * @param ts A pointer to a timespec structure where to store the intermediate
 * result.
 *
 * @return The scalar nanosecond representation of the timespec.
 */
static inline nsec_t HTime_GetNs(timespec_t* ts) {
    if (clock_gettime(USE_CLOCK, ts) == -1) {
        perror("HTime_GetNs:");
        exit(EXIT_FAILURE);
    }
    return HTime_TsToNs(ts);
}

/**
 * @brief Get the difference between the global timebase and a specific
 * timespec.
 *
 * @param ts The timespec of which to check the delta.
 *
 * @return The difference between the timespec and the global timebase in
 * nanoseconds.
 */
static inline nsec_t HTime_GetNsDelta(timespec_t* ts) {
    return HTime_GetNs(ts) - _timebase;
}

/**
 * @brief Initialize the global timebase, see `_timebase`.
 */
static inline void HTime_InitBase(void) {
    timespec_t ts;
    _timebase = HTime_GetNs(&ts);
}

#endif  // H_TIME_H