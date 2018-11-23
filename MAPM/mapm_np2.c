static unsigned next_pow2(unsigned v)
{
#if defined(__GNUC__) && defined(__i386)
  if (v <= 2) return v;
  __asm__("bsrl %1, %0" : "=r" (v) : "r" (v-1));
  return 2 << v;    // smallest pow-of-2 >= v
#else
  v--;              // from bit twiddling hacks
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  return v + 1;
#endif
}
