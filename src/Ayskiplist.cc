#include "Aytable.h"
#include "os.h"

static int32 max_skiplist_depth = 8;
static int32 probability_per_level = 4;

typedef struct skiplist_node* skiplist_node_t;
struct skiplist_node {
  const void* key;
  void* value;
  size_t level;
  skiplist_node_t* forward;
};

typedef int (*skiplist_cmp_fn)(const void* lhv, const void* rhv);
typedef struct skiplist* skiplist_t;
struct skiplist {
  Ay_table table;
  uint32 level;
  uint32 length;
  skiplist_node_t header;
  skiplist_cmp_fn cmp;
};

typedef struct skiplist_iter* skiplist_iter_t;
struct skiplist_iter {
  Ay_table_iter i;
  skiplist_node_t node;
};

static skiplist_node_t* updates = (skiplist_node_t*)malloc(sizeof(skiplist_t) * max_skiplist_depth);

static int string_cmp(const void* lhv, const void* rhv)
{
  const char* a = (char*)lhv;
  const char* b = (char*)rhv;
  return strcmp(a, b);
}

static size_t generate_level()
{
  int32 i = 0;

  while ((random() % probability_per_level) > (probability_per_level - 2)) 
    i++;
  
  return i > max_skiplist_depth - 1 ? max_skiplist_depth - 1 : i;
}

static skiplist_node_t skiplist_new_node(const void* key, void* value, size_t level)
{
  size_t forwardbytes = sizeof(skiplist_node_t) * (level + 1);
  skiplist_node_t node = (skiplist_node_t)malloc(sizeof(skiplist_node));
  if (node == NULL)
    return NULL;
  
  node->level = level;
  node->key = key;
  node->value = value;
  node->forward = (skiplist_node_t*)malloc(forwardbytes);
  memset(node->forward, 0, forwardbytes);

  return node;
}

static void skiplist_put(Ay_table_t table, const void* key, void* value)
{
  skiplist_t slist = (skiplist_t)table;
  uint32 level = generate_level();
  skiplist_cmp_fn cmp = slist->cmp;
  int res;

  skiplist_node_t node = skiplist_new_node(key, value, level);
  if (node == NULL) {
    errno = ENOMEM;
    return;
  }

  skiplist_node_t cur = slist->header;
  memset(updates, 0, sizeof(updates));
  for (int i = slist->level; i >= 0; --i) {
    while (cur->forward[i] && (res = cmp(key, cur->forward[i]->key)) > 0)
      cur = cur->forward[i];
    if (res == 0) {
      cur->forward[i]->value = value;
      return;
    }
    updates[i] = cur;
  }

  // pump skiplist level
  if (level > slist->level) {
    for (size_t i = slist->level + 1; i <= level; ++i)
      updates[i] = slist->header;
    slist->level = level;
  }

  for (int i = level; i >= 0; --i) {
    node->forward[i] = updates[i]->forward[i];
    updates[i]->forward[i] = node;
  }

  slist->length++;

  return;
}

static void skiplist_get(Ay_table* table, const void* key, void** value)
{
  skiplist_t slist = (skiplist_t)table;
  skiplist_cmp_fn cmp = slist->cmp;
  skiplist_node_t cur = slist->header;
  int res;

  for (int i = slist->level; i >= 0; --i) {
    while (cur->forward[i] && (res = cmp(key, cur->forward[i]->key)) > 0)
      cur = cur->forward[i];
    if (res == 0) {
      *value = cur->forward[i]->value;
      return;
    }
  }  

  *value = NULL;
  return;
}

static Aybool skiplist_del(Ay_table* table, const void* key)
{
  skiplist_t slist = (skiplist_t)table;
  skiplist_cmp_fn cmp = slist->cmp;
  skiplist_node_t node = NULL;
  int res, found = 0;
  
  if (slist == NULL) 
    return Ayfalse;

  skiplist_node_t cur = slist->header;
  memset(updates, 0, sizeof(updates));
  for (int i = slist->level; i >= 0; --i) {
    while (cur->forward[i] && (res = cmp(key, cur->forward[i]->key)) > 0)
      cur = cur->forward[i];
    if (res == 0) {
      found = 1;
      if (node == NULL)
	node = cur->forward[i];
      cur->forward[i] = cur->forward[i]->forward[i];
    }
  }
  
  for (int i = slist->level; i >=0; --i) {
    if (slist->header->forward[i])
      break;
    if (slist->level > 0)
      slist->level--;
  }

  if (found) 
    slist->length--;
  free(node);

  return Aytrue;
}

static uint32 skiplist_count(Ay_table_t table)
{
  skiplist_t slist = (skiplist_t)table;
  
  return slist->length;
}

static void skiplist_destroy(Ay_table_t table)
{
  skiplist_t slist = (skiplist_t)table;
  skiplist_node_t node = slist->header;
  skiplist_node_t curr;
  
  while (node->forward[0]) {
    curr = node->forward[0];
    node = node->forward[0];
    free(curr);
  }

  free(slist->header);
  free(slist);
}

static Ay_table_iter_t skiplist_iter_create(Ay_table_t table)
{
  skiplist_t slist = (skiplist_t)table;
  skiplist_iter_t iter = (skiplist_iter_t)malloc(sizeof(skiplist_iter));
  if (iter == NULL)
    return NULL;
  
  iter->i.table = table;
  iter->node = slist->header;
  
  return (Ay_table_iter_t)iter;
}

static Aybool skiplist_iter_has_next(Ay_table_iter_t iter)
{
  skiplist_iter_t si = (skiplist_iter_t)iter;
  skiplist_node_t node = si->node;

  return node->forward[0] == NULL ? Ayfalse : Aytrue;
}

static void* skiplist_iter_next(Ay_table_iter_t iter, void** value)
{
  skiplist_iter_t si = (skiplist_iter_t)iter;

  si->node = si->node->forward[0];
  *value = si->node->value;

  return (void*)si->node->key;
}

static void skiplist_iter_free(Ay_table_iter_t iter)
{
  skiplist_iter_t si = (skiplist_iter_t)iter;
  free(si);
}

Ay_table_t skiplist_create()
{
  skiplist_t sl = (skiplist_t)malloc(sizeof(skiplist));
  if (sl == NULL)
    return NULL;

  sl->table.put = skiplist_put;
  sl->table.get = skiplist_get;
  sl->table.del = skiplist_del;
  sl->table.count = skiplist_count;
  sl->table.destroy = skiplist_destroy;
  sl->table.iter_create = skiplist_iter_create;
  sl->table.iter_has_next = skiplist_iter_has_next;
  sl->table.iter_next = skiplist_iter_next;
  sl->table.iter_free = skiplist_iter_free;
  sl->length = 0;
  sl->level = 0;
  sl->header = skiplist_new_node(NULL, NULL, max_skiplist_depth);
  sl->cmp = string_cmp;
  
  return (Ay_table_t)sl;
}
