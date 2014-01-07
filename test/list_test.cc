#include "Aylist.h"
#include "Aytype.h"
#include "os.h"

typedef struct aylist* aylist_t;
typedef struct aylist {
  Ay_list_head list_head;
  uint32 len;
} aylist;

typedef struct aylist_node* aylist_node_t;
typedef struct aylist_node {
  Ay_list_head list;
  uint32 data;
} aylist_node;

int main()
{
  int res = 0;

  aylist_t list = (aylist_t)malloc(sizeof(aylist));
  aylist_node_t node;
  aylist_node_t iter;
  
  // TEST--Ay_init_list_head
  // TEST--Ay_list_add_tail
  Ay_init_list_head(&list->list_head);
  for (int i = 1; i <= 100; ++i) {
    node = (aylist_node_t)malloc(sizeof(aylist_node));
    node->data = i;
    Ay_list_add_tail(&list->list_head, &node->list);
  }
  
  // TEST--Ay_list_for_each_entry
  Ay_list_for_each_entry(iter, &list->list_head, list) {
    res += iter->data;
  }
  printf("[%s]\tAy_init_list_head\n", res == 5050 ? "PASS" : "FAIL");
  printf("[%s]\tAy_list_add_tail\n", res == 5050 ? "PASS" : "FAIL");  
  printf("[%s]\tAy_list_for_each_entry\n", res == 5050 ? "PASS" : "FAIL");

  // TEST--Ay_list_for_each_entry_reverse
  Ay_list_for_each_entry_reverse(iter, &list->list_head, list) {
    res -= iter->data;
  }
  printf("[%s]\tAy_list_for_each_entry_reverse\n", res == 0 ? "PASS" : "FAIL");

  // TEST--Ay_list_entry
  // TEST--Ay_list_del
  node = Ay_list_entry(list->list_head.next, struct aylist_node, list);
  Ay_list_del(&node->list);
  Ay_list_for_each_entry(iter, &list->list_head, list) {
    res += iter->data;
  }
  printf("[%s]\tAy_list_entry\n", res == 5049 ? "PASS" : "FAIL");
  printf("[%s]\tAy_list_del\n", res == 5049 ? "PASS" : "FAIL");
}
