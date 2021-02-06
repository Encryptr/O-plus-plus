global __oppio_println
global __oppio_printnumb
global __oppio_panic
global __oppio_printc

extern putc
; global _start
global main

section .text

__oppio_strlen:
    xor rax, rax
start_len:
    cmp byte [rdi], 0
    je end_len
    inc rax
    inc rdi
    jmp start_len
end_len:
    ret

__oppio_println:
	mov r10, rdi
    call __oppio_strlen
    mov r9, rax
    mov rax, 1
    mov rdi, 1
    mov rsi, r10
    mov rdx, r9
    syscall
    xor rax, rax
    ret

__oppio_printnumb:
    cmp rdi, 0
    jns posative
    neg rdi
    push rdi
    mov dil, '-'
    call putc
    pop rdi
posative:
    mov    rax, rdi
    mov    rcx, 0xa              
    push   rcx                   
    mov    rsi, rsp
    sub    rsp, 16
digit:
    xor    rdx, rdx
    div    rcx
    add    rdx, '0'
    dec    rsi
    mov    [rsi], dl
    test   rax,rax
    jnz  digit
    mov    rax, 0x2000004
    mov    rdi, 1
    lea    rdx, [rsp+16 + 1]
    sub    rdx, rsi
    syscall
    add  rsp, 24
    ret

__oppio_panic:
	mov rax, 60
	syscall

main:
    mov rdi, 'A'
    call __oppio_printc
    ret

section .data

