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
    size_t inc(const size_t amt = 1) {
        return FAA(&c, amt) + amt;
    }
    inline size_t readFast() {
        return c;
    }
    inline size_t readAccurate() {
        return c;
    }
    bool validate(long long threadChecksum, long long * counterChecksum) {
        *counterChecksum = readAccurate();
        return (threadChecksum == *counterChecksum);
    }
    void print() {}
};

#endif /* SINGLE_H */

