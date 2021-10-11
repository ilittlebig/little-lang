#include <stdarg.h>

#include "gen.h"

static int count() {
	static int c = 0;
	return c++;
}

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
		case ND_ADD:
		case ND_SUB:
		case ND_MUL:
		case ND_DIV:
		case ND_MOD:
			emit_expr(node);
			emit("	pushl %%ecx");
			break;
		default:
			emit_expr(node);
			emit("	push %%ax");
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

static void emit_cond_jmp(node_t* node, char* section, int c) {
	if (node->kind == ND_EQUAL) {
		emit("	jne .L.%s.%d", section, c);
	} else if (node->kind == ND_NOT_EQUAL) {
		emit("	je .L.%s.%d", section, c);
	} else if (node->kind == ND_LESS) {
		emit("	jng .L.%s.%d", section, c);
	} else if (node->kind == ND_LESS_EQUAL) {
		emit("	jnge .L.%s.%d", section, c);
	} else if (node->kind == ND_GREATER) {
		emit("	jnl .L.%s.%d", section, c);
	} else if (node->kind == ND_GREATER_EQUAL) {
		emit("	jnle .L.%s.%d", section, c);
	}
}

static void emit_stmt(node_t* node) {
	switch(node->kind) {
		case ND_BLOCK:
			for (node_t* n = node->body; n; n = n->next) {
				emit_stmt(n);
			}
			break;
		case ND_EXPR:
			emit_expr(node->lhs);
			break;
		case ND_DEFVAR:
			int args = push_args(node->rhs);
			emit("	call %s", node->rhs->lhs->var->name);
			args > 0 ? emit("	add $%d, %%esp", args * 4) : NULL;
			emit("	movl %%eax, %d(%%ebp)", node->lhs->var->offset);
			break;
		case ND_CALL:
			args = push_args(node);
			emit("	call %s", node->lhs->var->name);
			args > 0 ? emit("	add $%d, %%esp", args * 4) : NULL;
			break;
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
				default:
					emit_expr(node->lhs);
					break;
			}
			break;
		case ND_IF: {
			int c = count();

			emit_expr(node->cond);
			emit_cond_jmp(node->cond, "else", c);
			emit_stmt(node->then);
			emit("	jmp .L.end.%d", c);

			emit(".L.else.%d:", c);
			if (node->els) {
				emit_stmt(node->els);
			}

			emit(".L.end.%d:", c);
			break;
		}
		case ND_WHILE: {
			int c = count();

			emit(".L.begin.%d:", c);
			emit_expr(node->cond);
			emit_cond_jmp(node->cond, "end", c);
			emit_stmt(node->then);

			emit("	jmp .L.begin.%d", c);
			emit(".L.end.%d:", c);
			break;
		}
		case ND_FOR: {
			int c = count();

			emit_stmt(node->init);
			emit(".L.begin.%d:", c);
			emit_expr(node->cond);
			emit_cond_jmp(node->cond, "end", c);
			emit_stmt(node->then);
			emit_expr(node->loop);

			emit("	jmp .L.begin.%d", c);
			emit(".L.end.%d:", c);
			break;
		}
	}
}

static void emit_expr(node_t* node) {
	switch(node->kind) {
		case ND_NULL_EXPR:
			break;
		case ND_ASSIGN:
			switch(node->rhs->kind) {
				case ND_NUM:
					emit("	movl $%s, %d(%%ebp)", node->rhs->val, node->lhs->var->offset);
					break;
				case ND_VAR:
					if (node->rhs->var->init_data) {
						emit("	movl $%s, %d(%%ebp)", node->rhs->var->name, node->lhs->var->offset);
						break;
					}
					emit("	movl %d(%%ebp), %%eax", node->rhs->var->offset);
					emit("	movl %%eax, %d(%%ebp)", node->lhs->var->offset);
					break;
				default:
					emit_expr(node->rhs);
					break;
			}

			switch(node->rhs->kind) {
				case ND_ADD:
				case ND_SUB:
				case ND_MUL:
					emit("	movl %%ecx, %d(%%ebp)", node->lhs->var->offset);
					break;
				case ND_DIV:
				case ND_MOD:
					emit("	movl %%eax, %d(%%ebp)", node->lhs->var->offset);
					break;
				case ND_EQUAL:
				case ND_NOT_EQUAL:
					emit("	mov %%al, %d(%%ebp)", node->lhs->var->offset);
					break;
				case ND_LESS:
				case ND_LESS_EQUAL:
				case ND_GREATER:
				case ND_GREATER_EQUAL:
					emit("	movzbl %%al, %%eax");
					emit("	movl %%eax, %d(%%ebp)", node->lhs->var->offset);
					break;
			}
			return;
		case ND_STMT:
			for (node_t* n = node->body; n; n = n->next) {
				emit_stmt(n);
			}
			break;
		case ND_ADD:
		case ND_SUB:
		case ND_MUL:
		case ND_DIV:
		case ND_MOD:
			emit_expr(node->rhs);
			emit_expr(node->lhs);

			switch(node->rhs->kind) {
				case ND_NUM:
					emit("	pushl $%s", node->rhs->val);
					break;
				case ND_VAR:
					emit("	pushl %d(%%ebp)", node->rhs->var->offset);
					break;
			}
			emit("	movl (%%esp), %%ecx");

			switch(node->lhs->kind) {
				case ND_NUM:
					emit("	pushl $%s", node->lhs->val);
					break;
				case ND_VAR:
					emit("	pushl %d(%%ebp)", node->lhs->var->offset);
					break;
			}
			emit("	movl (%%esp), %%ecx");
			emit("	popl %%eax");

			if (node->kind == ND_ADD) {
				emit("	addl (%%esp), %%eax");
			} else if (node->kind == ND_SUB) {
				emit("	subl (%%esp), %%eax");
			} else if (node->kind == ND_MUL) {
				emit("	imul (%%esp), %%eax");
			} else if (node->kind == ND_DIV) {
				emit("	cdq");
				emit("	idiv (%%esp), %%eax");
			} else if (node->kind == ND_MOD) {
				emit("	cdq");
				emit("	idiv (%%esp), %%eax");
				emit("	movl %%edx, %%eax");
			}

			emit("	addl $4, %%esp");
			emit("	pushl %%eax");
			emit("	movl (%%esp), %%ecx");

			break;
		case ND_EQUAL:
		case ND_NOT_EQUAL:
		case ND_LESS:
		case ND_LESS_EQUAL:
		case ND_GREATER:
		case ND_GREATER_EQUAL:
			switch(node->lhs->kind) {
				case ND_NUM:
					emit("	movl $%s, %%eax", node->lhs->val);
					break;
				case ND_VAR:
					emit("	movl %d(%%ebp), %%eax", node->lhs->var->offset);
					break;
				default:
					emit_expr(node->lhs);
					break;
			}

			switch(node->rhs->kind) {
				case ND_NUM:
					emit("	movl $%s, %%edi", node->rhs->val);
					break;
				case ND_VAR:
					emit("	movl %d(%%ebp), %%edi", node->rhs->var->offset);
					break;
				default:
					emit_expr(node->rhs);
					break;
			}
			emit("	cmpl %%eax, %%edi");

			if (node->kind == ND_EQUAL) {
				emit("	sete %%al");
			} else if (node->kind == ND_NOT_EQUAL) {
				emit("	setne %%al");
			} else if (node->kind == ND_LESS) {
				emit("	setg %%al");
			} else if (node->kind == ND_LESS_EQUAL) {
				emit("	setge %%al");
			} else if (node->kind == ND_GREATER) {
				emit("	setl %%al");
			} else if (node->kind == ND_GREATER_EQUAL) {
				emit("	setle %%al");
			}
			break;
	}
	return;
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
