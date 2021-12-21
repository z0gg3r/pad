// SPDX-FileCopyrightText: 2021 zocker <zocker@10zen.eu>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <padding.h>

#define DEFAULT_LENGTH 80
#define DEFAULT_CHAR ' '
#define DEFAULT_MODE 0x3

int PARSE_ABORT = 0;
int HELP_FLAG = 1;

typedef struct options_t {
	int length;
	char _pad;
	int mode;
	char *s;
} options_t;

options_t *parse(int, char **);
char *last_standalone(int, char **);
int hash(char *);
void print_usage(char *);

void print_usage(char *s)
{
	fprintf(stderr,
	        "%s [-l LENGTH] [-c CHAR] [-m MODE] STRING\n"
	        "Modes are: left, right or both\n"
	        "%s v%s - Send Bug reports to %s\n",
	        s,
	        PACKAGE,
	        VERSION,
	        PACKAGE_BUGREPORT
	       );
}

int main(int argc, char **argv)
{
	options_t *o = parse(argc, argv);

	if (PARSE_ABORT)
		goto failure;

	char *p = calloc((o->length + 1), sizeof(char));

	if (o->mode == 0x1)
		pad_left(o->s, o->length, p, o->_pad);
	else if (o->mode == 0x2)
		pad_right(o->s, o->length, p, o->_pad);
	else
		pad_both(o->s, o->length, p, o->_pad);

	printf("%s\n", p);

	free(p);
	free(o);
	return 0;

failure:
	print_usage(argv[0]);
	return HELP_FLAG;
}

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

	if (m_flag && o->mode == 0x4) {
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

int hash(char *c)
{
	if (!strcmp(c, "left") || !strcmp(c, "LEFT")) {
		return 0x1;
	} else if (!strcmp(c, "right") || !strcmp(c, "RIGHT")) {
		return 0x2;
	} else if (!strcmp(c, "both") || !strcmp(c, "BOTH")) {
		return 0x3;
	} else {
		return 0x4;
	}
}
