#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "pcb.h"
#include "fifo_queue.h"
#include "priority_queue.h"
#include "cpu.h"

#define QUEUE_SIZE 16
#define MAX_PROCS 50
#define QUANTUM_S 34 * 1000 / (MAX_PROCS / 3)

static unsigned int sys_stack = 0;

int main() {

  //
  // Init all variables for processes
  //
  priority_queue pq = priority_queue_constructor();
  fifo_queue new_procs = fifo_queue_constructor();
  fifo_queue old_procs = fifo_queue_constructor();  

  // Used to manage current process
  PCB_p * current = malloc(sizeof(PCB_p));

  // manage pointers to "privelaged" processes and assign random pid values to work with
  PCB_p privelaged[4];
  unsigned int rand_pids[4];
  set_rand_pids(rand_pids);
  
  unsigned int * pc = malloc(sizeof(unsigned int));
  *pc = 0;
  unsigned int * Quantum_Timer = malloc(sizeof(unsigned int)); //QUantum Timer
  *Quantum_Timer = 1;
  unsigned int * IO_Timer = malloc(sizeof(unsigned int));
  *IO_Timer = *Quantum_Timer * 3;
  //Fibonacci sequesnce or length of quantum based on prioriety
  int quantum[] = { 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597 }; 
  int * count = malloc(sizeof(int));
  unsigned int * quantum_count = malloc(sizeof(unsigned int));
  int priv_len = 0;
  *quantum_count = 0;
  *count = 0;
  *current = NULL;
  srand(time(NULL));

  // setup quanutum values in queue
  int i;
  for (i = 0; i < QUEUE_SIZE; i++)
    q_setquantum(get_queue(pq, i), quantum[i] * 1000);
  for (i = 0; i < 4; i++)
    privelaged[i] = NULL;

  int iteration_count = 0;

  //
  // Init program sequence
  //
  // while (*count < MAX_PROCS) {
  //   add_n(count, rand_pids, privelaged, &priv_len, new_procs);

  //   // increment pc and quantum progress
  //   *pc += increment_pc(current, pq, quantum_count);

  //   // possible outcomes
  //   // 1 : terminate
  //   // 2 : timer_interrupt
  //   if (checkTerm() == 1 && current != NULL && *current != NULL && contains(rand_pids, (*current)->pid, 4) != 1) {
  //     terminate(rand_pids, current, pq, new_procs, old_procs, quantum_count, pc);
  //   } else {
  //     timer_interrupt(current, pq, new_procs, old_procs, quantum_count, pc);
  //   }

  //   // print output
  //   char * output = to_string_3(iteration_count++, pq, privelaged);
  //   printf("%s<CR>\n", output);
  //   free(output);
  // }

   
  while(1) {
    //Generate Processes randomly
    //scheduler(0)
    while (Quantum_Timer) {
        //running pc ++
        //CHeck trap Values vs pc
        //if true ^ -> call scheduler(IO_Trap)
        //Decrement Timers
        //if pc == max_PC -> term_Count++ & pc =0
        //if term_count == terminate -> call scheduler(terminate)
        //if(IO_timer == 0) -> scheduler(IO_ret)
    }
    //scheduler(timer interrupt)
  }

  //cleanup
  if (current != NULL && *current != NULL) destructor(*current);
  if (current!= NULL) free(current);
  if (new_procs!= NULL) fifo_destructor(new_procs);
  if (old_procs!= NULL) fifo_destructor(old_procs);
  if (pq!= NULL) priority_queue_destructor(pq);
  if (count != NULL) free(count);
  if (quantum_count != NULL) free(quantum_count);
  if (pc != NULL) free(pc);
  return 0;
}
//
//Adds a Random number of Processes to the queue. between 0-5
//
void add_n(int * count, unsigned int rand_pids[], PCB_p privelaged[4], int * priv_length, fifo_queue new_procs) {
  int num = rand() % 6;
  int i;
  for (i = 0; i <= num; i++) {
    PCB_p temp = constructor();
    q_enqueue(new_procs, temp);
    (*count)++;

    if (*priv_length < 4 && contains(rand_pids, temp->pid, 4) == 1) {
      privelaged[*priv_length] = temp;
      *priv_length += 1;
    }
  }
}

/*
  Randomly increments the PC for the process, based on quantum length.
  Simulates running a process for a quantum.
*/
unsigned int increment_pc(PCB_p * current, priority_queue pq, unsigned int * quantum_count) {
  if (pq == NULL || current == NULL || *current == NULL) return 0;
  unsigned int t = rand() % get_queue(pq, (*current)->priority)->quantum;
  *quantum_count += t;
  return t;
}

/*
  Timer Interrupt, sets running process to Interrupted then calls scheduler.
*/
int timer_interrupt(PCB_p * current, priority_queue pq, fifo_queue new_procs, fifo_queue old_procs, int * quantum, unsigned int * pc) {
  if (pq == NULL) return 1;
  sys_stack = *pc;
  enum state_type new_state = interrupted;
  if (current != NULL && (*current) != NULL) {
    (*current)->state = new_state;
    (*current)->context->pc = sys_stack;
  }
  scheduler(1, current, pq, new_procs, old_procs, quantum);
  pseudo_iret(pc);
  return 0;
}

/*
  Determines what to do for a certain interrupt, calls dispatcher if necessary.
*/
int scheduler(int schedule_bit, PCB_p * current, priority_queue pq, fifo_queue new_procs, fifo_queue old_procs, int * quantum) {
  if (pq == NULL) return 1;

  // add new processes to current processes
  while (new_procs->count > 0) {
    enum state_type new_state = ready;
    PCB_p to_enqueue = q_dequeue(new_procs);
    setState(to_enqueue, new_state);
    enqueue_ready(pq, 0, to_enqueue);
  }

  // in this case we will append to fifo_queue and dispatch
  switch (schedule_bit) {

    // interrupt case
  case 1:
    if (current != NULL && *current != NULL) {
      enum state_type new_state = ready;
      setState(*current, new_state);
      setPriority(*current, (*current)->priority + 1);  //lower prioriety of current
      enqueue_ready(pq, (*current)->priority, *current);
    }
    dispatcher(current, pq);
    break;

    // terminate case
  case 2:
    dispatcher(current, pq);
    if (old_procs->count > 10)
      while (old_procs->count > 0)//Clears zombie processes if there are 10 or more,
	destructor(q_dequeue(old_procs));
    break;
  case 3: //IO_trapCall
  case 4: //IO ret call
  default:
    break;
  }
  /*
    Returns the process froom IO queue
  */
 PCB_p IO_ret() {

 }
 /*
  Puts calling process in blocked queue
 */
 void IO_Trap(PCB_p thepcb){

 }
  //When Time_S is reached reset priorieties of all in Prioriety Queue.
  if (*quantum >= QUANTUM_S) {
    *quantum = 0;
    printf("\n\n Resetting priorities\n\n");
    reset_all_priority(pq);
  }
  return 0;
}

/*
  Puts first process into running state, and sets the sys_stack to this process.
*/
int dispatcher(PCB_p * current, priority_queue pq) {
  if (pq == NULL) return 1;
  if (current != NULL && (*current) != NULL)
    (*current)->context->pc = sys_stack;

  PCB_p new_proc = find_first_empty(pq);
  if (new_proc != NULL) {
    enum state_type new_state = running;
    new_proc->state = new_state;
    *current = new_proc;
    sys_stack = new_proc->context->pc;
  } else {
    *current = NULL;
  }
  return 0;
}

/*
  Finds the first element in the prioriety queue and returns it if it exists.
*/
PCB_p find_first_empty(priority_queue pq) {
  if (pq == NULL) return NULL;
  int i = 0;
  while (i < QUEUE_SIZE) {
    fifo_queue temp = get_queue(pq, i);
    if (get_queue(pq, i)->count > 0) {
      return q_dequeue(temp);
    }
    i++;
  }
  return NULL;
}

/*
  Terminates the running process and moves it to a zombie state.
*/
int terminate(int rand_pid[4], PCB_p * current, priority_queue pq, fifo_queue new_procs, fifo_queue old_procs, int * quantum, unsigned int * pc) {
  if (current == NULL || *current == NULL || contains(rand_pid, (*current)->priority, 4) == 1) return -1;
  enum state_type new = zombie;
  (*current)->state = new;
  q_enqueue(old_procs, *current);
  scheduler(2, current, pq, new_procs, old_procs, quantum);
  pseudo_iret(pc);
  return 0;
}

int checkTerm() {
  return (rand() % 100) < 15;
}

/*
  Generates 4 random processes to privledged that cant be killed.
  randomly chosen from first 1/5 of processes generated.
*/
void set_rand_pids(unsigned int * rand_pids) {
  int i;
  for (i = 0; i < 4; i++) *(rand_pids + i) = 0;
  for (i = 0; i < 4; i++) {
    int rand_pid;
    do {
      rand_pid = (unsigned) (rand() % (MAX_PROCS / 5));
    } while (contains(rand_pids, rand_pid, 4) == 1);
    rand_pids[i] = rand_pid;
  }
}

/*
  helper method to check if the array conains an id assigned to a privledged process.
*/
int contains(unsigned int arr[], unsigned int value, int size) {
  int i;
  for (i = 0; i < size; i++) {
    if (arr[i] == value)
      return 1;
  }
  return 0;
}

/*
  To_String method adhering to the format on ddocument for part 3.
*/
char * to_string_3(int iteration_count, priority_queue pq, PCB_p privelaged[4]) {
  char * ret = malloc(sizeof(char) * 1024);
  memset(ret, 0, sizeof(char) * 1024);

  char * iteration =  malloc(sizeof(char) * 32);
  memset(iteration, 0, sizeof(char) * 32);
  sprintf(iteration, "Iteration: %d\n", iteration_count);
  strcat(ret, iteration);
  free(iteration);

  unsigned int i;
  for (i = 0; i < QUEUE_SIZE; i++) {
    char * q_and_size = malloc(sizeof(char) * 32);
    memset(q_and_size, 0, sizeof(char) * 32);
    sprintf(q_and_size, "Queue %d: %d\n", i, get_queue(pq, i)->count);
    strcat(ret, q_and_size);
    free(q_and_size);
  }

  for (i = 0; i < 4; i++) {
    PCB_p temp = privelaged[i];
    if (temp != NULL) {
      char * q_and_size = malloc(sizeof(char) * 256);
      memset(q_and_size, 0, sizeof(char) * 256);
      sprintf(q_and_size, "PCB: PID %u, PRIORITY %u PC %u\n", temp->pid, temp->priority, temp->context->pc);
      strcat(ret, q_and_size);
      free(q_and_size);
    }
  }
  return ret;
}

/*
  helper method to reset all priorities of our processes.
*/
int reset_all_priority(priority_queue pq) {
  if (pq == NULL) return 1;
  int i;
  for (i = 1; i < QUEUE_SIZE; i++) {
    fifo_queue q = get_queue(pq, i);
    while (q->count > 0) {
      PCB_p pcb = q_dequeue(q);
      pcb->priority = 0;
      enqueue_ready(pq, 0, pcb);
    }
  }
  return 1;
}

/*
  Psuedo Iret, returns the pc value from sys stack.
*/
void pseudo_iret(unsigned int * pc) {
  *pc = sys_stack;
}
