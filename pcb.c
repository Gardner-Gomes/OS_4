#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pcb.h"
#include <errno.h>

static unsigned int pid = 0;

PCB_p constructor() {
  PCB_p pcb = malloc(sizeof(PCB_s));
  CPU_context_p context = malloc(sizeof(CPU_context_s));
  enum state_type state = new;
  context->pc = 0;
  context->ir = 0;
  context->psr = 0;
  context->r0 = 0;
  context->r1 = 0;
  context->r2 = 0;
  context->r3 = 0;
  context->r4 = 0;
  context->r5 = 0;
  context->r6 = 0;
  context->r7 = 0;
  pcb->priority = 0;
  pcb->context = context;
  pcb->state = state;
  pcb->parent = 0;
  pcb->mem = NULL;
  pcb->size = 0;
  pcb->channel_no = 0;
  pcb->parent = 0;
  assignPid(pcb, &pid);
  return pcb;
}
int destructor(PCB_p pcb) {
  if (pcb == NULL) return -1;
  if (pcb->mem != NULL) free(pcb->mem);
  if (pcb->context != NULL) free(pcb->context);
  free(pcb);
  return 0;
}
int assignPid(PCB_p pcb, unsigned int * currentPid) {
  if (pcb == NULL) return -1;
  pcb->pid = *currentPid;
  *currentPid = *currentPid + 1;
  return 0;
}
unsigned int getPid(PCB_p pcb) {
  if (pcb == NULL) return -1;
  return pcb->pid;
}
unsigned int getParent(PCB_p pcb) {
  if (pcb == NULL) return -1;
  return pcb->parent;
}
unsigned char getPriority(PCB_p pcb) {
  if (pcb == NULL) return -1;
  return pcb->priority;
}
unsigned char * getMem(PCB_p pcb) {
  if (pcb == NULL) return NULL;
  return pcb->mem;
}
unsigned int getSize(PCB_p pcb) {
  if (pcb == NULL) return -1;
  return pcb->size;
}
unsigned char getChannel_no(PCB_p pcb) {
  if (pcb == NULL) return -1;
  return pcb->channel_no;
}
CPU_context_p getContext(PCB_p pcb) {
  if (pcb == NULL) return NULL;
  return pcb->context;
}
int setPid(PCB_p pcb, unsigned int pid) {
  if (pcb == NULL) return -1;
  pcb->pid = pid;
  return 0;
}
int setParent(PCB_p pcb, unsigned int parent) {
  if (pcb == NULL) return -1;
  pcb->parent = parent;
  return 0;
}
int setPriority(PCB_p pcb, unsigned char priority) {
  if (pcb == NULL) return -1;
  pcb->priority = priority;
  return 0;
}
int setMem(PCB_p pcb, unsigned char * mem) {
  if (pcb == NULL) return -1;
  pcb->mem = mem;
  return 0;
}
int setSize(PCB_p pcb, unsigned int size) {
  if (pcb == NULL) return -1;
  pcb->size = size;
  return 0;
}
int setChannel_no(PCB_p pcb, unsigned char channel_no) {
  if (pcb == NULL) return -1;
  pcb->channel_no = channel_no;
  return 0;
}

char * toString(PCB_p pcb) {
  if (pcb == NULL) return NULL;
  char *str = malloc(512);
  memset(str, 0, 512);
  CPU_context_p context = pcb->context;
  char *contextstr = context_toString(context);
  sprintf(str,"pid: 0x%X, state: 0x%X, parent: 0x%X, priority: 0x%X, mem: %s, size: 0x%X, channel_no: 0x%X, %s\n", pcb->pid & 0xffff, pcb->state & 0xffff, pcb->parent & 0xffff, pcb->priority & 0xf, pcb->mem, pcb->size & 0xffff, pcb->channel_no & 0xffff, contextstr);
  free(contextstr);
  return str;
}
char * context_toString(CPU_context_p context) {
  if (context == NULL) return NULL;
  char *str = malloc(512);
  memset(str, 0, 512);
  sprintf(str, "cpu_context:, pc: 0x%X, ir: 0x%X, psr: 0x%X, r0: 0x%X, r1: 0x%X, r2: 0x%X , r3: 0x%X , r4: 0x%X , r5: 0x%X , r6: 0x%X , r7: 0x%X, ", context->pc, context->ir, context->psr, context->r0, context->r1, context->r2, context->r3, context->r4, context->r5, context->r6, context->r7);
  return str;
}

int setState(PCB_p pcb, enum state_type new_state) {
  if (pcb == NULL) return -1;
  pcb->state = new_state;
  return 0;
}
