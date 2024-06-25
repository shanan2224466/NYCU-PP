#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#define detail_of_parttime 1
#define usetimeidx 4
#define num_thread 4
#define ul 1000
#define timepiece 20
#define singleload 100000
#define Max_num 5
#define num_of_tasks 50
#define num_of_workers 60
#define task_efficient_range 10
#define task_efficient_default 1
int print_detail = 0;

struct Task_info{
    int idx;
    int Max;
    int current_num;
    int Total_load;
    int Complete;
	int Left;
};

struct Worker_info{
    int *prefer_task;
    int *task_efficient;
};

int main(){
    struct Task_info T[num_of_tasks];
    struct timeval TimeStart;
	struct timeval TimeEnd;

    for(int i = 0 ; i < num_of_tasks; ++i)
	{
        T[i].idx = i;
        T[i].Max = 1 + i;
        T[i].current_num = 0;
        T[i].Total_load = singleload;
        T[i].Complete = 0;
		T[i].Left = T[i].Total_load;
    }

    struct Worker_info W[num_of_workers];
    for(int i = 0 ; i < num_of_workers; ++i){
        W[i].prefer_task = (int*)malloc(num_of_tasks*sizeof(int));
        W[i].task_efficient = (int*)malloc(num_of_tasks*sizeof(int));
        
        for(int j = 0 ; j < num_of_tasks; ++j){
            W[i].prefer_task[j] = (i + 2 + j) % num_of_tasks;
            W[i].task_efficient[j] = task_efficient_default;
        }
    }

    int unit_load = ul;
    int world_time = 0;
    int complete_tasks = 0;
    int last_local_load[num_of_workers];
    int last_select_task[num_of_workers];

    for(int i = 0; i < num_of_workers; ++i){
        last_select_task[i] = -1;
        last_local_load[i] = -1;
    }

	struct timeval threadstart[8];
	struct timeval threadend[8];

	long int timecount[8];
	for(int i = 0 ; i < usetimeidx; ++i){
		timecount[i] = 0;
	}

	gettimeofday(&TimeStart, NULL);
	while(complete_tasks < num_of_tasks)
	{	
		if(detail_of_parttime ){
			gettimeofday(&threadstart[0],NULL);
		}

		world_time++;
		
		if(detail_of_parttime){
			gettimeofday(&threadend[0],NULL);
			timecount[0] += ((threadend[0].tv_sec - threadstart[0].tv_sec) * 1000000.0 + (threadend[0].tv_usec - threadstart[0].tv_usec));
		}

		for(int worker = 0 ; worker < num_of_workers ; ++worker)
		{
			int receipt = last_select_task[worker];
			int local_load = last_local_load[worker];
			
			if(print_detail)
				printf("Worker %d select task %d last time and remain %d works\n", worker, receipt, local_load);

			if(local_load > 0)
			{

				if(detail_of_parttime){
					gettimeofday(&threadstart[1],NULL);
				}

				local_load -=  W[worker].task_efficient[receipt];
				
				if(print_detail)
					printf("Worker %d can finish 100 * %d works in unit time, now remain %d works\n", worker, W[worker].task_efficient[receipt], local_load);


				if(local_load <= 0)
				{
					if(print_detail)
						printf("Worker %d finish his work\n", worker);

					T[receipt].Complete += unit_load;
					
					if(T[receipt].Complete >= T[receipt].Total_load)
					{
						complete_tasks++;
						if(print_detail)
							printf("Task %d complete by worker %d\n", receipt, worker);
					}

					if(T[receipt].Left > 0)
					{
						T[receipt].Left -= unit_load;
						local_load = unit_load;
						
						if(print_detail)
							printf("Worker %d receives %d works from task %d\n", worker, local_load, receipt);
					}
				}

				if(detail_of_parttime){
					gettimeofday(&threadend[1],NULL);
					timecount[1] += ((threadend[1].tv_sec - threadstart[1].tv_sec) * 1000000.0 + (threadend[1].tv_usec - threadstart[1].tv_usec));
				}
			}

			if(local_load <= 0)
			{

				if(detail_of_parttime){
					gettimeofday(&threadstart[2],NULL);
				}

				for(int select_task_id = 0; select_task_id < num_of_tasks; ++select_task_id)
				{
					int s = W[worker].prefer_task[select_task_id];
					
					if(T[s].Left > 0 && T[s].current_num < T[s].Max)
					{
						if (receipt != -1)
							T[last_select_task[worker]].current_num--;
						
						T[s].current_num++;
						receipt = s;
						
						if(print_detail)
							printf("Worker %d receives task %d sucessfully\n", worker, receipt);

						T[receipt].Left -= unit_load;
						local_load = unit_load;
						
						if(print_detail)
							printf("Worker %d receives %d works from task %d\n", worker, local_load, receipt);
						break;
					}
				}

				if(detail_of_parttime){
					gettimeofday(&threadend[2],NULL);
					timecount[2] += ((threadend[2].tv_sec - threadstart[2].tv_sec) * 1000000.0 + (threadend[2].tv_usec - threadstart[2].tv_usec));
				}
			}

			if(detail_of_parttime){
				gettimeofday(&threadstart[3], NULL);
			}
			
            last_select_task[worker] = receipt;
            last_local_load[worker] = local_load;

			if(detail_of_parttime){
				gettimeofday(&threadend[3],NULL);
				timecount[3] += ((threadend[3].tv_sec - threadstart[3].tv_sec) * 1000000.0 + (threadend[3].tv_usec - threadstart[3].tv_usec));
			}
		}

	}
	gettimeofday(&TimeEnd, NULL);

	printf("Info: num_of_tasks: %d   , num_of_workers: %d  , Task.load:  %d  \n", num_of_tasks, num_of_workers, singleload);
	printf("Virtual complete round is %d\n", world_time);
	
	long int elapse = (TimeEnd.tv_sec - TimeStart.tv_sec) * 1000000.0 + (TimeEnd.tv_usec - TimeStart.tv_usec);
	printf("Major process time = %ldms\n", elapse);

	if(detail_of_parttime){
		printf("serial world++ spend time:  part 0  %ld     detail:  ", timecount[0]);
		for(int i = 1 ;i <usetimeidx; ++i){
			printf("part %d spend time %ld,  ", i ,timecount[i]);
		}
	}
	printf("\n");

	for(int i = 0 ; i < num_of_workers; ++i){
        free(W[i].prefer_task);
        free(W[i].task_efficient);
    }
	return 0;
}
