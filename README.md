
```markdown
# Real-Time Operating System (RTOS) - Earliest Deadline First (EDF) Scheduler

## Project Overview
This project implements a real-time scheduling algorithm, Earliest Deadline First (EDF), using the `SCHED_DEADLINE` policy in Linux. The system consists of three periodic tasks, each designed to simulate real-time operations, with a focus on task scheduling and deadline management.

## Key Features
- Implements EDF scheduling using pthreads and Linux system calls.
- Allows configuration of runtime, deadlines, and periods for each task.
- Monitors missed deadlines and computes the Worst Case Execution Time (WCET) for tasks.
- Utilizes CPU affinity to bind tasks to specific CPU cores.

## Requirements
- Linux Kernel with `SCHED_DEADLINE` support
- GCC Compiler
- `pthread` library

## Installation Instructions

1. **Clone the Repository**
   ```bash
   git clone https://github.com/SaiVinay023/Advanced-Robot-Programing.git
   cd Advanced-Robot-Programing
   ```

2. **Compile the Code**
   ```bash
   gcc -o rtos_edf rtos_edf.c -lpthread
   ```

## Configuration
Before running the program, ensure the task attributes are set correctly in the code. Modify the following parameters within `main()`:
- `sched_runtime`: Defines the runtime for each task.
- `sched_deadline`: Sets the deadline for each task.
- `sched_period`: Specifies the period of each task.

## Execution Instructions

1. **Run the Program**
   ```bash
   sudo ./rtos_edf
   ```

2. **Monitor Output**
   The program will print the execution flow of each task along with information about missed deadlines and WCET. 

3. **Exit**
   The program will terminate after executing all tasks. Check the console output for task execution details and any missed deadlines.

## Example Output
```
deadline thread 1 start 1234
 1[ ]1 
deadline thread 2 start 1235
 2[ ]2 
deadline thread 3 start 1236
 3[ ]3 
...
U=0.5 Ulub=0.7 Schedulable Task Set
main dies[1237]
```

## Conclusion
This project provides a practical implementation of an EDF scheduler in a real-time operating system context, demonstrating the ability to manage multiple tasks with strict timing constraints.

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments
- Thanks to the community for resources on real-time scheduling and pthreads.
```

### Notes:
- Adjust the file names in the `gcc` command as needed.
- Ensure you have the necessary permissions to run with `sudo`, especially for `SCHED_DEADLINE`.
- This README includes basic usage, but you may expand it with more details as required for your specific implementation.
