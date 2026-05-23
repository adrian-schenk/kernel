#include "task.h"
#include "kmalloc.h"
#include "printf.h"
#include "cpu.h"

task_t *task_create(uint64_t entry) {
  task_t *task = (task_t*) kmalloc(sizeof(task_t));
  task->id = 1;
  uint64_t *rsp_location;
  uint64_t *stack = ((uint8_t*) kmalloc(4096)) + 4096;

  *--stack = 0xabcd; // return address (todo)
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

  *rsp_location = (uint64_t) stack;
  task->rsp = (uint64_t)stack;
  return task;
}

void task_switch_to(task_t *next) {
  _task_switch_to(&this_cpu(scheduler)->current, this_cpu(scheduler)->current, next);
}