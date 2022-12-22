#include <stdarg.h>

#include "gen.h"

/* Increment a static counter. Returns integer that is used
   for if-statements and loops. */

static int count() {
	static int c = 0;
	return c++;
}

/* Writes string to the output file descriptor. */

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
			if (node->array_len) {
				switch (node->array_len->kind) {
					case ND_NUM:
						if (node->var->is_param) {
							emit("	movl %d(%%ebp), %%eax", node->var->offset);
							emit("	addl $%d, %%eax", strtoul(node->array_len->val, NULL, 10) * 4);
							emit("	movl (%%eax), %%eax");
						} else {
							emit("	movl $%s, %%eax", node->array_len->val, node->var->offset + 4);
							emit("	movl %d(%%ebp, %%eax, 4), %%eax", node->var->offset + 4);
						}
						break;
					case ND_VAR:
						if (node->var->is_param) {
							emit("	movl %d(%%ebp), %%eax", node->var->offset);
							emit("	movl %d(%%ebp), %%ecx", node->array_len->var->offset);
							emit("	imul $4, %%ecx");
							emit("	addl %%ecx, %%eax");
							emit("	movl (%%eax), %%eax");
						} else {
							emit("	movl %d(%%ebp), %%eax", node->array_len->var->offset);
							emit("	movl %d(%%ebp, %%eax, 4), %%eax", node->var->offset + 4);
						}
						break;
					default:
						break;
				}
				emit("	pushl %%eax");
			} else if (node->var->is_array) {
				emit("	leal %d(%%ebp), %%eax", node->var->offset + 4);
				emit("	movl %%eax, %%edi");
				emit("	pushl %%eax");
			} else {
				emit("	pushl %d(%%ebp)", node->var->offset);
			}

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

	/* Reverse the order the arguments are pushed, but only
	   for builtin functions such as the printf function */
	if (node->lhs->var->is_builtin) {
		node_t* arg = node->args;
        node_t* prev = NULL;

        while (arg) {
            node_t* temp = arg->next;
            arg->next = prev;
            prev = arg;
            arg = temp;
            args++;
        }
        node->args = prev;
	} else {
		for (node_t* arg = node->args; arg; arg = arg->next) {
			args++;
		}
	}

	push_args2(node->args);
	return args;
}

/* Use a specific conditional jump depending on
   the type of the condition.

   conditional-jumps:
	   jne: ==
       je: !=
       jng: <
       jnge: <=
       jnl: >
       jnle: >= */

static void emit_cond_jmp(node_t* node, char* section, int c) {
	if (node->kind == ND_EQ) {
		emit("	jne .L.%s.%d", section, c);
	} else if (node->kind == ND_NOT_EQ) {
		emit("	je .L.%s.%d", section, c);
	} else if (node->kind == ND_LESS) {
		emit("	jng .L.%s.%d", section, c);
	} else if (node->kind == ND_LESS_EQ) {
		emit("	jnge .L.%s.%d", section, c);
	} else if (node->kind == ND_GREATER) {
		emit("	jnl .L.%s.%d", section, c);
	} else if (node->kind == ND_GREATER_EQ) {
		emit("	jnle .L.%s.%d", section, c);
	}
}

static void emit_stmt(node_t* node) {
	int args = 0;
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
			args = push_args(node->rhs);
			emit("	call %s", node->rhs->lhs->var->name);
			args > 0 ? emit("	add $%d, %%esp", args * 4) : NULL;
			emit("	movl %%eax, %d(%%ebp)", node->lhs->var->offset);
			break;
		case ND_CALL:
			args = push_args(node);
			if (node->lhs->var->is_builtin) {
				emit("	call _%s", node->lhs->var->name);
			} else {
				emit("	call %s", node->lhs->var->name);
			}
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
					if (node->lhs->array_len) {
						switch (node->lhs->array_len->kind) {
							case ND_NUM:
								if (node->lhs->var->is_param) {
									emit("	movl %d(%%ebp), %%eax", node->lhs->var->offset);
									emit("	addl $%d, %%eax", strtoul(node->lhs->array_len->val, NULL, 10) * 4);
									emit("	movl (%%eax), %%eax");
								} else {
									emit("	movl $%s, %%eax", node->lhs->array_len->val, node->lhs->var->offset + 4);
									emit("	movl %d(%%ebp, %%eax, 4), %%eax", node->lhs->var->offset + 4);
								}
								break;
							case ND_VAR:
								if (node->lhs->var->is_param) {
									emit("	movl %d(%%ebp), %%eax", node->lhs->var->offset);
									emit("	movl %d(%%ebp), %%ecx", node->lhs->array_len->var->offset);
									emit("	imul $4, %%ecx");
									emit("	addl %%ecx, %%eax");
									emit("	movl (%%eax), %%eax");
								} else {
									emit("	movl %d(%%ebp), %%eax", node->lhs->array_len->var->offset);
									emit("	movl %d(%%ebp, %%eax, 4), %%eax", node->lhs->var->offset + 4);
								}
								break;
							default:
								break;
						}
					} else {
						emit("	movl %d(%%ebp), %%eax", node->lhs->var->offset);
					}
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

static void emit_array_elem(node_t* node) {
	if (node->lhs->var->is_param) {
		emit("	leal 0(, %%eax, 4), %%edx");
		emit("	movl %d(%%ebp), %%eax", node->lhs->var->offset);

		if (node->lhs->array_len->var) {
			emit("	movl %d(%%ebp), %%ecx", node->lhs->array_len->var->offset);
		} else {
			emit("	movl $%s, %%ecx", node->lhs->array_len->val);
		}
		emit("	imul $4, %%ecx");

		emit("	addl %%ecx, %%eax");
		if (node->rhs->var->init_data) {
			emit("	movl $%s, %%ecx", node->rhs->var->name);
			emit("	movl %%ecx, (%%eax)");
		} else {
			emit("	movl %d(%%ebp), %%ecx", node->rhs->var->offset);
			emit("	movl %%ecx, (%%eax)");
		}
	} else if (node->rhs->var->is_param) {
		emit("	movl %d(%%ebp), %%eax", node->rhs->var->offset);
		if (node->rhs->array_len->var) {
			emit("	movl %d(%%ebp), %%ecx", node->rhs->array_len->var->offset);
		} else {
			emit("	movl $%s, %%ecx", node->rhs->array_len->val);
		}
		emit("	imul $4, %%ecx");

		emit("	addl %%ecx, %%eax");
		emit("	movl (%%eax), %%eax");
		emit("	movl %%eax, %d(%%ebp)", node->lhs->var->offset);
	} else {
		if (node->rhs->array_len) {
			emit("	movl %d(%%ebp, %%eax, 4), %%ecx", node->rhs->var->offset + 4);
			emit("	movl %%ecx, %d(%%ebp)", node->lhs->var->offset);
		} else if (node->rhs->var->init_data) {
			emit("	movl $%s, %%ecx", node->rhs->var->name);
			emit("	movl %%ecx, %d(%%ebp, %%eax, 4)", node->lhs->var->offset + 4);
		} else {
			emit("	movl %d(%%ebp), %%ecx", node->rhs->var->offset);
			emit("	movl %%ecx, %d(%%ebp, %%eax, 4)", node->lhs->var->offset + 4);
		}
	}
}

static void emit_array(node_t* node) {
	for (node_t* child = node->children; child; child = child->next) {
		switch (child->kind) {
			case ND_NUM:
				emit("	movl $%s, %d(%%ebp)", child->val, child->var->offset);
				break;
			case ND_VAR:
				if (child->var->init_data) {
					emit("	movl $%s, %d(%%ebp)", child->var->name, child->var->offset);
				}
				break;
			default:
				break;
		}
	}
}

static void emit_expr(node_t* node) {
	switch(node->kind) {
		case ND_NULL_EXPR:
			break;
		case ND_ASSIGN:
			if (node->lhs->array_len) {
				switch (node->lhs->array_len->kind) {
					case ND_NUM:
						if (node->lhs->var->is_param) {
							emit("	movl $%s, %%ecx", node->lhs->array_len->val);
						} else {
							emit("	movl $%s, %%eax", node->lhs->array_len->val);
						}
						break;
					case ND_VAR:
						if (node->lhs->var->is_param) {
							emit("	movl %d(%%ebp), %%ecx", node->lhs->array_len->var->offset);
						} else {
							emit("	movl %d(%%ebp), %%eax", node->lhs->array_len->var->offset);
						}
						break;
					default:
						break;
				}
			} else if (node->rhs->array_len) {
				switch (node->rhs->array_len->kind) {
					case ND_NUM:
						if (node->rhs->var->is_param) {
							emit("	movl $%s, %%ecx", node->rhs->array_len->val);
						} else {
							emit("	movl $%s, %%eax", node->rhs->array_len->val);
						}
						break;
					case ND_VAR:
						if (node->rhs->var->is_param) {
							emit("	movl %d(%%ebp), %%ecx", node->rhs->array_len->var->offset);
						} else {
							emit("	movl %d(%%ebp), %%eax", node->rhs->array_len->var->offset);
						}
						break;
					default:
						break;
				}
			}

			switch (node->rhs->kind) {
				case ND_NUM:
					if (node->lhs->array_len) {
						if (node->lhs->var->is_param) {
							emit("	movl %d(%%ebp), %%eax", node->lhs->var->offset);
							if (node->lhs->array_len->var) {
								emit("	movl %d(%%ebp), %%ecx", node->lhs->array_len->var->offset);
							} else {
								emit("	movl $%s, %%ecx", node->lhs->array_len->val);
							}
							emit("	imul $4, %%ecx");
							emit("	addl %%ecx, %%eax");
							emit("	movl $%s, (%%eax)", node->rhs->val);
						} else {
							emit("	movl $%s, %d(%%ebp, %%eax, 4)", node->rhs->val, node->lhs->var->offset + 4);
						}
					} else {
						emit("	movl $%s, %d(%%ebp)", node->rhs->val, node->lhs->var->offset);
					}
					break;
				case ND_VAR:
					if (node->lhs->array_len || node->rhs->array_len) {
						emit_array_elem(node);
					} else if (node->rhs->var->init_data) {
						emit("	movl $%s, %d(%%ebp)", node->rhs->var->name, node->lhs->var->offset);
					} else {
						emit("	movl %d(%%ebp), %%eax", node->rhs->var->offset);
						emit("	movl %%eax, %d(%%ebp)", node->lhs->var->offset);
					}
					break;
				case ND_ARRAY:
					emit_array(node->rhs);
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
				case ND_EQ:
				case ND_NOT_EQ:
					emit("	mov %%al, %d(%%ebp)", node->lhs->var->offset);
					break;
				case ND_LESS:
				case ND_LESS_EQ:
				case ND_GREATER:
				case ND_GREATER_EQ:
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
			switch(node->rhs->kind) {
				case ND_NUM:
					emit("	pushl $%s", node->rhs->val);
					break;
				case ND_VAR:
					emit("	pushl %d(%%ebp)", node->rhs->var->offset);
					break;
				default:
					emit_expr(node->rhs);
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
				default:
					emit_expr(node->lhs);
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
		case ND_EQ:
		case ND_NOT_EQ:
		case ND_LESS:
		case ND_LESS_EQ:
		case ND_GREATER:
		case ND_GREATER_EQ:
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

			if (node->kind == ND_EQ) {
				emit("	sete %%al");
			} else if (node->kind == ND_NOT_EQ) {
				emit("	setne %%al");
			} else if (node->kind == ND_LESS) {
				emit("	setg %%al");
			} else if (node->kind == ND_LESS_EQ) {
				emit("	setge %%al");
			} else if (node->kind == ND_GREATER) {
				emit("	setl %%al");
			} else if (node->kind == ND_GREATER_EQ) {
				emit("	setle %%al");
			}
			break;
	}
	return;
}

/* Assigns an offset to local variables including function
   parameters. */

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


	#ifdef _WIN32
		emit("	call _exit");
	#endif

	#ifdef linux
		emit("	int $0x80");
	#endif

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
