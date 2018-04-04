/**
 * Preliminary C++ implementation of binary search tree using LLX/SCX and DEBRA(+).
 * 
 * Copyright (C) 2015 Trevor Brown
 * 
 */

#ifndef MACHINECONSTANTS_H
#define	MACHINECONSTANTS_H

#include <cstdlib>

#ifndef MAX_TID_POW2
    #define MAX_TID_POW2 128 // MUST BE A POWER OF TWO, since this is used for some bitwise operations
#endif
#ifndef LOGICAL_PROCESSORS
    #define LOGICAL_PROCESSORS 128
#endif

// the following definition is only used to pad data to avoid false sharing.
// although the number of words per cache line is actually 8, we inflate this
// figure to counteract the effects of prefetching multiple adjacent cache lines.
#ifndef BYTES_IN_CACHE_LINE
#define BYTES_IN_CACHE_LINE (64)
#endif

#ifndef PREFETCH_SIZE_BYTES
#define PREFETCH_SIZE_BYTES (2*BYTES_IN_CACHE_LINE)
#endif

#ifndef PREFETCH_SIZE_WORDS
#define PREFETCH_SIZE_WORDS (PREFETCH_SIZE_BYTES/sizeof(size_t))
#endif

#endif	/* MACHINECONSTANTS_H */

