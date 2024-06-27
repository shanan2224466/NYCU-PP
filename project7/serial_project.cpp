#include "def.h"

void serial() {
    while (complete_tasks < NUM_OF_TASKS) {
        world_time++;
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

                    tasks[receipt].complete += UNIT_LOAD;

                    if (tasks[receipt].complete >= tasks[receipt].total_load) {
                        complete_tasks++;

                        if (print_detail)
                            cout << "Task " << receipt << " complete by worker " << worker << "\n";
                    }

                    if (tasks[receipt].left > 0) {
                        tasks[receipt].left -= UNIT_LOAD;
                        local_load = UNIT_LOAD;

                        if (print_detail)
                            cout << "Worker " << worker << " receives " << local_load << " works from task " << receipt << "\n";
                    }
                }
            }

            if (local_load <= 0) {
                for (int select_task_id = 0; select_task_id < NUM_OF_TASKS; ++select_task_id) {
                    int s = workers[worker].prefer_task[select_task_id];

                    if (tasks[s].left > 0 && tasks[s].current_num < tasks[s].max) {
                        if (receipt != -1)
                            tasks[receipt].current_num--;

                        tasks[s].current_num++;
                        receipt = s;

                        if (print_detail)
                            cout << "Worker " << worker << " receives task " << receipt << " successfully\n";

                        tasks[receipt].left -= UNIT_LOAD;
                        local_load = UNIT_LOAD;

                        if (print_detail)
                            cout << "Worker " << worker << " receives " << local_load << " works from task " << receipt << "\n";

                        break;
                    }
                }
            }

            last_select_task[worker] = receipt;
            last_local_load[worker] = local_load;
        }
    }
}
