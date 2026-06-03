.code64
.global syscall
syscall:
  int $48
  ret