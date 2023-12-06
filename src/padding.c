// SPDX-FileCopyrightText: 2021 zocker <zocker@10zen.eu>
//
// SPDX-License-Identifier: GPL-2.0-or-later

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
int pad_left(char *s, int size, struct str_buf *p, char *_pad)
{
	size_t slen = utf8_strlen(s);
	if (slen >= (size_t)size) {
		str_buf_cat(p, s);
		return str_buf_has_overflowed(p);
	}

	char *buf = padding(size - slen + 1, _pad);

	if (!buf) {
		return 0;
	}

	str_buf_cat(p, buf);
	str_buf_cat(p, s);

	free(buf);
	return str_buf_has_overflowed(p);
}

/*
 * Char will be added to both sides of the string.
 */
int pad_both(char *s, int size, struct str_buf *p, char *_pad)
{
	size_t slen = utf8_strlen(s);
	if (slen >= (size_t)size) {
		str_buf_cat(p, s);
		return str_buf_has_overflowed(p);
	}
	int b_size = (size - slen) / 2;

	char *buf = padding(b_size + 1, _pad);

	if (!buf) {
		return 0;
	}

	str_buf_cat(p, buf);
	str_buf_cat(p, s);
	str_buf_cat(p, buf);

	free(buf);

	return str_buf_has_overflowed(p);
}

/*
 * Char will be added to the left of the string.
 */
int pad_right(char *s, int size, struct str_buf *p, char *_pad)
{
	size_t slen = utf8_strlen(s);
	if (slen >= (size_t)size) {
		str_buf_cat(p, s);
		return str_buf_has_overflowed(p);
	}

	char *buf = padding(size - slen + 1, _pad);

	if (!buf) {
		return 0;
	}

	str_buf_cat(p, s);
	str_buf_cat(p, buf);

	free(buf);

	return str_buf_has_overflowed(p);
}

/*
 * Takes an integer size and a character p and a string
 * containing p size times. Size is the number of characters,
 * not the allocated memory!
 */
char *padding(int size, char *p)
{
	char *tmp = calloc(CHAR_WIDTH, sizeof(char));

	if (!tmp) {
		perror("pad - padding");
		return NULL;
	}

	utf8_int_string(utf8_char_int(p), tmp);

	int tmp_len = strlen(tmp);
	// Size is # chars not # bytes, so if we have an utf8 `char`
	// of length 1, that is not also strlen 1, we multiply size
	// by the length of the string (that is the # bytes in the
	// string)
	if (utf8_strlen(tmp) == 1 && tmp_len != 1)
		size *= tmp_len;

	char *s = calloc(size + 1, sizeof(char));

	if (!s) {
		perror("pad - padding");
		free(tmp);

		return NULL;
	}

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
