#include <stdio.h>
#include "builtins.h"

void builtins_init(FILE* outputfp) {
	const char* bootstrap =
		"print:\n"
		"	pushl %%ebp\n"
		"	movl %%esp, %%ebp\n"
		"	pushl 8(%%ebp)\n"
		"	call strlen\n"
		"	movl %%eax, %%edx\n"
		"	movl $4, %%eax\n"
		"	movl $1, %%ebx\n"
		"	movl 8(%%ebp), %%ecx\n"
		"	int $0x80\n"
		"	leave\n"
		"	ret\n"
		"strlen:\n"
		"	movl $0, %%edi\n"
		"	movl 4(%%esp), %%ebx\n"
		"strlen_loop:\n"
		"	movb (%%ebx), %%al\n"
		"	cmpb $0, %%al\n"
		"	je strlen_exit\n"
		"	incl %%edi\n"
		"	incl %%ebx\n"
		"	jmp strlen_loop\n"
		"strlen_exit:\n"
		"	movl %%edi, %%eax\n"
		"	ret\n";
	fprintf(outputfp, bootstrap);
}
