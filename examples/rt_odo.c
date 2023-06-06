/**
 * @file rt_odo.c
 * @author Jacopo Maltagliati
 * @date 6 Giu 2023
 * @brief Skeleton of an RT controller for the CoderBot platform.
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
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include <linux/sched.h>
#include <sched.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "../include/cbdef.h"
#include "../include/motor.h"
#include "../include/encoder.h"
#include "timespec.h"

/* RT SCHEDULING PARAMETERS ------------------------------------------------ */

#define UPTICK_RUNTIME 5 * NSEC_PER_MSEC //< Expected task runtime in ns
#define UPTICK_PERIOD 7 * NSEC_PER_MSEC //< Task period in ns
#define UPTICK_DEADLINE 6 * NSEC_PER_MSEC //< Task deadline in ns

#define ODO_RUNTIME 30 * NSEC_PER_MSEC //< Expected task runtime in ns
#define ODO_PERIOD 40 * NSEC_PER_MSEC //< Task period in ns
#define ODO_DEADLINE 31 * NSEC_PER_MSEC //< Task deadline in ns

/* CB PARAMETERS ----------------------------------------------------------- */

#define LEFT_WHEEL_RAY_MM 33.f
#define RIGHT_WHEEL_RAY_MM 33.f
#define TICKS_PER_REVOLUTION 16 //< Ticks per motor revolution
#define TRANSMISSION_RATIO 120

#define DISTANCE_FROM_GOAL 500.f //< Distance from goal in mm

#define DUTY_CYC_L .5f //< Duty cycle for the left wheel
#define DUTY_CYC_R DUTY_CYC_L //< Duty cycle for the right wheel

/* TYPEDEFS ---------------------------------------------------------------- */

/**
 * @brief Struct used by the task to communicate parameters to the scheduler
 * @see https://man7.org/linux/man-pages/man2/sched_getattr.2.html
 * @see https://www.i-programmer.info/programming/cc/13002-applying-c-deadline-scheduling.html?start=1
 */
struct sched_attr {
    uint32_t size;           /* Size of this structure */
    uint32_t sched_policy;   /* Policy (SCHED_*) */
    uint64_t sched_flags;    /* Flags */
    int32_t sched_nice;      /* Nice value (SCHED_OTHER, SCHED_BATCH) */
    uint32_t sched_priority; /* Static priority (SCHED_FIFO, SCHED_RR) */
    /* The following fields are for SCHED_DEADLINE */
    uint64_t sched_runtime;
    uint64_t sched_deadline;
    uint64_t sched_period;
};

/**
 * @brief A structure representing a task
 * @var worker::tid The POSIX Thread ID
 * @var worker::entry The entry point of the thread associated with the task
 */
typedef struct task {
    pthread_t tid;
    void* (*entry)(void*);
} task_t;

/* GLOBALS ----------------------------------------------------------------- */

cbMotor_t cbMotorLeft = {PIN_LEFT_FORWARD, PIN_LEFT_BACKWARD, forward};
cbMotor_t cbMotorRight = {PIN_RIGHT_FORWARD, PIN_RIGHT_BACKWARD, forward};

cbEncoder_t cbEncoderLeft = {
    PIN_ENCODER_LEFT_A, PIN_ENCODER_LEFT_B, GPIO_PIN_NC, 0, 0, 0};
cbEncoder_t cbEncoderRight = {
    PIN_ENCODER_RIGHT_A, PIN_ENCODER_RIGHT_B, GPIO_PIN_NC, 0, 0, 0};

volatile int ticks_L, ticks_R;
volatile int halt = 0;

pthread_mutex_t ticksMutex;

/* FUNCTIONS --------------------------------------------------------------- */

void cbInit() {
    if (gpioInitialise() < 0) exit(EXIT_FAILURE);
    // Left
    cbMotorGPIOinit(&cbMotorLeft);
    cbEncoderGPIOinit(&cbEncoderLeft);
    cbEncoderRegisterISRs(&cbEncoderLeft, 50);
    // Right
    cbMotorGPIOinit(&cbMotorRight);
    cbEncoderGPIOinit(&cbEncoderRight);
    cbEncoderRegisterISRs(&cbEncoderRight, 50);
}

void cbTerminate() {
    cbMotorReset(&cbMotorLeft);
    cbMotorReset(&cbMotorRight);
    cbEncoderCancelISRs(&cbEncoderLeft);
    cbEncoderCancelISRs(&cbEncoderRight);
    gpioTerminate();
}

/**
 * @brief Wrapper for the __NR_sched_setattr kernel call
 *
 * @param pid The PID of the process whose parameters are going to be modified
 * @param attr A pointer to a sched_attr structure used to specify attributes
 * @param flags ???
 *
 * @return On success, 0; on failure -1. The error code is stored in errno
 */
static inline int sched_setattr(pid_t pid, 
								const struct sched_attr* attr, 
								unsigned int flags) {  
    // TODO who knows what "flags" does? Is
    // it the same as "sched_attr.flags"?
    return syscall(__NR_sched_setattr, pid, attr, flags);
}

/**
 * @brief Handler to be called when a `SIGXCPU` signal is generated by the
 * scheduler when missing a deadline and `SCHED_FLAG_DL_OVERRUN` is on.
 *
 * @param sig The number of the received signal.
 */
void cbrtDlMissHandler(int sig) {
    // BEGIN User handler
    puts("Deadline miss!");
    gpioTerminate();
    // END User handler
    (void) signal(SIGXCPU, SIG_DFL);
}

/**
 * @brief A task dedicated to updating the ticks.
 *
 * @param args A pointer to a memory area representing arguments. By default,
 * `NULL` is passed: this parameter is mostly useful if you have a thread
 * pool or a worker that needs some specific parameters.
 * @see https://computing.llnl.gov/tutorials/pthreads/
 *
 * @return In our case, nothing is returned, but having the function signature
 * return `void*` lets the programmer return arbitrary data from the thread.
 */
void* cbrtUpdateTicksEntryPoint(void* args) {
    struct sched_attr attr = {.size = sizeof(attr),
                              .sched_flags = 0 | SCHED_FLAG_DL_OVERRUN,
                              .sched_policy = SCHED_DEADLINE,
                              .sched_runtime = UPTICK_RUNTIME, // ns
                              .sched_period = UPTICK_PERIOD, // ns
                              .sched_deadline = UPTICK_DEADLINE}; // ns
    (void) signal(SIGXCPU, cbrtDlMissHandler);  // Register signal handler
    if (sched_setattr(0, &attr, 0)) {
        perror("cbrtUpdateTicksEntryPoint: sched_setattr");
        exit(EXIT_FAILURE);
    }
    // BEGIN Worker variables
    // END Worker variables

    for (;;) {
        // BEGIN Worker code
        if (pthread_mutex_trylock(&ticksMutex) != 0) {                                      
    		if (errno == EBUSY) {
    			// The mutex was busy, yielding...                                                    
      			sched_yield();                            
    		} else {                                                                      
      			perror("cbrtUpdateTicksEntryPoint: pthread_mutex_trylock");                                  
      			exit(EXIT_FAILURE);                                                                  
    		}  
    	} else {
			// CRITICAL SECTION
    		// We have the lock on the mutex and we can update the ticks
    		
        	ticks_L = cbEncoderLeft.ticks;
        	ticks_R = cbEncoderRight.ticks;
        	if (pthread_mutex_unlock(&ticksMutex) != 0) {                                      
    			perror("cbrtUpdateTicksEntryPoint: pthread_mutex_unlock");                                     
    			exit(EXIT_FAILURE);                                                                    
  			}            
    	}
    	pthread_testcancel();
        // END Worker code
        sched_yield();
    };
    pthread_exit(EXIT_SUCCESS);
    return (NULL);
}

/**
 * @brief Odometry Task.
 *
 * @param args A pointer to a memory area representing arguments. By default,
 * `NULL` is passed: this parameter is mostly useful if you have a thread
 * pool or a worker that needs some specific parameters.
 * @see https://computing.llnl.gov/tutorials/pthreads/
 *
 * @return In our case, nothing is returned, but having the function signature
 * return `void*` lets the programmer return arbitrary data from the thread.
 */
void* cbrtOdoEntryPoint(void* args) {
    struct sched_attr attr = {.size = sizeof(attr),
                              .sched_flags = 0 | SCHED_FLAG_DL_OVERRUN,
                              .sched_policy = SCHED_DEADLINE,
                              .sched_runtime = ODO_RUNTIME, // ns
                              .sched_period = ODO_PERIOD, // ns
                              .sched_deadline = ODO_DEADLINE}; // ns
    (void)signal(SIGXCPU, cbrtDlMissHandler);  // Register signal handler
    if (sched_setattr(0, &attr, 0)) {
        perror("cbrtEntryPoint: sched_setattr");
        exit(EXIT_FAILURE);
    }
    // BEGIN Worker variables
    const float mmsPerTick_L = (LEFT_WHEEL_RAY_MM * 2 * M_PI) /
                      (TICKS_PER_REVOLUTION * TRANSMISSION_RATIO);

    const float mmsPerTick_R = (RIGHT_WHEEL_RAY_MM * 2 * M_PI) /
                      (TICKS_PER_REVOLUTION * TRANSMISSION_RATIO);
    float distFromGoal_mm = DISTANCE_FROM_GOAL;
    int myTicks_L, myTicks_R;
    int prevTicks_L = 0, prevTicks_R = 0;
    float travel_mm_L, travel_mm_R;
    //timespec_t clock;
    //tsSet(&clock);
    cbMotorMove(&cbMotorLeft, forward, DUTY_CYC_L);
    cbMotorMove(&cbMotorRight, forward, DUTY_CYC_R);
    // END Worker variables

    for (;;) {
        // BEGIN Worker code
        if (pthread_mutex_trylock(&ticksMutex) != 0) {                                      
    		if (errno == EBUSY) {
    			// The mutex was busy, yielding...                                                    
      			sched_yield();                            
    		} else {                                                                      
      			perror("cbrtUpdateTicksEntryPoint: pthread_mutex_trylock");                                  
      			exit(EXIT_FAILURE);                                                                  
    		}  
    	} else {
			// CRITICAL SECTION
    		// We have the lock on the mutex and we can update the ticks
        	myTicks_L = ticks_L;
        	myTicks_R = ticks_R;
        	if (pthread_mutex_unlock(&ticksMutex) != 0) {                                      
    			perror("cbrtUpdateTicksEntryPoint: pthread_mutex_unlock");                                     
    			exit(EXIT_FAILURE);                                                                    
  			}            
    	}
    	travel_mm_L = (myTicks_L - prevTicks_L) * mmsPerTick_L;
    	prevTicks_L = myTicks_L;
    	// speed_mm_s_L = (travel_mm_L / tsTickNs(&clock)) * NSEC_PER_MSEC;	
    	travel_mm_R = (myTicks_R - prevTicks_R) * mmsPerTick_R;
    	prevTicks_R = myTicks_R;
        distFromGoal_mm -= (travel_mm_L + travel_mm_R) / 2;
		// Takes A LOT of CPU time. Do not use with low sched values.
        //printf("TL: %f, TR: %f, DfG: %f\n", travel_mm_L, travel_mm_R, distFromGoal_mm);
        if(distFromGoal_mm < .0f) {
    		cbMotorReset(&cbMotorLeft);
    		cbMotorReset(&cbMotorRight);
        	break;
        }
        // END Worker code
        sched_yield();
    };
    pthread_exit(EXIT_SUCCESS);
    return (NULL);
}

task_t taskOdo = { .tid = 0, .entry = cbrtOdoEntryPoint };
task_t taskUpdateTicks = { .tid = 0, .entry = cbrtUpdateTicksEntryPoint };

int main(void) {
    cbInit();
    // Initialize the mutex 
    if (pthread_mutex_init(&ticksMutex, NULL) != 0) {                                  
    	perror("main: pthread_mutex_init");                                       
    	exit(EXIT_FAILURE);                                                                    
  	}
  	{ // Create the tasks
		if(pthread_create(&(taskOdo.tid), NULL, taskOdo.entry, NULL) != 0) {
			perror("main: pthread_create: taskOdo");
			exit(EXIT_FAILURE);
		} else {
	    	printf("main: taskOdo: Created.\n");
		}
		
		if(pthread_create(&(taskUpdateTicks.tid), NULL, 
						  taskUpdateTicks.entry, NULL) != 0) {
			perror("main: pthread_create: taskUpdateTicks");
			exit(EXIT_FAILURE);
		} else {
			printf("main: taskUpdateTicks: Created.\n");
		}
	}
	{ // Wait for task completion
	    if(pthread_join(taskOdo.tid, NULL) != 0) {
	    	perror("main: pthread_join");
	    	exit(EXIT_FAILURE);
	    } else {
	    	printf("main: taskOdo: Completed!\n");
	    	if (pthread_cancel(taskUpdateTicks.tid) != 0) {                                            
   				perror("main: pthread_cancel");
   				exit(EXIT_FAILURE);
 			}  
	    }

	    if(pthread_join(taskUpdateTicks.tid, NULL) != 0) {
	    	perror("main: pthread_join");
	    	exit(EXIT_FAILURE);
	    } else {
	    	printf("main: taskUpdateTicks: Completed!\n");
	    }
	}
    if (pthread_mutex_destroy(&ticksMutex) != 0) {                                  
    	perror("main: pthread_mutex_destroy");                                       
    	exit(EXIT_FAILURE);                                                                    
  	}
  	cbTerminate();
    exit(EXIT_SUCCESS);
}
