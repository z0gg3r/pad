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
#include "padding.h"
#include "common.h"

#define PACKAGE "pad"
#define VERSION "0.2.1"
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

// Helper flags for parsing
static int PARSE_ABORT = 0;
static int ABORT_WAS_ERROR = 1;

// Struct holding all possible options
struct options {
	int length;
	char *_pad;
	int mode;
	char *s;
};

// Functions
struct options *parse(int, char **);
char *last_standalone(int, char **);
int hash(char *);
void print_usage(char *);
int get_winsize(void);

/*
 * Prints a help message to stderr
 * The passed string is treated as the name of the binary
 */
void print_usage(char *s)
{
	fprintf(stderr,
		"%s [-l LENGTH] [-c CHAR] [-m MODE] STRING\n"
		"Modes are: left, right, centre or both\n"
		"%s v%s - Send Bug reports to %s\n",
		s, PACKAGE, VERSION, PACKAGE_BUGREPORT);
}

/*
 * Simple helper like function that returns the size
 * of the terminal window (as columns).
 */
int get_winsize(void)
{
	struct winsize ws;
	int fd;

	if ((fd = open("/dev/tty", O_RDONLY)) < 0) {
		fprintf(stderr, "Failed to open /dev/tty");
		return -1;
	}

	if (ioctl(fd, TIOCGWINSZ, &ws) < 0) {
		fprintf(stderr, "Failed to read terminal window size");
		close(fd);
		return -1;
	}

	int tmp = ws.ws_col;
	close(fd);
	return tmp;
}

/*
 * Parse options, if parsing is aborted print usage and exit
 * If parsing is not aborted allocate a string of length + 1
 * and call pad_{{MODE}}; Print the result and free the options
 * and the allocated string.
 *
 * Returns 0 on success and ABORT_WAS_ERROR on failure
 * Per default ABORT_WAS_ERROR is 1, but is set to 0 if 'help'
 * aborted parsing
 */
int main(int argc, char **argv)
{
	struct options *o = parse(argc, argv);

	if (PARSE_ABORT)
		goto failure;

	struct str_buf s = {
		.data = NULL,
		.size = 0,
		.len = 0,
	};
	// While we want length chars, they might be bigger
	// than sizeof(char) (y'know UTF8 and stuff), so we
	// just allocate 5 times length :).
	char *p = calloc(EXPAND_SIZE(o->length) + CHAR_WIDTH, sizeof(char));

	if (!p) {
		perror(PACKAGE);
		free(o);
		return 1;
	}

	str_buf_init(&s, p, EXPAND_SIZE(o->length) + CHAR_WIDTH);

	switch (o->mode) {
	case MODE_LEFT:
		pad_left(o->s, o->length, &s, o->_pad);
		break;
	case MODE_RIGHT:
		pad_right(o->s, o->length, &s, o->_pad);
		break;
	case MODE_CENTRE: {
		int ws = 0;
		if ((ws = get_winsize()) == -1) {
			// There was some error during execution of get_winsize
			// What went wrong was printed to stderr, so we just free
			// p and goto failure
			free(p);
			goto failure;
		}

		// We get half the screen size to get the middle
		int half = ws / 2;
		// And subtract 40 to get the starting point.
		// NOTE: This assumes 80 character lines, but
		//       that much should be expected c:
		int left = half - 40;
		size_t len = EXPAND_SIZE(strlen(o->s) + left);
		p = realloc(p, len + CHAR_WIDTH);

		if (!p) {
			perror(PACKAGE);
			free(o);
			free(s.data);

			return 1;
		}

		str_buf_init(&s, p, len + CHAR_WIDTH);
		pad_left("", left, &s, o->_pad);
		str_buf_cat(&s, o->s);
	} break;
	default:
		pad_both(o->s, o->length, &s, o->_pad);
	}

	if (!p) {
		free(o);
		return 1;
	}

	printf("%s\n", str_buf_str(&s));

	free(s.data);
	free(o);
	return 0;

failure:
	free(o);
	print_usage(PACKAGE);
	return ABORT_WAS_ERROR;
}

/*
 * Parses argv to look for any encountered options
 * If there is an error with the option, the error is printed
 * to stderr and the parsing is aborted, in which case parse
 * returns NULL. If the options is 'help' the parsing is also
 * aborted, but no error is printed and ABORT_WAS_ERROR is set to 0.
 *
 * RETURNS NULL IF ABORTED, CHECK PARSE_ABORT BEFORE USING RETURN VALUE
 */
struct options *parse(int argc, char **argv)
{
	struct options *o = calloc(1, sizeof(struct options));

	int flag_length = 0;
	int flag_char = 0;
	int flag_mode = 0;
	int flag_string = 0;

	char *err = "";
	for (int i = 1; i < argc; ++i) {
		if (CHECK_OPT(argv[i], "-l", "--length")) {
			if (argc > (i + 1)) {
				flag_length = 1;
				o->length = atoi(argv[i + 1]);
				++i;
			} else {
				err = "-l was set, but no length was given.";
				goto abort;
			}
		} else if (CHECK_OPT(argv[i], "-c", "--char")) {
			if (argc > (i + 1)) {
				flag_char = 1;
				o->_pad = argv[i + 1];
				++i;
			} else {
				err = "-c was set, but no char was given.";
				goto abort;
			}
		} else if (CHECK_OPT(argv[i], "-h", "--help")) {
			ABORT_WAS_ERROR = 0;
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
		}
	}

	if (flag_length && o->length < 1) {
		err = "Length should be a non-zero positive integer.";
		goto abort;
	}

	if (!flag_length)
		o->length = DEFAULT_LENGTH;

	if (!flag_char)
		o->_pad = DEFAULT_CHAR;

	if (!flag_mode)
		o->mode = DEFAULT_MODE;

	if (!flag_string) {
		o->s = last_standalone(argc, argv);
		if (!strcmp(o->s, "")) {
			err = "No string was passed. If you want to pad an empty string, please use --string";
			goto abort;
		}
	}

	return o;
abort:
	fprintf(stderr, "%s\n", err);
help:
	PARSE_ABORT = 1;
	free(o);
	return NULL;
}

/*
 * Searches the command line arguments for the last freestanding
 * string, that is the last string that is not a recognized option
 * or an argument to a recognized option. Since this is called after
 * parsing we know that all set options are valid and we just can
 * increment the index if they are encountered.
 *
 * Returns either the found string or an empty string.
 * Since we cannot distinguish between an actual empty string as
 * argument and the case of not having a last string.
 * empty strings are treated as no argument in parse() and need to
 * be passed to pad with -s instead.
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

/*
 * A simple function that takes in a string as an argument
 * and retunrs an integer. This is used to 'quickly' decide
 * the requested padding mode.
 */
int hash(char *c)
{
	if (!strcasecmp(c, "left")) {
		return MODE_LEFT;
	} else if (!strcasecmp(c, "right")) {
		return MODE_RIGHT;
	} else if (!strcasecmp(c, "both")) {
		return MODE_BOTH;
	} else if (!strcasecmp(c, "center") || !strcasecmp(c, "centre")) {
		return MODE_CENTRE;
	}

	return DEFAULT_MODE;
}
