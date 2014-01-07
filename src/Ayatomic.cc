#include "Ayatomic.h"

uint32 Ay_atomic_read32(volatile uint32 *mem)
{
  return *mem;
}

void Ay_atomic_set32(volatile uint32 *mem, uint32 val)
{
  *mem = val;
}

uint32 Ay_atomic_add32(volatile uint32 *mem, uint32 val)
{
  __asm__ __volatile__("lock; xaddl %0, %1"
		       : "=r"(val),"=m"(*mem)
		       : "0"(val),"m"(*mem)
		       :"memory","cc");
  return val;
}

void Ay_atomic_sub32(volatile uint32 *mem, uint32 val)
{
  __asm__ __volatile__("lock; subl %1,%0"
		       :
		       :"m"(*(mem)),"r"(val)
		       :"memory","cc");
}

uint32 Ay_atomic_inc32(volatile uint32 *mem)
{
  return Ay_atomic_add32(mem, 1);
}

int Ay_atomic_dec32(volatile uint32 *mem)
{
  unsigned char prev;
  __asm__ __volatile__("lock; decl %0; setnz %1"
		       :"=m"(*mem),"=qm"(prev)
		       :"m"(*mem)
		       :"memory");
  return prev;
}

uint32 Ay_atomic_cas32(volatile uint32 *mem, uint32 with, 
			    uint32 cmp)
{
  uint32 prev;
  __asm__ __volatile__("lock;cmpxchgl %1, %2"
		       :"=a"(prev)
		       :"r"(with),"m"(*(mem)),"0"(cmp)
		       :"memory","cc");
  return prev;
}

uint32 Ay_atomic_xchg32(volatile uint32 *mem, uint32 val)
{
  uint32 prev = val;
  __asm__ __volatile__("xchgl %0, %1"
		       :"=r"(prev),"+m"(*mem)
		       :"0"(prev));
  return prev;
}

void* Ay_atomic_casptr(volatile void **mem, void *with, 
			      const void *cmp)
{
  void *prev;
  __asm__ __volatile__("lock;cmpxchgl %2, %1"
		       :"=a"(prev),"=m"(*mem)
		       :"r"(with),"m"(*mem),"0"(cmp));
  return prev;
}

void* Ay_atomic_xchgptr(volatile void **mem, void *with)
{
  void *prev;
  __asm__ __volatile__("xchgl %2, %1"
		       :"=a"(prev),"+m"(*mem)
		       :"0"(with));
  return prev;
}
