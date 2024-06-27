#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void construct_matrices(int *n_ptr, int *m_ptr, int *l_ptr,
                        int **a_mat_ptr, int **b_mat_ptr)
{
	int world_rank, world_size;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	if (world_rank == 0)
	{
		scanf("%d %d %d", n_ptr, m_ptr, l_ptr);
		if (*n_ptr > 10000 || *m_ptr > 10000 || *l_ptr > 10000)
		{
			printf("******Error: The size of the matrix is out of range 10000!!******\n");
			printf("******Please give a within constraint size.\n");
			exit(1);
		}
		
		*a_mat_ptr = (int*)malloc(sizeof(int) * *n_ptr * *m_ptr);
		*b_mat_ptr = (int*)malloc(sizeof(int) * *m_ptr * *l_ptr);

		for (int i = 0; i < *n_ptr; i++)
		{
			for (int j = 0; j < *m_ptr; j++)
			{
				scanf("%d", (*a_mat_ptr + i * *m_ptr + j));
				if (*(*a_mat_ptr + i * *m_ptr + j) > 100)
				{
					printf("******Error: The value within the matrix is out of range 100!!******\n");
					printf("******Please give a within constraint value.\n");
					exit(1);
				}
			}
		}

		for (int i = 0; i < *m_ptr; i++)
		{
			for (int j = 0; j < *l_ptr; j++)
			{
				scanf("%d", (*b_mat_ptr + i * *l_ptr + j));
				if (*(*b_mat_ptr + i * *l_ptr + j) > 100)
				{
					printf("******Error: The value within the matrix is out of range 100!!******\n");
					printf("******Please give a within constraint value.\n");
					exit(1);
				}
			}
		}

		int size[3] = {*n_ptr, *m_ptr, *l_ptr};
		MPI_Bcast(size, 3, MPI_INT, 0, MPI_COMM_WORLD);
	}
	else if (world_rank > 0)
	{
		int size[3];
		MPI_Bcast(size, 3, MPI_INT, 0, MPI_COMM_WORLD);
		*n_ptr = size[0];
		*m_ptr = size[1];
		*l_ptr = size[2];

		*a_mat_ptr = (int*)malloc(sizeof(int) * (*n_ptr/world_size) * *m_ptr);
		*b_mat_ptr = (int*)malloc(sizeof(int) * *m_ptr * *l_ptr);
		
	}

	MPI_Scatter(*a_mat_ptr, (*n_ptr/world_size) * *m_ptr, MPI_INT, *a_mat_ptr, (*n_ptr/world_size) * *m_ptr, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(*b_mat_ptr, *m_ptr * *l_ptr, MPI_INT, 0, MPI_COMM_WORLD);
}

void matrix_multiply(const int n, const int m, const int l,
                     const int *a_mat, const int *b_mat)
{
	int world_rank, world_size;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	int *global_c, *local_c = (int*)malloc(sizeof(int) * (n/world_size) * l);
	if (world_rank == 0)
	{
		global_c = (int*)malloc(sizeof(int) * n * l);
	}
	
	for (int i = 0; i < n/world_size; i++)
	{
		for (int j = 0; j < l; j++)
		{
			int c_ij = 0;
			for (int k = 0; k < m; k++)
			{
				c_ij += *(a_mat + i * m + k) * *(b_mat + k * l + j);
			}
			*(local_c + i * l + j) = c_ij;
		}
	}
	
	MPI_Gather(local_c, (n/world_size) * l, MPI_INT, global_c, (n/world_size) * l, MPI_INT, 0, MPI_COMM_WORLD);

	if (world_rank == 0)
	{
		if (world_size * (n/world_size) !=  n)
		{
			for (int i = world_size * (n/world_size); i < n; i++)
			{
				for (int j = 0; j < l; j++)
				{
					int c_ij = 0;
					for (int k = 0; k < m; k++)
					{
						c_ij += *(a_mat + i * m + k) * *(b_mat + k * l + j);
					}
					*(global_c + i * l + j) = c_ij;
				}
			}
		}
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < l; j++)
			{
				printf("%d ", *(global_c + i * l + j));
			}
			printf("\n");
		}
	}

	free(local_c);
	if (world_rank == 0)
		free(global_c);
}

void destruct_matrices(int *a_mat, int *b_mat)
{
	free(a_mat);
	free(b_mat);
}
