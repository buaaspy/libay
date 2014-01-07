#include "Aytable.h"
#include <stdio.h>

void Ay_table_put(Ay_table* table, const void* key, void* value)
{
  table->put(table, key, value);
}

void Ay_table_get(Ay_table* table, const void* key, void** value)
{
  table->get(table, key, value);
}

Aybool Ay_table_del(Ay_table* table, const void* key)
{
  return table->del(table, key);
}

uint32 Ay_table_count(Ay_table_t table)
{
  return table->count(table);
}

void Ay_table_destroy(Ay_table* table)
{
  table->destroy(table);
}

Ay_table_iter_t Ay_table_iter_create(Ay_table* table)
{
  return table->iter_create(table);
}

Aybool Ay_table_iter_has_next(Ay_table_iter_t iter)
{
  return iter->table->iter_has_next(iter);
}

void* Ay_table_iter_next(Ay_table_iter_t iter, void** value)
{
  return iter->table->iter_next(iter, value);
}

void Ay_table_iter_free(Ay_table_iter_t iter)
{
  iter->table->iter_free(iter);
}
