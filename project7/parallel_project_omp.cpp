#include "def.h"

void omp() {
    while (complete_tasks < NUM_OF_TASKS) {

        #pragma omp for
        for (int worker = 0; worker < NUM_OF_WORKERS; ++worker) {
            int receipt = last_select_task[worker];
            int local_load = last_local_load[worker];

            if (print_detail)
                cout << "Worker " << worker << " select task " << receipt << " last time and remain " << local_load << " works\n";

            if (local_load > 0) {
                local_load -= workers[worker].task_efficient[receipt];

                if (print_detail)
                    cout << "Worker " << worker << " can finish 100 * " << workers[worker].task_efficient[receipt] << " works in unit time, now remain " << local_load << " works\n";

                if (local_load <= 0) {
                    if (print_detail)
                        cout << "Worker " << worker << " finish his work\n";

                    local_load = 0;

                    __sync_fetch_and_add(&tasks[receipt].complete, UNIT_LOAD);

                    omp_set_lock(&omp_lock[receipt][0]);
                    if ((tasks[receipt].complete >= tasks[receipt].total_load) && (tasks[receipt].end == 0)) {
                        tasks[receipt].end = 1;
                        __sync_add_and_fetch(&complete_tasks, 1);

                        if (print_detail)
                            cout << "Task " << receipt << " complete by worker " << worker << "\n";
                    }
                    omp_unset_lock(&omp_lock[receipt][0]);

                    omp_set_lock(&omp_lock[receipt][1]);
                    if (tasks[receipt].left > 0) {
                        tasks[receipt].left -= UNIT_LOAD;
                        local_load = UNIT_LOAD;

                        if (print_detail)
                            cout << "Worker " << worker << " receives " << local_load << " works from task " << receipt << "\n";
                    }
                    omp_unset_lock(&omp_lock[receipt][1]);
                }
            }

            if (local_load <= 0) {
                for (int select_task_id = 0; select_task_id < NUM_OF_TASKS; ++select_task_id) {
                    int s = workers[worker].prefer_task[select_task_id];

                    omp_set_lock(&omp_lock[s][1]);
                    if (tasks[s].left > 0 && tasks[s].current_num < tasks[s].max) {
                        if (receipt != -1)
                            tasks[last_select_task[worker]].current_num--;

                        tasks[s].current_num++;
                        receipt = s;

                        if (print_detail)
                            cout << "Worker " << worker << " receives task " << receipt << " successfully\n";

                        tasks[receipt].left -= UNIT_LOAD;
                        local_load = UNIT_LOAD;
                        omp_unset_lock(&omp_lock[s][1]);

                        if (print_detail)
                            cout << "Worker " << worker << " receives " << local_load << " works from task " << receipt << "\n";

                        break;
                    }
                    omp_unset_lock(&omp_lock[s][1]);
                }
            }

            last_select_task[worker] = receipt;
            last_local_load[worker] = local_load;
        }
    }
}
