#ifndef SPOOF_H
#define SPOOF_H

#include <sys/types.h>

#ifndef _USE_JEMALLOC
#include <stdlib.h>

#define _malloc_(x) malloc(x)
#define _calloc_(x, y) calloc(x, y)
#define _realloc_(x, y) realloc(x, y)
#define _free_(x) free(x)

#else

#include <jemalloc/jemalloc.h>

#define _malloc_(x) malloc(x)
#define _calloc_(x, y) calloc(x, y)
#define _realloc_(x, y) realloc(x, y)
#define _free_(x) free(x)

#endif


#endif
