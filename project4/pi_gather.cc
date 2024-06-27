#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    // --- DON'T TOUCH ---
    MPI_Init(&argc, &argv);
    double start_time = MPI_Wtime();
    double pi_result;
    long long int tosses = atoi(argv[1]);
    int world_rank, world_size;
    // ---

    // TODO: MPI init
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    unsigned long long int global_cnt[world_size], local_cnt = 0, local_tasks = tosses / world_size;
    unsigned int SEED = 331, local_seed = SEED * world_rank;
    
    for (int i = 0; i < local_tasks; i++)
    {
        float x = (float) rand_r(&local_seed) / RAND_MAX;
        float y = (float) rand_r(&local_seed) / RAND_MAX;
        if (x*x + y*y <= 1.f)
            local_cnt++;
    }

    // TODO: use MPI_Gather
    MPI_Gather(&local_cnt, 1, MPI_UNSIGNED_LONG, global_cnt, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);

    if (world_rank == 0)
    {
        // TODO: PI result
        pi_result = 0;
        for (int i = 0; i < world_size; i++)
        {
            pi_result += global_cnt[i];
        }
        pi_result = 4 * pi_result / (double) tosses;

        // --- DON'T TOUCH ---
        double end_time = MPI_Wtime();
        printf("%lf\n", pi_result);
        printf("MPI running time: %lf Seconds\n", end_time - start_time);
        // ---
    }
    
    MPI_Finalize();
    return 0;
}
