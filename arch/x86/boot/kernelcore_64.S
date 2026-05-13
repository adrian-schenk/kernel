.code64

.text
.global _start64
_start64:
  # Enable x87/SSE in control registers before executing C code that may use float ops.
  mov %cr0, %rax
  btr $2, %rax      # CR0.EM = 0 (do not emulate x87)
  btr $3, %rax      # CR0.TS = 0 (task-switched off)
  bts $1, %rax      # CR0.MP = 1 (monitor coprocessor)
  mov %rax, %cr0

  mov %cr4, %rax
  bts $9, %rax      # CR4.OSFXSR = 1 (enable SSE instructions)
  bts $10, %rax     # CR4.OSXMMEXCPT = 1 (enable SSE exceptions)
  mov %rax, %cr4

  fninit

  call kernel_main
  hlt