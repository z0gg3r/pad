// SPDX-FileCopyrightText: 2021 zocker <zocker@10zen.eu>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef PADDING_H
#define PADDING_H
#include "common.h"

#define CHAR_WIDTH 5

#define EXPAND_SIZE(x) x * CHAR_WIDTH

// input, size of result, result string, padding
int pad_left(char *, size_t, struct str_buf *, char *);
int pad_right(char *, size_t, struct str_buf *, char *);
int pad_both(char *, size_t, struct str_buf *, char *);
char *padding(size_t, char *);

#endif
