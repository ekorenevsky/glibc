#ifdef ASLR
#include <stdint.h>
#include <link.h>
#include <ldsodefs.h>
#include <sys/random.h>
#include <limits.h>

/*
 * When mapping shared ELF binaries (ET_DYN), mmap() function is used.
 * Its first argument, mappref/addr, when it is not NULL, is taken as a hint by
 * the POSIX-compliant kernel about where to place the mapping.
 *
 * Use this feature to implement an opportunistic non-invasive ASLR.
 * 1) Assign a reasonable range of addresses below the base of ld.so
 *    to ASLR space
 * 2) Randomly generate 'mappref' inside this ASLR space
 * 3) Pass generated random 'mappref' as mmap() first argument
 *    when mapping the memory for ELF image
 *
 * If kernel's mmap() implementation encounters a collision with previously
 * mapped memory range, it will assign a suitable address range just like
 * it happens when NULL is passed as a first argument of mmap().
 *
 * This is a Monte-Carlo randomized algorithm.
 */

#define ASLR_SPACE_MIN 0x100000

#if __x86_64__ || __ppc64__
#define ASLR_SPACE_MAX 0x2000000000
#else
#define ASLR_SPACE_MAX 0x8000000
#endif

static ElfW(Addr) ldso_addr, aslr_space;

static unsigned int sys_rand (void)
{
  int rc, val = 0;

  do
    rc = getrandom (&val, sizeof(val), GRND_RANDOM);
  while (rc < 0 && errno == EINTR);

  if (rc < 0)
    return 0;

  return val;
}

ElfW(Addr) aslr_get_hint (size_t maplength)
{
  ElfW(Addr) hint, len, space;
  uint64_t rnd64;

  space = aslr_space;
  if (space == 0)
    space = ASLR_SPACE_MAX;

  /* map ELF before lowest-mapped ld.so with random offset */
  len = ((maplength + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));
  rnd64 = ((uint64_t) sys_rand () << 32) | sys_rand ();
  hint = (ldso_addr - space) + (ElfW(Addr))(rnd64 % (space - len));
  hint &= ~((ElfW(Addr))PAGE_SIZE - 1);
  return hint;
}

void aslr_init (ElfW(Addr) addr)
{
  ldso_addr = addr;
}

void aslr_set_space (ElfW(Addr) space)
{
  if (space > ASLR_SPACE_MAX)
    space = ASLR_SPACE_MAX;
  if (space < ASLR_SPACE_MIN)
    space = ASLR_SPACE_MIN;
  aslr_space = space;
}

#endif
