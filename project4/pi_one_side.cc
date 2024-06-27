#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

int fnz (unsigned long long int *schedule, unsigned long long int *oldschedule, int world_size)
{
    int diff = 0;

    for (int i = 1; i < world_size; i++)
       diff |= (schedule[i] == oldschedule[i]);

    if (diff)
       return 0;
    return 1;
}

int main(int argc, char **argv)
{
    // --- DON'T TOUCH ---
    MPI_Init(&argc, &argv);
    double start_time = MPI_Wtime();
    double pi_result;
    long long int tosses = atoi(argv[1]);
    int world_rank, world_size;
    // ---

    MPI_Win win;

    // TODO: MPI init
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    unsigned long long int global_cnt = 0, local_cnt = 0, local_tasks = tosses / world_size;
    unsigned int SEED = 331, local_seed = SEED * world_rank;
    
    for (int i = 0; i < local_tasks; i++)
    {
        float x = (float) rand_r(&local_seed) / RAND_MAX;
        float y = (float) rand_r(&local_seed) / RAND_MAX;
        if (x*x + y*y <= 1.f)
            local_cnt++;
    }

    if (world_rank == 0)
    {
        // Master
		unsigned long long int *schedule, oldschedule[world_size];
    	MPI_Alloc_mem(world_size * sizeof(long long int), MPI_INFO_NULL, &schedule);

		for (int i = 0; i < world_size; i++)
		{
			schedule[i] = 0;
			oldschedule[i] = 0;
		}

		MPI_Win_create(schedule, world_size * sizeof(unsigned long long int),
		sizeof(unsigned long long int), MPI_INFO_NULL, MPI_COMM_WORLD, &win);

		int ready = 0;
		while (!ready)
		{
			// Without the lock/unlock schedule stays forever filled with 0s
			MPI_Win_lock(MPI_LOCK_SHARED, 0, 0, win);
			ready = fnz(schedule, oldschedule, world_size);
			MPI_Win_unlock(0, win);
		}

		global_cnt += local_cnt;
        for (int i = 1; i < world_size; i++)
        {
            global_cnt += schedule[i];
        }

		// Free the allocated memory
		MPI_Free_mem(schedule);
    }
    else
    {
        // Workers

		// Worker processes do not expose memory in the window
		MPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &win);

		// Register with the master
		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, win);
		MPI_Put(&local_cnt, 1, MPI_UNSIGNED_LONG, 0, world_rank, 1, MPI_UNSIGNED_LONG, win);
		MPI_Win_unlock(0, win);
    }

    MPI_Win_free(&win);

    if (world_rank == 0)
    {
        // TODO: handle PI result
		pi_result = 4 * global_cnt / (double) tosses;

        // --- DON'T TOUCH ---
        double end_time = MPI_Wtime();
        printf("%lf\n", pi_result);
        printf("MPI running time: %lf Seconds\n", end_time - start_time);
        // ---
    }
    
    MPI_Finalize();
    return 0;
}