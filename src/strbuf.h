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
char *strbuf_str(struct strbuf *);
size_t strbuf_get_buf(struct strbuf *, char **);
void strbuf_clear(struct strbuf *);
void strbuf_init(struct strbuf *, char *, size_t);
int strbuf_has_overflowed(struct strbuf *);
void strbuf_set_overflow(struct strbuf *);
size_t strbuf_buffer_left(struct strbuf *);
size_t strbuf_used(struct strbuf *);
void strbuf_commit(struct strbuf *, int);


#endif
