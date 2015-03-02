#ifndef UTILS_H
#define UTILS_H

#include <assert.h>
#define ASSERT(x) assert(x)
#define STATIC_ASSERT(x) static_assert(x, #x)

#ifdef _MSC_VER
#define SPRINTF sprintf_s
#else
#define SPRINTF sprintf
#endif

#endif