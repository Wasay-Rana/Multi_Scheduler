/*
This code is done by Manahil and Wasay, as a Semester Project of Operating System Course, All CopyRights reserved...
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include <sys/time.h>

#define MOD_CONSTANT 10000
#define UNIT_COUNT 5
#define TASK_QUEUE_SIZE 100

enum SchedulingAlgorithm {
    ROUND_ROBIN,
    FCFS,
    PRIORITY,   
    RANDOM
};

struct Task {
    int id;
    int value;
    int unit_count;
    int *unit_operations;
    int initial_value;
    int processing_time;
    struct timespec arrival_time;
    pthread_t thread_id;
};

struct ExecutionUnit {
    int id;
    pthread_t thread;
    pthread_mutex_t queue_mutex;
    struct Task *task_queue[TASK_QUEUE_SIZE];
    int front, rear;
    enum SchedulingAlgorithm algorithm; // Added field for scheduling algorithm
};

long clock_gettime_diff(struct timespec *start_time) {
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    return (end_time.tv_sec - start_time->tv_sec) * 1000000 +
           (end_time.tv_nsec - start_time->tv_nsec) / 1000;
}

void initialize_execution_units(enum SchedulingAlgorithm algorithm);
void read_and_enqueue_tasks(FILE *input_file, enum SchedulingAlgorithm algorithm);
struct Task *create_task(FILE *input_file, pthread_t thread_id);
void join_execution_units_threads();
void *unit_execution_function(void *arg);
void process_task(struct Task *task);
void print_task(struct Task *task, int descriptive, long elapsed);
void enqueue_task(struct ExecutionUnit *unit, struct Task *task);
struct Task *dequeue_task(struct ExecutionUnit *unit);
int calculate_power_mod_m(int value);
const char *unit_operation_description(int operation_id);

struct ExecutionUnit execution_units[UNIT_COUNT];

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;  // Global mutex to print the correct thread-safe

void initialize_execution_units(enum SchedulingAlgorithm algorithm) {
/*
initialize_execution_units:
    INPUT: None
    WHAT DOES IT DO: Initializes the execution units by setting their IDs, queue parameters, 
    and creating threads for each execution unit using pthread_create.
    OUTPUT: None
*/
    for (int i = 0; i < UNIT_COUNT; ++i) {
        execution_units[i].id = i;
        execution_units[i].front = execution_units[i].rear = 0;
        pthread_mutex_init(&execution_units[i].queue_mutex, NULL);
        pthread_create(&execution_units[i].thread, NULL, unit_execution_function, (void *)&execution_units[i]);
        execution_units[i].algorithm = algorithm; // Set scheduling algorithm
    }
}

void read_and_enqueue_tasks(FILE *input_file, enum SchedulingAlgorithm algorithm) {
/*
read_and_enqueue_tasks:
    INPUT: Pointer to a file (input_file)
    WHAT DOES IT DO: Reads tasks from the input file and enqueues them to the execution units in a round-robin.
    OUTPUT: None
*/
    unsigned int seed = (unsigned int)time(NULL);  // Generate a unique seed for each run
    int unit_index = 0;
    while (!feof(input_file)) {
        struct Task *task = create_task(input_file, execution_units[unit_index].thread);
        if (task != NULL) {
            enqueue_task(&execution_units[unit_index], task);
            switch (algorithm) {
                case ROUND_ROBIN:
                case FCFS:
                    unit_index = (unit_index + 1) % UNIT_COUNT;
                    break;
                case PRIORITY:
                    // Implement priority-based logic here if needed
                    break;
                case RANDOM:
                    unit_index = rand() % UNIT_COUNT;  // Randomly select an execution unit
                    break;
                default:
                    break;
            }
        }
    }
}
struct Task *create_task(FILE *input_file, pthread_t thread_id) {
/*
create_task:
    INPUT: Pointer to a file (input_file)
    WHAT DOES IT DO: Reads task information from the input file, allocates memory for a new task, and initializes its parameters.
    OUTPUT: Returns a pointer to the created task or NULL if an error occurs
*/
    struct Task *task = (struct Task *)malloc(sizeof(struct Task));
    if (!task) {
        perror("Error allocating memory for task");
        exit(EXIT_FAILURE);
    }

    if (fscanf(input_file, "%d %d %d", &task->id, &task->value, &task->unit_count) != 3) {
        free(task);
        return NULL;
    }

    task->unit_operations = (int *)malloc(task->unit_count * sizeof(int));
    if (!task->unit_operations) {
        perror("Error allocating memory for unit operations");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < task->unit_count; ++i) {
        fscanf(input_file, "%d", &task->unit_operations[i]);
    }

    clock_gettime(CLOCK_MONOTONIC, &task->arrival_time);
    task->initial_value = task->value;
    task->processing_time = 0;
    task->thread_id = thread_id;  // Assign the thread ID here

    return task;
}


void join_execution_units_threads() {
/*
join_execution_units_threads:
    INPUT: None
    WHAT DOES IT DO: Waits for all execution unit threads to finish using pthread_join.
    OUTPUT: None
*/
    for (int i = 0; i < UNIT_COUNT; ++i) {
        pthread_join(execution_units[i].thread, NULL);
    }
}

void *unit_execution_function(void *arg) {
/*unit_execution_function:
    INPUT: Pointer to an ExecutionUnit structure
    WHAT DOES IT DO: Continuously dequeues tasks from the unit's task queue, assigns the thread ID, and processes the tasks using process_task.
    OUTPUT: None
*/
    struct ExecutionUnit *unit = (struct ExecutionUnit *)arg;

    while (true) {
        struct Task *task = dequeue_task(unit);

        if (task != NULL) {
            printf("Unit %d (Thread %ld) processing Task %d\n", unit->id, (long)unit->thread, task->id);
            // Add more print statements as needed
            struct timespec start_time;
            clock_gettime(CLOCK_MONOTONIC, &start_time);
            
            process_task(task);

            struct timespec end_time;
            clock_gettime(CLOCK_MONOTONIC, &end_time);
            long elapsed = (end_time.tv_sec - start_time.tv_sec) * 1000000 +
                           (end_time.tv_nsec - start_time.tv_nsec) / 1000;
            printf("Unit %d (Thread %ld) finished processing Task %d in %ld us\n", unit->id, (long)unit->thread, task->id, elapsed);
            
            free(task);
        }
    }
    return NULL;
}


void process_task(struct Task *task) {
/*
process_task:
    INPUT: Pointer to a Task structure
    WHAT DOES IT DO: Processes the operations specified in the task, calculates elapsed time, and updates task information.
    OUTPUT: None
*/
    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    long elapsed = 0;

    for (int i = 0; i < task->unit_count; ++i) {
        switch (task->unit_operations[i]) {
        case 0:
            task->value = (task->value + 7) % MOD_CONSTANT;
            printf("Task %d (Thread %ld): Operation +7, Result: %d\n", task->id, (long)task->thread_id, task->value);
            break;
        case 1:
            task->value = (task->value * 2) % MOD_CONSTANT;
            printf("Task %d (Thread %ld): Operation *2, Result: %d\n", task->id, (long)task->thread_id, task->value);
            break;
        case 2:
            task->value = calculate_power_mod_m(task->value);
            printf("Task %d (Thread %ld): Operation ^5, Result: %d\n", task->id, (long)task->thread_id, task->value);
            break;
        case 3:
            task->value = 19 - task->value;
            printf("Task %d (Thread %ld): Operation -19, Result: %d\n", task->id, (long)task->thread_id, task->value);
            break;
        case 4:
            elapsed = (clock_gettime_diff(&start_time) / 1000);
            pthread_mutex_lock(&print_mutex);  // Lock before printing
            print_task(task, 1, elapsed);
            pthread_mutex_unlock(&print_mutex);  // Unlock after printing
            break;
        default:
            break;
        }
    }

    if (elapsed == 0) {
        elapsed = (clock_gettime_diff(&start_time) / 1000);
        task->processing_time += elapsed;
    }

    usleep(500000);
}

void print_task(struct Task *task, int descriptive, long elapsed) {
/*

print_task:
    INPUT: Pointer to a Task structure, descriptive flag, elapsed time
    WHAT DOES IT DO: Prints information about a task, including arrival time, total time in the system, current and original values, and task operations.
    OUTPUT: None
*/
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    long total_time_in_system = (current_time.tv_sec - task->arrival_time.tv_sec) * 1000000 +
                                (current_time.tv_nsec - task->arrival_time.tv_nsec) / 1000;

    if (descriptive) {
        printf("\033[1;35m");
        printf("Task %d (Thread %ld): Arrival Time: %ld us, Total Time in System: %ld us, Current Value: %d, Original Value: %d\n",
               task->id, (long)task->thread_id, task->arrival_time.tv_sec * 1000000 + task->arrival_time.tv_nsec / 1000,
               total_time_in_system, task->value, task->initial_value);
        printf("\033[1;33m");
        printf("Operations taken: ");
        for (int i = 0; i < task->unit_count; ++i) {
            printf("%s, ", unit_operation_description(task->unit_operations[i]));
        }
        printf("\n");
    } else {
        printf("Task %d (Thread %ld): Arrival Time: %ld us, Total Time in System: %ld us, Current Value: %d, Original Value: %d\n",
               task->id, (long)task->thread_id, task->arrival_time.tv_sec * 1000000 + task->arrival_time.tv_nsec / 1000,
               total_time_in_system, task->value, task->initial_value);
    }
}

const char *unit_operation_description(int operation_id) {
    switch (operation_id) {
        case 0:
            return "+7";
        case 1:
            return "*2";
        case 2:
            return "^5";
        case 3:
            return "-19";
        case 4:
            return "print";
        default:
            return "unknown";
    }
}

void enqueue_task(struct ExecutionUnit *unit, struct Task *task) {
/*
enqueue_task:
    INPUT: Pointer to an ExecutionUnit structure, pointer to a Task structure
    WHAT DOES IT DO: Enqueues a task in the task queue of the specified execution unit.
    OUTPUT: None
*/
    pthread_mutex_lock(&unit->queue_mutex);

    if ((unit->rear + 1) % TASK_QUEUE_SIZE == unit->front) {
        fprintf(stderr, "Error: Task queue for Unit %d is full.\n", unit->id);
        exit(EXIT_FAILURE);
    }

    unit->task_queue[unit->rear] = task;
    unit->rear = (unit->rear + 1) % TASK_QUEUE_SIZE;

    pthread_mutex_unlock(&unit->queue_mutex);
}


struct Task *dequeue_task(struct ExecutionUnit *unit) {
/*
dequeue_task:
    INPUT: Pointer to an ExecutionUnit structure
    WHAT DOES IT DO: Dequeues a task from the task queue of the specified execution unit.
    OUTPUT: Returns a pointer to the dequeued task or NULL if the queue is empty.
*/
    pthread_mutex_lock(&unit->queue_mutex);

    if (unit->front == unit->rear) {
        pthread_mutex_unlock(&unit->queue_mutex);
        return NULL;
    }

    struct Task *task = unit->task_queue[unit->front];
    unit->front = (unit->front + 1) % TASK_QUEUE_SIZE;

    pthread_mutex_unlock(&unit->queue_mutex);

    return task;
}


int calculate_power_mod_m(int value) {
    int result = 1;
    for (int i = 0; i < 5; ++i) {
        result = (result * value) % MOD_CONSTANT;
    }
    return result;
}
/*
main:
Reads the input file name, opens the file, initializes execution units from struct, reads and enqueues tasks, 
closes the input file, and joins execution unit threads.
*/
int main() {
    FILE *input_file;
    char file_name[256];
    printf("Enter the input file name: ");
    scanf("%s", file_name);
    input_file = fopen(file_name, "r");
    if (!input_file) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }
    int algorithm_choice;
    printf("Select scheduling algorithm (0: Round Robin, 1: FCFS, 2: Priority, 3: Random): ");
    scanf("%d", &algorithm_choice);

    enum SchedulingAlgorithm selected_algorithm;

    switch (algorithm_choice) {
        case 0:
            selected_algorithm = ROUND_ROBIN;
            break;
        case 1:
            selected_algorithm = FCFS;
            break;
        case 2:
            selected_algorithm = PRIORITY;
            break;
        case 3:
            selected_algorithm = RANDOM;
            break;  
        default:
            fprintf(stderr, "Invalid algorithm choice. Defaulting to Round Robin.\n");
            selected_algorithm = ROUND_ROBIN;
    }
    initialize_execution_units(selected_algorithm);
    read_and_enqueue_tasks(input_file, selected_algorithm);
    fclose(input_file);
    join_execution_units_threads();

    return 0;
}

/* This is some insights which we got from our Results...
Round Robin (Algorithm 0):

    Task Execution:
        Task 0 arrives first, processed by Unit 0.
        Task 1 arrives next, processed by Unit 1.
        Task 2 arrives last, processed by Unit 2.

    Task Operations:
        Operations are executed in a round-robin fashion among the units.
        Each unit performs its designated operation on the task's value.

    Time in System:
        The "Total Time in System" for each task indicates the time spent in the system until reaching the print unit.

FCFS (Algorithm 1):

    Task Execution:
        Tasks are executed in the order of their arrival.

    Task Operations:
        Similar to Round Robin, each unit performs its operation on the task's value.

    Time in System:
        The "Total Time in System" for each task indicates the time spent in the system until reaching the print unit.

Priority (Algorithm 2):

    Task Execution:
        Tasks are executed based on a priority scheme.
        The priority scheme is not explicitly mentioned in the provided output, but the algorithm assigns priorities to tasks.

    Task Operations:
        Similar to Round Robin, each unit performs its operation on the task's value.

    Time in System:
        The "Total Time in System" for each task indicates the time spent in the system until reaching the print unit.

Random (Algorithm 3):

    Task Execution:
        Tasks are executed in a random order.

    Task Operations:
        Similar to Round Robin, each unit performs its operation on the task's value.

    Time in System:
        The "Total Time in System" for each task indicates the time spent in the system until reaching the print unit.

General Observations:

    Concurrency:
        Multiple threads (units) are working concurrently on different tasks.

    Task Queuing:
        Tasks are queued based on the receptor's order of reading from the input file.

    Task Structure:
        The task structure includes an ID, value, arrival time, number of units, and a list of unit IDs.

    Print Unit:
        The print unit displays the time stamp, task ID, task value, and value after operations.

    Modular Division:
        Units use modular division to avoid value overflow.

Overall, the program simulates a complex system with multiple processing units and provides flexibility in terms of scheduling algorithms,
task execution order, and concurrent processing...
*/