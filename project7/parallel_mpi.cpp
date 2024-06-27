#include "def.h"

void mpi() {
    MPI_Init(NULL, NULL);

    int world_rank, world_size;
    unsigned int tag = 0;
    MPI_Status status;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    world_size--; // Exclude rank 0 host

    if (world_rank > 0) {
        while(complete_tasks < NUM_OF_TASKS) {
			for(int worker = (world_rank - 1) * (NUM_OF_WORKERS / world_size); worker < world_rank * (NUM_OF_WORKERS / world_size); ++worker) {
				int receipt = last_select_task[worker];
				int local_load = last_local_load[worker];

				if(print_detail)
					printf("Worker %d select task %d last time and remain %d works\n", worker, receipt, local_load);

				if(local_load > 0) {
					local_load -= workers[worker].task_efficient[receipt];

					if(print_detail)
						printf("Worker %d can finish %d works in unit time, now remain %d works\n", worker, workers[worker].task_efficient[receipt], local_load);

					if(local_load <= 0) {
						if(print_detail)
							printf("Worker %d finish his work\n", worker);

						local_load = 0;

						// Case 2: finish unit load
						int lockNum[3] = {2, receipt, -1};
						MPI_Send(lockNum, 3, MPI_INT, 0, tag, MPI_COMM_WORLD);
						MPI_Recv(&tasks[receipt].complete, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);

						// Case 3: update task info
						lockNum[0] = 3;
						int localBuffer[3];
						MPI_Send(lockNum, 3, MPI_INT, 0, tag, MPI_COMM_WORLD);
						MPI_Recv(localBuffer, 3, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
						tasks[receipt].complete = localBuffer[0];
						tasks[receipt].total_load = localBuffer[1];
						tasks[receipt].end = localBuffer[2];

						if((tasks[receipt].complete >= tasks[receipt].total_load) && (tasks[receipt].end == 0)) {
							tasks[receipt].end = 1;
							complete_tasks++;

							localBuffer[0] = 1;
							localBuffer[1] = tasks[receipt].end;
							localBuffer[2] = complete_tasks;
							MPI_Send(localBuffer, 3, MPI_INT, 0, tag, MPI_COMM_WORLD);

							if(print_detail)
								printf("\nTask %d **complete** by worker %d\n\n", receipt, worker);
						} else {
							localBuffer[0] = 0;
							MPI_Send(localBuffer, 3, MPI_INT, 0, tag, MPI_COMM_WORLD);
						}

						// Case 4: take the next unit load
						lockNum[0] = 4;
						MPI_Send(lockNum, 3, MPI_INT, 0, tag, MPI_COMM_WORLD);
						MPI_Recv(localBuffer, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
						tasks[receipt].left = localBuffer[0];
						if(tasks[receipt].left > 0) {
							tasks[receipt].left -= UNIT_LOAD;

							local_load = UNIT_LOAD;

							localBuffer[0] = 1;
							localBuffer[1] = tasks[receipt].left;
							MPI_Send(localBuffer, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);

							if(print_detail)
								printf("Worker %d receives %d works from task %d\n", worker, local_load, receipt);
						} else {
							localBuffer[0] = 0;
							MPI_Send(localBuffer, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);
						}
					}
				}

				if(local_load <= 0) {
					for(int select_task_id = 0; select_task_id < NUM_OF_TASKS; ++select_task_id) {
						int s = workers[worker].prefer_task[select_task_id];

						if(print_detail)
							printf("Worker %d want to choose tasks %d\n", worker, s);

						// Case 5: worker changes task and take unit load
						int lockNum[3] = {5, s, last_select_task[worker]};
						int localBuffer[4];
						MPI_Send(lockNum, 3, MPI_INT, 0, tag, MPI_COMM_WORLD);
						MPI_Recv(localBuffer, 4, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
						tasks[s].left = localBuffer[0];
						tasks[s].current_num = localBuffer[1];
						tasks[last_select_task[worker]].current_num = localBuffer[2];
						if(tasks[s].left > 0 && tasks[s].current_num < tasks[s].max) {
							tasks[s].current_num++;
							receipt = s;
							tasks[receipt].left -= UNIT_LOAD;
							local_load = UNIT_LOAD;

							if(receipt != -1) {
								tasks[last_select_task[worker]].current_num--;
							}

							localBuffer[0] = 1;
							localBuffer[1] = tasks[s].current_num;
							localBuffer[2] = tasks[receipt].left;
							localBuffer[3] = tasks[last_select_task[worker]].current_num;
							MPI_Send(localBuffer, 4, MPI_INT, 0, tag, MPI_COMM_WORLD);

							if(print_detail)
								printf("Worker %d receives task %d sucessfully and receives %d works from it\n", worker, receipt, local_load);

							select_task_id = NUM_OF_TASKS;
						} else {
							localBuffer[0] = 0;
							MPI_Send(localBuffer, 4, MPI_INT, 0, tag, MPI_COMM_WORLD);
						}
					}
				}

				last_select_task[worker] = receipt;
				last_local_load[worker] = local_load;
			}

            int check[3] = {6, -1, -1};
            MPI_Send(check, 3, MPI_INT, 0, tag, MPI_COMM_WORLD);
            MPI_Recv(&complete_tasks, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        }
        int Case[3] = {0, -1, -1};
        MPI_Send(Case, 3, MPI_INT, 0, tag, MPI_COMM_WORLD);
    } else if (world_rank == 0) {
        int finish = 0;
        int buffer[2], Buffer[3], temp[4];

        while (finish < world_size) {
            int lockNum[6];
            MPI_Status tmp;

            MPI_Recv(&lockNum, 3, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
            switch (lockNum[0]) {
                case 1:
                    world_time += TIMEPIECE;
                    MPI_Bcast(&world_time, 1, MPI_INT, 0, MPI_COMM_WORLD);
                    break;
				// The task has completed unit load
                case 2:
                    tasks[lockNum[1]].complete += UNIT_LOAD;
                    MPI_Send(&tasks[lockNum[1]].complete, 1, MPI_INT, status.MPI_SOURCE, tag, MPI_COMM_WORLD);
                    break;
				// Update task info
                case 3:
                    Buffer[0] = tasks[lockNum[1]].complete;
                    Buffer[1] = tasks[lockNum[1]].total_load;
                    Buffer[2] = tasks[lockNum[1]].end;
                    MPI_Send(Buffer, 3, MPI_INT, status.MPI_SOURCE, tag, MPI_COMM_WORLD);
                    MPI_Recv(Buffer, 3, MPI_INT, status.MPI_SOURCE, tag, MPI_COMM_WORLD, &tmp);
                    if (Buffer[0]) {
                        tasks[lockNum[1]].end = Buffer[1];
                        complete_tasks = Buffer[2];
                    }
                    break;
				// Take the next unit load
                case 4:
                    buffer[0] = tasks[lockNum[1]].left;
                    MPI_Send(buffer, 1, MPI_INT, status.MPI_SOURCE, tag, MPI_COMM_WORLD);
                    MPI_Recv(buffer, 2, MPI_INT, status.MPI_SOURCE, tag, MPI_COMM_WORLD, &tmp);
                    if (buffer[0]) {
                        tasks[lockNum[1]].left = buffer[1];
                    }
                    break;
				// Change worker's task and take unit load
                case 5:
                    temp[0] = tasks[lockNum[1]].left;
                    temp[1] = tasks[lockNum[1]].current_num;
                    temp[2] = tasks[lockNum[2]].current_num;
                    MPI_Send(temp, 4, MPI_INT, status.MPI_SOURCE, tag, MPI_COMM_WORLD);
                    MPI_Recv(temp, 4, MPI_INT, status.MPI_SOURCE, tag, MPI_COMM_WORLD, &tmp);
                    if (temp[0]) {
                        tasks[lockNum[1]].current_num = temp[1];
                        tasks[lockNum[1]].left = temp[2];
                        tasks[lockNum[2]].current_num = temp[3];
                    }
                    break;
                case 6:
                    MPI_Send(&complete_tasks, 1, MPI_INT, status.MPI_SOURCE, tag, MPI_COMM_WORLD);
                    break;
                case 0:
                    finish++;
                    break;
            }
        }
    }
    MPI_Finalize();
}
