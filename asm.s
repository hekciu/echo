.global _start, syscall5, sys_recvfrom
.text


_start:
    xor     %rbp, %rbp
    pop     %rdi
    mov     %rsp, %rsi
    and     $-16, %rsp
    call    main

    mov     %rax, %rdi
    mov     $0x3c,%rax 
    syscall

    ret


sys_recvfrom:
    mov     $0x2d, %rax
    mov     %rdi, %rdi
    mov     %rsi, %rsi
    mov     %rdx, %rdx
    mov     %rcx, %r10
    mov     %r8, %r8
    mov     %r9, %r9
    syscall

    ret


syscall5:
    mov     %rdi, %rax
    mov     %rsi, %rdi
    mov     %rdx, %rsi
    mov     %rcx, %rdx
    mov     %r8, %r10
    mov     %r9, %r8
    syscall

    ret

