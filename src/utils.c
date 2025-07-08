#include "err_codes.h"
#include "str.h"
#ifdef __unix__
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include "err.h"
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

str_t get_file_to_memory(str_view_t path)
{
#ifndef __unix__
    ASSERT(0, "Supported only on unix");
#endif
    char filename[max_filename_len] = {0};
    if (string_view_to_cstr(&path, filename, max_filename_len - 1) == false)
    {
        return (str_t){nullptr, 0, 0, false};
    }
    s32 fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        return (str_t){nullptr, 0, 0, false};
    }
    struct stat stats;
    if (fstat(fd, &stats) < 0)
    {
        return (str_t){nullptr, 0, 0, false};
    }
    u64 size = stats.st_size;
    void *file_buffer = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_buffer == nullptr)
    {
        return (str_t){nullptr, 0, 0, false};
    }
    str_t ret = string_create_from_buff(size, file_buffer);
    munmap(file_buffer, size);
    close(fd);
    return ret;
}

err_t send_file_to_user(str_view_t path, s32 client_fd)
{
    char filename[max_filename_len] = {0};
    if (string_view_to_cstr(&path, filename, max_filename_len - 1) == false)
    {
        return EMEMORY;
    }

    s32 fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        return ESOCKET;
    }

    struct stat stats;
    if (fstat(fd, &stats) < 0)
    {
        return ESYS;
    }

    auto sended = sendfile(client_fd, fd, 0, stats.st_size);
    if (sended != stats.st_size)
    {
        return ESYS;
    }
    return SUCCESS;
}
