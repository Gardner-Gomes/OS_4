#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pcb.h"
#include "fifo_queue.h"
#include "priority_queue.h"
#include "cpu.h"

#define QUEUE_SIZE 16
#define MAX_PROCS 50
#define QUANTUM_S 100

static unsigned int sys_stack = 0;

int main() {

  //
  // Init all variables for processes
  //
  priority_queue pq = priority_queue_constructor();
  fifo_queue new_procs = fifo_queue_constructor();
  fifo_queue old_procs = fifo_queue_constructor(); 
  fifo_queue IO_Queue = fifo_queue_constructor(); 
  // Used to manage current process
  PCB_p * current = malloc(sizeof(PCB_p));
  
  unsigned int * pc = malloc(sizeof(unsigned int));
  *pc = 0;
  unsigned int * Quantum_Timer = malloc(sizeof(unsigned int)); //QUantum Timer
  *Quantum_Timer = 1;
  unsigned int * IO_Timer = malloc(sizeof(unsigned int));
  *IO_Timer = *Quantum_Timer * 3;
  //Fibonacci sequesnce or length of quantum based on prioriety
  int quantum[] = { 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597 }; 
  int * count = malloc(sizeof(int));
  *count = 0;
  int * quantum_count = malloc(sizeof(int));
  *quantum_count = 0;
  *current = NULL;
  srand(time(NULL));
  int i;
  for (i = 0; i < QUEUE_SIZE; i++)
    q_setquantum(get_queue(pq, i), quantum[i] * 1000);
 
  while(1) {
    //Generate Processes randomly
    //if pq < maxprocs
    //if(pq) {
    printf("0\n"); 
    add_n(new_procs);

    printf("\nof\n");
    scheduler(0,current,pq,new_procs,old_procs,quantum_count);
    printf("\npid:%d\n", (*current)->pid);

    *Quantum_Timer = quantum[(*current)->priority] + 1000;
    while (*Quantum_Timer) {
        //running pc ++
      printf("1\n");
      printf("PC:%d\n", (*current)->context->pc);
      (*current)->context->pc++;
        //CHeck trap Values vs pc
      printf("2\n");
      if(contains((*current)->IO_1_TRAPS, (*current)->context->pc, 4) ||
          contains((*current)->IO_2_TRAPS, (*current)->context->pc, 4)) {
        IO_Trap(*current);
      }
        //if true ^ -> call IO_Trap

        //Decrement Timers
      printf("3\n");
      *Quantum_Timer--;
      *IO_Timer--;
        //if pc == max_PC -> term_Count++ & pc =0
      printf("4\n");
      if((*current)->context->pc == (*current)->MAX_PC) {
        (*current)->term_count++;
        (*current)->context->pc = 0;
      }
        //if term_count == terminate -> call terminate
      printf("5\n");
      if ((*current)->term_count == (*current)->terminate) {
        terminate(current, pq, new_procs, old_procs, quantum_count);
      }

        //if(IO_Timer == 0) -> IO_Ret
      printf("6\n");
      if(*IO_Timer == 0) {
        IO_ret();
      }
    }
    printf("7\n");
    *quantum_count++;

    //timerinterruptcall()
    printf("8\n");
    timer_interrupt(current, pq, new_procs, old_procs, quantum_count);

    char * why = to_string_3(*quantum_count, pq);
    printf("%s\n", why);
    free(why);
    
  }

  //cleanup
  if (current != NULL && *current != NULL) destructor(*current);
  if (current!= NULL) free(current);
  if (new_procs!= NULL) fifo_destructor(new_procs);
  if (old_procs!= NULL) fifo_destructor(old_procs);
  if (pq!= NULL) priority_queue_destructor(pq);
  if (count != NULL) free(count);
  if (pc != NULL) free(pc);
  return 0;
}
//
//Adds a Random number of Processes to the queue. between 0-5
//
void add_n( fifo_queue new_procs) {
  int num = rand() % 6 + 1;
  int i;
  printf("\n%d Processes\n", num);
  for (i = 0; i < num; i++) {
    printf("\nprocess: %d\n\n", i);
    PCB_p temp = constructor();

    //set MAX_PC Randomly
    unsigned int max = rand() % 10000;
    printf("\nmax pc: %d\n\n", max);
    set_MAX_PC(temp, max);

    //TODO Generate Trap Arrays set them.
    int IO1[4];
    int IO2[4];
    int j;
    for (j = 0; j <= 4; j++) {
      /*

      DEBUG THIS SHIT

      */
      IO1[j] = rand() % max;
      IO2[j] = rand() % max;

      printf("IO1[%d]: %d, IO2[%d]: %d\n", j, IO1[j], j, IO2[j]);
    }
    set_IO_1_TRAPS(temp, IO1);
    set_IO_2_TRAPS(temp, IO2);

    //set Term Count Randomly

    int termC = rand() % 11;
    printf("\nTerm Count: %d\n", termC);
    set_term_count(temp, termC);

    char * why = toString(temp);
    printf("\n%s\n", why);
    free(why);
    q_enqueue(new_procs, temp);

    why = q_toString(new_procs);
    printf("\n%s\n", why);
    free(why);

  }

  char * why = q_toString(new_procs);
  printf("\n%s\n", why);
  free(why);

  
}


/*
  Timer Interrupt, sets running process to Interrupted then calls scheduler.
*/
int timer_interrupt(PCB_p * current, priority_queue pq, fifo_queue new_procs, fifo_queue old_procs, int * quantum_count) {
  if (pq == NULL) return 1;
  sys_stack = (*current)->context->pc;
  enum state_type new_state = interrupted;
  if (current != NULL && (*current) != NULL) {
    (*current)->state = new_state;
    (*current)->context->pc = sys_stack;
  }
  scheduler(1, current, pq, new_procs, old_procs, quantum_count);
  pseudo_iret(&(*current)->context->pc);
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
  case 3: //dispatcher
  case 4: //dispatcher
  default:
    dispatcher(current, pq);
    break;
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
  Returns the process froom IO queue
*/
PCB_p IO_ret() {
    
}
/*
   Puts calling process in blocked queue
*/
void IO_Trap(PCB_p thepcb){
    
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
int terminate( PCB_p * current, priority_queue pq, fifo_queue new_procs, fifo_queue old_procs, int * quantum_count) {
  if (current == NULL || *current == NULL) return -1;
  enum state_type new = zombie;
  (*current)->state = new;
  q_enqueue(old_procs, *current);
  scheduler(2, current, pq, new_procs, old_procs, quantum_count);
  pseudo_iret(&(*current)->context->pc);
  return 0;
}

// int checkTerm() {
//   return (rand() % 100) < 15;
// }

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
char * to_string_3(int iteration_count, priority_queue pq) {
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
