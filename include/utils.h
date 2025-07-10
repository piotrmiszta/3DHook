#ifndef UTILS_H_
#define UTILS_H_

#include "str.h"
#include <stdio.h>

FILE *file_open(str_view_t path, str_view_t mode);
u64 get_file_len(FILE file[static 1]);
str_t get_file_to_memory(str_view_t path);
#endif
