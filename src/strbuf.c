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

/**
 * strbuf_str() - Return @s->@data as a nul-terminated string
 *
 * @s: The managed cstring
 *
 * Sometimes we need to operate on the whole string. This functions adds the
 * final NUL byte after the string (wherever that may be) and then returns
 * it.
 *
 * Returns: The managed string, with a final NUL byte
 */
char *strbuf_str(struct strbuf *s)
{
	if (!s->size)
		return "";

	size_t data_end = min(s->size - 1, s->len);

	s->data[data_end] = '\0';

	return s->data;
}

/**
 * strbuf_get_buf() - Return a pointer to append to the current string
 *
 * @s: The managed cstring
 * @bufp: Pointer to a char pointer
 *
 * If there is still space left to write stuff, set *@bufp to the current end of
 * the cstring (that is @s->data + @s->len); If not, set *@bufp to NULL.
 *
 * Returns:
 * * Pointer to writeable memory
 * * NULL, if there is no space left to write anything
 */
size_t strbuf_get_buf(struct strbuf *s, char **bufp)
{
	size_t buffer = strbuf_buffer_left(s);

	if (buffer)
		*bufp = s->data + s->len;
	else
		*bufp = NULL;

	return buffer;
}

/**
 * strbuf_clear() - Virtually clear written data
 *
 * @s: The managed cstring
 *
 * This (virtually) clears the managed cstring, by setting @s->len to 0 and
 * optionally (if @s->size is non-zero) setting s->data[0] to \0. This clears
 * the cstring for further writing, with calls to strbuf_get_buf() et al, but
 * does _not_ free the underlying data buffer.
 */
void strbuf_clear(struct strbuf *s)
{
	s->len = 0;
	if (s->size)
		s->data[0] = '\0';
}

/**
 * strbuf_init() - Initalise a strbuf struct
 *
 * @s: Local strbuf struct
 * @data: The allocated buffer
 * @size: Size of @data
 *
 * Initalise @s with @data and @size and then call strbuf_clear() to setup @s->len
 * tracking.
 *
 * See also: strbuf_clear()
 */
void strbuf_init(struct strbuf *s, char *data, size_t size)
{
	s->data = data;
	s->size = size;
	strbuf_clear(s);
}

/**
 * strbuf_has_overflowed() - Check if a cstring has overflowed
 *
 * @s: The managed cstring
 *
 * A managed cstring @s has overflowed if the written length (@s->len) has exceeded
 * the allocated size of the buffer (@s->size), or hopefully only would have
 * exceeded said buffer.
 *
 * Returns:
 * * 0, if @s has not overflowed
 * * 1, if @s has overflowed
 */
int strbuf_has_overflowed(struct strbuf *s)
{
	return s->len > s->size;
}

/**
 * strbuf_set_overflow() - Set cstring overflow condtion
 *
 * @s: The managed cstring
 *
 * If an operation overfloweed or would have caused an overflow of the managed
 * buffer @s->data, set the written length @s->len to be one more than the size
 * of the managed buffer (@s->size)
 */
void strbuf_set_overflow(struct strbuf *s)
{
	s->len = s->size + 1;
}

/**
 * strbuf_buffer_left() - Check how much of the buffer is left
 *
 * @s: The managed cstring
 *
 * To check how much of the buffer we have left, first we check if @s has
 * overflowed and then, if it has not, calculate the unused space by
 * subtracting the written size from the maximum size (that is @s->size - @s->len)
 *
 * Returns:
 * * The size of the remaining buffer
 * * 0, if there is no remaining buffer
 */
size_t strbuf_buffer_left(struct strbuf *s)
{
	if (strbuf_has_overflowed(s))
		return 0;

	return s->size - s->len;
}

/**
 * strbuf_used() - How much of the buffer is currently used
 *
 * @s: The managed cstring
 *
 * Determine how much of the buffer has been used.
 *
 * Returns: The smaller of @s->len and @s->size
 */
size_t strbuf_used(struct strbuf *s)
{
	return min(s->len, s->size);
}

/**
 * strbuf_commit() - Commit written data
 *
 * @s: The managed cstring
 * @num: Number of bytes written
 *
 * If @num is less than zero, there is an overflow condtion --- that is the user
 * either did, tried or would have written more data than the buffer can hold ---
 * and we take note of that. Else @num is added to the written length s->len.
 *
 * See also: strbuf_set_overflow()
 */
void strbuf_commit(struct strbuf *s, int num)
{
	if (num < 0)
		strbuf_set_overflow(s);
	else
		s->len += num;
}
