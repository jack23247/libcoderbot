ARCH = $(shell uname -m)

SRC := $(wildcard *.c)
EXE := $(SRC:%.c=%.$(ARCH))

CFLAGS := -std=gnu99 -pedantic
LDFLAGS := -L..
LDLIBS := -lpigpio -lpthread -l:libcoderbot.a

DEBUG ?= 1
ifeq (DEBUG, 1)
 CFLAGS += -g -O0 -Wall -Werror -Wextra -DDEBUG
else
 CFLAGS += -O2 -march=native -DNDEBUG
endif

.PHONY: all clean

all: $(EXE)

./%.$(ARCH): ./%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	$(RM) $(EXE)
