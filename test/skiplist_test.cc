#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include "Aytable.h"
#include "Aytype.h"

const size_t max_str_len = 16;
char ascii_table[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
const size_t ascii_table_len = sizeof(ascii_table) / sizeof(ascii_table[0]);

char* random_generate_string()
{
  char* str;
  size_t len;
  size_t i;

  len = random() % max_str_len + 2;
  str = (char*)malloc(len * sizeof(char));
  memset(str, 0, sizeof(str));
  for (i = 0; i < len - 1; ++i) {
    str[i] = ascii_table[random() % ascii_table_len];
  }
  str[i] = '\0';

  return str;
}

int main()
{
  size_t size = 2;
  char *key, *value;
  Ay_table_t table;
  const char* keys[] = {"key1", "key2", "key3", "key4", "key5", 
			"key6", "key7", "key8", "key9", "key10",
			"key11","key12","key13","key14","key15",
			"key16","key17","key18","key19","key20",
			"key21","key22","key23","key24","key25",
			"key26","key27","key28","key29","key30"};

  const char* values[] = {"value1", "value2", "value3", "value4", "value5", 
			  "value6", "value7", "value8", "value9", "value10",
			  "value11","value12","value13","value14","value15",
			  "value16","value17","value18","value19","value20",
			  "value21","value22","value23","value24","value25",
			  "value26","value27","value28","value29","value30"};


  // TEST--skiplist_create
  // TEST--Ay_table_put
  printf("---- START put\n");
  table = skiplist_create();
  for (uint32 i = 0; i < size; ++i) {
    Ay_table_put(table, (const void*)keys[i], (void*)values[i]);
    printf("[PUT] %d / %s\t%s\n", Ay_table_count(table), keys[i], values[i]);
  }
  printf("---- END\n\n");


  // TEST--Ay_table_iter_create
  // TEST--Ay_table_iter_has_next
  // TEST--Ay_table_iter_next
  // TEST--Ay_table_iter_free
  printf("---- START iter\n");
  Ay_table_iter_t iter = Ay_table_iter_create(table);
  while (Ay_table_iter_has_next(iter)) {
    key = (char*)Ay_table_iter_next(iter, (void**)&value);
    printf("[ITE] %s\t%s\n", key, value);
  }
  Ay_table_iter_free(iter);
  printf("---- END\n\n");

  
  // TEST--Ay_table_get
  printf("---- START iter\n");
  for (uint32 i = 0; i < size; ++i) {
    Ay_table_get(table, keys[i], (void**)&value);
    printf("[GET] %s\t%s\n", keys[i], value);
  }
  printf("---- END\n\n");


  // TEST--Ay_table_del
  printf("---- START del\n");
  Ay_table_del(table, keys[12]);

  Ay_table_get(table, keys[12], (void**)&value);
  printf("[GET] %s\t%s\n", keys[12], value);

  Ay_table_get(table, "key999", (void**)&value);
  printf("[GET] key999\t%s\n", value);

  Ay_table_get(table, keys[1], (void**)&value);
  printf("[GET] %s\t%s\n", keys[1], value);

  Ay_table_put(table, keys[1], (void*)"value1987");
  Ay_table_get(table, keys[1], (void**)&value);
  printf("[GET] %s\t%s\n", keys[1], value);

  iter = Ay_table_iter_create(table);
  while (Ay_table_iter_has_next(iter)) {
    key = (char*)Ay_table_iter_next(iter, (void**)&value);
    Ay_table_del(table, (const void*)key);
  }
  Ay_table_iter_free(iter);

  iter = Ay_table_iter_create(table);
  while (Ay_table_iter_has_next(iter)) {
    key = (char*)Ay_table_iter_next(iter, (void**)&value); 
    printf("[DEL] %s\t%s\n", key, value);
  }
  Ay_table_iter_free(iter);
  printf("---- END\n\n");


  printf("---- START random test\n");
  int n_del = 0;
  iter = Ay_table_iter_create(table);
  while (Ay_table_iter_has_next(iter)) {
    key = (char*)Ay_table_iter_next(iter, (void**)&value);
    Ay_table_del(table, (const void*)key);
    n_del++;
  }
  Ay_table_iter_free(iter);

  size_t testnum = 100000;
  for (uint32 i = 0; i < testnum; ++i) {
    key = random_generate_string();
    value = random_generate_string();
    Ay_table_put(table, (const void*)key, (void*)value);
  }
  printf("%d/%d\n", n_del, Ay_table_count(table));
  printf("---- END\n\n");


  // TEST--Ay_table_destroy
  Ay_table_destroy(table);
}
