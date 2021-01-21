global __oppio_println
global __oppio_printnumb
global __oppio_panic
global _start

extern main

section .text

; @Name: __oppio_strlen
; @Desc: Return len of str
; @Param: char* (str)
; @Ret: int (length)
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

; @Name: __oppio_println
; @Desc: Print zero terminated string
; @Param: char* (str)
; @Ret: None
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
	ret

; @Name: __oppio_panic
; @Desc: Panic
; @Param: int
; @Ret: None
__oppio_panic:
	mov rax, 60
	syscall

_start:
    call main
    mov rdi, rax
    jmp __oppio_panic

section .data

