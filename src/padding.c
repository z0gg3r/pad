// SPDX-FileCopyrightText: 2021 zocker <zocker@10zen.eu>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "wee-utf8.h" // stolen from Weechat (https://weechat.org)
#include "padding.h"

/**
 * pad_left() - Left pad a string
 *
 * @s: The string that shall be padded
 * @size: Size of the padded string
 * @p: Buffer to hold the padded string
 * @padding_char: Padding character
 *
 * We crate a string that holds @padding_characcter @size times (minus the length
 * of @s) and cat it onto @p so that we get a string of the form <PADDING>@s.
 *
 * Returns:
 * * 0 on success
 * * 1 on failure or str_buf overflow
 */
int pad_left(char *s, size_t size, struct str_buf *p, char *padding_char)
{
	size_t slen = utf8_strnlen(s, size + CHAR_WIDTH);
	if (slen >= (size_t)size) {
		str_buf_cat(p, s);
		return str_buf_has_overflowed(p);
	}

	char *buf = padding(size - slen + 1, padding_char);

	if (!buf) {
		return 0;
	}

	str_buf_cat(p, buf);
	str_buf_cat(p, s);

	free(buf);
	return str_buf_has_overflowed(p);
}

/**
 * pad_both() - Left and right pad a string
 *
 * @s: The string that shall be padded
 * @size: Size of the padded string
 * @p: Buffer to hold the padded string
 * @padding_char: Padding character
 *
 * Like pad_left(), but taking the form <PADDING>@s<PADDING>.
 *
 * Returns:
 * * 0 on success
 * * 1 on failure or str_buf overflow
 *
 * See pad_left()
 */
int pad_both(char *s, size_t size, struct str_buf *p, char *padding_char)
{
	size_t slen = utf8_strnlen(s, size + CHAR_WIDTH);
	if (slen >= (size_t)size) {
		str_buf_cat(p, s);
		return str_buf_has_overflowed(p);
	}
	int b_size = (size - slen) / 2;

	char *buf = padding(b_size + 1, padding_char);

	if (!buf) {
		return 0;
	}

	str_buf_cat(p, buf);
	str_buf_cat(p, s);
	str_buf_cat(p, buf);

	free(buf);

	return str_buf_has_overflowed(p);
}

/**
 * pad_right() - Right pad a string
 *
 * @s: The string that shall be padded
 * @size: Size of the padded string
 * @p: Buffer to hold the padded string
 * @pading_char: Padding character
 *
 * Like pad_left() but with the form @s<PADDING>.
 *
 * Returns:
 * * 0 on success
 * * 1 on failure or str_buf overflow
 *
 * See pad_left()
 */
int pad_right(char *s, size_t size, struct str_buf *p, char *padding_char)
{
	size_t slen = utf8_strnlen(s, size + CHAR_WIDTH);
	if (slen >= (size_t)size) {
		str_buf_cat(p, s);
		return str_buf_has_overflowed(p);
	}

	char *buf = padding(size - slen + 1, padding_char);

	if (!buf) {
		return 0;
	}

	str_buf_cat(p, s);
	str_buf_cat(p, buf);

	free(buf);

	return str_buf_has_overflowed(p);
}

/**
 * padding() - Create a padding string
 *
 * @size: Length of the padding string
 * @p: The character to create the padding string with
 *
 * First we calculate how much space the padding string will take by checking
 * if @p is of length one or of utf8-length one and then multiplying by both @size
 * and strnlen(). Then we copy @p into the buffer @size times.
 *
 * Returns:
 * * A padded string
 * * NULL on any error
 */
char *padding(size_t size, char *p)
{
	char *tmp = calloc(CHAR_WIDTH, sizeof(char));

	if (!tmp) {
		perror("pad - padding");
		return NULL;
	}

	utf8_int_string(utf8_char_int(p), tmp);

	int tmp_len = strnlen(tmp, EXPAND_SIZE(CHAR_WIDTH));
	// Size is # chars, not # bytes --- so if we have an utf8 `char`
	// of length 1, that is not also strlen 1, we multiply size
	// by the length of the string (that is the # bytes in the
	// string)
	if (utf8_strnlen(tmp, CHAR_WIDTH) == 1 && tmp_len != 1)
		size *= tmp_len;

	char *s = calloc(size + 1, sizeof(char));

	if (!s) {
		perror("pad - padding");
		free(tmp);

		return NULL;
	}

	// Copy the `character` into `s`
	// Cannot use strncat since that fucks the string up
	// and since the `character` may be multi-byte, we can't
	// just do `s[i] = tmp` :(
	for (size_t i = 0; i < size - tmp_len; i += tmp_len)
		for (int j = 0; j < tmp_len; ++j)
			s[i + j] = tmp[j];
	s[size] = '\0';

	free(tmp);

	return s;
}
