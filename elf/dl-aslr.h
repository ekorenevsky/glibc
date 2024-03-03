#ifndef _DL_ASLR_H
#define _DL_ASLR_H	1

#ifdef ASLR

void aslr_init (ElfW(Addr) ldso_addr);

ElfW(Addr) aslr_get_hint (size_t maplength);

void aslr_set_space (ElfW(Addr) space);

#else
static inline void aslr_init (ElfW(Addr) ldso_addr)
{
}

static inline ElfW(Addr) aslr_get_hint (size_t maplength)
{
  return 0;
}
static inline void aslr_set_space (ElfW(Addr) space)
{
}
#endif

#endif
