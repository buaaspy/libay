#ifndef LIBAY_OPEN_AYTABLE_H
#define LIBAY_OPEN_AYTABLE_H

#include "Aytype.h"

struct Ay_table;
typedef Ay_table* Ay_table_t;

struct Ay_table_iter;
typedef Ay_table_iter* Ay_table_iter_t;

extern void Ay_table_put(Ay_table_t table, const void* key, void* value);
extern void Ay_table_get(Ay_table_t table, const void* key, void** value);
extern Aybool Ay_table_del(Ay_table_t table, const void* key);
extern uint32 Ay_table_count(Ay_table_t table);
extern void Ay_table_destroy(Ay_table_t table);
extern Ay_table_iter_t Ay_table_iter_create(Ay_table_t table);
extern Aybool Ay_table_iter_has_next(Ay_table_iter_t iter);
extern void* Ay_table_iter_next(Ay_table_iter_t iter, void** value);
extern void Ay_table_iter_free(Ay_table_iter_t iter);

extern Ay_table_t skiplist_create();
extern Ay_table_t hashtable_create();
extern Ay_table_t array_create();

#endif
