# ==============================================================================
#
# Makefile.real
#
# ==============================================================================


# ==============================================================================
# Variables
# ==============================================================================

EXTRAFLAGS := $(EXTRAFLAGS1) $(EXTRAFLAGS2) $(EXTRAFLAGS3) $(EXTRAFLAGS4)
CC       := gcc
CFLAGS   := -g -Wall -Winline -O3 $(EXTRAFLAGS)
CPP      := $(CC)
CPPFLAGS := $(CFLAGS)
LD       := $(CC)
LDFLAGS  += $(EXTRAFLAGS)

LIBTL2 := libtl2.a

SRCS := \
	tl2.c \
	tmalloc.c \
#
OBJS := ${SRCS:.c=.o}

AR      := ar
RANLIB  := ranlib

RM := rm -f

# ADD GPROF INSTRUMENTATION
#CFLAGS += -pg
#LDFLAGS += -pg -Winline

# ==============================================================================
# Rules
# ==============================================================================

.PHONY: default
default: lazy

.PHONY: clean
clean:
	$(RM) $(LIBTL2) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBTL2): $(OBJS)
	$(AR) cru $@ $^
	$(RANLIB) $@

.PHONY: lazy
lazy: $(LIBTL2)

.PHONY: eager
eager: CFLAGS += -DTL2_EAGER
eager: $(LIBTL2)

.PHONY: lazy-nocm
lazy-nocm: CFLAGS += -DTL2_NOCM
lazy-nocm: $(LIBTL2)

.PHONY: eager-nocm
eager-nocm: CFLAGS += -DTL2_EAGER
eager-nocm: CFLAGS += -DTL2_NOCM
eager-nocm: $(LIBTL2)

.PHONY: otm
otm: CFLAGS += -m32
otm: $(LIBTL2)


# ==============================================================================
# Dependencies
# ==============================================================================

%.o: %.h


# ==============================================================================
#
# End of Makefile.real
#
# ==============================================================================
