#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <omp.h>

#define detail_of_parttime 1
#define usetimeidx 5
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
	int checkpoint;
	int End;
};

struct Worker_info{
    int prefer_task[num_of_tasks];
    int task_efficient[num_of_tasks];
};

int main(){
	omp_lock_t task_lock[num_of_tasks][16];
    struct Task_info T[num_of_tasks];
    struct Worker_info W[num_of_workers];
    struct timeval TimeStart;
	struct timeval TimeEnd;
	struct timeval threadstart[num_thread][4];
	struct timeval threadend[num_thread][4];

    long int timecount[num_thread][16];
    int local_time[num_thread][16];
    for(int idx = 0; idx < num_thread ; ++idx){
        local_time[idx][0] = 0;
        for(int j = 0 ; j < usetimeidx;++j)
            timecount[idx][j] = 0;
    }

    for(int i = 0 ; i < num_of_tasks; ++i)
	{
        T[i].idx = i;
        T[i].Max = Max_num;
        T[i].current_num = 0;
        T[i].Total_load = singleload;
        T[i].Complete = 0;
		T[i].Left = T[i].Total_load;
		T[i].checkpoint = 0;
		T[i].End = 0;
        for(int key = 0 ; key < 8 ; ++key){
            omp_init_lock(&task_lock[i][key]);
        }
    }

	for(int i = 0 ; i < num_of_workers; ++i){      
        for(int j = 0 ; j < num_of_tasks; ++j){
            W[i].prefer_task[j] = (i + 2 + j) % num_of_tasks;
            W[i].task_efficient[j] = task_efficient_default;
        }
    }
    int unit_load = ul;
    int world_time = 0;
    int complete_tasks = 0;
    int last_local_load[num_of_workers][16];
    int last_select_task[num_of_workers][16];

    for(int i = 0; i < num_of_workers; ++i){
        last_select_task[i][0] = -1;
        last_local_load[i][0] = -1;
    }
    
    gettimeofday(&TimeStart, NULL);

    omp_set_num_threads(num_thread);
    #pragma omp parallel
    {
        int count = 0;
        int tid = omp_get_thread_num();
        while(complete_tasks < num_of_tasks)
        {
            if(detail_of_parttime){
                gettimeofday(&threadstart[tid][0], NULL);
		    }

            if(local_time[tid][0] >= world_time + timepiece){
                if( ((world_time + timepiece) % (num_thread * timepiece)) / timepiece == tid) {
                    world_time += timepiece;
                }

                if(detail_of_parttime){
                    gettimeofday(&threadend[tid][0], NULL);
                    timecount[tid][0] += ((threadend[tid][0].tv_sec - threadstart[tid][0].tv_sec) * 1000000.0 + (threadend[tid][0].tv_usec - threadstart[tid][0].tv_usec));
                    
                    count++;
                }
            }
            
            else{               
			    local_time[tid][0]++;	
                			
                if(detail_of_parttime){
                    gettimeofday(&threadend[tid][0], NULL);
                    timecount[tid][0] += ((threadend[tid][0].tv_sec - threadstart[tid][0].tv_sec) * 1000000.0 + (threadend[tid][0].tv_usec - threadstart[tid][0].tv_usec));
                    count++;
                }

                #pragma omp for
                for(int worker = 0 ; worker < num_of_workers ; ++worker)
                {
                    int receipt = last_select_task[worker][0];
                    int local_load = last_local_load[worker][0];
                    
                    if(print_detail)
                        printf("Worker %d select task %d last time and remain %d works\n", worker, receipt, local_load);

                    if(local_load > 0)
                    {
                        if(detail_of_parttime){
							gettimeofday(&threadstart[tid][1], NULL);
						}

                        local_load -= W[worker].task_efficient[receipt];
                        
                        if(print_detail)
                            printf("Worker %d can finish 100 * %d works in unit time, now remain %d works\n", worker, W[worker].task_efficient[receipt], local_load);
                        
                        if(local_load <= 0)
                        {
                            if(print_detail)
                                printf("Worker %d finish his work\n", worker);
                            
                            local_load = 0;
                            __sync_fetch_and_add(&T[receipt].Complete, unit_load);

                            omp_set_lock(&task_lock[receipt][0]);
                            if( (T[receipt].Complete >= T[receipt].Total_load) && (T[receipt].End == 0))
                            {
                                T[receipt].End = 1;
                                __sync_add_and_fetch(&complete_tasks, 1);
                                if(print_detail)
                                    printf("\nTask %d **complete** by worker %d\n\n", receipt, worker);
                            }
                            omp_unset_lock(&task_lock[receipt][0]);


                            omp_set_lock(&task_lock[receipt][1]);
                            if(T[receipt].Left > 0)
                            {
                                T[receipt].Left -= unit_load;
                                local_load = unit_load;
                                
                                if(print_detail)
                                    printf("Worker %d receives %d works from task %d\n", worker, local_load, receipt);
                            }
                            omp_unset_lock(&task_lock[receipt][1]);
                        }
                    
						if(detail_of_parttime){
							gettimeofday(&threadend[tid][1], NULL);
							timecount[tid][1] += ((threadend[tid][1].tv_sec - threadstart[tid][1].tv_sec) * 1000000.0 + (threadend[tid][1].tv_usec - threadstart[tid][1].tv_usec));
						}                    
                    }

                    if(local_load <= 0)
                    {
						if(detail_of_parttime)
						{			
							gettimeofday(&threadstart[tid][2], NULL);
						}
                        int select_task_id;
                        for(select_task_id = 0; select_task_id < num_of_tasks; ++select_task_id)
                        {
                            int s = W[worker].prefer_task[select_task_id];

                            if(print_detail)
                                printf("Worker %d want to choose tasks %d\n", worker, s);
                            
                            omp_set_lock(&task_lock[s][1]);
                            if(T[s].Left > 0 && T[s].current_num < T[s].Max)
                            {
                                if (receipt != -1)
                                {
                                    T[last_select_task[worker][0]].current_num--;
                                }

                                T[s].current_num++;
                                receipt = s;
                                T[receipt].Left -= unit_load;
                                local_load = unit_load;
                                
                                if(print_detail)
                                    printf("Worker %d receives task %d sucessfully and receives %d works from it\n", worker, receipt, local_load);
                                select_task_id = num_of_tasks;
                            }
                            omp_unset_lock(&task_lock[s][1]);
                        }
 						if(detail_of_parttime)
						{
							gettimeofday(&threadend[tid][2], NULL);
							timecount[tid][2] += ((threadend[tid][2].tv_sec - threadstart[tid][2].tv_sec) * 1000000.0 + (threadend[tid][2].tv_usec - threadstart[tid][2].tv_usec));
						}                   
                    }
                    if(detail_of_parttime)
                    {	
                        gettimeofday(&threadstart[tid][3], NULL);
                    }
                    last_select_task[worker][0] = receipt;
                    last_local_load[worker][0] = local_load;

                    if(detail_of_parttime)
                    {
                        gettimeofday(&threadend[tid][3], NULL);
                        timecount[tid][3] += ((threadend[tid][3].tv_sec - threadstart[tid][3].tv_sec) * 1000000.0 + (threadend[tid][3].tv_usec - threadstart[tid][3].tv_usec));
                    }
                }
            }
        }
        printf("===%d, %d===\n", tid, count);
	}
    gettimeofday(&TimeEnd, NULL);
    
    printf("Info: num_of_tasks: %d   , num_of_workers: %d  , Task.load:  %d  \n", num_of_tasks, num_of_workers, singleload);
	printf("Virtual complete round is %d\n", world_time);
	
	long int elapse = (TimeEnd.tv_sec - TimeStart.tv_sec) * 1000000.0 + (TimeEnd.tv_usec - TimeStart.tv_usec);
	printf("Major process time = %ldms\n", elapse);

	if(detail_of_parttime ){
		for(int idx = 0 ; idx < num_thread; ++idx){
			printf("thread %d in world_time sync spend  part 0  %ld       detail:", idx, timecount[idx][0]);
			if(detail_of_parttime){
				for(int j = 1 ; j < usetimeidx; ++j){
					printf("   part %d spend time %ld", j, timecount[idx][j]);
				}
			}
			printf("\n");		
		}
	}
    
    for(int i = 0 ; i < num_of_tasks; ++i)
	{
        for(int key = 0 ; key < 8 ; ++key){
            omp_destroy_lock(&task_lock[i][key]);
        }
    }

	return 0;
}


