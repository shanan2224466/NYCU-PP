CXX := gcc
CXXFLAGS := -I./common -O0 -std=c11 -Wall

all: s p p2

p2: Project_v3p_part_pthread.c
	$(CXX) $(CXXFLAGS) -pthread Project_v3p_part_pthread.c -o p2

s: Project_v3_part.c
	$(CXX) $(CXXFLAGS) Project_v3_part.c -o s

p: Project_v3p_part.c
	$(CXX) $(CXXFLAGS) -fopenmp Project_v3p_part.c -o p

clean:
	rm -f p s p2