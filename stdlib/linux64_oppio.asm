global __oppio_println
global __oppio_printnumb

; Flags
global __oppio_READ
global __oppio_WRITE
global __oppio_RDWR

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

	ret

__oppio_printnumb:
	ret

__oppio_open_io:
	ret

__oppio_read_io:
	
	ret


section .data
__oppio_READ dq 0
__oppio_WRITE dq 1
__oppio_RDWR dq 2