#include "utils.h"
#include <stdio.h>

constexpr u64 max_filename_len = 255;
constexpr u64 max_mode_len = 4;

FILE *file_open(str_view_t path, str_view_t mode)
{
    char filename[max_filename_len] = {0};
    char char_mode[max_mode_len] = {0};
    if (string_view_to_cstr(&path, filename, max_filename_len - 1) == false)
    {
        return nullptr;
    }
    if (string_view_to_cstr(&mode, char_mode, max_mode_len - 1) == false)
    {
        return nullptr;
    }

    FILE *file = fopen(filename, char_mode);
    return file;
}

u64 get_file_len(FILE file[static 1])
{
    u64 end = fseek(file, 0, SEEK_END);
    u64 begin = fseek(file, 0, SEEK_SET);
    return end - begin;
}
