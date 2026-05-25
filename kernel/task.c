#include "task.h"
#include "kmalloc.h"
#include "printf.h"
#include "cpu.h"
#include "interrupt.h"
#include "spinlock.h"

task_t *task_create(uint64_t entry) {
  task_t *task = (task_t*) kmalloc(sizeof(task_t));
  task->id = 1;
  task->rsp_base = (uint64_t) kmalloc(4096);
  uint64_t *rsp_location;
  uint64_t *stack = ((uint8_t*) task->rsp_base) + 4096;

  *--stack = task->id;
  *--stack = task_return; // fake rbp
  *--stack = task_return; // return address
  *--stack = 0x10; // ss
  rsp_location = --stack; // save rsp for later
  *--stack = 0x202; // rflags
  *--stack = 0x8; // CS
  *--stack = entry; // return address for when the task is switched to

  --stack; --stack; // mock interrupt number and error

  for (int i = 0; i < 15; i++)
    *--stack = 0x00; // mock registers

  for (int i = 0; i < (152 / 8); i++)
    *--stack = 0x00; // mock function call stack from interrupt_handler to task_switch_to (152 bytes)

  *rsp_location = (uint64_t*) rsp_location + 2;
  task->rsp = (uint64_t)stack;
  return task;
}

static void task_return(char a, char b, char c, char d, char e, char f, long long task_id) {
  printf("task returned, exiting cpu %d and %l at %p\n", this_cpu(cpu_id), task_id, &task_id);
  cli();
  lock(&this_cpu(scheduler)->lock);
  for (int i = 0; i < SCHEDULER_QUEUE_SIZE; i++) {
    if (this_cpu(scheduler)->queue[i] && this_cpu(scheduler)->queue[i]->id == task_id) {
      kfree(this_cpu(scheduler)->queue[i]->rsp_base);
      this_cpu(scheduler)->queue[i] = (void*)0;
      this_cpu(scheduler)->count--;
      this_cpu(scheduler)->current = (void*)0;
      break;
    }
  }
  unlock(&this_cpu(scheduler)->lock);
  sti();
  for (;;);
}

void task_switch_to(task_t *next) {
  _task_switch_to(&this_cpu(scheduler)->current, this_cpu(scheduler)->current, next);
}