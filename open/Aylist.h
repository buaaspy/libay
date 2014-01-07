#ifndef LIBAY_OPEN_AYLIST_H
#define LIBAY_OPEN_AYLIST_H

/*
 * from Linux kernel
 */

#define offset(type, member)  ((size_t)&(((type*)0)->member))

#define container_of(ptr, type, member) ({		\
  const typeof(((type*)0)->member)* _memptr = ptr;	\
  (type*)((char*)_memptr - offset(type, member)); })

typedef struct Ay_list_head* Ay_list_head_t;
typedef struct Ay_list_head {
  Ay_list_head_t prev, next;
} Ay_list_head;

#define Ay_list_entry(ptr, type, member) container_of(ptr, type, member)

static inline void Ay_init_list_head(Ay_list_head_t head)
{
  head->prev = head;
  head->next = head;
}

static inline void _Ay_list_add(Ay_list_head_t entry, Ay_list_head_t prev, Ay_list_head_t next)
{
  prev->next = entry;
  entry->prev = prev;
  next->prev = entry;
  entry->next = next;
}

static inline void Ay_list_add_head(Ay_list_head_t head, Ay_list_head_t entry)
{ _Ay_list_add(entry, head, head->next); }

static inline void Ay_list_add_tail(Ay_list_head_t head, Ay_list_head_t entry)
{ _Ay_list_add(entry, head->prev, head); }

static inline void _Ay_list_del(Ay_list_head_t prev, Ay_list_head_t next)
{
  prev->next = next;
  next->prev = prev;
}

static inline void Ay_list_del(Ay_list_head_t head)
{
  _Ay_list_del(head->prev, head->next);
}

static inline void Ay_list_clear_all_nodes(Ay_list_head_t head)
{
  head->prev = head;
  head->next = head;
}

static inline int Ay_list_is_empty(Ay_list_head_t head)
{ return head->next == head; }

#define Ay_list_for_each(iterator, head)					   \
  for (iterator = head->next; iterator != head; iterator = head->next)

#define Ay_list_for_each_entry(iterator, head, member)			           \
  for (iterator = Ay_list_entry((head)->next, typeof(*iterator), member);          \
       &(iterator->member) != (head);					           \
       iterator = Ay_list_entry(iterator->member.next, typeof(*iterator), member))

#define Ay_list_for_each_entry_reverse(iterator, head, member)		           \
  for (iterator = Ay_list_entry((head)->prev, typeof(*iterator), member);          \
       &(iterator->member) != (head);					           \
       iterator = Ay_list_entry(iterator->member.prev, typeof(*iterator), member))

#endif
