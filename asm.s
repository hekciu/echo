.global _start, syscall5, syscall6
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


syscall6:
    mov     %rdi, %rax
    mov     %rsi, %rdi
    mov     %rdx, %rsi
    mov     %rcx, %rdx
    mov     %r8, %r10
    mov     %r9, %r8
    pop     %r9
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

