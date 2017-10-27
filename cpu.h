void add_n( fifo_queue new_procs);
int checkTerm();
// void set_rand_pids(unsigned int rand_pids[]);
int contains(unsigned int arr[], unsigned int value, int size);

PCB_p find_first_empty(priority_queue pq);
int dispatcher(PCB_p * current, priority_queue pq);
int scheduler(int schedule_bit, PCB_p * current, priority_queue pq, fifo_queue new_procs, fifo_queue old_procs, int * quantum);

char * to_string_3(int iteration_count, priority_queue pq);
int reset_all_priority(priority_queue pq);
unsigned int increment_pc(PCB_p * current, priority_queue pq, unsigned int * quantum_count);

int terminate(int rand_pid[4], PCB_p * current, priority_queue pq, fifo_queue new_procs, fifo_queue old_procs, int * quantum, unsigned int * pc);
int timer_interrupt(PCB_p * current, priority_queue pq, fifo_queue new_procs, fifo_queue old_procs, int * quantum, unsigned int * pc);
void pseudo_iret(unsigned int * pc);
