#include "database.h"
#include "err_codes.h"
#include <stdlib.h>

static Database database;

err_t database_create(void)
{
    database.size = 0;
    database.capacity = 100;
    database.rows = malloc(sizeof(*database.rows) * database.capacity);
    for (s32 i = 0; i < 10; i++)
    {
        database_add(i, 1.01 * i);
    }
    return SUCCESS;
}

err_t database_add(u32 category, double price)
{
    if (database.capacity == database.size)
    {
        database.capacity += 100;
        auto temp =
            realloc(database.rows, database.capacity * sizeof(*database.rows));
        if (temp == nullptr)
        {
            return EMEMORY;
        }
        database.rows = temp;
    }

    database.rows[database.size] = (Row){price, category, database.size};
    database.size++;
    return SUCCESS;
}

err_t database_remove(u32 index)
{
    if (index >= database.size)
    {
        return EBUFOVERFLOW;
    }
    memmove(&database.rows[index], &database.rows[index + 1],
            sizeof(*database.rows) * (database.size - (index + 1)));
    database.size--;

    return SUCCESS;
}

Database *database_get_ptr(void)
{
    return &database;
}

err_t database_destroy(void)
{
    free(database.rows);
    return SUCCESS;
}
