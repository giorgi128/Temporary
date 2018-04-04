/**
 * todo:
 *      add papi
 *      output latencies in cycles?
 *      add other counter implementations
 *      test on tapuz (when is our server coming?)
 *      numa aware counters?
 */

#include <thread>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <chrono>
#include <ctime>

#include <iomanip>
#include <locale>

#include "common.h"
#include "random.h"
#include "binding.h"

// include the appropriate counter class, according to the make build target
#include XSTR(COUNTER_CLASS_H)

using namespace std;

__thread int tid;
__thread int socket;
__thread Random * ___rng;

const int SAMPLE_SIZE = 2;

/**
 * Configure global statistics using stats_global.h
 */

#define __HANDLE_STATS(handle_stat) \
    handle_stat(LONG_LONG, num_ops, 1, { \
            stat_output_item(PRINT_RAW, SUM, BY_THREAD) \
          C stat_output_item(PRINT_RAW, AVERAGE, TOTAL) \
          C stat_output_item(PRINT_RAW, SUM, TOTAL) \
    }) \
    handle_stat(LONG_LONG, latency_increments_nanosec, 100000, { \
            stat_output_item(PRINT_HISTOGRAM_LOG, NONE, FULL_DATA) \
          C stat_output_item(PRINT_RAW, MIN, TOTAL) \
          C stat_output_item(PRINT_RAW, MAX, TOTAL) \
          C stat_output_item(PRINT_RAW, AVERAGE, TOTAL) \
    }) \
    handle_stat(LONG_LONG, timer_latency, 1, {})

#define USE_GSTATS
#include "stats_global.h"
GSTATS_DECLARE_STATS_OBJECT(MAX_TID_POW2);
GSTATS_DECLARE_ALL_STAT_IDS;

struct globals_t {
PAD;
    Random rngs[MAX_TID_POW2]; // create per-thread random number generators (padded to avoid false sharing)
PAD;
    char * bindstr;
    int nthreads;
    int millis;
    int param;
PAD;
    chrono::time_point<chrono::high_resolution_clock> startTime;
    volatile int running;
    volatile bool done;
    volatile bool start;
PAD;
    counter * c;
    int arraySize;
    long * array;
PAD;
    
    globals_t() {
        bindstr = NULL;
        nthreads = 0;
        millis = 0;
        param = 0;
        running = 0;
        done = false;
        start = false;
    }
} __attribute__((aligned(COH_BYTES))) g;

// include "tm.h" AFTER globals_t, since it uses globals_t (and i'm too lazy to move globals_t to its own .h file)
#include "tm.h"

void threadRun(void * unused) {
    tid = thread_getId();
    TM_THREAD_ENTER();
    //tid = __tid;
    socket = tid >= g.nthreads/2; // NOTE: filthy hack assuming threads are bound to socket 0 first (could use libnuma to do this properly)
    printf("tid=%d socket=%d\n", tid, socket);
    
    // initialize
    constexpr int OPS_BETWEEN_TIME_CHECKS = 50;
    binding::bindThread(tid, LOGICAL_PROCESSORS);
    ___rng = &g.rngs[tid];
    int cnt = 0;

    // start barrier
    FAA(&g.running, 1);
//    cout<<"running="<<g.running<<endl;
    while (!g.start) {}
    
    // perform operations
    chrono::time_point<chrono::high_resolution_clock> __endTime = g.startTime;
    while (!g.done) {
        
        // only check the time once every few operations
        if (((++cnt) % OPS_BETWEEN_TIME_CHECKS) == 0) {
            __endTime = chrono::high_resolution_clock::now();
            if (chrono::duration_cast<chrono::milliseconds>(__endTime-g.startTime).count() >= g.millis) {
                g.done = true;
                //MFENCE; // fire coherence transaction immediately (not needed because next instruction is FAA, which implies flush on x86/64)
                break;
            }
        }
        
        // perform an operation
        
#ifdef MEASURE_LATENCY
        if (!GSTATS_IS_FULL(tid, latency_increments_nanosec)) GSTATS_TIMER_RESET(tid, timer_latency);
#endif
        
        g.c->inc();
        
#ifdef MEASURE_LATENCY
        if (!GSTATS_IS_FULL(tid, latency_increments_nanosec)) GSTATS_TIMER_APPEND_ELAPSED(tid, timer_latency, latency_increments_nanosec);
#endif
        
        GSTATS_ADD(tid, num_ops, 1);
    }
    
//    cout<<"thread "<<tid<<" waiting until g.running = 0"<<endl;
    // end barrier
    FAA(&g.running, -1);
    while (g.running > 0) {}
//    cout<<"thread "<<tid<<" calling tm thread exit"<<endl;

    // deinitialize
    TM_THREAD_EXIT();
//    cout<<"thread "<<tid<<" exited."<<endl;
}

void threadRunTL2Bench(void * unused) {
    tid = thread_getId();
    TM_THREAD_ENTER();
    //tid = __tid;
    socket = tid >= g.nthreads/2; // NOTE: filthy hack assuming threads are bound to socket 0 first (could use libnuma to do this properly)
    
    // initialize
    constexpr int OPS_BETWEEN_TIME_CHECKS = 50;
    binding::bindThread(tid, LOGICAL_PROCESSORS);
    ___rng = &g.rngs[tid];
    int cnt = 0;

    // start barrier
    FAA(&g.running, 1);
    while (!g.start) {}
    
    // perform operations
    chrono::time_point<chrono::high_resolution_clock> __endTime = g.startTime;
    while (!g.done) {
        
        // only check the time once every few operations
        if (((++cnt) % OPS_BETWEEN_TIME_CHECKS) == 0) {
            __endTime = chrono::high_resolution_clock::now();
            if (chrono::duration_cast<chrono::milliseconds>(__endTime-g.startTime).count() >= g.millis) {
                g.done = true;
                //MFENCE; // fire coherence transaction immediately (not needed because next instruction is FAA, which implies flush on x86/64)
                break;
            }
        }
        
        // perform an operation
        
#ifdef MEASURE_LATENCY
        if (!GSTATS_IS_FULL(tid, latency_increments_nanosec)) GSTATS_TIMER_RESET(tid, timer_latency);
#endif
        
        int ix[SAMPLE_SIZE+1];

//        int maxval = -1;
//        for (int i=0;i<SAMPLE_SIZE+1;++i) {
//            ix[i] = (int) ___rng->nextNatural() & 0x7fffffff;
//            if (ix[i] > maxval) maxval = ix[i];
//        }
//        sort(ix, ix + SAMPLE_SIZE + 1);
////        unsigned maxover = 0;
//        for (int i=0;i<SAMPLE_SIZE;++i) {
//            ix[i] = (int) ((double) ix[i] * (double) g.arraySize / (double) maxval); // i == SAMPLE_SIZE+1 would yield ix[i] == g.arraySize exactly
//            if (ix>0 && ix[i] <= ix[i-1]) ix[i] = ix[i-1]+1; // deal with duplicates once we scale down -- this may push the range greater than g.arraySize-1
//            //assert(ix[i] >= 0 && ix[i] <= g.arraySize - 1);
//            //if (ix[i] - (g.arraySize-1) > maxover) maxover = ix[i] - (g.arraySize-1); // figure out how much we went over g.arraySize-1
//        }
//        int maxover = (ix[SAMPLE_SIZE-1] > (g.arraySize-1) ? ix[SAMPLE_SIZE-1] - (g.arraySize-1) : 0);
//        if (maxover > 0) {
//            // manually adjust for overflow at the end of the indexes by just shifting left as much as possible (this biases the distribution!!)
//            //cout<<"maxover was "<<maxover<<endl;
//            if (ix[0] >= maxover) ix[0] -= maxover;
//            else ix[0] = 0;
//
//            for (int i=1;i<SAMPLE_SIZE;++i) {
//                if (ix[i] > maxover + ix[i-1]) ix[i] -= maxover;
//                else ix[i] = ix[i-1]+1;
//            }
//        }
//        
//#ifndef NDEBUG
//        // perform validation on indices generated by the above
//        for (int i=0;i<SAMPLE_SIZE;++i) {
//            if (ix[i] < 0) { cout<<"FAILURE: ix["<<i<<"]="<<ix[i]<<endl; exit(-1); }
//            if (ix[i] >= g.arraySize) { cout<<"FAILURE: ix["<<i<<"]="<<ix[i]<<endl; exit(-1); }
//            if (i>0 && ix[i] <= ix[i-1]) { cout<<"FAILURE: ix["<<i-1<<"]="<<ix[i-1]<<" and ix["<<i<<"]="<<ix[i]<<endl; exit(-1); }
//        }
//#endif
        
        ix[0] = (int) ___rng->nextNatural(g.arraySize);
        do {
            ix[1] = (int) ___rng->nextNatural(g.arraySize);
        } while (ix[1] == ix[0]);
        
        TM_BEGIN();
            for (int i=0;i<SAMPLE_SIZE;++i) {
                long v = TM_SHARED_READ_L(g.array[ix[i]]);
                TM_SHARED_WRITE_L(g.array[ix[i]], v+1);
            }
        TM_END();
        
#ifdef MEASURE_LATENCY
        if (!GSTATS_IS_FULL(tid, latency_increments_nanosec)) GSTATS_TIMER_APPEND_ELAPSED(tid, timer_latency, latency_increments_nanosec);
#endif
        
        GSTATS_ADD(tid, num_ops, 1);
    }
    
    // end barrier
    FAA(&g.running, -1);
    while (g.running > 0) {}

    // deinitialize
    TM_THREAD_EXIT();
}

bool validateArray(long long threadChecksum, long long * arrayChecksum) {
    *arrayChecksum = 0;
    for (int i=0;i<g.arraySize;++i) {
        *arrayChecksum += g.array[i];
    }
    bool retval = ((SAMPLE_SIZE * threadChecksum) == *arrayChecksum);
    if (!retval) {
        if (threadChecksum * SAMPLE_SIZE > *arrayChecksum) {
            cout<<"threadChecksum * SAMPLE_SIZE = "<<threadChecksum<<" * "<<SAMPLE_SIZE<<" > *arrayChecksum = "<<(*arrayChecksum)<<endl;
            cout<<"diff = "<<((threadChecksum*SAMPLE_SIZE) - *arrayChecksum)<<endl;
            //cout<<"diff / SAMPLE_SIZE = "<<(((threadChecksum*SAMPLE_SIZE) - *arrayChecksum) / SAMPLE_SIZE)<<endl;
        } else {
            cout<<(*arrayChecksum)<<" = *arrayChecksum > threadChecksum * SAMPLE_SIZE = "<<threadChecksum<<" * "<<SAMPLE_SIZE<<endl;
            cout<<"diff = "<<(*arrayChecksum - (threadChecksum*SAMPLE_SIZE))<<endl;
            //cout<<"diff / SAMPLE_SIZE = "<<((*arrayChecksum - (threadChecksum*SAMPLE_SIZE)) / SAMPLE_SIZE)<<endl;
        }
//        long long min = *arrayChecksum;
//        long long max = 0;
//        cout<<"array:";
//        for (int i=0;i<g.arraySize;++i) {
//            cout<<" "<<g.array[i];
//            if (g.array[i] < min) min = g.array[i];
//            if (g.array[i] > max) max = g.array[i];
//        }
//        cout<<endl;
//        cout<<"array min="<<min<<" max="<<max<<" diff="<<(max-min)<<" scaledDiff="<<((max-min) / SAMPLE_SIZE)<<endl;
    }
//    g.c->print();
    return retval;
}

template<typename T>
std::string formatHumanReadable(T value)
{
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << value;
    return ss.str();
}

#define PRINTN(x) cout<<XSTR(x)<<"="<<x<<endl;
#define PRINTH(x) cout<<XSTR(x)<<"="<<formatHumanReadable(x)<<endl;
#define PRINTDEF(x) cout<<#x<<"="<<XSTR(x)<<endl;

int main(int argc, char** argv) {

    // parse args
    g.array = NULL;
    for (int i=1;i<argc;++i) {
        if (strcmp(argv[i], "-n") == 0) {
            g.nthreads = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-t") == 0) {
            g.millis = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-param") == 0) {
            g.param = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-size") == 0) {
            g.arraySize = atoi(argv[++i]);
            g.array = new long[g.arraySize];
            for (int i=0;i<g.arraySize;++i) {
                g.array[i] = 0;
            }
        } else if (strcmp(argv[i], "-bind") == 0) {
            g.bindstr = argv[++i]; // e.g., "-bind 1,2,3,8-11,4-7,0"
        } else {
            cout<<"bad argument "<<argv[i]<<endl;
            exit(1);
        }
    }
    
#ifdef TL2_BENCH
    if (g.array == NULL) {
        cout<<"Error: set array size (-size) for TL2 bench"<<endl;
        exit(1);
    }
#endif

    PRINTDEF(COUNTER_CLASS_H);
    PRINTN(g.nthreads);
    PRINTN(g.millis);
    if (g.bindstr) {
        binding::parseCustom(g.bindstr);
        cout<<"parsed custom binding: "<<g.bindstr<<endl;
    }
    
    g.c = new counter(g.nthreads, g.param);

    cout<<endl;
    
    // setup thread pinning/binding
    binding::configurePolicy(g.nthreads, LOGICAL_PROCESSORS);
    
    cout<<"ACTUAL_THREAD_BINDINGS=";
    for (int i=0;i<g.nthreads;++i) {
        cout<<(i?",":"")<<binding::getActualBinding(i, LOGICAL_PROCESSORS);
    }
    cout<<endl;
    if (!binding::isInjectiveMapping(g.nthreads, LOGICAL_PROCESSORS)) {
        cout<<"ERROR: thread binding maps more than one thread to a single logical processor"<<endl;
        exit(-1);
    }

    // setup per-thread statistics
    GSTATS_CREATE_ALL;
    
    cout<<endl;
    
    // start threads
    //  get random number generator seeded with time
    //  we use this rng to seed per-thread rng's that use a different algorithm
    srand(time(NULL));    
    cout<<"thread main: starting threads"<<endl;
    TM_STARTUP(g.nthreads);
    //thread * t[g.nthreads];
    for (int i=0;i<g.nthreads;++i) {
        //t[i] = new thread(threadRun, i);
        g.rngs[i].setSeed(rand());
    }
    thread_startup(g.nthreads);

    // reuse main thread as a work thread (needed for STM library)    
    SOFTWARE_BARRIER;
    g.startTime = chrono::high_resolution_clock::now();
    SOFTWARE_BARRIER;
    g.start = true;
#ifdef TL2_BENCH
    thread_start(threadRunTL2Bench, NULL);
#else
    thread_start(threadRun, NULL);
#endif

    // wait until all threads are initialized
    //while (g.running < g.nthreads) {}
    //SOFTWARE_BARRIER;
    //g.startTime = chrono::high_resolution_clock::now();
    //g.start = true;
    //MFENCE;
    //cout<<"thread main: trial begins..."<<endl;

//    // sleep for the length of time we want threads to run
//    timespec tsExpected;
//    tsExpected.tv_sec = g.millis / 1000;
//    tsExpected.tv_nsec = (g.millis % 1000) * ((__syscall_slong_t) 1000000);
//    nanosleep(&tsExpected, NULL);
//    MFENCE;
//    
//    // test for non-terminating threads
//    const size_t MAX_NAPPING_MILLIS = 5000;
//    size_t elapsedMillis = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - g.startTime).count();
//    size_t elapsedMillisNapping = 0;
//    // take a sequence of short naps until we exhaust MAX_NAPPING_MILLIS
//    timespec tsNap;
//    tsNap.tv_sec = 0;
//    tsNap.tv_nsec = 10000000; // 10ms
//    while (g.running > 0 && elapsedMillisNapping < MAX_NAPPING_MILLIS) {
//        nanosleep(&tsNap, NULL);
//        elapsedMillisNapping = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - g.startTime).count() - elapsedMillis;
//    }
//    if (g.running > 0) {
//        cout<<endl;
//        cout<<"Validation FAILURE: "<<g.running<<" thread(s) hypothesized to be non-terminating; exiting..."<<endl;
//        cout<<endl;
//        cout<<"elapsedMillis="<<elapsedMillis<<" elapsedMillisNapping="<<elapsedMillisNapping<<endl;
//        exit(1);
//    }
    size_t elapsedMillis = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - g.startTime).count();

    // record end time (purely informational -- NOT needed for throughput calculations, because of the "done" mechanism)
    cout<<"thread main: trial ends after "<<(elapsedMillis / 1000.)<<"s"<<endl;

    //// join threads
    //for (int i=0;i<g.nthreads;++i) {
    //    t[i]->join();
    //    delete t[i];
    //}
    thread_shutdown();
    TM_SHUTDOWN();
    cout<<"thread main: joined threads"<<endl;

    cout<<endl;
    
    // output and validate
    GSTATS_PRINT;
    cout<<endl;
    
    long long threadChecksum = GSTATS_GET_STAT_METRICS(num_ops, TOTAL)[0].sum;
    long long dataChecksum = 0;
#ifdef TL2_BENCH
    bool success = validateArray(threadChecksum, &dataChecksum);
#else
    bool success = g.c->validate(threadChecksum, &dataChecksum);
#endif
    if (success) {
        cout<<"Validation OK: checksums="<<formatHumanReadable(threadChecksum)<<endl;
    } else {
        cout<<"Validation FAILED: threadChecksum="<<formatHumanReadable(threadChecksum)<<" counterChecksum="<<formatHumanReadable(dataChecksum)<<endl;
    }
    
    // only output "read" results if validation succeeds
    if (success) {
        cout<<endl;
        cout<<"====================== validated output ======================"<<endl;
        cout<<"throughput="<<(GSTATS_GET_STAT_METRICS(num_ops, TOTAL)[0].sum * (long long) 1000 / g.millis)<<endl;
        cout<<"=============================================================="<<endl;
    }
    
    // deinitialize
    GSTATS_DESTROY;  
    if (g.array) delete g.array;
    return 0;
}
