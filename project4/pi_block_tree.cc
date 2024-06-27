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
    unsigned long long int local_cnt = 0, local_tasks = tosses / world_size;
    unsigned int SEED = 331, local_seed = SEED * world_rank, tag = 0;
    MPI_Status status;

    for (int i = 0; i < local_tasks; i++)
    {
        float x = (float) rand_r(&local_seed) / RAND_MAX;
        float y = (float) rand_r(&local_seed) / RAND_MAX;
        if (x*x + y*y <= 1.f)
            local_cnt++;
    }

    // TODO: binary tree redunction
    int base = 1;
    while (base < world_size)
    {
        int new_rank = world_rank / base;
        if (new_rank % 2 == 1)
        {
            int dest = world_rank - base;
            MPI_Send(&local_cnt, 1, MPI_UNSIGNED_LONG, dest, tag, MPI_COMM_WORLD);
        }
        else
        {
            int source = world_rank + base;
            unsigned long long int local_recv;
            MPI_Recv(&local_recv, 1, MPI_UNSIGNED_LONG, source, tag, MPI_COMM_WORLD, &status);
            local_cnt += local_recv;
        }
        base *= 2;
    }

    if (world_rank == 0)
    {
        // TODO: PI result
        pi_result = 4 * local_cnt / (double) tosses;

        // --- DON'T TOUCH ---
        double end_time = MPI_Wtime();
        printf("%lf\n", pi_result);
        printf("MPI running time: %lf Seconds\n", end_time - start_time);
        // ---
    }

    MPI_Finalize();
    return 0;
}
