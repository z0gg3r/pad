// SPDX-FileCopyrightText: 2021 zocker <zocker@10zen.eu>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "wee-utf8.h" // stolen from Weechat (https://weechat.org)
#include "padding.h"

/*
 * Takes an input string, a result string buffer and a character
 * and pads the input string to the size of the result buffer,
 * with the differnce in size being filled up with the char.
 * Char will be added to the left of the string.
 * Size should not count the null terminator.
 */
char *pad_left(char *s, int size, char *p, char *_pad)
{
	if (strlen(s) >= (size_t)size)
		return s;

	char *buf = padding(size - strlen(s), _pad);

	int pc = EXPAND_SIZE(size);

	/*
	strncat(p, buf, strlen(buf));
	strncat(p, s, strlen(s));
	*/
	strncat(p, buf, pc);
	strncat(p, s, pc);

	free(buf);
	return p;
}

/*
 * Char will be added to both sides of the string.
 */
char *pad_both(char *s, int size, char *p, char *_pad)
{
	if (strlen(s) >= (size_t)size)
		return s;
	int b_size = (size - strlen(s)) / 2;

	char *buf = padding(b_size, _pad);

	int pc = EXPAND_SIZE(size);

	strncat(p, buf, pc);
	strncat(p, s, pc);
	strncat(p, buf, pc);

	free(buf);

	return p;
}

/*
 * Char will be added to the left of the string.
 */
char *pad_right(char *s, int size, char *p, char *_pad)
{
	if (strlen(s) >= (size_t)size)
		return s;

	char *buf = padding(size - strlen(s), _pad);

	int pc = EXPAND_SIZE(size);
	strncat(p, s, pc);
	strncat(p, buf, pc);

	free(buf);

	return p;
}

/*
 * Takes an integer size and a character p and a string
 * containing p size times. Size is the number of characters,
 * not the allocated memory!
 */
char *padding(int size, char *p)
{
	char *tmp = calloc(CHAR_WIDTH, sizeof(char));
	utf8_int_string(utf8_char_int(p), tmp);

	int tmp_len = strlen(tmp);
	// Size is # chars not # bytes, so if we have an utf8 `char`
	// of length 1, that is not also strlen 1, we multiply size
	// by the length of the string (that is the # bytes in the
	// string)
	if (utf8_strlen(tmp) == 1 && tmp_len != 1)
		size *= tmp_len;

	char *s = calloc(size + 1, sizeof(char));

	// Copy the `character` into `s`
	// Cannot use strncat since that fucks the string up
	// Since the `character` may be multi-byte, we can't
	// just do `s[i] = tmp` :(
	for (int i = 0; i < size - tmp_len; i += tmp_len)
		for (int j = 0; j < tmp_len; ++j)
			s[i + j] = tmp[j];
	s[size] = '\0';

	free(tmp);

	return s;
}
