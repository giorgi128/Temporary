#ifndef MULTI_H
#define MULTI_H

#include "common.h"
#include "random.h"
#include <cstdlib>

extern __thread Random * ___rng;
extern __thread int tid;

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
            : counters(new single_counter_t[max(2, sizeMultiple*numThreads)+1]) // don't use first entry (to effectively add padding at the start of the array)
            , numCounters(max(2, sizeMultiple*numThreads)) {
            
        for (int i=0;i<numCounters;++i) {
            counters[i+1].v = 0; // don't use first entry (to effectively add padding at the start of the array)
        }
    }
    ~counter() {
        delete[] counters;
    }
    inline size_t inc(const size_t amt = 1) {
#ifdef USE_LOCAL_HACK
        bool cheap = ___rng->nextNatural(1024) < 512;
        if (cheap) {
            return FAA(&counters[tid+1].v, amt) + amt; // don't use first entry (to effectively add padding at the start of the array)
        }
#endif
        
        const int i = ___rng->nextNatural(numCounters)+1; // don't use first entry (to effectively add padding at the start of the array)
        int j;
        do {
            j = ___rng->nextNatural(numCounters)+1; // don't use first entry (to effectively add padding at the start of the array)
        } while (i == j);
        size_t vi = counters[i].v;
        size_t vj = counters[j].v;
        return FAA((vi < vj ? &counters[i].v : &counters[j].v), amt) + amt;
    }
    inline size_t readFast() {
        const int i = ___rng->nextNatural(numCounters)+1; // don't use first entry (to effectively add padding at the start of the array)
        return numCounters * counters[i].v;
    }
    size_t readAccurate() {
        size_t sum = 0;
        for (int i=0;i<numCounters;++i) {
            sum += counters[i+1].v; // don't use first entry (to effectively add padding at the start of the array)
        }
        return sum;
    }
    bool validate(long long threadChecksum, long long * counterChecksum) {
        *counterChecksum = readAccurate();
        return (threadChecksum == *counterChecksum);
    }
    void print() {
        size_t min = (numCounters > 0 ? counters[0].v : 0);
        size_t max = 0;
        cout<<"counters:";
        for (int i=0;i<numCounters;++i) {
            cout<<" "<<counters[i+1].v; // don't use first entry (to effectively add padding at the start of the array)
            if (counters[i+1].v < min) min = counters[i+1].v; // don't use first entry (to effectively add padding at the start of the array)
            if (counters[i+1].v > max) max = counters[i+1].v; // don't use first entry (to effectively add padding at the start of the array)
        }
        cout<<endl;
        cout<<"counters min="<<min<<" max="<<max<<endl;
    }
};

#endif /* MULTI_H */

