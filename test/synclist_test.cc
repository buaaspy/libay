#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "Aysynclist.h"

const long usec_per_sec = 1000000;
const int maxtestcase = 10000000;
int dat[maxtestcase];

int32 cmp_int(const void* lhv, const void* rhv)
{
  int a = *((int*)lhv);
  int b = *((int*)rhv);

  if (a > b)
    return 1;
  else if (a < b)
    return -1;
  else
    return 0;
}

int32 cmp_always_put_head(const void* lhv, const void* rhv)
{
  return 1;
}

void run_one_pass_test(int testcasenum)
{
  sync_list_t list;
  long time_in_sec = 0;
  long time_in_usec = 0;
  long time_total = 0;
  long errnum = 0;
  Aybool p;
  struct timeval start, end;

  srand(time(NULL));
  memset(dat, 0, sizeof(int) * testcasenum);
  for (int i = 0; i < testcasenum; ++i)
    dat[i] = random() % testcasenum;

  list = Ay_sync_list_create(cmp_always_put_head);

  gettimeofday(&start, NULL);
  for (int i = 0; i < testcasenum; ++i) {
    p = Ay_sync_list_put(list, (const void*)(dat + i));
    if (p == Ayfalse) {
      printf("test %d fail: Ay_sync_list_put(%d) FAIL\n", i, dat[i]);
      errnum++;
    }
  }
  gettimeofday(&end, NULL);

  time_total = (end.tv_sec - start.tv_sec) * usec_per_sec + (end.tv_usec - start.tv_usec);
  time_in_sec = time_total / usec_per_sec;
  time_in_usec = time_total % usec_per_sec;
  printf("total:\t%ld(sec) %ld(us)\n", time_in_sec, time_in_usec);
  printf("  ops:\t%.2f(put and get)\n", (float)(testcasenum) / (float)(time_in_sec));
  printf("error:\t%ld\n", errnum);  
}

int main()
{
  int a = 1, b = 2, c =3, d = 4, e = 999;
  Aybool na, nb, nc, nd, ne;
  sync_list_t list;

  // trivial test
  list = Ay_sync_list_create(cmp_int);
  na = Ay_sync_list_put(list, (const void*)&a);
  nb = Ay_sync_list_put(list, (const void*)&b);
  nc = Ay_sync_list_put(list, (const void*)&c);
  nd = Ay_sync_list_put(list, (const void*)&d);
  printf("Ay_sync_list_put [a = 1]\t%s\n", na != Ayfalse ? "PASS" : "FAIL");
  printf("Ay_sync_list_put [b = 1]\t%s\n", nb != Ayfalse ? "PASS" : "FAIL");
  printf("Ay_sync_list_put [c = 1]\t%s\n", nc != Ayfalse ? "PASS" : "FAIL");
  printf("Ay_sync_list_put [d = 1]\t%s\n", nd != Ayfalse ? "PASS" : "FAIL");

  na = Ay_sync_list_get(list, (const void*)&a);
  nb = Ay_sync_list_get(list, (const void*)&b);
  nc = Ay_sync_list_get(list, (const void*)&c);
  nd = Ay_sync_list_get(list, (const void*)&d);
  ne = Ay_sync_list_get(list, (const void*)&e);
  printf("Ay_sync_list_get [a = 1]\t%s\n", na != Ayfalse ? "PASS" : "FAIL");
  printf("Ay_sync_list_get [b = 2]\t%s\n", nb != Ayfalse ? "PASS" : "FAIL");
  printf("Ay_sync_list_get [c = 3]\t%s\n", nc != Ayfalse ? "PASS" : "FAIL");
  printf("Ay_sync_list_get [d = 4]\t%s\n", nd != Ayfalse ? "PASS" : "FAIL");
  printf("Ay_sync_list_get [e = 999]\t%s\n", ne == Ayfalse ? "PASS" : "FAIL");

  // iterate test
  int inc = 1000000;
  for (int i = inc; i < maxtestcase; i += inc) {
    printf("=== start %d\n", i);
    run_one_pass_test(i);
  }

  return 0;
}


