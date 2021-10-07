#include <stdarg.h>

#include "gen.h"

static void emit(char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vfprintf(outputfp, fmt, args);
	va_end(args);
	fprintf(outputfp, "\n");
}

static void push_args2(node_t* node) {
	if (!node) {
		return;
	}

	switch(node->kind) {
		case ND_NUM:
			emit("	pushl $%s", node->val);
			break;
		case ND_VAR:
			if (node->var->init_data) {
				emit("	pushl $%s", node->var->name);
				break;
			}
			emit("	pushl %d(%%ebp)", node->var->offset);
			break;
	}

	push_args2(node->next);
}

static int push_args(node_t* node) {
	int args = 0;
	for (node_t* arg = node->args; arg; arg = arg->next) {
		args++;
	}
	push_args2(node->args);
	return args;
}

static void emit_stmt(node_t* node) {
	switch(node->kind) {
		case ND_BLOCK:
			for (node_t* n = node->body; n; n = n->next) {
				emit_stmt(n);
			}
			return;
		case ND_EXPR:
			emit_expr(node->lhs);
			return;
		case ND_DEFVAR:
			int args = push_args(node->rhs);
			emit("	call %s", node->rhs->lhs->var->name);
			args > 0 ? emit("	add $%d, %%esp", args * 4) : NULL;
			emit("	movl %%eax, %d(%%ebp)", node->lhs->var->offset);
			return;
		case ND_CALL:
			args = push_args(node);
			emit("	call %s", node->lhs->var->name);
			args > 0 ? emit("	add $%d, %%esp", args * 4) : NULL;
			return;
		case ND_RETURN:
			switch(node->lhs->kind) {
				case ND_NUM:
					emit("	movl $%s, %%eax", node->lhs->val);
					break;
				case ND_VAR:
					if (node->lhs->var->init_data) {
						emit("	movl $%s, %%eax", node->lhs->var->name);
						break;
					}
					emit("	movl %d(%%ebp), %%eax", node->lhs->var->offset);
					break;
			}
			return;
	}
}

static void emit_expr(node_t* node) {
	switch(node->kind) {
		case ND_NULL_EXPR:
			return;
		case ND_ASSIGN:
			switch(node->rhs->kind) {
				case ND_NUM:
					emit("	movl $%s, %d(%%ebp)", node->rhs->val, node->lhs->var->offset);
					return;
				case ND_VAR:
					if (node->rhs->var->init_data) {
						emit("	movl $%s, %d(%%ebp)", node->rhs->var->name, node->lhs->var->offset);
						return;
					}
					emit("	movl %d(%%ebp), %%eax", node->rhs->var->offset);
					emit("	movl %%eax, %d(%%ebp)", node->lhs->var->offset);
					return;
			}
			return;
		case ND_STMT:
			for (node_t* n = node->body; n; n = n->next) {
				emit_stmt(n);
			}
			return;
		case ND_COND:
			return;
	}
}

static void assign_lvar_offsets(obj_t* fn) {
	int stack_size = 0;
	int offset = 8;

	for (obj_t* var = fn->params; var; var = var->next) {
		if (var->offset) {
			continue;
		}
		var->offset = offset;
		offset += 4;
	}

	offset = -4;
	for (obj_t* var = fn->locals; var; var = var->next) {
		if (var->offset) {
			continue;
		}
		stack_size += 4;
		var->offset = offset;
		offset -= 4;
	}

	fn->stack_size = stack_size;
}

void codegen(obj_t* globals) {
	outputfp = fopen("bin/assembly.asm", "w");

	emit(".section .text");
	emit("	.global _start");
	emit("_start:");
	emit("	call main");
	emit("	movl %%eax, %%ebx");
	emit("	movl $1, %%eax");
	emit("	int $0x80");

	for (obj_t* var = globals; var; var = var->next) {
		if (var->is_function || !var->is_definition) {
			continue;
		}

		if (var->init_data) {
			emit(".section .data");
			emit("%s:", var->name);
			emit("	.string \"%s\"", var->init_data);
			emit(".section .text");
		}
	}

	for (obj_t* fn = globals; fn; fn = fn->next) {
		if (!fn->is_function || !fn->is_definition) {
			continue;
		}
		assign_lvar_offsets(fn);

		emit("%s:", fn->name);
		emit("	pushl %%ebp");
		emit("	movl %%esp, %%ebp");
		emit("	sub $%d, %%esp", fn->stack_size);

		emit_stmt(fn->body);

		emit("	leave");
		emit("	ret");
	}
	fclose(outputfp);
}
