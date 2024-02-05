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
	 -pedantic -pedantic-errors -Wall -Wextra

SFLAGS = -Wbitwise -Wbitwise-pointer -Wcast-truncate \
	 -Wdecl -Wdefault-bitfield-sign -Wdo-while \
	 -Wenum-mismatch -Wflexible-array-sizeof \
	 -Winit-cstring -Wnewline-eof -Wpointer-arith \
	 -Wreturn-void -Wshadow -Wshift-count-negative \
	 -Wshift-count-overflow

VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all -s --track-origins=yes

OBJQ = main.o padding.o wee-utf8.o strbuf.o

%.o: src/%.c
	@echo CC $<
	@$(CC) -c -o $@ $^ $(CFLAGS)

pad: $(OBJQ)
	@echo CC $^
	@$(CC) $(CFLAGS) $(LIBS) -o $@ $^

check:
	@echo Checking all files with sparse
	@sparse $(SFLAGS) src/*

test: pad
	@echo Checking pad with mode left
	@valgrind $(VALGRIND_FLAGS) ./pad -m left -l 25 -c "᪥" "String᪥"
	@echo Checking pad with mode right
	@valgrind $(VALGRIND_FLAGS) ./pad -m right -l 25 -c "᪥" "String᪥"
	@echo Checking pad with mode both
	@valgrind $(VALGRIND_FLAGS) ./pad -m both -l 25 -c "᪥" "String᪥"
	@echo Checking pad with mode centre
	@valgrind $(VALGRIND_FLAGS) ./pad -m centre -l 25 -c "᪥" "String᪥"

clean:
	@rm -f pad
	@rm -f $(OBJQ)
