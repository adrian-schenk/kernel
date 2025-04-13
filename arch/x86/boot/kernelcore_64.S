.code64

.text
.global _start64

_start64:

  call kernel_main
  hlt
