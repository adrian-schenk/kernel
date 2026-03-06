#include "util.S"

.code64

.globl intr_handler

.macro ISR_NOERR n
.global intr\n
intr\n:
    pushq $0
    pushq $\n
    jmp intr_handler
.endm

.macro ISR_ERR n
.global intr\n
intr\n:
    pushq $\n
    jmp intr_handler
.endm

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR 8
ISR_NOERR 9
ISR_ERR 10
ISR_ERR 11
ISR_ERR 12
ISR_ERR 13
ISR_ERR 14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31
ISR_NOERR 32
ISR_NOERR 33
ISR_NOERR 34
ISR_NOERR 35
ISR_NOERR 36
ISR_NOERR 37
ISR_NOERR 38
ISR_NOERR 39
ISR_NOERR 40
ISR_NOERR 41
ISR_NOERR 42
ISR_NOERR 43
ISR_NOERR 44
ISR_NOERR 45
ISR_NOERR 46
ISR_NOERR 47

intr48:
    pushq $0
    pushq $48
    jmp syscl_handler

intr_handler:
syscl_handler:
    PUSH_REGS
    
intr_return:
syscl_return:
    POP_REGS
    addq $16, %rsp
    iretq

.global isr_stub_table
isr_stub_table:
    .quad intr0
    .quad intr1
    .quad intr2
    .quad intr3
    .quad intr4
    .quad intr5
    .quad intr6
    .quad intr7
    .quad intr8
    .quad intr9
    .quad intr10
    .quad intr11
    .quad intr12
    .quad intr13
    .quad intr14
    .quad intr15
    .quad intr16
    .quad intr17
    .quad intr18
    .quad intr19
    .quad intr20
    .quad intr21
    .quad intr22
    .quad intr23
    .quad intr24
    .quad intr25
    .quad intr26
    .quad intr27
    .quad intr28
    .quad intr29
    .quad intr30
    .quad intr31
    .quad intr32
    .quad intr33
    .quad intr34
    .quad intr35
    .quad intr36
    .quad intr37
    .quad intr38
    .quad intr39
    .quad intr40
    .quad intr41
    .quad intr42
    .quad intr43
    .quad intr44
    .quad intr45
    .quad intr46
    .quad intr47
    .quad intr48