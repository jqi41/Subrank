OBJS = 
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
CC = g++ -std=c++0x
endif
ifeq ($(UNAME), Darwin)
CC = clang++ -std=c++0x -stdlib=libc++ -Weverything
endif

DEBUG = -ggdb3
PROF = -pg
FAST = -O3
CFLAGS = -Wall -c -w $(DEBUG) $(PROF) $(FAST)
LFLAGS = -Wall -w $(DEBUG) $(PROF) $(FAST)

Submod_NDCG : $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) main.cc -o main

clean:
	\rm main
