#include "def.h"

void* pth(void* threadId) {
    int tid = *(int*)threadId;

    while (complete_tasks < NUM_OF_TASKS) {
        for (int worker = tid * (NUM_OF_WORKERS / NUM_THREAD); worker < (tid + 1) * (NUM_OF_WORKERS / NUM_THREAD); ++worker) {
            int receipt = last_select_task[worker];
            int local_load = last_local_load[worker];

            if (local_load > 0) {
                local_load -= workers[worker].task_efficient[receipt];

                if (local_load <= 0) {
                    local_load = 0;
                    __sync_fetch_and_add(&tasks[receipt].complete, unit_load);

                    pthread_mutex_lock(&pth_lock[receipt][0]);
                    if ((tasks[receipt].complete >= tasks[receipt].total_load) && (tasks[receipt].end == 0)) {
                        tasks[receipt].end = 1;
                        complete_tasks++;
                    }
                    pthread_mutex_unlock(&pth_lock[receipt][0]);

                    pthread_mutex_lock(&pth_lock[receipt][1]);
                    if (tasks[receipt].left > 0) {
                        tasks[receipt].left -= unit_load;
                        local_load = unit_load;
                    }
                    pthread_mutex_unlock(&pth_lock[receipt][1]);
                }
            }

            if (local_load <= 0) {
                for (int select_task_id = 0; select_task_id < NUM_OF_TASKS; ++select_task_id) {
                    int s = workers[worker].prefer_task[select_task_id];

                    pthread_mutex_lock(&pth_lock[s][1]);
                    if (tasks[s].left > 0 && tasks[s].current_num < tasks[s].max) {
                        if (receipt != -1) {
                            tasks[last_select_task[worker]].current_num--;
                        }

                        tasks[s].current_num++;
                        receipt = s;
                        tasks[receipt].left -= unit_load;
                        local_load = unit_load;
                        pthread_mutex_unlock(&pth_lock[s][1]);
                        break;
                    }
                    pthread_mutex_unlock(&pth_lock[s][1]);
                }
            }

            last_select_task[worker] = receipt;
            last_local_load[worker] = local_load;
        }
    }

    return NULL;
}
