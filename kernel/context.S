.code64

.global _task_switch_to
_task_switch_to:
  # %rdi = current task, %rsi = new task
  # Save current task's state
  mov %rsp, (%rdi)      # current->stack_ptr = rsp
  
  # Load new task's state
  mov (%rsi), %rsp      # rsp = new_task->stack_ptr
  
  # Pop callee-saved registers (in reverse order of push in task_init)
  pop %r15
  pop %r14
  pop %r13
  pop %r12
  pop %rbp
  pop %rbx

  # Return to entry point (or next return address on stack)
  ret