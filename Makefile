SOURCES := main.c io.c parser.c user.c output.c state.c stridx.c
OBJS := $(patsubst %.c,%.o,$(SOURCES))

all: statzni

CFLAGS += -O2 -g -W -Wall -pedantic
GLIB_CFLAGS += `pkg-config glib-2.0 --cflags`
LDFLAGS += -W -Wall -lbz2 -lz -llzma
GLIB_LIBS += `pkg-config glib-2.0 --libs`

$(foreach src,$(SOURCES),$(eval $(patsubst %.c,%.o,$(src)): $(shell grep '^#\s*include\s*"' $(src)|cut -d'"' -f2)))

%.o: %.c Makefile
	$(CC) $(CFLAGS) $(GLIB_CFLAGS) -c $< -o $@

statzni: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(GLIB_LIBS)

clean:
	rm -f $(OBJS)
