#include "typer.h"
#include "error.h"

/* Match two token types. The type specifier and the expression is
   almost always not the same type, unless you're comparing variables.
   That's why there are multiple cases. */

static int match_types(token_type_t type1, token_type_t type2) {
	if (type1 == type2) {
		return 1;
	}
	if (type1 == STRING && type2 != STRING_LITERAL) {
		return 0;
	} else if (type1 == INT && type2 != NUMBER) {
		return 0;
	}
	return 1;
}

/* Match arguments passed to function. Throws warning in two cases,
   passed too few arguments to function, and passed too many arguments
   to function. */

static void match_function_arguments(obj_t* fn, node_t* var) {
	int params = 0, args = 0;
	for (obj_t* param = fn->params; param; param = param->next) {
		params++;
	}
	for (node_t* arg = var->args; arg; arg = arg->next) {
		// Match arithmetic operators in case one was passed as an argument.
		match_arithmetic_operators(arg);
		args++;
	}

	if (args < params && !fn->is_builtin) {
		error_at(var->token, "too few arguments to function '%s'", fn->name);
	} else if (args > params && !fn->is_builtin) {
		error_at(var->token, "too many arguments to function '%s'", fn->name);
	}
}

/* Match return buffer. Throws warning in two cases,
   returning with value when function is declared void and when
   the return type and the function type mismatches. */

void match_function_return_buffer(node_t* return_buffer, obj_t* fn) {
	token_type_t return_type = return_buffer->var ? return_buffer->var->type : return_buffer->type;
	if (fn->func_type == VOID && (return_buffer->var || return_buffer->val)) {
		warning_at(return_buffer->token, "'return' with a value, in function returning void");
	}

	if (!match_types(fn->func_type, return_type)) {
		warning_at(return_buffer->token,
			"returning '%s' from a function with return type '%s'",
			token_to_str(return_type), token_to_str(fn->func_type));
	}
}

/* Validate arithmetic operation. Throws warning in case of
   an invalid operand type. */

void match_arithmetic_operators(node_t* op) {
	switch (op->kind) {
		case ND_ADD:
		case ND_SUB:
		case ND_MUL:
		case ND_DIV:
		case ND_MOD:
			match_arithmetic_operators(op->lhs);
			match_arithmetic_operators(op->rhs);

			token_type_t left_type =
				op->lhs->var ? op->lhs->var->type : op->lhs->token->type,
				right_type = op->rhs->var ? op->rhs->var->type : op->rhs->token->type;

			if (left_type == NUMBER && right_type == LEFT_PAREN) {
				break;
			} else if (left_type == LEFT_PAREN && right_type == NUMBER) {
				break;
			} else if (left_type == LEFT_PAREN && right_type == LEFT_PAREN) {
				break;
			}

			if (!match_types(INT, left_type) || !match_types(INT, right_type)) {
				warning_at(op->lhs->token,
					"unsupported operand type(s) '%s' and '%s'",
					token_to_str(left_type), token_to_str(right_type));
			}
			break;
		default:
			break;
	}
}

/* Match variable type specifier and expression. Throws warning
   when the types mismatch. */

static void match_variable_type(node_t* var) {
	switch (var->rhs->kind) {
		case ND_ADD:
		case ND_SUB:
		case ND_MUL:
		case ND_DIV:
		case ND_MOD:
			match_arithmetic_operators(var->rhs);
			return;
	}

	token_type_t right_type = var->rhs->var ? var->rhs->var->type : var->rhs->token->type;
	if (!match_types(var->lhs->var->type, right_type)) {
		warning_at(var->token, "initialization of '%s' from '%s'",
			token_to_str(var->lhs->var->type), token_to_str(right_type));
	}
}

void check_body(node_t* node) {
	for (node_t* n = node->body; n; n = n->next) {
		switch(n->kind) {
			case ND_CALL:
				if (!n->lhs || !n->lhs->var) {
					break;
				}
				match_function_arguments(n->lhs->var, n);
				break;
			case ND_BLOCK:
				if (n->body && n->body->lhs->kind == ND_ASSIGN) {
					match_variable_type(n->body->lhs);
				}
				break;
			default:
				break;
		}
	}
}

void type_check_program(obj_t* globals) {
	for (obj_t* fn = globals; fn; fn = fn->next) {
		if (!fn->is_function || !fn->is_definition) {
			continue;
		}

		check_body(fn->body);
	}
}
