all: matmul

matmul: main.cc matrix.cc
	mpicxx -Wall main.cc matrix.cc -o matmul
clean:
	rm -f matmul
