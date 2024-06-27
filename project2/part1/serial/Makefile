all: pi.out

pi.out: pi_serial.cpp
	gcc -I./common -O3 -std=c++17 -Wall pi_serial.cpp -o pi.out
clean:
	rm -f *.o *.s pi.out *~

