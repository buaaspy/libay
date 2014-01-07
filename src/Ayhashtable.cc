#include "Aytable.h"
#include "os.h"

/*
 * Theorem based on TAOCP Algorithm D
 * Implementation based on GCC
 *
 * TODO:abstraction penalty is too high :(
 */

#define ENTRY_EMPTY    ((void*)0)
#define ENTRY_DELETED  ((void*)-1)

typedef uint32 hashval_t;

typedef hashval_t (*hashtable_fun_hash)(const void* element);
typedef int32 (*hashtable_fun_cmp)(const void* lhv, const void* rhv);
typedef void* (*hashtable_fun_alloc)(const size_t n_elements, const size_t elements_bytes);
typedef void (*hashtable_fun_free)(void* mem);
typedef void (*hashtable_fun_finalize)(void* arg);

typedef struct hashtable* hashtable_t;
typedef struct hashtable {
  Ay_table table;
  hashtable_fun_alloc alloc;
  hashtable_fun_free free;
  hashtable_fun_hash hash;
  hashtable_fun_cmp cmp;
  hashtable_fun_finalize finalize;
  size_t size;
  uint32 n_elements;
  uint32 n_deleted;
  size_t search;
  size_t collision;

  // entry stores generic-pointer here
  // custom-made entry requires:
  //  * self-defined entry type
  //  * hashtable_fun_cmp 
  //  * hashtable_fun_hash
  void** entries;
} hashtable;

const size_t default_hashtable_size = 1024;

typedef struct hashtable_iter* hashtable_iter_t;
typedef struct hashtable_iter {
  Ay_table_iter i;
  size_t index;
} hashtable_iter;

static void* hashtable_get_entry(hashtable_t hashtab, const void* element);
static void** hashtable_find_slot_for_put(hashtable_t hashtable, void* element);
static void hashtable_put_entry(hashtable_t hashtab, void* element);
static int32 hashtable_expand(hashtable_t hashtab);
static void hashtable_del_entry(hashtable_t hashtab, const void* element);
static void hashtable_destroy(Ay_table_t table);
static Ay_table_iter_t hashtable_iter_create(Ay_table_t table);
static Aybool hashtable_iter_has_next(Ay_table_iter_t iter);
static void* hashtable_iter_next(Ay_table_iter_t iter, void** value);
static void hashtable_iter_free(Ay_table_iter_t iter);

// taken from gcc/trunk/libiberty/hastab.c
// elements in table are primes nearest 2's power
static uint32 prime_tab[] = {
  7,            /* 2  */    /* 8B   */
  13,           /* 3  */    /* 16B  */
  31,           /* 4  */    /* 32B  */
  61,           /* 5  */    /* 64B  */
  127,          /* 6  */    /* 128B */
  251,          /* 7  */    /* 256B */
  509,          /* 8  */    /* 512B */
  1021,         /* 9  */    /* 1K   */
  2039,         /* 10 */    /* 2K   */
  4093,         /* 11 */    /* 4K   */
  8191,         /* 12 */    /* 8K   */
  16381,        /* 13 */    /* 16K  */
  32749,        /* 14 */    /* 32K  */
  65521,        /* 15 */    /* 64K  */
  131071,       /* 16 */    /* 128K */
  262139,       /* 17 */    /* 256K */
  524287,       /* 18 */    /* 512K */
  1048573,      /* 19 */    /* 1M   */
  2097143,      /* 20 */    /* 2M   */
  4194301,      /* 21 */    /* 4M   */
  8388593,      /* 22 */    /* 8M   */
  16777213,     /* 23 */    /* 16M  */
  33554393,     /* 24 */    /* 32M  */
  67108859,     /* 25 */    /* 64M  */
  134217689,    /* 26 */    /* 128M */
  268435399,    /* 27 */    /* 256M */
  536870909,    /* 28 */    /* 512M */
  1073741789,   /* 29 */    /* 1G   */ 
  2147483647,   /* 30 */    /* 2G   */
  0xfffffffb
};

static size_t prime_tab_size = sizeof(prime_tab) / sizeof(prime_tab[0]);

static uint32 determine_hashtable_size(uint32 n)
{
  uint32 low = 0;
  uint32 high = prime_tab_size;
  
  while (low < high) {
    uint32 mid = low + (high - low) / 2;
    if (n > prime_tab[mid])
      low = mid + 1;
    else
      high = mid;
  }

  if (n > prime_tab[low]) {
    errno = ERANGE;
    return 0;
  }
  
  return prime_tab[low];
}

static inline hashval_t hash_mode(hashval_t hash, uint32 size)
{
  return hash % size;
}

static inline hashval_t hash_1(hashval_t hash, hashtable_t hashtab)
{ 
  return  hash_mode(hash, hashtab->size);
}

static inline hashval_t hash_2(hashval_t hash, hashtable_t hashtab)
{
  return 1 + hash_mode(hash, hashtab->size - 2);
}

static hashtable_t hashtable_create_ex(size_t size, 
				       hashtable_fun_alloc f_alloc, 
				       hashtable_fun_free f_free,
				       hashtable_fun_hash f_hash,
				       hashtable_fun_cmp f_cmp,
				       hashtable_fun_finalize f_finalize)
{
  hashtable_t hashtab;
  size_t hashtab_size;

  hashtab = (hashtable_t)f_alloc(1, sizeof(hashtable));
  hashtab_size = determine_hashtable_size(size);
  if (!hashtab || !hashtab_size)
    return NULL;

  // maybe a huge block 
  // see prime-table
  hashtab->entries = (void**)f_alloc(hashtab_size, sizeof(void*));
  if (hashtab->entries == NULL) {
    errno = ENOMEM;
    if (f_free)
      f_free(hashtab);
    return NULL;
  }

  // maybe a time-consuming ops
  // all entries are ENTRY_EMPTY now
  memset(hashtab->entries, 0, hashtab_size * sizeof(void*));
  hashtab->alloc = f_alloc;
  hashtab->free = f_free;
  hashtab->hash = f_hash;
  hashtab->cmp = f_cmp;
  hashtab->finalize = f_finalize;
  hashtab->size = hashtab_size;
  hashtab->n_elements = 0;
  hashtab->n_deleted = 0;
  hashtab->search = 0;
  hashtab->collision = 0;

  return hashtab;
}

static void hashtable_destroy(Ay_table_t table)
{
  hashtable_t hashtab = (hashtable_t)table;
  size_t size = hashtab->size;
  hashtable_fun_finalize f_finalize = hashtab->finalize;
  hashtable_fun_free f_free = hashtab->free;

  if (f_finalize) {
    for (int32 i = size - 1; i >= 0; --i) {
      if (hashtab->entries[i] != ENTRY_EMPTY 
	  && hashtab->entries[i] != ENTRY_DELETED)
	f_finalize(hashtab->entries[i]);
    }
  }

  if (f_free) {
    f_free(hashtab->entries);
    f_free(hashtab);
  }
}

static void* hashtable_find_with_hash(hashtable_t hashtab, const void* element, hashval_t hash)
{
  void* entry;
  void** entries = hashtab->entries;
  int32 oindex = hash_1(hash, hashtab);
  int32 nindex = oindex;
  size_t size = hashtab->size;

  hashtab->search++;

  entry = entries[oindex];
  if (entry != ENTRY_EMPTY 
      && entry != ENTRY_DELETED 
      && (hashtab->cmp(element, entry) == 0)) {
    return entry;
  }

  int32 c = hash_2(hash, hashtab);
  do {
    hashtab->collision++;
    nindex -= c;
    if (nindex < 0)
      nindex += size;
    entry = entries[nindex];
    if (entry != ENTRY_EMPTY 
	&& entry != ENTRY_DELETED 
	&& (hashtab->cmp(element, entry) == 0))
      return entry;
  } while (nindex != oindex);

  return NULL;
}

static void* hashtable_get_entry(hashtable_t hashtab, const void* element)
{
  return hashtable_find_with_hash(hashtab, element, hashtab->hash(element));
}

static void** hashtable_find_slot_for_put(hashtable_t hashtab, void* element)
{
  void** entry;
  void** entries = hashtab->entries;
  int32 oindex = hash_1(hashtab->hash(element), hashtab);
  int32 nindex = oindex;
  size_t size = hashtab->size;

  hashtab->search++;

  entry = &entries[oindex];
  
  if (*entry == ENTRY_EMPTY) {
    hashtab->n_elements++;
    return entry;
  }
  if (*entry == ENTRY_DELETED) {
    hashtab->n_deleted--;
    return entry;
  }
  if (hashtab->cmp(element, *entry) == 0)
    return entry;

  int32 c = hash_2(hashtab->hash(element), hashtab);
  do {
    hashtab->collision++;
    nindex -= c;
    if (nindex < 0)
      nindex += size;
    entry = &entries[nindex];
    if (*entry == ENTRY_EMPTY) {
      hashtab->n_elements++;
      return entry;
    } 
    if (*entry == ENTRY_DELETED) {
      hashtab->n_deleted--;
      return entry;
    } 
    if (hashtab->cmp(element, *entry) == 0)
      return entry;
  } while (nindex != oindex);  

  return NULL;
}

static int32 hashtable_expand(hashtable_t hashtab)
{
  size_t osize = hashtab->size;
  size_t oelts = hashtab->n_elements - hashtab->n_deleted;
  size_t nsize;
  void** nentries;
  void** oentries = hashtab->entries;
  
  // * too full  -- expand hashtable
  // * too empty -- clear ENTRY_EMPTY
  if (oelts * 2 >= osize || oelts * 8 < osize)
    nsize = determine_hashtable_size(oelts * 2);
  else
    nsize = osize;

  nentries = (void**)hashtab->alloc(nsize, sizeof(void*));
  if (nentries == NULL) {
    errno = ENOMEM;
    return 0;
  }
  
  memset(nentries, 0, nsize * sizeof(void*));
  hashtab->entries = nentries;
  hashtab->size = nsize;
  hashtab->n_elements -= hashtab->n_deleted;
  hashtab->n_deleted = 0;

  for (uint32 i = 0; i < osize; ++i) {
    void* entry = oentries[i];
    if (entry != ENTRY_EMPTY && entry != ENTRY_DELETED) {
      void** slot = hashtable_find_slot_for_put(hashtab, entry);
      *slot = entry;
    }
  }

  if (hashtab->free) {
    hashtab->free(oentries);
  }

  return 1;
}

static void hashtable_put_entry(hashtable_t hashtab, void* element)
{
  size_t oelts = hashtab->n_elements - hashtab->n_deleted;
  size_t osize = hashtab->size;
  void** slot;
  
  if (oelts * 4 >= osize * 3) {
    if (!hashtable_expand(hashtab)) {
      errno = EPROTO;
      return;
    }
  }

  slot = hashtable_find_slot_for_put(hashtab, element);
  if (slot != NULL)
    *slot = element;

  return;  
}

static void hashtable_del_entry(hashtable_t hashtab, const void* element)
{
  void** entry;
  void** entries = hashtab->entries;
  int32 oindex = hash_1(hashtab->hash(element), hashtab);
  int32 nindex = oindex;
  size_t size = hashtab->size;

  hashtab->search++;

  entry = &entries[oindex];
  if (*entry == ENTRY_EMPTY) 
    return;
  else if (*entry != ENTRY_DELETED && (hashtab->cmp(element, *entry) == 0)) {
    *entry = ENTRY_DELETED;
    hashtab->n_deleted++;
    return;
  }

  int32 c = hash_2(hashtab->hash(element), hashtab);
  do {
    hashtab->collision++;
    nindex -= c;
    if (nindex < 0)
      nindex += size;

    entry = &entries[nindex];
    if (*entry == ENTRY_EMPTY) 
      return;
    else if (*entry != ENTRY_DELETED && (hashtab->cmp(element, *entry) == 0)) {
      *entry = ENTRY_DELETED;
      hashtab->n_deleted++;
      return;
    }
  } while (nindex != oindex);

  return;  
}

static void* hashtable_alloc(const size_t n_elements, const size_t elements_bytes)
{
  return malloc(n_elements * elements_bytes);
}

static void hashtable_free(void* mem)
{
  free(mem);
}

static hashval_t hashtable_hash_string (const void* p)
{
  const unsigned char *str = (const unsigned char*)p;
  hashval_t r = 0;
  unsigned char c;

  while ((c = *str++) != 0)
    r = r * 67 + c - 113;

  return r;
}

/*
 * APIs below provide a common-scene implementation
 */
typedef struct pair* pair_t;
typedef struct pair {
  const void* key;
  void* value;
} pair;

static hashval_t hashtable_hash(const void* element)
{
  pair_t p = (pair_t)element;

  return hashtable_hash_string(p->key);
}

static int32 hashtable_cmp(const void* lhv, const void* rhv)
{
  const pair_t lp = (pair_t)lhv;
  const pair_t rp = (pair_t)rhv;

  return strcmp((const char*)lp->key, (const char*)rp->key);
}

static void hashtable_put(Ay_table_t table, const void* key, void* value)
{
  hashtable_t hashtab = (hashtable_t)table;
  pair_t p = (pair_t)hashtab->alloc(1, sizeof(pair));
  if(!p) {
    errno = ENOMEM;
    return;
  }

  p->key = key;
  p->value = value;
  
  hashtable_put_entry(hashtab, (void*)p);
}

static void hashtable_get(Ay_table_t table, const void* key, void** value)
{
  hashtable_t hashtab = (hashtable_t)table;
  pair_t p = (pair_t)hashtab->alloc(1, sizeof(pair));
  pair_t r;
  
  p->key = key;
  r = (pair_t)hashtable_get_entry(hashtab, (const void*)p);
  if (r)
    *value = r->value;
  else
    *value = NULL;
}

static Aybool hashtable_del(Ay_table_t table, const void* key)
{
  hashtable_t hashtab = (hashtable_t)table;
  pair_t p = (pair_t)hashtab->alloc(1, sizeof(pair));

  p->key = key;
  hashtable_del_entry(hashtab, (const void*)p);

  return Aytrue;
}

static uint32 hashtable_count(Ay_table_t table)
{
  hashtable_t hashtab = (hashtable_t)table;

  return hashtab->n_elements - hashtab->n_deleted;
}

static Ay_table_iter_t hashtable_iter_create(Ay_table_t table)
{
  hashtable_t hashtab = (hashtable_t)table;
  hashtable_iter_t iter = (hashtable_iter_t)hashtab->alloc(1, sizeof(hashtable_iter));
  
  if (iter == NULL) {
    errno = ENOMEM;
    return NULL;
  }

  iter->i.table = (Ay_table_t)hashtab;
  iter->index = 0;

  return (Ay_table_iter_t)iter;
}

static Aybool hashtable_iter_has_next(Ay_table_iter_t iter)
{
  hashtable_iter_t hi = (hashtable_iter_t)iter;
  hashtable_t hashtab = (hashtable_t)hi->i.table;
  size_t index = hi->index;
  size_t i;
  void** entries;

  if (hi == NULL || hashtab == NULL)
    return Ayfalse;

  entries = hashtab->entries;
  for (i = index + 1; i < hashtab->size; ++i) {
    if (entries[i] != ENTRY_EMPTY && entries[i] != ENTRY_DELETED)
      break;
  }
  
  return i == hashtab->size ? Ayfalse : Aytrue;
}

static void* hashtable_iter_next(Ay_table_iter_t iter, void** value)
{
  hashtable_iter_t hi = (hashtable_iter_t)iter;
  hashtable_t hashtab = (hashtable_t)hi->i.table;
  size_t index = hi->index;
  size_t i;
  void** entries;
  pair_t p;


  if (hi == NULL || hashtab == NULL)
    return NULL;

  entries = hashtab->entries;
  for (i = index + 1; i < hashtab->size; ++i) {
    if (entries[i] != ENTRY_EMPTY && entries[i] != ENTRY_DELETED)
      break;
  }

  hi->index = i;
  p = (pair_t)entries[i];
  *value = p->value;

  return (void*)p->key;
}

static void hashtable_iter_free(Ay_table_iter_t iter)
{
  hashtable_iter_t hi = (hashtable_iter_t)iter;
  hashtable_t hashtab = (hashtable_t)hi->i.table;

  hashtab->free(hi);
}

Ay_table_t hashtable_create()
{
  hashtable_t hashtab = NULL;

  hashtab = hashtable_create_ex(default_hashtable_size, 
				hashtable_alloc,
				hashtable_free,
				hashtable_hash,
				hashtable_cmp,
				NULL);
  if (hashtab == NULL)
    return NULL;

  hashtab->table.put = hashtable_put;
  hashtab->table.get = hashtable_get;
  hashtab->table.del = hashtable_del;
  hashtab->table.count = hashtable_count;
  hashtab->table.destroy = hashtable_destroy;
  hashtab->table.iter_create = hashtable_iter_create;
  hashtab->table.iter_has_next = hashtable_iter_has_next;
  hashtab->table.iter_next = hashtable_iter_next;
  hashtab->table.iter_free = hashtable_iter_free;
  
  return (Ay_table_t)hashtab;
}
