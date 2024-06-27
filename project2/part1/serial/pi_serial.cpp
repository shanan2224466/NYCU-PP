#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "fasttime.h"

int main(int argc, char *argv[]){
	
	// If count is not initialized first, the value of pi will be wrong.
	int i, count = 0;
	long long int MAX = atoi(argv[1]);
	float x, y;
	
	srand(time(NULL));
	fasttime_t time1 = gettime();
	for (i = 0; i < MAX; i++)
	{
		x = rand() / ((float) RAND_MAX + 1);
		y = rand() / ((float) RAND_MAX + 1);
		if( x * x + y * y <= 1 )
			count++;
	}
	fasttime_t time2 = gettime();
	printf("pi = %f\n", 4.0 * count / MAX);

	double elapsedf = tdiff(time1, time2);
	printf("The execution time of pi.out (serial version): %lf sec.\n", elapsedf); 

	return 0;
}

