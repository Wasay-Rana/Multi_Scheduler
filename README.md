# Task Scheduling and Execution System

## Project Overview

This project, created by Manahil and Wasay as a semester project for the Operating Systems course, implements a task scheduling and execution system using multiple scheduling algorithms. The system is designed to simulate real-world scenarios where tasks are processed by multiple execution units (threads), showcasing the behavior of different scheduling algorithms.

![alt text](/images/dispatcher_workflow.png)
## Key Features

- **Multiple Scheduling Algorithms**: Supports Round Robin, First-Come-First-Serve (FCFS), Priority, and Random scheduling algorithms.
- **Concurrent Processing**: Utilizes multi-threading to simulate concurrent task execution.
- **Detailed Task Operations**: Each task undergoes a series of operations, and their progress is tracked and printed.
- **Task Queuing and Management**: Tasks are read from an input file and managed using a queue structure for each execution unit.
- **Thread-Safe Logging**: Ensures accurate and synchronized logging of task operations and statuses.

## Code Overview

### Main Components

1. **Structures**:
   - `Task`: Represents a task with an ID, value, operations, arrival time, and more.
   - `ExecutionUnit`: Represents an execution unit (thread) that processes tasks.

2. **Functions**:
   - `initialize_execution_units(enum SchedulingAlgorithm algorithm)`: Initializes and creates threads for execution units.
   - `read_and_enqueue_tasks(FILE *input_file, enum SchedulingAlgorithm algorithm)`: Reads tasks from an input file and enqueues them based on the selected scheduling algorithm.
   - `create_task(FILE *input_file, pthread_t thread_id)`: Creates and initializes a task from the input file data.
   - `join_execution_units_threads()`: Waits for all execution unit threads to complete.
   - `unit_execution_function(void *arg)`: Main execution loop for each unit, dequeuing and processing tasks.
   - `process_task(struct Task *task)`: Processes a task's operations and tracks elapsed time.
   - `print_task(struct Task *task, int descriptive, long elapsed)`: Prints detailed information about a task.
   - `enqueue_task(struct ExecutionUnit *unit, struct Task *task)`: Adds a task to an execution unit's queue.
   - `dequeue_task(struct ExecutionUnit *unit)`: Removes a task from an execution unit's queue.
   - `calculate_power_mod_m(int value)`: Calculates the power modulo operation used in task processing.

### Execution Flow

1. **Initialization**: 
   - The user is prompted to enter the input file name and select a scheduling algorithm.
   - Execution units are initialized based on the selected algorithm.

2. **Task Reading and Enqueuing**:
   - Tasks are read from the input file and enqueued in the respective execution unit based on the scheduling algorithm.

3. **Task Processing**:
   - Execution units process tasks from their queue, performing the specified operations and printing results.
   - Threads run concurrently, demonstrating real-time task execution and scheduling.

4. **Completion**:
   - The program waits for all threads to finish processing before exiting.

## Usage

### Running the Program

1. Compile the code using a C compiler (e.g., `gcc`):
   ```bash
   gcc -o task_scheduler task_scheduler.c -lpthread -lm
   ```
2. Run the executable and follow the prompts:
   ```bash
   ./task_scheduler
   ```
3. Enter the input file name when prompted and select the desired scheduling algorithm:
   ```
   Enter the input file name: tasks.txt
   Select scheduling algorithm (0: Round Robin, 1: FCFS, 2: Priority, 3: Random): 0
   ```

### Input File Format

The input file should contain tasks in the following format:
```
task_id task_value unit_count unit_operation_1 unit_operation_2 ...
```
Example:
```
1 10 3 0 1 2
2 20 2 1 3
```

## Scheduling Algorithms Insights

### Round Robin (Algorithm 0)
- Tasks are processed in a cyclic order among the execution units.

### FCFS (Algorithm 1)
- Tasks are processed in the order they arrive.

### Priority (Algorithm 2)
- Tasks are processed based on a priority scheme (not explicitly detailed in this implementation).

### Random (Algorithm 3)
- Tasks are randomly assigned to execution units.

## Conclusion

This project demonstrates the implementation of a multi-threaded task scheduling and execution system with various scheduling algorithms. It showcases the complexities of task management and the impact of different scheduling strategies on task execution. Any colloboration and addition of features is greatly appreciated.
