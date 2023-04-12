#!/bin/bash
gcc template.c -o template.$(uname -m) -L.. -l:libcoderbot.a -lpigpio
gcc read_encoders.c -o read_encoders.$(uname -m) -L.. -l:libcoderbot.a -lpigpio