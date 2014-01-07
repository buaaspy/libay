#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include "Aytable.h"
#include "Aytype.h"

int main()
{
  int32 size = 10;
  int32 *key, *value, i;
  Ay_table_t table;

  // TEST--array_create
  // TEST--Ay_table_put
  printf("---- START put\n");
  table = array_create();
  for (i = 0; i < size; ++i) {
    int32 v = 999;
    Ay_table_put(table, (const void*)&i, (void*)&v);
    printf("[PUT] %d\t%d\n", i, v);
  }
  printf("---- END\n\n");


  // TEST--Ay_table_iter_create
  // TEST--Ay_table_iter_has_next
  // TEST--Ay_table_iter_next
  // TEST--Ay_table_iter_free
  printf("---- START iter\n");
  Ay_table_iter_t iter = Ay_table_iter_create(table);
  while (Ay_table_iter_has_next(iter)) {
    key = (int32*)Ay_table_iter_next(iter, (void**)&value);
    printf("[ITE] %d\t%d\n", *key, *value);
  }
  Ay_table_iter_free(iter);
  printf("---- END\n\n");

  
  // TEST--Ay_table_get
  printf("---- START iter\n");
  for (i = 0; i < size; ++i) {
    Ay_table_get(table, &i, (void**)&value);
    printf("[GET] %d\t%d\n", i, *value);
  }
  printf("---- END\n\n");


  // TEST--Ay_table_del
  i = 7;
  printf("---- START del\n");
  Ay_table_del(table, &i);

  Ay_table_get(table, &i, (void**)&value);
  printf("[GET] %d\t%d\n", i, *value);

  i = 999;
  Ay_table_get(table, &i, (void**)&value);
  printf("[GET] 999\t%d\n", value == NULL ? 1234567890 : *value);

  i = 1;
  Ay_table_get(table, &i, (void**)&value);
  printf("[GET] %d\t%d\n", i, *value);
  
  int32 tmp = 1987;
  Ay_table_put(table, &i, (void*)&tmp);
  Ay_table_get(table, &i, (void**)&value);
  printf("[GET] %d\t%d\n", i, *value);

  i = 0;
  while (Ay_table_count(table))
    Ay_table_del(table, &i);

  iter = Ay_table_iter_create(table);
  while (Ay_table_iter_has_next(iter)) {
    key = (int32*)Ay_table_iter_next(iter, (void**)&value);
    printf("[ITE] %d\t%d\n", *key, *value);
  }
  Ay_table_iter_free(iter);  

  // TEST--Ay_table_destroy
  Ay_table_destroy(table);
}
