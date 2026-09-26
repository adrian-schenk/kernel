#include "util.S"

# Stack Layout of the new Task
#
# 
# +0x80 - RSP

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