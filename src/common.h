// SPDX-FileCopyrightText: Modifications by zocker <zocker@10zen.eu>; Original by "Steven Rostedt (Google)" <rostedt@goodmis.org>, Sergey Senozhatsky <senozhatsky@chromium.org>, Petr Mladek <pmladek@suse.com> et al
// SPDX-License-Identifier: GPL-2.0-only
// Adapted from include/linux/seq_buf.h
// Modifications are:
// - Replacing seq_ with str_
// - Renaming 'buffer' to 'data'
// - Adding str_buf_cat()
// - Dropping some kernel-specific stuff (like WARN_ON)
// - Dropping all functions w/o bodies in the header
#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>

#define min(x, y) (x < y) ? x : y

struct str_buf {
	char *data; /* The wrapped string */
	size_t size; /* Maximum size of data */
	size_t len; /* Current length of data */
};

void str_buf_cat(struct str_buf *, char *);

static inline void str_buf_clear(struct str_buf *s)
{
	s->len = 0;
	if (s->size)
		s->data[0] = '\0';
}

static inline void str_buf_init(struct str_buf *s, char *data, size_t size)
{
	s->data = data;
	s->size = size;
	str_buf_clear(s);
}

static inline int str_buf_has_overflowed(struct str_buf *s)
{
	return s->len > s->size;
}

static inline void str_buf_set_overflow(struct str_buf *s)
{
	s->len = s->size + 1;
}

static inline size_t str_buf_buffer_left(struct str_buf *s)
{
	if (str_buf_has_overflowed(s))
		return 0;

	return s->size - s->len;
}

static inline size_t str_buf_used(struct str_buf *s)
{
	return min(s->len, s->size);
}

static inline char *str_buf_str(struct str_buf *s)
{
	if (!s->size)
		return "";

	if (str_buf_buffer_left(s))
		s->data[s->len] = '\0';
	else
		s->data[s->size - 1] = '\0';

	return s->data;
}

static inline size_t str_buf_get_buf(struct str_buf *s, char **bufp)
{
	if (s->len < s->size) {
		*bufp = s->data + s->len;

		return s->size - s->len;
	}

	*bufp = NULL;

	return 0;
}

static inline void str_buf_commit(struct str_buf *s, int num)
{
	if (num < 0)
		str_buf_set_overflow(s);
	else
		s->len += num;
}

#endif
