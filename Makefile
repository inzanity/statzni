SOURCES := main.c io.c parser.c user.c output.c state.c
OBJS := $(patsubst %.c,%.o,$(SOURCES))

all: statzni

CFLAGS := -O2 -g -W -Wall -pedantic `pkg-config glib-2.0 --cflags`
LDFLAGS := -W -Wall `pkg-config glib-2.0 --libs` -lbz2 -lz -llzma

$(foreach src,$(SOURCES),$(eval $(patsubst %.c,%.o,$(src)): $(shell grep '^#\s*include\s*"' $(src)|cut -d'"' -f2)))

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

statzni: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)
