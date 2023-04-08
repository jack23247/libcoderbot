SDIR := ./src
ODIR := ./obj
IDIR := ./include
DDIR := ./docs/html

LIB := libcoderbot.a
SRC := $(wildcard $(SDIR)/*.c)
OBJ := $(SRC:$(SDIR)/%.c=$(ODIR)/%.o)

CFLAGS := -std=c99 -pedantic
LDFLAGS :=
LDLIBS := -lpigpio

DEBUG ?= 1
ifeq (DEBUG, 1)
    CFLAGS += -g -O0 -Wall -Werror -Wextra -DDEBUG
else
	CFLAGS += -O2 -march=native -DNDEBUG
endif

.PHONY: all clean

all: $(LIB) $(DDIR)

$(LIB): $(OBJ)
	ar rcs $@ $^

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(ODIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@ -I$(IDIR)

clean:
	@$(RM) -rv $(ODIR) $(DDIR)
	@$(RM) $(LIB)

$(DDIR):
	doxygen

-include $(OBJ:.o=.d)