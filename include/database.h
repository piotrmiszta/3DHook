#ifndef DATABASE_H_
#define DATABASE_H_

#include "err_codes.h"
#include "logger.h"
#include "str.h"

typedef struct
{
    double price;
    u32 category;
    u32 id;
} Row;

typedef struct
{
    Row *rows;
    u32 size;
    u32 capacity;
} Database;

err_t database_create(void);
err_t database_add(u32 category, double price);

/*TODO: remove this */
Database *database_get_ptr(void);
err_t database_destroy(void);
#endif
