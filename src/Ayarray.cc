#include "Aytable.h"
#include "os.h"

const size_t default_n_per_block = 1024;

typedef void* (*array_f_alloc)(size_t n_elt, size_t elt_bytes);
typedef void* (*array_f_realloc)(void* mem, size_t new_size_in_bytes);
typedef void (*array_f_free)(void* mem);

typedef struct array* array_t;
struct array {
  Ay_table table;
  uint32 n_elts;
  size_t n_per_block;
  size_t n_blocks;
  size_t n_total;
  array_f_alloc f_alloc;
  array_f_realloc f_realloc;
  array_f_free f_free;
  void** mem;
};

typedef struct array_iter* array_iter_t;
struct array_iter {
  Ay_table_iter i;
  uint32 idx;
};

static Aybool array_expand(array_t a, size_t new_n_blocks)
{
  array_f_realloc realloc = a->f_realloc;
  size_t new_n_elts = a->n_per_block * new_n_blocks;
  
  if (realloc == NULL)
    return Ayfalse;

  a->mem = (void**)realloc(a->mem, sizeof(void*) * new_n_elts);
  if (a->mem == NULL) {
    errno = ENOMEM;
    return Ayfalse;
  }

  memset(a->mem + sizeof(void*) * a->n_elts, 
	 0, 
	 sizeof(void*) * (new_n_elts - a->n_elts));

  a->n_blocks = new_n_blocks;
  a->n_total = a->n_per_block * a->n_blocks;

  return Aytrue;
}

static void array_put(Ay_table_t table, const void* key, void* value)
{
  array_t a = (array_t)table;
  int32 len = a->n_total;
  int32 idx = *((int32*)key);

  if (idx < 0) {
    errno = EINVAL;
    return;
  }

  if (idx >= len) {
    size_t n_blocks = idx / a->n_per_block + 2;
    if (array_expand(a, n_blocks) == Ayfalse)
      return;
  }

  void** entry = a->mem + sizeof(void*) * idx;
  *entry = value;
  
  a->n_elts++;
  
  return;
}


static void array_get(Ay_table_t table, const void* key, void** value)
{
  array_t a = (array_t)table;
  int32 len = a->n_elts;
  int32 idx = *((int32*)key);

  if (idx < 0 || idx >= len) {
    *value = NULL;
    errno = EINVAL;
    return;
  }

  void** entry = a->mem + sizeof(void*) * idx;
  *value = *entry;
  
  return;  
}

static Aybool array_del(Ay_table_t table, const void* key)
{
  array_t a = (array_t)table;
  int32 len = a->n_elts;
  int32 idx = *((int32*)key);

  if (idx < 0 || idx >= len) {
    errno = EINVAL;
    return Ayfalse;
  }

  memmove(a->mem + sizeof(void*) * idx,
	  a->mem + sizeof(void*) * (idx + 1),
	  sizeof(void*) * (a->n_elts - idx - 1)
	  );

  a->n_elts--;

  return Aytrue;
}

static uint32 array_count(Ay_table_t table)
{
  array_t a = (array_t)table;
  
  return a->n_elts;
}

static void array_destroy(Ay_table_t table)
{
  array_t a = (array_t)table;

  if (a->f_free)
    a->f_free(a);
}

static Ay_table_iter_t array_iter_create(Ay_table_t table)
{
  array_t a = (array_t)table;
  array_iter_t iter = (array_iter_t)a->f_alloc(1, sizeof(array_iter));
  
  if (iter == NULL) {
    errno = ENOMEM;
    return NULL;
  }

  iter->i.table = (Ay_table_t)a;
  iter->idx = 0;

  return (Ay_table_iter_t)iter;
}

static Aybool array_iter_has_next(Ay_table_iter_t iter)
{
  array_iter_t ai = (array_iter_t)iter;
  array_t a = (array_t)ai->i.table;
  
  if (iter == NULL || a == NULL)
    return Ayfalse;

  return ai->idx < a->n_elts ? Aytrue : Ayfalse; 
}

/*
 * haunting again :(
 */
static int32* res = (int32*)malloc(sizeof(int32));

static void* array_iter_next(Ay_table_iter_t iter, void** value)
{
  array_iter_t ai = (array_iter_t)iter;
  array_t a = (array_t)ai->i.table;
  void** entry;

  entry = a->mem + sizeof(void*) * ai->idx;
  *value = *entry;
  *res = ai->idx++;

  return (void*)res;
}

static void array_iter_free(Ay_table_iter_t iter)
{
  array_iter_t ai = (array_iter_t)iter;
  array_t a = (array_t)ai->i.table;

  a->f_free(ai);
}

static void* array_alloc(size_t n_elt, size_t elt_bytes)
{
  return malloc(n_elt * elt_bytes);
}

static void* array_realloc(void* mem, size_t new_size_in_bytes)
{
  return realloc(mem, new_size_in_bytes);
}

static void array_free(void* mem)
{
  free(mem);
}

Ay_table_t array_create()
{
  array_t a = (array_t)malloc(sizeof(array));
  if (a == NULL)
    return NULL;

  a->table.put = array_put;
  a->table.get = array_get;
  a->table.del = array_del;
  a->table.count = array_count;
  a->table.destroy = array_destroy;
  a->table.iter_create = array_iter_create;
  a->table.iter_has_next = array_iter_has_next;
  a->table.iter_next = array_iter_next;
  a->table.iter_free = array_iter_free;
  a->n_elts = 0;
  a->n_per_block = default_n_per_block;
  a->f_alloc = array_alloc;
  a->f_realloc = array_realloc;
  a->f_free = array_free;
  array_expand(a, 1);

  return (Ay_table_t)a;  
}
