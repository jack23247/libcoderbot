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
- `doxygen` - Provides the Doxygen documentation generator (required, dev).
- `pigpio` - Provides the PiGPIO library (required, runtime, dev).

To install all of the above you can use the command:

```
apt install pigpio git build-essential doxygen 
```

## License

`libcoderbot` is Copyright © 2023, Jacopo Maltagliati and is released under the
GNU GPLv3 License. A copy of the license is provided in `COPYING`.

The original Python API is Copyright © 2014-2019 Roberto Previtera, Antonio Vivace, CoderBot contributors and is released under the GNU GPLv2 License. See `CODERBOT_LICENSE` for more informations.
