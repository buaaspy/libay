#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int uint32;

typedef struct node* node_t;
struct node {
  const void* key;
  uint32 next;
} __attribute__((aligned(sizeof(uint32))));

#define cast(type, expr) ((type)(expr))
#define node_next(p) cast(node_t, (((p)->next) & ~0x3))
#define get_node_next(p) node_next(p)
#define set_node_next(p, v) do { (p)->next = ((p)->next & 0x3) | cast(uint32, v); } while (0)
#define node_mark(p) cast(uint32, (((p)->next) & 0x1))
#define set_node_marked(p) (((p)->next) |= 0x1)
#define set_node_unmarked(p) (((p)->next) &= ~0x1)
#define flip_node_mark(p) (((p)->next) ^= 0x1)

int main()
{
  int testcase = 5;
  int key = 999;
  int i;
  node_t n = NULL;
  node dummy_head;
  node_t head = &dummy_head;

  printf("===========================\n");
  printf("type\t\twidth\n");
  printf("===========================\n");
  printf("char\t\t%d\n", sizeof(char));
  printf("unsigned char\t%d\n", sizeof(unsigned char));
  printf("unsigned int\t%d\n", sizeof(unsigned int));
  printf("int\t\t%d\n", sizeof(int));
  printf("float\t\t%d\n", sizeof(float));
  printf("double\t\t%d\n", sizeof(double));
  printf("long\t\t%d\n", sizeof(long));
  printf("long long\t%d\n", sizeof(long long));
  printf("===========================\n");

  for (i = 0; i < testcase; ++i) {
    n = (node_t)malloc(sizeof(node));
    n->key = (const void*)&key;
    set_node_next(head, n);
    if (i != 3)
      set_node_marked(head);
    head = n;
  }

  i = 1;
  node_t p = node_next(&dummy_head);
  while (p) {
    printf("%d\t%p\t%d\t%s\t%x\n", i, p, *cast(int*, p->key), node_mark(p) ? "marked" : "unmarked", p->next);
    p = node_next(p);
    ++i;
  }

  return 0;
}
