#ifndef ERR_CODES_H_
#define ERR_CODES_H_

#include <errno.h>
#include <stdint.h>
#include <string.h>

#define ASSERT(cond, msg) assert(cond &&msg)
#define error() strerror(errno)

typedef int32_t err_t;

constexpr err_t SUCCESS = 0;
constexpr err_t EGENRIC = -1;
constexpr err_t EMEMORY = -2;
constexpr err_t ESOCKET = -3;

#endif
