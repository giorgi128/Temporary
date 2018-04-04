CXX = g++

BASICOPTS =
#BASICOPTS += -DNDEBUG
BASICOPTS += -g
#BASICOPTS += -m64
BASICOPTS += -std=c++11
BASICOPTS += -O3
BASICOPTS += -DGVERSIONS_TO_WRITE_IN_FUTURE=\(24*20\)
#BASICOPTS += -DGVERSIONS_TO_WRITE_IN_FUTURE=1000

#BASICOPTS += -DMEASURE_LATENCY

MACHINE_SPECIFIC = 
MACHINE_SPECIFIC += -DCPU_FREQ_GHZ=2.1
MACHINE_SPECIFIC += -DMAX_TID_POW2=64
MACHINE_SPECIFIC += -DLOGICAL_PROCESSORS=48
MACHINE_SPECIFIC += -DBYTES_IN_CACHE_LINE=\(64\)
MACHINE_SPECIFIC += -DPREFETCH_SIZE_BYTES=\(2*BYTES_IN_CACHE_LINE\)

CXXFLAGS = 
CXXFLAGS += $(BASICOPTS)
CXXFLAGS += $(MACHINE_SPECIFIC)
CXXFLAGS += -I./common
CXXFLAGS += -I./counters
#CXXFLAGS += -DUSE_LOCAL_HACK

LDFLAGS =
LDFLAGS += -lpthread
#LDFLAGS += -v

all: counterbench tl2relax tl2original
counterbench: counter_single counter_single_casonce counter_single_casloop counter_multi counter_multi_numa
tl2relax: tl2relax_single tl2relax_single_casonce tl2relax_single_casloop tl2relax_multi tl2relax_multi_numa

counter_%: main.cpp common/*.h
	$(CXX) -DCOUNTER_CLASS_H=$@_impl.h -o $@ main.cpp $(CXXFLAGS) $(LDFLAGS)
tl2relax_%: main.cpp common/*.h
	$(CXX) -DTL2_BENCH -DCOUNTER_CLASS_H=${@:tl2relax_%=counter_%}_impl.h -o $@ main.cpp $(CXXFLAGS) $(LDFLAGS)
tl2original: main.cpp common/*.h
	$(CXX) -DTL2_BENCH -DTL2_ORIGINAL -DCOUNTER_CLASS_H=counter_single_impl.h -o $@ main.cpp $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f counter_* tl2relax_* tl2original
