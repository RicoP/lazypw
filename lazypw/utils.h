#ifndef UTILS_H
#define UTILS_H

#ifdef _MSC_VER 
#include <type_traits>
#include <iosfwd>
#define STATIC_ASSERT(x) static_assert(x, #x)
#else
#define STATIC_ASSERT(x) 
#endif 
#endif