CXX := clang++
CXXFLAGS := -I./common -O3 -std=c++17 -Wall

ifeq (/usr/bin/clang++-11,$(wildcard /usr/bin/clang++-11*))
    CXX=clang++-11
endif

all: pi.out

pi.out: pi.cpp
	$(CXX) $(CXXFLAGS) -pthread -O3 -std=c++17 -Wall -Rpass=loop-vectorize -Rpass-missed=loop-vectorize -Rpass-analysis=loop-vectorize -mavx2 pi.cpp -o pi.out
clean:
	rm -f *.o *.S pi.out

