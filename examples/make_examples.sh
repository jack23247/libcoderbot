#!/bin/bash
for SRC in *.c; do
    EXN=$(echo $SRC | cut -d '.' -f 1)
    gcc $SRC -o $EXN.$(uname -m) -L.. -l:libcoderbot.a -lpigpio
done