#include "util.S"

.code64

.global _task_switch_to
_task_switch_to:
  # %rdi = &current, %rsi = current task, %rdx = new  task
  # Save current task's state
  test %rsi, %rsi
  jle .skip
  mov %rsp, (%rsi)      # current->rsp = rsp (only if current != nullptr)
  
.skip:

  mov %rdx, (%rdi)

  mov 24(%rdx), %rax # load tasks page table
  mov %rax, %cr3 # set page table to tasks page table

  # Load new task's state
  mov (%rdx), %rsp      # rsp = new_task->rsp
  
  # Return to entry point (or next return address on stack)
  addq $152, %rsp // remove function call stack from interrupt_handler to task_switch_to
  POP_REGS
  addq $16, %rsp
  iretq