#ifndef MULTI_H
#define MULTI_H

#include "common.h"
#include "random.h"
#include <cstdlib>

extern __thread int tid;
extern __thread int socket;
extern __thread Random * ___rng;

struct single_counter_t {
    union {
        PAD;
        volatile size_t v;
    };
};

class counter {
private:
    PAD;
    single_counter_t * counters;
    const int numCounters;
    PAD;
public:
    counter(const int numThreads, const int sizeMultiple)
            /* NOTE: the following magic "4" is defined in the paper's proof,
               and is needed to guarantee quality for the relaxation. */
            : counters(new single_counter_t[4*sizeMultiple*numThreads])
            , numCounters(sizeMultiple*numThreads) {
            
        for (int i=0;i<numCounters;++i) {
            counters[i].v = 0;
        }
    }
    ~counter() {
        delete[] counters;
    }
    inline size_t inc(const size_t amt = 1) {
        int i;
        do {
            i = ___rng->nextNatural(numCounters);
        } while (!(i >= socket*numCounters/2 && i < (socket+1)*numCounters/2));
        int j;
        do {
            j = ___rng->nextNatural(numCounters);
        } while (!(j >= socket*numCounters/2 && j < (socket+1)*numCounters/2) || i == j);
        size_t vi = counters[i].v;
        size_t vj = counters[j].v;
        return FAA((vi < vj ? &counters[i].v : &counters[j].v), amt) + amt;
    }
    inline size_t readFast() {
        const int i = ___rng->nextNatural(numCounters);
        return numCounters * counters[i].v;
    }
    size_t readAccurate() {
        size_t sum = 0;
        for (int i=0;i<numCounters;++i) {
            sum += counters[i].v;
        }
        return sum;
    }
    bool validate(long long threadChecksum, long long * counterChecksum) {
        *counterChecksum = readAccurate();
        return (threadChecksum == *counterChecksum);
    }
    void print() {}
};

#endif /* MULTI_H */

