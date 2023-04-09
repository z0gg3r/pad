CC = gcc

CFLAGS = -march=native -O2 \
	  -fstack-protector-strong -fcf-protection -fpie \
	  -fPIC -pedantic -pedantic-errors \
	  -fno-delete-null-pointer-checks -Wall -Wextra \
	  -std=c99 -D_XOPEN_SOURCE=600

SFLAGS = -Wbitwise -Wbitwise-pointer -Wcast-truncate \
	 -Wdecl -Wdefault-bitfield-sign -Wdo-while \
	 -Wenum-mismatch -Wflexible-array-sizeof \
	 -Winit-cstring -Wnewline-eof -Wpointer-arith \
	 -Wreturn-void -Wshadow -Wshift-count-negative \
	 -Wshift-count-overflow

OBJQ = main.o padding.o wee-utf8.o

%.o: src/%.c
	@echo CC $<
	@$(CC) -c -o $@ $^ $(CFLAGS)

pad: $(OBJQ)
	@echo CC $^
	@$(CC) $(CFLAGS) $(LIBS) -o $@ $^

check:
	@echo Checking all files with sparse
	@sparse $(SFLAGS) src/*

clean:
	@rm -f pad
	@rm -f $(OBJQ)