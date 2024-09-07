// SPDX-FileCopyrightText: 2023 zocker <zocker@10zen.eu>
// SPDX-License-Identifier: GPL-2.0-or-later
#include <string.h>
#include "strbuf.h"

/**
 * strbuf_cat() - Concatenate a string onto a strbuf-managed string
 *
 * @s: The managed cstring
 * @b: The cstring to add
 *
 * First we get the size of @b with strnlen() (we care about the number of
 * bytes in @b, not the number of chars and if @b is larger than the space
 * allocated for @s , we only care that it _is_ bigger) and get a temporary
 * buffer-handle  into @s->data. If that is not possible (because @s->data
 * has overflowed), we do nothing. Else we call strncat() with the now
 * assembled arguments.
 *
 * Lastly we have to commit the written data, so we first check if the length
 * of @b is larger than the max_size we can use for @s->data and assing -1
 * if that is the case, so we can simplify our call to strbuf_commit() to
 * one.
 *
 * See also: strbuf_get_buf() and strbuf_commit()
 */
void strbuf_cat(struct strbuf *s, char *b)
{
	char *data = NULL;
	size_t max_size = strbuf_get_buf(s, &data);
	size_t b_size = strnlen(b, max_size + CHAR_WIDTH);

	if (!data || !max_size)
		return;

	memccpy(data, b, '\0', max_size);

	if (b_size > max_size)
		b_size = -1;

	strbuf_commit(s, b_size);
}

char *strbuf_str(struct strbuf *s)
{
	if (!s->size)
		return "";

	if (strbuf_buffer_left(s))
		s->data[s->len] = '\0';
	else
		s->data[s->size - 1] = '\0';

	return s->data;
}

size_t strbuf_get_buf(struct strbuf *s, char **bufp)
{
	size_t buffer = strbuf_buffer_left(s);

	if (buffer)
		*bufp = s->data + s->len;
	else
		*bufp = NULL;

	return buffer;
}

void strbuf_clear(struct strbuf *s)
{
	s->len = 0;
	if (s->size)
		s->data[0] = '\0';
}

void strbuf_init(struct strbuf *s, char *data, size_t size)
{
	s->data = data;
	s->size = size;
	strbuf_clear(s);
}

int strbuf_has_overflowed(struct strbuf *s)
{
	return s->len > s->size;
}

void strbuf_set_overflow(struct strbuf *s)
{
	s->len = s->size + 1;
}

size_t strbuf_buffer_left(struct strbuf *s)
{
	if (strbuf_has_overflowed(s))
		return 0;

	return s->size - s->len;
}

size_t strbuf_used(struct strbuf *s)
{
	return min(s->len, s->size);
}

void strbuf_commit(struct strbuf *s, int num)
{
	if (num < 0)
		strbuf_set_overflow(s);
	else
		s->len += num;
}
