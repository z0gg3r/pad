// SPDX-FileCopyrightText: 2023 zocker <zocker@10zen.eu>
// SPDX-License-Identifier: GPL-2.0-or-later
#include <string.h>
#include "common.h"

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
