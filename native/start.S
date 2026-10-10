.syntax unified
.arch armv7-a
.arm
.global _start
.type _start,%function
_start:
    mov fp, #0
    mov lr, #0
    mov r4, r0
    ldr r1, [sp]
    add r2, sp, #4
    bic sp, sp, #7
    mov r0, #0
    push {r0}
    push {sp}
    push {r4}
    push {r0}
    ldr r0, =main
    mov r3, #0
    bl __libc_start_main
    udf #0
.section .note.GNU-stack,"",%progbits
