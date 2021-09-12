#include <stdio.h>

#include "asm.h"
#include "file.h"

vec_t strings;
char* lc_strings;

static int has_ident(char* ident, vec_t* vec) {
	for (int i = 0; i < vec_length(vec); ++i) {
		char* string = vec_get(vec, i);
		if (strcmp(string, ident) == 0) {
			return 0;
		}
	}
	return 1;
}

static int get_id_from_str(char* ident, vec_t* vec) {
	for (int i = 0; i < vec_length(vec); ++i) {
		char* string = vec_get(vec, i);
		if (strcmp(string, ident) == 0) {
			return i;
		}
	}
	return 0;
}

static int get_variable_id(char* ident, vec_t* vec) {
	for (int i = 0; i < vec_length(vec); ++i) {
		char** var = vec_get(vec, i);
		char* string = var[0];
		if (strcmp(string, ident) == 0) {
			return i + 1;
		}
	}
	return vec_length(vec) + 1;
}

static int get_variable_index(char* ident, vec_t* vec) {
	for (int i = 0; i < vec_length(vec); ++i) {
		char** var = vec_get(vec, i);
		char* string = var[0];
		if (strcmp(string, ident) == 0) {
			return i;
		}
	}
	return 0;
}

char* lil_print(ast_t* ast, vec_t* vars) {
	char* movl_buff = malloc(sizeof(char));
	vec_t vec = ast->list;

	for (int i = 0; i < vec_length(&vec); ++i) {
		ast_t* ast = vec_get(&vec, i);
		if (ast->type == AST_STRING) {
			char* template = "\tmovl $4, %%eax\n"
							"\tmovl $1, %%ebx\n"
							"\tmovl $LC%d, %%ecx\n"
							"\tmovl $%d, %%edx\n"
							"\tint $0x80\n";

			int id = get_id_from_str(ast->value, &strings);
			movl_buff = realloc(movl_buff, strlen(movl_buff) + strlen(template) + 100);
			sprintf(movl_buff, template, id, strlen(vec_get(&strings, id)));
		} else if (ast->type == AST_VARIABLE) {
			char* template = "\tmovl $4, %%eax\n"
							"\tmovl $1, %%ebx\n"
							"\tmovl -%i(%%ebp), %%ecx\n"
							"\tmovl $%i, %%edx\n"
							"\tint $0x80\n";

			int var_id = get_variable_id(ast->name, vars);
			int var_index = get_variable_index(ast->name, vars);
			char** v = vec_get(vars, var_index);

			movl_buff = realloc(movl_buff, strlen(movl_buff) + strlen(template) + 100);
			sprintf(movl_buff, template, var_id * 4, strlen(v[1]));
		}
	}

	return movl_buff;
}

void asm_string(ast_t* ast) {
	char* string_buff = malloc(sizeof(char));
	char* template = "LC%d:\n"
					"\t.string \"%s\"\n";

	if (has_ident(ast->value, &strings) == 1) {
		int id = vec_length(&strings);
		string_buff = realloc(string_buff, strlen(template) + strlen(ast->value) + 1);
		sprintf(string_buff, template, id, ast->value);

		vec_push_back(&strings, ast->value);
		lc_strings = realloc(lc_strings, strlen(lc_strings) + strlen(string_buff) + 1);
		strcat(lc_strings, string_buff);
	}
}

char* asm_call(ast_t* ast, vec_t* vars) {
	char* call_buff = malloc(sizeof(char));

	vec_t vec = ast->list;
	for (int i = 0; i < vec_length(&vec); ++i) {
		ast_t* ast = vec_get(&vec, i);
		if (ast->type == AST_STRING) {
			asm_string(ast);
		} else if (ast->type == AST_VARIABLE) {
		} else if (ast->type == AST_INT) {
			char* value = malloc(2048);
			sprintf(value, "%d", ast->value);
			free(value);
		}
	}

	if (strcmp(ast->name, "print") == 0) {
		free(call_buff);
		return lil_print(ast, vars);
	}

	char* template = "\tcall %s\n";
	call_buff = realloc(call_buff, strlen(template) + strlen(ast->name) + 1);
	snprintf(call_buff, strlen(template) + strlen(ast->name) + 1, template, ast->name);

	return call_buff;
}

char* asm_return(ast_t* ast, vec_t* vars) {
	char* ret_buff = malloc(sizeof(char));

	ast_t* ast_value = ast->value;
	if (ast_value->type == AST_VARIABLE) {
		int var_id = get_variable_id(ast_value->name, vars);
		char* template = "\tmovl -%i(%%ebp), %%eax\n\tpopl %%ebp\n\tret\n";
		ret_buff = realloc(ret_buff, strlen(template) + 1);
		snprintf(ret_buff, strlen(template) + 1, template, var_id * 4);
	} else {
		char* value = malloc(2048);
		sprintf(value, "$%d", ast_value->value);

		char* template = "\tmovl %s, %%eax\n\tpopl %%ebp\n\tret\n";
		ret_buff = realloc(ret_buff, strlen(template) + strlen(value) + 1);
		snprintf(ret_buff, strlen(template) + strlen(value) + 1, template, value);

		free(value);
	}

	return ret_buff;
}

char* asm_assignment(ast_t* ast, vec_t* vars) {
	char* assign_buff = malloc(sizeof(char));

	ast_t* ast_value = ast->value;
	if (ast_value->type == AST_STRING) {
		char** vec = calloc(strlen(ast->name) + strlen(ast_value->value), sizeof(char));
		vec[0] = ast->name;
		vec[1] = ast_value->value;

		vec_push_back(vars, vec);
		asm_string(ast_value);

		int id1 = get_id_from_str(ast_value->value, &strings);
		int id2 = get_variable_id(ast->name, vars);

		char* template = "\tmovl $LC%i, -%i(%%ebp)\n";
		assign_buff = realloc(assign_buff, strlen(template) + strlen(ast_value->value) + 1);
		sprintf(assign_buff, template, id1, id2 * 4);
	} else if (ast_value->type == AST_INT) {
		char* value = malloc(2048);
		sprintf(value, "$%d", ast_value->value);

		vec_push_back(vars, ast->name);
		int id = get_variable_id(ast->name, vars);

		char* template = "\tmovl %s, -%i(%%ebp)\n";
		assign_buff = realloc(assign_buff, strlen(template) + strlen(value) + 1);
		snprintf(assign_buff, strlen(template) + strlen(value) + 1, template, value, id * 4);

		free(value);
	}

	return assign_buff;
}

char* asm_block(ast_t* ast, vec_t* vars) {
	char* block_buff = malloc(sizeof(char));
	*block_buff = '\0';

	vec_t vec = ast->list;

	for (int i = 0; i < vec_length(&vec); ++i) {
		ast_t* ast = vec_get(&vec, i);
		block_buff = strdup(block_buff);
		if (ast->type == AST_ASSIGNMENT) {
			char* comp_buff = asm_assignment(ast, vars);
			block_buff = realloc(block_buff, strlen(block_buff) + strlen(comp_buff) + 1);
			strcat(block_buff, comp_buff);
		} else if (ast->type == AST_RETURN) {
			char* comp_buff = asm_return(ast, vars);
			block_buff = realloc(block_buff, strlen(block_buff) + strlen(comp_buff) + 1);
			strcat(block_buff, comp_buff);
		} else if (ast->type == AST_CALL) {
			char* comp_buff = asm_call(ast, vars);
			block_buff = realloc(block_buff, strlen(block_buff) + strlen(comp_buff) + 1);
			strcat(block_buff, comp_buff);
		} else if (ast->type == AST_BLOCK) {
			char* comp_buff = asm_block(ast, vars);
			block_buff = realloc(block_buff, strlen(block_buff) + strlen(comp_buff) + 1);
			strcat(block_buff, comp_buff);
		}
	}

	return block_buff;
}

char* asm_function(ast_t* ast) {
	char* func_buff = malloc(sizeof(char));
	*func_buff = '\0';

	char* template = "%s:\n\tpushl %%ebp\n\tmovl %%esp, %%ebp\n";
	func_buff = realloc(func_buff, strlen(func_buff) + strlen(template) + strlen(ast->name) + 1);
	snprintf(func_buff, strlen(template) + strlen(ast->name), template, ast->name);

	vec_t vars;
	vec_init(&vars, 4);

	if (ast->value != NULL) {
		ast_t* value = ast->value;
		switch(value->type) {
			case AST_BLOCK: {
				char* block_buff = asm_block(value, &vars);
				func_buff = realloc(func_buff, strlen(func_buff) + strlen(block_buff) + 1);
				strcat(func_buff, block_buff);
			}
		}
	}

	if (strstr(func_buff, "ret") == 0) {
		func_buff = realloc(func_buff, strlen(func_buff) + strlen("\tpopl %ebp\n\tret\n") + 1);
		strcat(func_buff, "\tpopl %ebp\n\tret\n");
	}

	return func_buff;
}

void asm_init(vec_t asts) {
	char* template = ".section .text\n"
					"\t.global _start\n"
					"_start:\n"
					"\tcall main\n"
					"\tmovl %eax, %ebx\n"
					"\tmovl $1, %eax\n"
					"\tint $0x80\n";
	char* src = (char*)malloc(strlen(template) + 1);
	strcpy(src, template);

	vec_init(&strings, 4);
	lc_strings = calloc(1, sizeof(char));

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

	src = realloc(src, strlen(src) + strlen(lc_strings) + 1);
	strcat(src, lc_strings);

	printf("%s\n", src);
	write_file(src, "bin/assembly.asm");
}
