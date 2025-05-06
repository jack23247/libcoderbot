# libcoderbot

`libcoderbot` is an effort to write a fast, low-level control library for the CoderBot platform. The CoderBot is a Raspberry Pi-based programmable robot intended for educational purposes and, in its original form, exposed a REST API to the programmer for ease of use.

More informations about the CoderBot platform are available on the [project's website](https://www.coderbot.org), and technical documentation is available on the [Wiki](https://github.com/CoderBotOrg/coderbot/wiki).

## Features

|                     | Python API | `libcoderbot` |
| :------------------ | ---------- | ------------- |
| CoderBot V4 Support | Yes        | No            |
| CoderBot V5 Support | Yes        | Yes           |

### CoderBot V5 Shield Feature Support Matrix

| Feature               | Support |
| :-------------------- | ------- |
| Motor Driver (L293DD) | Yes     |
| Encoders              | Yes     |
| Sonars                | No      |
| MPU (ATMega328)       | Planned |

## Prerequisites

`libcoderbot` requires the following packages to be installed:
- `build-essential` - Provides the compiler and a minimal but functional build environment (required, dev).
- `git` - Provides the git revision control system (optional, dev).
- `doxygen` - Provides the Doxygen documentation generator (optional, dev).
- `pigpio` - Provides the PiGPIO library (required, runtime, dev).

To install all of the above you can use the command:

```
apt install pigpio git build-essential doxygen
```

## Building

A Makefile is provided for convenience. If you just want to build the library, you can type `make` from the project's root folder. You can `make DEBUG=1` to compile with debugging options enabled (no optimizations, no symbol stripping).

A Doxyfile is provided and can be used for generating the documentation in HTML. To generate the documentation, you can simply invoke `doxygen` from the project's root folder.

## Usage

`libcoderbot` depends on `pigpio`, which must be initialized before trying to use the GPIO port. You may also need to run your programs as root. The following example shows how to write a simple program that makes the robot turn on the spot:

```c
// stop.c

#include <pigpio.h>
#include <stdlib.h>

#include "libcoderbot/include/cbdef.h"
#include "libcoderbot/include/motor.h"

cbMotor_t mot_l = {PIN_LEFT_FORWARD, PIN_LEFT_BACKWARD, forward};
cbMotor_t mot_r = {PIN_RIGHT_FORWARD, PIN_RIGHT_BACKWARD, forward};

void init() {
	if (gpioInitialise() < 0) exit(EXIT_FAILURE);
	cbMotorGPIOinit(&mot_l);
	cbMotorGPIOinit(&mot_r);
}

void terminate() {
	cbMotorReset(&mot_l);
	cbMotorReset(&mot_r);
	gpioTerminate();
}

int main(void) {
	init();
	atexit(terminate);
	printf("Killing the motors.\n");
	exit(EXIT_SUCCESS);
}
```

In this example we assume that the library is located in a subfolder (`libcoderbot/`) of the current working directory.

```shell
cc stop.c libcoderbot/libcoderbot.a -l pigpio -o stop
```

The resulting program can then be executed with:

```console
./stop
```

You may need to run it as `root` if your user doesn't have permission to access the GPIO port.

## License

`libcoderbot` is Copyright © 2023, Jacopo Maltagliati and is released under the
GNU GPLv3 License. A copy of the license is provided in `COPYING`.

The original Python API is Copyright © 2014-2019 Roberto Previtera, Antonio Vivace, CoderBot contributors and is released under the GNU GPLv2 License. See `CODERBOT_LICENSE` for more informations.
