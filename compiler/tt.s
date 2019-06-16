.data
str: .asciz "Hello Jacek"
.text
.global main
main: 
	sub $32, %rsp
	lea str(%rip), %rcx
	call puts
	add $32, %rsp
	xor %eax, %eax
	ret
