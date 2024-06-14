include config.mk

CC = gcc

VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all -s --track-origins=yes

OBJQ = main.o padding.o wee-utf8.o strbuf.o pad-seccomp.o

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

test: pad
	@echo Checking pad with mode left
	@valgrind $(VALGRIND_FLAGS) ./pad -m left -l 25 -c "᪥" "String※"
	@echo Checking pad with mode right
	@valgrind $(VALGRIND_FLAGS) ./pad -m right -l 25 -c "᪥" "String※"
	@echo Checking pad with mode both
	@valgrind $(VALGRIND_FLAGS) ./pad -m both -l 25 -c "᪥" "String※"
	@echo Checking pad with mode centre
	@valgrind $(VALGRIND_FLAGS) ./pad -m centre -l 25 -c "᪥" "String※"

clean:
	@rm -f pad
	@rm -f $(OBJQ)

.PHONY: clean, check, install, test
