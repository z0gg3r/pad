// SPDX-FileCopyrightText: 2023 zocker <zocker@10zen.eu>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "padding.h"
#include "common.h"
#include "pad-seccomp.h"

#define PACKAGE "pad"
#define VERSION "0.5.1"
#define PACKAGE_BUGREPORT "zocker@10zen.eu"

#define MODE_LEFT 0x00
#define MODE_RIGHT 0x01
#define MODE_BOTH 0x02
#define MODE_CENTRE 0x03

// Defaults if not specified by commandline
#define DEFAULT_LENGTH 80
#define DEFAULT_CHAR " "
#define DEFAULT_MODE MODE_BOTH

#define CHECK_OPT(x, y, z) !strcmp(x, y) || !strcmp(x, z)
#define FREE_MERGED_ARGV(x)      \
	do {                     \
		if (x) {         \
			free(x); \
		}                \
	} while (0)

/**
 * struct options - All commandline options
 *
 * @length: Length of the final string
 * @padding_char: Char to pad with
 * @mode: How to pad
 * @s: What to pad
 * @help: Help flag
 * @abort: Was parsing aborted
 */
struct options {
	size_t length;
	char *padding_char;
	int mode;
	char *s;
	int err;
	char *merged_argv;
};

// Functions
struct options *parse(int, char **);
char *last_standalone(int, char **);
int hash(char *);
void print_usage(void);
int get_winsize(void);
int ceildiv(int, int);
char *merge_argv(int, char **, int);
size_t slen_args(int, char **, int);

/**
 * print_usage() - Print usage information to stderr
 *
 * Prints usage information (options, modes, version and author) to stderr.
 */
void print_usage(void)
{
	fprintf(stderr,
		"%s [-l LENGTH] [-c CHAR] [-m MODE] STRING\n"
		"Modes are: left, right, centre or both\n"
		"%s v%s - Send Bug reports to %s\n",
		PACKAGE, PACKAGE, VERSION, PACKAGE_BUGREPORT);
}

/**
 * get_winsize() - Return the column number of /dev/tty
 *
 * Report the number of columns avaiable in /dev/tty by first opening it read-only
 * to get a file descriptor, then passing it to ioctl() with the TIOCGWINSZ request
 * to get information about the dimensions of /dev/tty.
 *
 * Returns:
 * * Number of columns
 * * -1 on any error
 */
int get_winsize(void)
{
	struct winsize ws;
	int fd;

	if ((fd = open("/dev/tty", O_RDONLY)) < 0) {
		perror(PACKAGE);
		return -1;
	}

	if (ioctl(fd, TIOCGWINSZ, &ws) < 0) {
		perror(PACKAGE);
		close(fd);
		return -1;
	}

	int tmp = ws.ws_col;
	close(fd);
	return tmp;
}

/**
 * ceildiv() - Divide two integers, ceiled if needed
 *
 * @dividend: The dividend
 * @divisor: The divisor
 *
 * Do a simple division, but if the interger result is
 * smaller than the double result, ceil it.
 *
 * Return: ⌈@dividend / @divisor⌉
 */
int ceildiv(int dividend, int divisor)
{
	double result = (double)dividend / (double)divisor;
	int ret = (int)(dividend / divisor);

	return ((double)ret < result) ? ++ret : ret;
}

/**
 * main() - Main function
 *
 * @argc: Number of arguments
 * @argv: Argument array
 *
 * Parses the options, does the padding and then prints the result.
 *
 * Returns:
 * * 0, if successfull
 * * 1, if not
 */
int main(int argc, char **argv)
{
	int ws;
#ifndef _PAD_DEBUG
	ws = get_winsize();
	if (enable_seccomp() != 0)
		return 1;
#endif
	struct options *o = parse(argc, argv);

	if (!o)
		return 1;

	if (o->err) {
		print_usage();
		int exit_code = o->err - 1;
		FREE_MERGED_ARGV(o->merged_argv);
		free(o);
		return exit_code;
	}

	struct strbuf s = {
		.data = NULL,
		.size = 0,
		.len = 0,
	};

	char *p = calloc(EXPAND_SIZE(o->length) + CHAR_WIDTH, sizeof(char));

	if (!p) {
		perror(PACKAGE);
		FREE_MERGED_ARGV(o->merged_argv);
		free(o);
		return 1;
	}

	strbuf_init(&s, p, EXPAND_SIZE(o->length) + CHAR_WIDTH);

	switch (o->mode) {
	case MODE_LEFT:
		pad_left(o->s, o->length, &s, o->padding_char);
		break;
	case MODE_RIGHT:
		pad_right(o->s, o->length, &s, o->padding_char);
		break;
	case MODE_CENTRE: {
#if _PAD_DEBUG
		ws = get_winsize();
#endif
		if (ws == -1) {
			// There was some error during execution of get_winsize
			// What went wrong was printed to stderr, so we just free
			// p and o, and return 1
			free(p);
			FREE_MERGED_ARGV(o->merged_argv);
			free(o);

			return 1;
		}

		// Get the middle by slicing the size in half
		int half = ceildiv(ws, 2);

		int left = half - ceildiv(o->length, 2);
		size_t len = EXPAND_SIZE(strlen(o->s) + left);
		p = realloc(p, len + CHAR_WIDTH);

		if (!p) {
			perror(PACKAGE);
			free(o);
			free(s.data);

			return 1;
		}

		strbuf_init(&s, p, len + CHAR_WIDTH);
		pad_left("", left, &s, o->padding_char);
		strbuf_cat(&s, o->s);
	} break;
	default:
		pad_both(o->s, o->length, &s, o->padding_char);
	}

	if (!p) {
		FREE_MERGED_ARGV(o->merged_argv);
		free(o);
		return 1;
	}

	printf("%s\n", strbuf_str(&s));

	free(s.data);
	FREE_MERGED_ARGV(o->merged_argv);
	free(o);
	return 0;
}

/**
 * parse() - Parse commandline options
 *
 * @argc: Number of arguments
 * @argv: Argument array
 *
 * Check every element of @argv, but the first, to see if it is one of our defined
 * arguments and set the coressponding values for the struct otpions. Parsing ends
 * early if given the help flag.
 *
 * Returns:
 * * a struct options with all necessary data
 * * NULL on allocation failure
 */
struct options *parse(int argc, char **argv)
{
	char *err = "";
	struct options *o = calloc(1, sizeof(struct options));

	if (!o) {
		perror(PACKAGE);

		return NULL;
	}

	o->merged_argv = NULL; // Ensure that o->merged_argv defaults to NULL

	int flag_length = 0;
	int flag_char = 0;
	int flag_mode = 0;
	int flag_string = 0;
	int flag_merge = 0;

	int i;
	for (i = 1; i < argc; ++i) {
		if (CHECK_OPT(argv[i], "-l", "--length")) {
			if (argc > (i + 1)) {
				flag_length = 1;
				char *tmp;
				o->length = strtoull(argv[i + 1], &tmp, 0);
				if (argv[i + 1] == tmp || errno == ERANGE) {
					err = "Invalid length passed to -l!";
					goto abort;
				}
				++i;
			} else {
				err = "-l was set, but no length was given.";
				goto abort;
			}
		} else if (CHECK_OPT(argv[i], "-c", "--char")) {
			if (argc > (i + 1)) {
				if (strlen(argv[i + 1]) == 0)
					goto char_err;

				flag_char = 1;
				o->padding_char = argv[i + 1];
				++i;
			} else {
char_err:
				err = "-c was set, but no char was given.";
				goto abort;
			}
		} else if (CHECK_OPT(argv[i], "-h", "--help")) {
			goto help;
		} else if (CHECK_OPT(argv[i], "-m", "--mode")) {
			if (argc > (i + 1)) {
				o->mode = hash(argv[i + 1]);
				flag_mode = 1;
				++i;
			} else {
				err = "-m was set, but no mode was given.";
				goto abort;
			}
		} else if (CHECK_OPT(argv[i], "-s", "--string")) {
			if (argc > (i + 1)) {
				flag_string = 1;
				o->s = argv[i + 1];
				++i;
			} else {
				err = "-s was set, but no string was given";
				goto abort;
			}
		} else if (CHECK_OPT(argv[i], "--", "--")) {
			flag_merge = 1;
			++i;
			goto skip;
		} else if (argv[i][0] == '-') {
			err = "Unknown argument";
			goto abort;
		}
	}

skip:
	if (flag_length && o->length < 1) {
		err = "Length should be a non-zero positive integer.";
		goto abort;
	}

	if (!flag_length)
		o->length = DEFAULT_LENGTH;

	if (!flag_char)
		o->padding_char = DEFAULT_CHAR;

	if (!flag_mode)
		o->mode = DEFAULT_MODE;

	if (flag_merge) {
		o->merged_argv = merge_argv(argc, argv, i);
		if (!o->merged_argv) {
			err = "Tried to merge argv, but failed!";
			goto abort;
		}

		o->s = o->merged_argv;
	} else if (!flag_string) {
		o->s = last_standalone(argc, argv);
		if (!strcmp(o->s, "")) {
			err = "No string was passed. If you want to pad an empty string, please use --string";
			goto abort;
		}
	}

	return o;
abort:
	fprintf(stderr, "%s\n", err);
	o->err = 1;
help:
	o->err += 1;
	return o;
}

/**
 * last_standalone() - Return the last standalone argument
 *
 * @argc: Number of arguments
 * @argv: Argument array
 *
 * For every element of @argv, after the first one, check if its free standing,
 * i.e. it is not an option to one of the arguments. If it is free standing, keep
 * track of it until we hit the next one and replace it.
 *
 * Since this uses an empty char * to keep track of standalones empty strings
 * have to be padded with the explicit option.
 *
 * Returns: Either the last standalone string or an empty string
 */
char *last_standalone(int argc, char **argv)
{
	char *s = "";

	for (int i = 1; i < argc; ++i) {
		if (CHECK_OPT(argv[i], "-l", "--length") ||
		    CHECK_OPT(argv[i], "-c", "--char") ||
		    CHECK_OPT(argv[i], "-m", "--mode"))
			++i;
		else
			s = argv[i];
	}

	return s;
}

/**
 * merge_argv() - Merge arguments
 *
 * @argc: Number of arguments
 * @argv: Argument array
 * @i: First argument
 *
 * Merges all arguments from @i until @argc into one string
 *
 * Returns:
 * * A string
 * * NULL on any error
 */
char *merge_argv(int argc, char **argv, int i)
{
	size_t size = EXPAND_SIZE(slen_args(argc, argv, i));

	if (!size)
		return NULL;

	char *s = calloc(size + CHAR_WIDTH, sizeof(char));

	if (!s) {
		perror(PACKAGE);
		return NULL;
	}

	struct strbuf buf = {
		.data = s,
		.size = 0,
		.len = 0,
	};

	strbuf_init(&buf, s, size + CHAR_WIDTH);

	for (; i < argc; ++i) {
		strbuf_cat(&buf, argv[i]);
		strbuf_cat(&buf, " ");
	}

	return strbuf_str(&buf);
}

/**
 * slen_args() - Length of all strings from i upwards
 *
 * @argc: Number of arguments
 * @argv: Argument array
 * @i: From where to sum
 *
 * Sums all strlen()'s of @argv from @i until @argc
 *
 * Returns: Sum of strlen()
 */
size_t slen_args(int argc, char **argv, int i)
{
	size_t size = 0;

	for (; i < argc; ++i)
		size += strlen(argv[i]);

	return size;
}

/**
 * hash() - Quick and dirty switch-hack
 *
 * @c: A string
 *
 * To use a switch when selection the mode, @c needs to be a integer. For that
 * we use this function to strcasecmp() @c to see which mode is requested.
 *
 * Returns: The mode-integer
 */
int hash(char *c)
{
	if (!strncasecmp(c, "left", 4)) {
		return MODE_LEFT;
	} else if (!strncasecmp(c, "right", 5)) {
		return MODE_RIGHT;
	} else if (!strncasecmp(c, "both", 4)) {
		return MODE_BOTH;
	} else if (!strncasecmp(c, "center", 6) ||
		   !strncasecmp(c, "centre", 6)) {
		return MODE_CENTRE;
	}

	return DEFAULT_MODE;
}
