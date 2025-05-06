SDIR := ./src
ODIR := ./obj
IDIR := ./include

LIB := libcoderbot.a
SRC := $(wildcard $(SDIR)/*.c)
OBJ := $(SRC:$(SDIR)/%.c=$(ODIR)/%.o)

CFLAGS := -std=c99 -pedantic
LDLIBS := -lpigpio

DEBUG ?= 0
ifeq ($(DEBUG), 1)
 CFLAGS += -g -O0 -Wall -Werror -Wextra -DDEBUG
else
 CFLAGS += -O2 -march=native -DNDEBUG
endif

.PHONY: all clean

all: $(LIB)

$(LIB): $(OBJ)
	ar rcs $@ $^

$(ODIR):
	mkdir -p obj/

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@ -I$(IDIR)

clean:
	@$(RM) -rv $(ODIR)
	@$(RM) $(LIB)

-include $(OBJ:.o=.d)
