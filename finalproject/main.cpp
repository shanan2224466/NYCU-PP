#include "def.h"

// Global Variables
bool print_detail = false;
int unit_load = UL;
int world_time = 0;
int complete_tasks = 0;

vector<TaskInfo> tasks(NUM_OF_TASKS);
vector<WorkerInfo> workers(NUM_OF_WORKERS);
vector<int> last_local_load(NUM_OF_WORKERS, -1);
vector<int> last_select_task(NUM_OF_WORKERS, -1);

timeval TimeStart, TimeEnd;
omp_lock_t omp_lock[NUM_OF_TASKS][2];
pthread_mutex_t pth_lock[NUM_OF_TASKS][2];

void initial() {
    for (int i = 0; i < NUM_OF_TASKS; ++i) {
        for (int j = 0; j < 2; ++j) {
            omp_init_lock(&omp_lock[i][j]);
            pthread_mutex_init(&pth_lock[i][j], nullptr);
        }
    }
}

void initializeTasks() {
    for (int i = 0; i < NUM_OF_TASKS; ++i) {
        tasks[i] = {i, MAX_NUM, 0, SINGLELOAD, 0, SINGLELOAD, 0, 0};
    }
}

void initializeWorkers() {
    for (int i = 0; i < NUM_OF_WORKERS; ++i) {
        workers[i].prefer_task.resize(NUM_OF_TASKS);
        workers[i].task_efficient.resize(NUM_OF_TASKS);

        for (int j = 0; j < NUM_OF_TASKS; ++j) {
            workers[i].prefer_task[j] = (i + 2 + j) % NUM_OF_TASKS;
            workers[i].task_efficient[j] = TASK_EFFICIENT_DEFAULT;
        }
    }
}

void resetGlobals() {
    world_time = 0;
    complete_tasks = 0;
    last_local_load.assign(NUM_OF_WORKERS, -1);
    last_select_task.assign(NUM_OF_WORKERS, -1);
}

void printinfo(const char *test) {
	long int elapsed_time = (TimeEnd.tv_sec - TimeStart.tv_sec) * 1000000 + (TimeEnd.tv_usec - TimeStart.tv_usec);
	cout << "*************** " << test << " ******************" << endl;
	cout << "Virtual complete round = " << world_time << "\n";
    cout << "Major process time = " << elapsed_time << "ms\n";
}

void destory() {
    for (int i = 0; i < NUM_OF_TASKS; ++i) {
        for (int j = 0; j < 2; ++j) {
            omp_destroy_lock(&omp_lock[i][j]);
            pthread_mutex_destroy(&pth_lock[i][j]);
        }
    }
}

void freeResources() {
    for (auto& worker : workers) {
        worker.prefer_task.clear();
        worker.task_efficient.clear();
    }
}

int main() {
    initial();
    initializeTasks();
    initializeWorkers();

    gettimeofday(&TimeStart, NULL);
    serial();
    gettimeofday(&TimeEnd, NULL);

	printinfo("serial");
    resetGlobals();
    initializeTasks();

    gettimeofday(&TimeStart, NULL);
    omp();
    gettimeofday(&TimeEnd, NULL);

	printinfo("omp");
    resetGlobals();
    initializeTasks();

    gettimeofday(&TimeStart, NULL);
    int threadData[NUM_THREAD];
	pthread_t threadId[NUM_THREAD];
    for (int i = 0; i < NUM_THREAD; ++i) {
        threadData[i] = i;
        pthread_create(&threadId[i], NULL, pth, (void*)&threadData[i]);
    }
    for (int i = 0; i < NUM_THREAD; ++i) {
        pthread_join(threadId[i], NULL);
    }
    gettimeofday(&TimeEnd, NULL);

	printinfo("pthread");
    destory();
    freeResources();

    return 0;
}
