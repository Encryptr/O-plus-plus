global __oppio_println
global __oppio_printnumb
global __oppio_printchar
global __oppio_strlen

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
    mov rsi, rdi
    call __oppio_strlen
    mov rdx, rax
    mov rax, 0x2000004
    mov rdi, 1
    syscall
    ret

__oppio_printchar:
	mov byte [rsp-1], dil
	mov rax, 0x2000004
	mov rdi, 1
	lea rsi, [rsp-1]
	mov rdx, 1        
    syscall
    ret

__oppio_printnumb:
	cmp rdi, 0
	jns posative
	neg rdi
	push rdi
	mov dil, '-'
	call __oppio_printchar
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