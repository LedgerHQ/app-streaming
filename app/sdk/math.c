#include <limits.h>

/* http://lists.infradead.org/pipermail/barebox/2017-October/031269.html */
typedef int word_type;
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
        w.s.high = (unsigned int)uu.s.low << -bm;
    } else {
        const unsigned int carries = (unsigned int)uu.s.low >> bm;
        w.s.low = (unsigned int)uu.s.low << b;
        w.s.high = ((unsigned int)uu.s.high << b) | carries;
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
        w.s.low = (unsigned int)uu.s.high >> -bm;
    } else {
        const unsigned int carries = (unsigned int)uu.s.high << bm;

        w.s.high = (unsigned int)uu.s.high >> b;
        w.s.low = ((unsigned int)uu.s.low >> b) | carries;
    }

    return w.ll;
}
