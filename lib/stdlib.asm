.section .data
MINUS:
	.string "-"
.section .text
print:
	pushl %ebp
	movl %esp, %ebp
	pushl 8(%ebp)
	call strlen
	movl %eax, %edx
	movl $4, %eax
	movl $1, %ebx
	movl 8(%ebp), %ecx
	int $0x80
	leave
	ret
printi:
	pushl %ebp
	movl %esp, %ebp
	movl 8(%ebp), %eax
	cmpl $0, %eax
	jge printi_loop
	neg %eax
	pushl %eax
	movl $4, %eax
	movl $1, %ebx
	movl $1, %edx
	movl $MINUS, %ecx
	int $0x80
	popl %eax
	call printi_loop
printi_loop:
	movl $0, %edx
	movl $10, %ebx
	divl %ebx
	addl $48, %edx
	pushl %edx
	incl %esi
	cmpl $0, %eax
	jz   printi_next
	jmp printi_loop
printi_next:
	cmpl $0, %esi
	jz   printi_exit
	decl %esi
	movl $4, %eax
	movl %esp, %ecx
	movl $1, %ebx
	movl $1, %edx
	int  $0x80
	addl $4, %esp
	jmp  printi_next
printi_exit:
	movl $4, %eax
	movl $1, %ebx
	pushl $0x0a
	movl $1, %edx
	int $0x80
	leave
	ret
strlen:
	movl $0, %edi
	movl 4(%esp), %ebx
strlen_loop:
	movb (%ebx), %al
	cmpb $0, %al
	je strlen_exit
	incl %edi
	incl %ebx
	jmp strlen_loop
strlen_exit:
	movl %edi, %eax
	ret
