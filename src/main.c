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
#define DEFAULT_CHAR ' '
#define DEFAULT_MODE 0x3

// Helper flags for parsing
int PARSE_ABORT = 0;
int HELP_FLAG = 1;

// Struct holding all possible options
typedef struct options_t {
	int length;
	char _pad;
	int mode;
	char *s;
} options_t;

// Functions
options_t *parse(int, char **);
char *last_standalone(int, char **);
int hash(char *);
void print_usage(char *);


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
 * Parse options, if parsing is aborted print usage and exit
 * If parsing is not aborted allocate a string of length + 1
 * and call pad_{{MODE}}; Print the result and free the options
 * and the allocated string.
 *
 * Returns 0 on success and HELP_FLAG on failure
 * Per default HELP_FLAG is 1, but is set to 0 if 'help' was an
 * option
 */
int main(int argc, char **argv)
{
	options_t *o = parse(argc, argv);

	if (PARSE_ABORT)
		goto failure;

	char *p;
	if (o->mode != 0x4)
		p = calloc((o->length + 1), sizeof(char));

	if (o->mode == 0x1) {
		pad_left(o->s, o->length, p, o->_pad);
	} else if (o->mode == 0x2) {
		pad_right(o->s, o->length, p, o->_pad);
	} else if (o-> mode == 0x4) {
		struct winsize ws;
		int fd;
		fd = open("/dev/tty", O_RDWR);
		if (fd < 0)
			err(1, "/dev/tty");
		if (ioctl(fd, TIOCGWINSZ, &ws) < 0)
			err(1, "/dev/tty");

		int half = ws.ws_col / 2;
		int right = half + 40;
		int left = half - 40;
		for (int i = 0; i < right; ++i) {
			if (i >= left)
				printf("%s", o->s);
			else
				printf("%c", o->_pad);
		}
		printf("\n");
		close(fd);
		goto centered;
	} else {
		pad_both(o->s, o->length, p, o->_pad);
	}

	printf("%s\n", p);

centered:
	free(p);
	free(o);
	return 0;

failure:
	free(o);
	print_usage(argv[0]);
	return HELP_FLAG;
}

/*
 * parse parses argv to look for any encountered options
 * If there is an error with the option, the error is printed
 * to stderr and the parsing is aborted, in which case parse
 * returns NULL. If the options is 'help' the parsing is also
 * aborted, but no error is printed and HELP_FLAG is set to 0.
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
				o->_pad = argv[i + 1][0];
				++i;
			} else {
				err = "-c was set, but no char was given.";
				goto abort;
			}
		} else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			HELP_FLAG = 0;
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

	if (m_flag && o->mode == 0x5) {
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
	} else {
		return 0x5;
	}
}
