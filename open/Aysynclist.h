#ifndef LIBAY_OPEN_AYSYNCLIST_H
#define LIBAY_OPEN_AYSYNCLIST_H

#include "Aytype.h"

typedef struct node* node_t;
typedef struct sync_list* sync_list_t;
typedef int32 (*sync_list_f_cmp)(const void* lhv, const void* rhv);

extern Aybool Ay_sync_list_put(sync_list_t list, const void* key);
extern Aybool Ay_sync_list_del(sync_list_t list, const void* key);
extern Aybool Ay_sync_list_get(sync_list_t list, const void* key);
extern sync_list_t Ay_sync_list_create(sync_list_f_cmp f_cmp);

#endif
