#include "util.S"

.code64

.global _task_switch_to
_task_switch_to:

  mov %rdx, (%rdi)

  mov 24(%rdx), %rax # load tasks page table
  mov %rax, %cr3 # set page table to tasks page table

  # Load new task's state
  mov (%rdx), %rsp      # rsp = new_task->rsp
  
  # Return to entry point (or next return address on stack)
  #addq $152, %rsp // remove function call stack from interrupt_handler to task_switch_to
  POP_REGS
  addq $16, %rsp
  iretq

.global _task_return
_task_return:
  mov %rax, %rdi
  call task_return