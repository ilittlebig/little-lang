#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "gen.h"

static void emit(char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vfprintf(outputfp, fmt, args);
	va_end(args);
	fprintf(outputfp, "\n");
}

static char* make_label() {
	static int c = 0;
	char* buff = calloc(4, sizeof(char));
	sprintf(buff, "LC%d", c++);
	return buff;
}

void emit_literal(ast_t* literal) {
	if (!literal->label) {
		literal->label = make_label();
		emit(".section .data");
		emit("%s:", literal->label);
		emit("	.string \"%s\"", literal->value);
		emit(".section .text");
	}
}

void emit_lvar(ast_t* expr) {
	emit("	movl %d(%%ebp), %d(%%ebp)", expr->arg_offset, expr->offset);
}

void emit_number(ast_t* expr) {
	emit("	movl $%s, %d(%%ebp)", expr->value, expr->offset);
}

void emit_call(ast_t* expr) {
	for (int i = vec_length(&expr->list) - 1; i >= 0; --i) {
		ast_t* arg = vec_get(&expr->list, i);
		if (arg->type == AST_INT) {
			emit("	pushl $%s", arg->value);
			free(arg->value);
		} else if (arg->type == AST_STRING) {
			emit_literal(arg);
			emit("	pushl $%s", arg->label);
			free(arg->value);
			free(arg->label);
		} else if (arg->type == AST_IDENTIFIER) {
			emit("	pushl %d(%%ebp)", arg->arg_offset);
			free(arg->name);
		}
		free(arg);
	}
	emit("	call %s", expr->name);
	emit("	add $%d, %%esp", vec_length(&expr->list)*4);

	if (expr->type == AST_FUNCALL) {
		emit("	movl %%eax, %d(%%ebp)", expr->offset);
	}

	free(expr->name);
	free(expr);
	vec_free(&expr->list);
}

void emit_return(ast_t* expr) {
	ast_t* ret = expr->value;
	if (ret->type == AST_INT) {
		emit("	movl $%s, %%eax", ret->value);
		free(ret->value);
	} else if (ret->type == AST_STRING) {
		emit_literal(ret);
		emit("	movl $%s, %%eax", ret->label);
		free(ret->value);
		free(ret->label);
	} else if (ret->type == AST_IDENTIFIER) {
		emit("	movl %d(%%ebp), %%eax", ret->offset);
		free(ret->name);
	}

	free(ret);
	free(expr);
}

void emit_defvar(ast_t* expr) {
	// TODO: free memory allocated by 'defvar' if
	// there's no funcall to clear the memory
}

void emit_funcall(ast_t* expr) {
	expr->name = expr->value;
	emit_call(expr);

	ast_t* defvar = expr->defvar;
	if (defvar) {
		free(defvar->name);
		free(defvar);
	}
}

void emit_assignment(ast_t* expr) {
	ast_t* expr_value = expr->value;
	if (expr_value->type == AST_INT) {
		emit("	movl $%s, %d(%%ebp)", expr_value->value, expr->offset);
		free(expr_value->value);
	} else if (expr_value->type == AST_STRING) {
		emit_literal(expr_value);
		emit("	movl $%s, %d(%%ebp)", expr_value->label, expr->offset);
		free(expr_value->value);
		free(expr_value->label);
	} else if (expr_value->type == AST_IDENTIFIER) {
		emit("	movl %d(%%ebp), %%eax", expr_value->offset);
		emit("	movl %%eax, %d(%%ebp)", expr->offset);
		free(expr_value->name);
	}

	free(expr_value);
	free(expr->name);
	free(expr);
}

void emit_body(ast_t* body) {
	for (int i = 0; i < vec_length(&body->list); i++) {
		ast_t* expr = vec_get(&body->list, i);
		emit_expr(expr);
	}
}

void emit_stmt(ast_t* stmt) {
	switch(stmt->type) {
		case AST_BLOCK: emit_body(stmt); return;
		default:
			return;
	}
}

void emit_expr(ast_t* expr) {
	switch(expr->type) {
		case AST_IDENTIFIER: emit_lvar(expr);		return;
		case AST_INT:		 emit_number(expr);		return;
		case AST_CALL:		 emit_call(expr);		return;
		case AST_STRING:	 emit_literal(expr);	return;
		case AST_RETURN:	 emit_return(expr);		return;
		case AST_DEFVAR:	 emit_defvar(expr);		return;
		case AST_FUNCALL:	 emit_funcall(expr);	return;
		case AST_ASSIGNMENT: emit_assignment(expr);	return;
		default:
			return;
	}
}

void gen_asm(vec_t asts) {
	outputfp = fopen("bin/assembly.asm", "w");

	emit(".section .text");
	emit("	.global _start");
	emit("_start:");
	emit("	call main");
	emit("	movl %%eax, %%ebx");
	emit("	movl $1, %%eax");
	emit("	int $0x80");

	for (int i = 0; i < vec_length(&asts); ++i) {
		ast_t* func = vec_get(&asts, i);
		if (func->type != AST_FUNCTION) {
			continue;
		}

		int vars = 0;
		for (ast_t* var = func->vars; var; var = var->next) {
			vars++;
		}

		emit("%s:", func->name);
		emit("	pushl %%ebp");
		emit("	movl %%esp, %%ebp");

		if (vars > 0) {
			emit("	sub $%d, %%esp", vars * 4);
		}

		ast_t* body = func->body;
		emit_stmt(body);

		emit("	leave");
		emit("	ret");

		for (ast_t* var = func->params; var; var = var->next) {
			free(var->name);
			free(var);
		}

		vec_free(&body->list);

		free(body);
		free(func->name);
		free(func);
	}

	fclose(outputfp);
}
