CXX := mpic++
CXXFLAGS := -I./common -O3 -std=c++17 -Wall

# Object files
OBJS := serial.o parallel_omp.o parallel_pthread.o parallel_mpi.o main.o

all: main

main.o: main.cpp def.h
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

serial.o: serial_project.cpp def.h
	$(CXX) $(CXXFLAGS) -c serial_project.cpp -o serial.o

parallel_omp.o: parallel_project_omp.cpp def.h
	$(CXX) $(CXXFLAGS) -fopenmp -c parallel_project_omp.cpp -o parallel_omp.o

parallel_pthread.o: parallel_project_pthread.cpp def.h
	$(CXX) $(CXXFLAGS) -pthread -c parallel_project_pthread.cpp -o parallel_pthread.o

parallel_mpi.o: parallel_mpi.cpp def.h
	$(CXX) $(CXXFLAGS) -c parallel_mpi.cpp -o parallel_mpi.o

main: $(OBJS)
	$(CXX) $(CXXFLAGS) -fopenmp -pthread $(OBJS) -o main

clean:
	rm -f *.o main
