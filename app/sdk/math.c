#include <limits.h>
int
__clzsi2 (int val)
{
  int i = 32;
  int j = 16;
  int temp;

  for (; j; j >>= 1)
    {
        temp = val >> j;
      if (temp)
	{
	  if (j == 1)
	    {
	      return (i - 2);
	    }
	  else
	    {
	      i -= j;
	      val = temp;
	    }
	}
    }
  return (i - val);
}

unsigned int __udivsi3(unsigned int n, unsigned int d)
{
    const unsigned n_uword_bits = sizeof(unsigned int) * CHAR_BIT;
    unsigned int q;
    unsigned int r;
    unsigned sr;
    /* special cases */
    if (d == 0)
        return 0; /* ?! */
    if (n == 0)
        return 0;
    sr = __builtin_clz(d) - __builtin_clz(n);
    /* 0 <= sr <= n_uword_bits - 1 or sr large */
    if (sr > n_uword_bits - 1)  /* d > r */
        return 0;
    if (sr == n_uword_bits - 1)  /* d == 1 */
        return n;
    ++sr;
    /* 1 <= sr <= n_uword_bits - 1 */
    /* Not a special case */
    q = n << (n_uword_bits - sr);
    r = n >> sr;
    unsigned int carry = 0;
    for (; sr > 0; --sr)
    {
        /* r:q = ((r:q)  << 1) | carry */
        r = (r << 1) | (q >> (n_uword_bits - 1));
        q = (q << 1) | carry;
        /* carry = 0;
         * if (r.all >= d.all)
         * {
         *      r.all -= d.all;
         *      carry = 1;
         * }
         */
        const int s = (int)(d - r - 1) >> (n_uword_bits - 1);
        carry = s & 1;
        r -= d & s;
    }
    q = (q << 1) | carry;
    return q;
}

/* https://chromium.googlesource.com/chromiumos/third_party/coreboot/+/firmware-paine-6301.58.B/src/arch/mips/ashldi3.c */
typedef unsigned word_type;
long long __ashldi3(long long u, word_type b);
struct DWstruct {
	int low, high;
};
typedef union {
	struct DWstruct s;
	long long ll;
} DWunion;
long long __ashldi3(long long u, word_type b)
{
	DWunion uu, w;
	word_type bm;
	if (b == 0)
		return u;
	uu.ll = u;
	bm = 32 - b;
	if (bm <= 0) {
		w.s.low = 0;
		w.s.high = (unsigned int) uu.s.low << -bm;
	} else {
		const unsigned int carries = (unsigned int) uu.s.low >> bm;
		w.s.low = (unsigned int) uu.s.low << b;
		w.s.high = ((unsigned int) uu.s.high << b) | carries;
	}
	return w.ll;
}

long long __lshrdi3(long long u, word_type b)
{
	DWunion uu, w;
	word_type bm;

	if (b == 0)
		return u;

	uu.ll = u;
	bm = 32 - b;

	if (bm <= 0) {
		w.s.high = 0;
		w.s.low = (unsigned int) uu.s.high >> -bm;
	} else {
		const unsigned int carries = (unsigned int) uu.s.high << bm;

		w.s.high = (unsigned int) uu.s.high >> b;
		w.s.low = ((unsigned int) uu.s.low >> b) | carries;
	}

	return w.ll;
}
