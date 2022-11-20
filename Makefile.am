LDADD = -lpadding

if JEMALLOC
LDADD += -ljemalloc
AM_LDFLAGS=-lm -lstdc++ -lgcc -lm -pthread
endif

bin_PROGRAMS = pad
pad_SOURCES = src/main.c src/spoof.h
man1_MANS = pad.1
