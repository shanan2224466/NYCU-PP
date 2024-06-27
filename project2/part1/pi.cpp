#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "fasttime.h"
#define N 32

typedef struct {
	int count;
	int seed;
	unsigned long long int threadAmount;
}ForThread;

void* countpi(void* argument);

int main (int argc, char *argv[]){
	
	int i, nthread = atoi(argv[1]), sum = 0;
	long long int MAX = atoi(argv[2]), amount = MAX / nthread;
	void *temp;
	ForThread *data;

	// Set the attribute of the thread to be joinable.
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	srand(time(NULL));
	
	pthread_t *thread = (pthread_t*) malloc(nthread * sizeof(pthread_t));

	// The package for each thread.
	ForThread forThread[nthread];
	
	fasttime_t time1 = gettime();
	for (i = 0; i < nthread; i++)
	{
		// The last thread will need to finish the rest part, if argv[2] (the total amount of task) isn't divided by argv[1] (amount of thread).
		if (i == nthread -1 && MAX % nthread )
		{
			forThread[i].threadAmount = amount + MAX - (nthread * amount);
		}
		else
		{
			forThread[i].threadAmount = amount;
		}
		
		// Randomly generate their seed.
		forThread[i].seed = rand();
		
		pthread_create(&thread[i], NULL, countpi, (void *)&forThread[i]);
	}

	pthread_attr_destroy(&attr);

	// Threads join back to parent thread, and parent thread receive the count of each thread.
	for (i = 0; i < nthread; i++)
	{
		pthread_join(thread[i], &temp);
		data = (ForThread *)temp;
		sum += data->count;
	}
	fasttime_t time2 = gettime();

	printf("%f\n", 4.0 * sum / MAX);
	double elapsedf = tdiff(time1, time2);
	// printf("The execution time of pi.out (thread version): %lf sec.\n", elapsedf);

	free(thread);
	return 0;
}

void *countpi(void *argument){

	int i, j;
	ForThread *arg = (ForThread *)argument;
	float x[N] __attribute__((aligned (32))) = {0.0}, y[N] __attribute__((aligned (32))) = {0.0}, z[N] __attribute__((aligned (32))) = {0.0};
	
	// Make the loop be a constant.
	const int loop = arg->threadAmount;

	// Each thread has its own seed.
	unsigned int temp = arg->seed;
	arg->count = 0;

	// Thread processes their tasks.
	for (i = 0; i < loop; i++)
	{
		if (i % N == 0 && i)
		{
			// Use compiler to loop vectorize the loop here.
			for (j = 0; j < N; j++)
			{
				z[j] = x[j] * x[j] + y[j] * y[j];
			}

			for (j = 0; j < N; j++)
			{
				if (z[j] <= 1)
					arg->count++;
			}
		}

		// This part is serial processing. Feed their own seed.
		x[i % N] = rand_r(&temp) / ((float) RAND_MAX + 1);
		y[i % N] = rand_r(&temp) / ((float) RAND_MAX + 1);
	}

	// Finish the remnant data that may missed, if the tasks isn't divided by N.
	if (loop % N)
	{
		for (i = (loop / N) * N, j = 0; i < loop; i++, j++)
		{
			z[j] = x[j] * x[j] + y[j] * y[j];
			if (z[j] <= 1)
					arg->count++;
		}
	}
	
	// Send back the result, instead of using mutex.
	pthread_exit((void *)arg);
}