#ifndef LIBAY_OPEN_AYATOMIC_H
#define LIBAY_OPEN_AYATOMIC_H

#include "Aytype.h"

extern uint32 Ay_atomic_read32(volatile uint32 *mem);
extern void Ay_atomic_set32(volatile uint32 *mem, uint32 val);
extern uint32 Ay_atomic_add32(volatile uint32 *mem, uint32 val);
extern void Ay_atomic_sub32(volatile uint32 *mem, uint32 val);
extern uint32 Ay_atomic_inc32(volatile uint32 *mem);
extern int Ay_atomic_dec32(volatile uint32 *mem);
extern uint32 Ay_atomic_cas32(volatile uint32 *mem, uint32 with, uint32 cmp);
extern uint32 Ay_atomic_xchg32(volatile uint32 *mem, uint32 val);
extern void* Ay_atomic_casptr(volatile void **mem, void *with, const void *cmp);
extern void* Ay_atomic_xchgptr(volatile void **mem, void *with);

#endif
