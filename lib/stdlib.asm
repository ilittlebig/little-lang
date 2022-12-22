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
