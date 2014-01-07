#include "os.h"
#include "Aytype.h"
#include "Ayatomic.h"
#include "Aysynclist.h"

/*
 * from: A pragmatic imlementation of Non-Blocking Linked-Lists by Tim. Harris
 */

typedef int32 (*sync_list_f_cmp)(const void* lhv, const void* rhv);

typedef struct node* node_t;
struct node {
  const void* key;
  node_t next;
} __attribute__((aligned(sizeof(uint32))));

#define DUMMY_HEAD_KEY ((node_t)0)
#define DUMMY_TAIL_KEY ((node_t)-1)

struct sync_list {
  node_t head;
  node_t tail;
  sync_list_f_cmp cmp;
};

#define cast(type, expr) ((type)(expr))
#define is_marked_reference(p) (cast(uint32, p) & 0x1)
#define get_unmarked_reference(p) cast(node_t, cast(uint32, p) & ~0x3)
#define get_marked_reference(p) cast(node_t, cast(uint32, p) | 0x1)

static inline void* atomic_casptr(volatile void **mem, void *with, const void *cmp)
{
  return (void*)Ay_atomic_casptr(mem, with, cmp);
}

static inline node_t safe_new_node(const void* key)
{
  node_t n = (node_t)malloc(sizeof(node));
  if (n == NULL) {
    errno = ENOMEM;
    fprintf(stderr, "can not malloc from system");
    exit(1);
  }

  n->key = key;

  return n;
}

static node_t search(sync_list_t list, const void* key, node_t* left_node)
{
  node_t left_node_next, right_node;
  
 search_again:
  do {
    node_t t = list->head;
    node_t t_next = list->head->next;
    
    /*
     * 1 ::: find left_node and right_node
     */
    do {
      if (!is_marked_reference(t_next)) {
	*left_node = t;
	left_node_next = t_next;
      }
      t = get_unmarked_reference(t_next);
      if (t == list->tail)
	break;
      t_next = t->next;
    } while (is_marked_reference(t_next) || list->cmp(key, t->key) < 0);
    right_node = t;

    /*
     * 2 ::: check nodes are adjacent
     */
    if (left_node_next == right_node) {
      if (right_node != list->tail && is_marked_reference(right_node->next))
	goto search_again;
      else
	return right_node;
    }

    /*
     * 3 ::: remove one or more marked nodes
     */
    if (left_node_next == atomic_casptr(cast(volatile void**, &((*left_node)->next)), 
					cast(void*, right_node), 
					cast(void*, left_node_next))) {
      if (right_node != list->tail && is_marked_reference(right_node->next))
	goto search_again;
      else
	return right_node;
    }
  } while (Aytrue);
}

Aybool Ay_sync_list_put(sync_list_t list, const void* key)
{
  node_t new_node = safe_new_node(key);
  node_t right_node, left_node;

  do {
    right_node = search(list, key, &left_node);
    if (right_node != list->tail && list->cmp(right_node->key, key) == 0)
      return Ayfalse;
    new_node->next = right_node;
    if (right_node == atomic_casptr(cast(volatile void**, &(left_node->next)), 
				    cast(void*, new_node), 
				    cast(void*, right_node)))
      return Aytrue;
  } while (Aytrue);
}

Aybool Ay_sync_list_del(sync_list_t list, const void* key)
{
  node_t right_node, left_node, right_node_next;

  do {
    right_node = search(list, key, &left_node);
    if (right_node == list->tail || list->cmp(right_node->key, key) != 0)
      return Ayfalse;
    right_node_next = right_node->next;
    if (!is_marked_reference(right_node_next)) {
      /*
       * TODO: -Wsequence-point
       */
      if (cast(void*, right_node_next) == atomic_casptr(cast(volatile void**, &(right_node->next)),
							cast(void*, get_marked_reference(right_node_next)),
							cast(void*, right_node_next)))
	break;
    }
  } while (Aytrue);
  if (right_node != atomic_casptr(cast(volatile void**, &(left_node->next)), 
				  cast(void*, right_node_next), 
				  cast(void*, right_node)))
    right_node = search(list, right_node->key, &left_node);
  return Aytrue;
}

Aybool Ay_sync_list_get(sync_list_t list, const void* key)
{
  node_t right_node, left_node;
  
  right_node = search(list, key, &left_node);
  if (right_node == list->tail || list->cmp(right_node->key, key) != 0)
    return Ayfalse;
  else
    return Aytrue;
}

sync_list_t Ay_sync_list_create(sync_list_f_cmp f_cmp)
{
  sync_list_t sync_list = (sync_list_t)malloc(sizeof(sync_list));
  if (sync_list == NULL) {
    errno = ENOMEM;
    return NULL;
  }

  sync_list->cmp = f_cmp;
  sync_list->head = safe_new_node(DUMMY_HEAD_KEY);
  sync_list->tail = safe_new_node(DUMMY_TAIL_KEY);
  sync_list->head->next = sync_list->tail;
  
  return sync_list;
}
