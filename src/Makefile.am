AM_CPPFLAGS = $(GLIB_CFLAGS) $(ZLIB_CFLAGS) $(LZMA_CFLAGS)
ACLOCAL_AMFLAGS = -I m4

statzni_SOURCES = io.c \
		  main.c \
		  output.c \
		  parser.c \
		  state.c \
		  stridx.c \
		  user.c \
		  io.h \
		  output.h \
		  parser.h \
		  state.h \
		  stridx.h \
		  user.h
statzni_LDADD = $(GLIB_LIBS) $(ZLIB_LIBS) $(LZMA_LIBS) $(BZ_LIBS)

bin_PROGRAMS = statzni
