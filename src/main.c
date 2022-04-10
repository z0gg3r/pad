// SPDX-FileCopyrightText: 2021 zocker <zocker@10zen.eu>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <padding.h>

// Defaults if not specified by commandline
#define DEFAULT_LENGTH 80
#define DEFAULT_CHAR " "
#define DEFAULT_MODE 0x3

// Helper flags for parsing
int PARSE_ABORT = 0;
int ABORT_WAS_ERROR = 1;

// Struct holding all possible options
typedef struct options_t {
	int length;
	char *_pad;
	int mode;
	char *s;
} options_t;

// Functions
options_t *parse(int, char **);
char *last_standalone(int, char **);
int hash(char *);
void print_usage(char *);
int get_winsize();


/*
 * Prints a help message to stderr
 * The passed string is treated as the name of the binary
 */
void print_usage(char *s)
{
	fprintf(stderr,
		"%s [-l LENGTH] [-c CHAR] [-m MODE] STRING\n"
		"Modes are: left, right, center or both\n"
		"%s v%s - Send Bug reports to %s\n",
		s,
		PACKAGE,
		VERSION,
		PACKAGE_BUGREPORT
	);
}

/*
 * Simple helper like function that returns the size
 * of the terminal window (as columns).
 */
int get_winsize()
{
	struct winsize ws;
	int fd;

	if ((fd = open("/dev/tty", O_RDWR)) < 0) {
		fprintf(stderr, "Failed to open /dev/tty");
		return -1;
	}
	
	if (ioctl(fd, TIOCGWINSZ, &ws) < 0) {
		fprintf(stderr, "Failed to read terminal window size");
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
	options_t *o = parse(argc, argv);

	if (PARSE_ABORT)
		goto failure;

	// While we want length chars, they might be bigger
	// than sizeof(char) (y'know UTF8 and stuff), so we
	// just allocate 5 times length :).
	char *p = calloc((o->length * 5) + 1, sizeof(char));

	switch (o->mode) {
	case 0x01:
		pad_left(o->s, o->length, p, o->_pad);
		break;
	case 0x02:
		pad_right(o->s, o->length, p, o->_pad);
		break;
	case 0x04: {
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
		int left = half - 40;
		p = realloc(p, ((strlen(o->s) + left) * 5) + 1);
		pad_left("", left, p, o->_pad);
		strncat(p, o->s, strlen(o->s));
		}
		break;
	default:
		pad_both(o->s, o->length, p, o->_pad);
	}

	printf("%s\n", p);

	free(p);
	free(o);
	return 0;

failure:
	free(o);
	print_usage(argv[0]);
	return ABORT_WAS_ERROR;
}

/*
 * parse parses argv to look for any encountered options
 * If there is an error with the option, the error is printed
 * to stderr and the parsing is aborted, in which case parse
 * returns NULL. If the options is 'help' the parsing is also
 * aborted, but no error is printed and ABORT_WAS_ERROR is set to 0.
 *
 * RETURNS NULL IF ABORTED, CHECK PARSE_ABORT BEFORE USING RETURN VALUE
 */
options_t *parse(int argc, char **argv)
{
	options_t *o = malloc(sizeof(options_t));

	int l_flag = 0;
	int c_flag = 0;
	int m_flag = 0;
	int s_flag = 0;

	char *err = "";
	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--length")) {
			if (argc > (i + 1)) {
				l_flag =  1;
				o->length = atoi(argv[i + 1]);
				++i;
			} else {
				err = "-l was set, but no length was given.";
				goto abort;
			}
		} else if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--char")) {
			if (argc > (i + 1)) {
				c_flag = 1;
				o->_pad = argv[i + 1];
				++i;
			} else {
				err = "-c was set, but no char was given.";
				goto abort;
			}
		} else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			ABORT_WAS_ERROR = 0;
			goto help;
		} else if (!strcmp(argv[i], "-m") || !strcmp(argv[i], "--mode")) {
			if (argc > (i + 1)) {
				o->mode = hash(argv[i + 1]);
				m_flag = 1;
				++i;
			} else {
				err = "-m was set, but no mode was given.";
				goto abort;
			}
		} else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--string")) {
			if (argc > (i + 1)) {
				s_flag = 1;
				o->s = argv[i + 1];
				++i;
			} else {
				err = "-s was set, but no string was given";
				goto abort;
			}
		}
	}

	if (l_flag && o->length < 1) {
		err = "Length should be a non-zero positive integer.";
		goto abort;
	}

	if (m_flag && !o->mode) {
		err = "Please provide a supported padding mode. See --help for a list of modes.";
		goto abort;
	}


	if (!l_flag)
		o->length = DEFAULT_LENGTH;
	if (!c_flag)
		o->_pad = DEFAULT_CHAR;
	if (!m_flag)
		o->mode = DEFAULT_MODE;
	if (!s_flag) {
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
		if (!strcmp(argv[i], "-l")
	        	|| !strcmp(argv[i], "--length")
	                || !strcmp(argv[i], "-c")
	                || !strcmp(argv[i], "--char")
	                || !strcmp(argv[i], "-m")
	                || !strcmp(argv[i], "--mode"))
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
 * Possible return values:
 * 0x1 = left 
 * 0x2 = right
 * 0x3 = both
 * 0x4 = center
 * 0x5 = invalid
 */
int hash(char *c)
{
	if (!strcmp(c, "left") || !strcmp(c, "LEFT")) {
		return 0x1;
	} else if (!strcmp(c, "right") || !strcmp(c, "RIGHT")) {
		return 0x2;
	} else if (!strcmp(c, "both") || !strcmp(c, "BOTH")) {
		return 0x3;
	} else if (!strcmp(c, "center") || !strcmp(c, "CENTER")) {
		return 0x4;
	} else if(!strcmp(c, "centre") || !strcmp(c, "CENTRE")) {
		return 0x4;
	} else {
		return 0x0;
	}
}
