// SPDX-FileCopyrightText: Modifications by zocker <zocker@10zen.eu>; Original from include/linux/seq_buf.h, see seq_buf.authors for list of committers
// SPDX-License-Identifier: GPL-2.0-only
// Adapted from include/linux/seq_buf.h
// Modifications are:
// - Replacing seq_ with str_
// - Renaming 'buffer' to 'data'
// - Adding strbuf_cat()
// - Dropping some kernel-specific stuff (like WARN_ON)
// - Dropping all functions w/o bodies in the header
#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>

#define CHAR_WIDTH 5
#define EXPAND_SIZE(x) x * CHAR_WIDTH
#define min(x, y) (x < y) ? x : y

struct strbuf {
	char *data; /* The wrapped string */
	size_t size; /* Maximum size of data */
	size_t len; /* Current length of data */
};

void strbuf_cat(struct strbuf *, char *);

static inline void strbuf_clear(struct strbuf *s)
{
	s->len = 0;
	if (s->size)
		s->data[0] = '\0';
}

static inline void strbuf_init(struct strbuf *s, char *data, size_t size)
{
	s->data = data;
	s->size = size;
	strbuf_clear(s);
}

static inline int strbuf_has_overflowed(struct strbuf *s)
{
	return s->len > s->size;
}

static inline void strbuf_set_overflow(struct strbuf *s)
{
	s->len = s->size + 1;
}

static inline size_t strbuf_buffer_left(struct strbuf *s)
{
	if (strbuf_has_overflowed(s))
		return 0;

	return s->size - s->len;
}

static inline size_t strbuf_used(struct strbuf *s)
{
	return min(s->len, s->size);
}

static inline char *strbuf_str(struct strbuf *s)
{
	if (!s->size)
		return "";

	if (strbuf_buffer_left(s))
		s->data[s->len] = '\0';
	else
		s->data[s->size - 1] = '\0';

	return s->data;
}

static inline size_t strbuf_get_buf(struct strbuf *s, char **bufp)
{
	if (s->len < s->size) {
		*bufp = s->data + s->len;

		return s->size - s->len;
	}

	*bufp = NULL;

	return 0;
}

static inline void strbuf_commit(struct strbuf *s, int num)
{
	if (num < 0)
		strbuf_set_overflow(s);
	else
		s->len += num;
}

#endif
