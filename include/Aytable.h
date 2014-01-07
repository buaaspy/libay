#ifndef LIBAY_INCLUDE_AYTABLE_H
#define LIBAY_INCLUDE_AYTABLE_H

#include "../open/Aytable.h"

typedef void (*Ay_table_fun_put)(Ay_table* table, const void* key, void* value);
typedef void (*Ay_table_fun_get)(Ay_table* table, const void* key, void** value);
typedef Aybool (*Ay_table_fun_del)(Ay_table* table, const void* key);
typedef uint32 (*Ay_table_fun_count)(Ay_table* table);
typedef void (*Ay_table_fun_destroy)(Ay_table* table);
typedef Ay_table_iter_t (*Ay_table_fun_iter_create)(Ay_table* table);
typedef Aybool (*Ay_table_fun_iter_has_next)(Ay_table_iter_t iter);
typedef void* (*Ay_table_fun_iter_next)(Ay_table_iter_t iter, void** value);
typedef void (*Ay_table_fun_iter_free)(Ay_table_iter_t iter);

struct Ay_table {
  Ay_table_fun_put put;
  Ay_table_fun_get get;
  Ay_table_fun_count count;
  Ay_table_fun_del del;
  Ay_table_fun_destroy destroy;
  Ay_table_fun_iter_create iter_create;
  Ay_table_fun_iter_has_next iter_has_next;
  Ay_table_fun_iter_next iter_next;
  Ay_table_fun_iter_free iter_free;
};

struct Ay_table_iter {
  Ay_table* table;
};

#endif
