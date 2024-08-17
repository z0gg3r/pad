#!/bin/sh

TEST_INPUT_COUNT="$(find tests -type f | wc -l)"
VALGRIND_ERROR_EXIT="$(shuf -i 1-125 -n 1)"

cleanup_binary()
{
	rm -f binary
}

compile_binary_debug()
{
	cleanup_binary
	cc -g -O0 -lseccomp -D_PAD_DEBUG src/*.c -o binary
}

compile_binary_prodish()
{
	cleanup_binary
	cc -g -O2 -lseccomp -D_PAD_DEBUG -pipe -march=native -fstack-protector-strong -fcf-protection \
		-fpie -fPIC -std=c99 -D_DEFAULT_SOURCE -fno-delete-null-pointer-checks \
		-fno-strict-overflow -fno-strict-aliasing \
		-ftrivial-auto-var-init=zero -fstrict-flex-arrays=3 \
		-fstack-clash-protection \
		-Wformat=2 -Wtrampolines \
		-Wimplicit-fallthrough \
		-fsanitize=address \
		-pedantic -pedantic-errors -Wall -Wextra -lseccomp -Wl,-z,defs \
		-Wl,-z,now -Wl,-z,relro -Wl,-z,nodlopen -Wl,-z,noexecstack \
		src/*.c -o binary
}


run_test()
{
	FILE="$1"

	valgrind --quiet --error-exitcode="$VALGRIND_ERROR_EXIT" --leak-check=full \
		--show-leak-kinds=all -s --track-origins=yes ./binary --string "$(cat "$FILE")" > /dev/null

	printf '%b\n' "$?"
}

run_tests_debug()
{
	printf 'RUNNING DEBUG TESTS\n'
	compile_binary_debug
	failed_tests=0
	i=1
	find tests -type f | while read -r testcase
	do
		printf '\nRunning test #%b of %b\n' "$i" "$TEST_INPUT_COUNT"
		if [ "$(run_test "$testcase")" = "$VALGRIND_ERROR_EXIT" ]
		then
			failed_tests=$((failed_tests + 1))
			printf 'TEST NO. %b (%b) FAILED!\n' "$i" "$testcase"
		fi
		i=$((i + 1))
	done
}


run_tests_prodish()
{
	printf 'RUNNING PROD(-ish) TESTS\n'
	compile_binary_prodish
	failed_tests=0
	i=1
	find tests -type f | while read -r testcase
	do
		printf '\nRunning test #%b of %b\n' "$i" "$TEST_INPUT_COUNT"
		if [ "$(run_test "$testcase")" = "$VALGRIND_ERROR_EXIT" ]
		then
			failed_tests=$((failed_tests + 1))
			printf 'TEST NO. %b (%b) FAILED!\n' "$i" "$testcase"
		fi
		i=$((i + 1))
	done
}

trap cleanup_binary QUIT INT TERM EXIT
run_tests_debug
run_tests_prodish
