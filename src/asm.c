#include <stdio.h>

#include "asm.h"
#include "file.h"

void asm_call() {

}

char* asm_return(ast_t* ast) {
	char* ret_buff = malloc(sizeof(char));
	char* value = malloc(2048);
	sprintf(value, "%d", ast->value);

	char* template = "\tmov $%s, %%ebx\n\tret\n";
	ret_buff = realloc(ret_buff, strlen(template) + strlen(value) + 1);
	snprintf(ret_buff, strlen(template) + strlen(value) + 1, template, value);

	free(value);

	return ret_buff;
}

char* asm_compound(ast_t* ast) {
	char* comp_buff = malloc(sizeof(char));
	*comp_buff = '\0';

	vec_t vec = ast->list;

	for (int i = 0; i < vec_length(&vec); ++i) {
		ast_t* ast = vec_get(&vec, i);
		comp_buff = strdup(comp_buff);
		if (ast->type == AST_RETURN) {
			char* ret_buff = asm_return(ast);
			comp_buff = realloc(comp_buff, strlen(comp_buff) + strlen(ret_buff) + 1);
			strcat(comp_buff, ret_buff);
		}
	}

	return comp_buff;
}

char* asm_block(ast_t* ast) {
	char* block_buff = malloc(sizeof(char));
	*block_buff = '\0';

	vec_t vec = ast->list;

	for (int i = 0; i < vec_length(&vec); ++i) {
		ast_t* ast = vec_get(&vec, i);
		block_buff = strdup(block_buff);
		if (ast->type == AST_COMPOUND) {
			char* comp_buff = asm_compound(ast);
			block_buff = realloc(block_buff, strlen(block_buff) + strlen(comp_buff) + 1);
			strcat(block_buff, comp_buff);
		} else if (ast->type == AST_BLOCK) {
			char* comp_buff = asm_block(ast);
			block_buff = realloc(block_buff, strlen(block_buff) + strlen(comp_buff) + 1);
			strcat(block_buff, comp_buff);
		}
	}

	return block_buff;
}

char* asm_function(ast_t* ast) {
	char* func_buff = malloc(sizeof(char));
	*func_buff = '\0';

	snprintf(func_buff, strlen(ast->name) + 3, "%s:\n", ast->name);

	if (ast->value != NULL) {
		ast_t* value = ast->value;
		switch(value->type) {
			case AST_BLOCK: {
				char* block_buff = asm_block(value);
				func_buff = realloc(func_buff, strlen(func_buff) + strlen(block_buff) + 1);
				strcat(func_buff, block_buff);
			}
		}
	}

	return func_buff;
}

void asm_init(vec_t asts) {
	char* placeholder = ".section .text\n"
					"\t.global _start\n"
					"_start:\n"
					"\tmov %eax, %edi\n"
					"\tcall main\n\n"
					"\tmovl $1, %eax\n"
					"\tint $0x80\n";
	char* src = (char*)malloc(strlen(placeholder) + 1);
	strcpy(src, placeholder);

	for (int i = 0; i < vec_length(&asts); ++i) {
		ast_t* ast = vec_get(&asts, i);
		src = strdup(src);
		switch(ast->type) {
			case AST_FUNCTION:
				char* func_buff = asm_function(ast);
				src = realloc(src, strlen(src) + strlen(func_buff) + 1);
				strcat(src, func_buff);
				break;
		}
	}

	printf("%s\n", src);
	write_file(src, "bin/assembly.asm");
}
