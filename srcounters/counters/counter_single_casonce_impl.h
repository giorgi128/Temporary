#ifndef SINGLE_H
#define SINGLE_H

#include "common.h"
#include "random.h"
#include <cstdlib>

class counter {
private:
    PAD;
    union {
        volatile size_t c;
        PAD;
    };
public:
    counter(const int numThreads, const int unused) {
        c = 0;
    }
    ~counter() {}
    inline size_t inc(const size_t amt = 1) {
        size_t old = c;
        size_t ret = CASV(&c, old, old+amt);
        return (ret == old) ? old+amt : ret;
    }
    inline size_t readFast() {
        return c;
    }
    size_t readAccurate() {
        return c;
    }
    bool validate(long long threadChecksum, long long * counterChecksum) {
        *counterChecksum = readAccurate();
        return true;
    }
    void print() {}
};

#endif /* SINGLE_H */

