#ifndef LIST_H_
#define LIST_H_

typedef struct list_t
{
    struct list_t *next;
    struct list_t *prev;
} list_t;

#define LIST_INIT(name) (name) = {&(name), &(name)}

#define list_get_ptr(ptr, type, member)                                        \
    (type *)((char *)(list_t *)(ptr) - offsetof(type, member))

static inline void list_init(list_t list[static 1])
{
    list->next = list;
    list->prev = list;
}

static inline void list_add_between(list_t entry[static 1],
                                    list_t prev[static 1],
                                    list_t next[static 1])
{
    entry->prev = prev;
    entry->next = next;
    prev->next = entry;
    next->prev = entry;
}

static inline void list_add_head(list_t entry[static 1], list_t head[static 1])
{
    list_add_between(entry, head, head->next);
}

static inline void list_add_tail(list_t entry[static 1], list_t head[static 1])
{
    list_add_between(entry, head->prev, head);
}

static inline void list_rem_entry(list_t entry[static 1])
{
    list_t *temp = entry->prev;
    entry->prev->next = entry->next;
    entry->next->prev = temp;
    entry->next = nullptr;
    entry->prev = nullptr;
}

#endif
