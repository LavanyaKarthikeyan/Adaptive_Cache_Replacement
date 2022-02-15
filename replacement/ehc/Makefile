CODE_DIR = src

CPPFLAGS = -std=c++11 -Wall -g -O3
CC = g++

CACHE1 = cache1.out
CACHE2 = cache2.out
CACHE3 = cache3.out
CACHE4 = cache4.out

CACHE_SOURCES := $(wildcard $(CODE_DIR)/*.cc)
CACHE1_OBJS := $(CACHE_SOURCES:%.cc=%.o1)
CACHE2_OBJS := $(CACHE_SOURCES:%.cc=%.o2)
CACHE3_OBJS := $(CACHE_SOURCES:%.cc=%.o3)
CACHE4_OBJS := $(CACHE_SOURCES:%.cc=%.o4)

all: $(CACHE1) $(CACHE2) $(CACHE3) $(CACHE4)

%.o1: %.cc
	$(CC) $(CPPFLAGS) -c $< -o $@ -DNUM_CORE=1
%.o2: %.cc
	$(CC) $(CPPFLAGS) -c $< -o $@ -DNUM_CORE=1
%.o3: %.cc
	$(CC) $(CPPFLAGS) -c $< -o $@ -DNUM_CORE=4
%.o4: %.cc
	$(CC) $(CPPFLAGS) -c $< -o $@ -DNUM_CORE=4


$(CACHE1): $(CACHE1_OBJS) 
	$(CC) $(CPPFLAGS) $^ -o $@ -lpthread -lm lib/config1.a -DNUM_CORE=1
	
$(CACHE2): $(CACHE2_OBJS) 
	$(CC) $(CPPFLAGS) $^ -o $@ -lpthread -lm lib/config2.a -DNUM_CORE=1
	
$(CACHE3): $(CACHE3_OBJS) 
	$(CC) $(CPPFLAGS) $^ -o $@ -lpthread -lm lib/config3.a -DNUM_CORE=4
	
$(CACHE4): $(CACHE4_OBJS) 
	$(CC) $(CPPFLAGS) $^ -o $@ -lpthread -lm lib/config4.a -DNUM_CORE=4

clean:
	rm -rf $(CACHE1) $(CACHE2) $(CACHE3) $(CACHE4) *.o */*.o */*/*.o *.o1 */*.o1 */*/*.o1 *.o2 */*.o2 */*/*.o2 *.o3 */*.o3 */*/*.o3 *.o4 */*.o4 */*/*.o4 *~ */*~ */*/*~
