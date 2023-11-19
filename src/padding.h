// SPDX-FileCopyrightText: 2021 zocker <zocker@10zen.eu>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef PADDING_H
#define PADDING_H
#include "common.h"

#define CHAR_WIDTH 5

#define EXPAND_SIZE(x) x * CHAR_WIDTH

// input, size of result, result string, padding
char *pad_left(char *, int, struct str_buf *, char *);
char *pad_right(char *, int, struct str_buf *, char *);
char *pad_both(char *, int, struct str_buf *, char *);
char *padding(int, char *);

#endif
