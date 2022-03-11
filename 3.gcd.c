#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define IMAX_BITS(m) ((m)/((m)%255+1) / 255%255*8 + 7-86/((m)%255+12))
#define RAND_MAX_WIDTH IMAX_BITS(RAND_MAX)
_Static_assert((RAND_MAX & (RAND_MAX + 1u)) == 0, "RAND_MAX not a Mersenne number");

uint64_t rand64(void) {
  uint64_t r = 0;
  for (int i = 0; i < 64; i += RAND_MAX_WIDTH) {
    r <<= RAND_MAX_WIDTH;
    r ^= (unsigned) rand();
  }
  return r;
}

uint64_t gcd64(uint64_t u, uint64_t v)
{
    if (!u || !v)
        return u | v;

    int shift = __builtin_ctzll(u | v);
    
    u = u >> __builtin_ctzll(u);

    do {
        v = v >> __builtin_ctzll(v);
        if (u < v) {
            v -= u;
        } else {
            uint64_t t = u - v;
            u = v;
            v = t;
        }
    } while (v);

    return u << shift;
}

int main(){
    uint64_t res = gcd64(rand64(), rand64());
    
    return 0;
}
