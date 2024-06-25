#ifndef FINAL_PROJECT_H_
#define FINAL_PROJECT_H_

#include <iostream>
#include <vector>
#include <sys/time.h>
#include <unistd.h>
#include <iomanip>
#include <omp.h>
#include <pthread.h>

// Constants Experssion
constexpr int DETAIL_OF_PARTTIME = 1;
constexpr int USETIMEIDX = 4;
constexpr int NUM_THREAD = 4;
constexpr int UL = 100;
constexpr int TIMEPIECE = 20;
constexpr int SINGLELOAD = 100000;
constexpr int MAX_NUM = 5;
constexpr int NUM_OF_TASKS = 50;
constexpr int NUM_OF_WORKERS = 60;
constexpr int TASK_EFFICIENT_RANGE = 10;
constexpr int TASK_EFFICIENT_DEFAULT = 1;

using namespace std;

// Struct Definitions
struct TaskInfo {
    int idx;
    int max;
    int current_num;
    int total_load;
    int complete;
    int left;
    int checkpoint;
    int end;
};

struct WorkerInfo {
    vector<int> prefer_task;
    vector<int> task_efficient;
};

// Global Variables
extern bool print_detail;
extern int unit_load;
extern int world_time;
extern int complete_tasks;

extern omp_lock_t omp_lock[NUM_OF_TASKS][2];
extern pthread_mutex_t pth_lock[NUM_OF_TASKS][2];

extern vector<TaskInfo> tasks;
extern vector<WorkerInfo> workers;
extern vector<int> last_local_load;
extern vector<int> last_select_task;

void  serial(void);
void  omp(void);
void* pth(void* threadId);

#endif