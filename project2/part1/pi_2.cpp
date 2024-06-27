#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "fasttime.h"
#define N 100

typedef struct {
	int count;
	int seed;
	unsigned long long int threadAmount;
}ForThread;

void *countpi(void* argument);
int sum = 0;
pthread_mutex_t mutex;

int main (int argc, char *argv[]){
	
	int i, nthread = atoi(argv[1]);
	long long int MAX = atoi(argv[2]), amount = MAX / nthread;
	void *temp;
	ForThread *data;

	// Set the attribute of the thread to be joinable.
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	srand(time(NULL));
	
	pthread_t *thread = (pthread_t*) malloc(nthread * sizeof(pthread_t));

	ForThread forThread[nthread];
	fasttime_t time1 = gettime();
	for (i = 0; i < nthread; i++)
	{
		if (i == nthread -1 && MAX % nthread )
		{
			forThread[i].threadAmount = amount + MAX - (nthread * amount);
		}
		else
		{
			forThread[i].threadAmount = amount;
		}
		forThread[i].seed = rand();
		
		pthread_create(&thread[i], NULL, countpi, (void *)&forThread[i]);
	}

	pthread_attr_destroy(&attr);

	for (i = 0; i < nthread; i++)
	{
		pthread_join(thread[i], NULL);
	}
	fasttime_t time2 = gettime();
	pthread_mutex_destroy(&mutex);
	printf("pi = %f\n", 4.0 * sum / MAX);
	double elapsedf = tdiff(time1, time2);
	printf("The execution time of pi.out (thread version): %lf sec.\n", elapsedf);

	free(thread);
	return 0;
}

void *countpi(void *argument){

	int i, j;
	ForThread *arg = (ForThread *)argument;
//	float x[N] __attribute__((aligned (32))) = {0.0}, y[N] __attribute__((aligned (32))) = {0.0}, z[N] __attribute__((aligned (32))) = {0.0};
	float x[N], y[N], z[N];
	printf("%p, %p, %p\n", x, y, z);
	const int loop = arg->threadAmount;
	unsigned int temp = arg->seed;
	arg->count = 0;
	for (i = 0; i < loop; i++)
	{
		x[i % N] = rand_r(&temp) / ((float) RAND_MAX + 1);
		y[i % N] = rand_r(&temp) / ((float) RAND_MAX + 1);
		if (i % N == 0 && i)
		{
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
	}

	pthread_mutex_lock(&mutex);
	sum += arg->count;
	pthread_mutex_unlock(&mutex);

	pthread_exit((void *)0);
}