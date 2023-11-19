// SPDX-FileCopyrightText: 2023 zocker <zocker@10zen.eu>
// SPDX-License-Identifier: GPL-2.0-or-later
#include <string.h>
#include "common.h"

void str_buf_cat(struct str_buf *s, char *b)
{
	size_t b_size = strlen(b);

	char *data = NULL;
	size_t max_size = str_buf_get_buf(s, &data);

	if (!data || !max_size)
		return;

	strncat(data, b, max_size);

	if (b_size > max_size)
		b_size = -1;

	str_buf_commit(s, b_size);
}
