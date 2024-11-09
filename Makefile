CC = gcc

CFLAGS = -pipe -march=native -O2 \
         -fstack-protector-strong -fcf-protection \
         -fpie -fPIC -std=c99 -D_DEFAULT_SOURCE \
         -fno-delete-null-pointer-checks \
         -fno-strict-overflow -fno-strict-aliasing \
         -ftrivial-auto-var-init=zero \
         -fstrict-flex-arrays=3 \
         -fstack-clash-protection \
         -Wformat=2 -Wtrampolines \
         -Wimplicit-fallthrough \
         -pedantic -pedantic-errors -Wall -Wextra -lseccomp

LDFLAGS = -Wl,-z,defs -Wl,-z,now -Wl,-z,relro -Wl,-z,nodlopen -Wl,-z,noexecstack

DESTDIR ?= /usr/local
BINDIR ?= $(DESTDIR)/bin
MANDIR ?= $(DESTDIR)/share/man/man1

VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all -s --track-origins=yes

OBJQ = pad.o padding.o wee-utf8.o strbuf.o pad-seccomp.o

%.o: src/%.c
	@echo CC $<
	@$(CC) -c -o $@ $^ $(CFLAGS)

pad: $(OBJQ)
	@echo CC $^
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

install: pad
	install -m755 pad $(BINDIR)
	install -m644 pad.1 $(MANDIR)

check: pad
	@echo "Expected result: 25"
	@./pad -m left -l 25 -c "᪥" "String※" | tr -d '\n' | wc -m
	@./pad -m right -l 25 -c "᪥" "String※" | tr -d '\n' | wc -m
	@./pad -m both -l 25 -c "᪥" "String※" | tr -d '\n' | wc -m
	@./pad -m centre -c " " "This should be (visually) centred"
	@./pad -m centre -l 25 -c "᪥" -- This should not return an error --help
	@./pad -m centre -l 25 -c "᪥" --invalid-argument || printf 'Returned error as expected\n'

test:
	/bin/sh run_tests.sh
	rm -f binary

clean:
	@rm -f pad
	@rm -f $(OBJQ)

.PHONY: clean, check, install, test
