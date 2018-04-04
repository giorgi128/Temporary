#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#define COH_BYTES 128
#define FAA __sync_fetch_and_add
#define CASB __sync_bool_compare_and_swap
#define CASV __sync_val_compare_and_swap
#define MFENCE __sync_synchronize()
#ifndef SOFTWARE_BARRIER
    #define SOFTWARE_BARRIER asm volatile("": : :"memory")
#endif
    
#define CAT2(x, y) x##y
#define CAT(x, y) CAT2(x, y)

#define STR(x) #x 
#define XSTR(x) STR(x)

#define PAD64 volatile char CAT(___padding, __COUNTER__)[COH_BYTES/2]
#define PAD volatile char CAT(___padding, __COUNTER__)[COH_BYTES]

// DEFINITIONS FOR TL2

#define __INLINE__                      static __inline__
#define __ATTR__                        __attribute__((always_inline))

enum {
    NEVER  = 0,
    ALWAYS = 1,
};

    
    
#ifdef __cplusplus
}
#endif

#endif /* COMMON_H */

