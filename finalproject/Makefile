CXX := g++
CXXFLAGS := -I./common -O3 -std=c++17 -Wall

all: serial parallel_omp parallel_pthread

serial: serial_project.cpp
	$(CXX) $(CXXFLAGS) serial_project.cpp -o serial

parallel_omp: parallel_project_omp.cpp
	$(CXX) $(CXXFLAGS) -fopenmp parallel_project_omp.cpp -o parallel_omp

parallel_pthread: parallel_project_pthread.cpp
	$(CXX) $(CXXFLAGS) -pthread parallel_project_pthread.cpp -o parallel_pthread

clean:
	rm -f serial parallel_omp parallel_pthread
