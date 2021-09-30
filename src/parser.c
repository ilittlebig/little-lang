#include "parser.h"
#include "error.h"
#include "file.h"
#include "gen.h"

token_t* peek_token(parser_t* parser) {
	token_t* token = vec_get(&parser->tokens, parser->tokens_parsed);
	if (token != NULL) {
		parser->head = token;
		return parser->head;
	}
}

void advance_token(parser_t* parser) {
	token_t* token = vec_get(&parser->tokens, parser->tokens_parsed);
	if (token) {
		parser->tokens_parsed++;
	}
}

void advance_token_type(parser_t* parser, token_type_t token_type) {
	token_t* token = peek_token(parser);
	if (token && token->type == token_type) {
		parser->tokens_parsed++;
	} else {
		advance_token(parser);
		go_error_at(parser->location, "token '%s' was expecting '%s'", token_to_str(token->type), token_to_str(token_type));
	}
}

ast_t* find_var(const char* varname) {
	if (!current_func) { return NULL; }
	// TODO: fix better solution for params
	for (ast_t* var = current_func->params; var; var = var->next) {
		if (strcmp(var->name, varname) == 0) {
			return var;
		}
	}
	for (ast_t* var = current_func->vars; var; var = var->next) {
		if (strcmp(var->name, varname) == 0) {
			return var;
		}
	}
	return NULL;
}

ast_t* parse_func(parser_t* parser) {
	advance_token_type(parser, FN);

	ast_t* identifier = parse_identifier(parser);

	ast_t* func = init_ast(AST_FUNCTION);
	func->name = identifier->name;
	current_func = func;

	locals = NULL;
	parse_params(parser);
	func->params = locals;

	advance_token_type(parser, COLON);
	func->type_specifier = peek_token(parser)->type;
	advance_token_type(parser, func->type_specifier);

	locals = NULL;
	func->body = parse_body(parser);
	func->vars = locals;

	free(identifier);
	return func;
}

void parse_params(parser_t* parser) {
	advance_token_type(parser, LEFT_PAREN);

	int offset = 8;
	while (peek_token(parser)->type != RIGHT_PAREN) {
		ast_t* param = init_ast(AST_IDENTIFIER);
		param->next = locals;
		param->offset = offset;
		locals = param;
		offset += 4;

		param->type_specifier = peek_token(parser)->type;
		advance_token(parser);

		param->name = peek_token(parser)->value;
		advance_token(parser);

		if (peek_token(parser)->type != RIGHT_PAREN) {
			advance_token_type(parser, COMMA);
		}
	}
	advance_token_type(parser, RIGHT_PAREN);
}

ast_t* parse_body(parser_t* parser) {
	ast_t* body = init_ast(AST_BLOCK);
	vec_init(&body->list, 1);

	advance_token_type(parser, LEFT_CURLY);

	int offset = -4;
	while (peek_token(parser)->type != END_OF_FILE && peek_token(parser)->type != RIGHT_CURLY) {
		ast_t* expr = parse_expr(parser);
		if (!expr) { continue; }
		if (expr->type == AST_ASSIGNMENT || expr->type == AST_DEFVAR) {
			ast_t* var = find_var(expr->name);
			if (var) {
				expr->offset = var->offset;
			} else {
				expr->offset = offset;
				offset -= 4;
			}
			expr->next = locals;
			locals = expr;
		}
		current_func->vars = locals; // TODO: fix hacky solution
		vec_push_back(&body->list, expr);
	}

	advance_token_type(parser, RIGHT_CURLY);
	return body;
}

void push_args(parser_t* parser, ast_t* compound) {
	if (peek_token(parser)->type != RIGHT_PAREN) {
		vec_init(&compound->list, 1);
	}

	int offset = 8;
	while (peek_token(parser)->type != RIGHT_PAREN) {
		ast_t* arg = parse_expr(parser);
		if (arg->type == AST_IDENTIFIER && current_func) {
			ast_t* ast = find_var(arg->name);
			if (ast) {
				// TODO: fix a better solution for params
				if (ast->value) {
					ast_t* var = ast->value;
					arg->value = var->value;
				}
				arg->arg_offset = ast->offset;
				arg->type_specifier = ast->type_specifier;
			} else {
				go_error_at(parser->location, "undefined reference to '%s'", arg->name);
			}
		}
		arg->offset = offset;
		offset += 4;

		vec_push_back(&compound->list, arg);
	}
}

ast_t* parse_return(parser_t* parser) {
	ast_t* ret = init_ast(AST_RETURN);
	advance_token_type(parser, RETURN);

	if (peek_token(parser)->type != RIGHT_PAREN) {
		ast_t* expr = parse_expr(parser);
		if (expr->type == AST_IDENTIFIER) {
			ast_t* var = find_var(expr->name);
			if (var) {
				expr->offset = var->offset;
			} else {
				go_error_at(parser->location, "undefined reference to '%s'", expr->name);
			}
		}
		ret->value = expr;
	}

	advance_token_type(parser, RIGHT_PAREN);
	return ret;
}

ast_t* parse_defvar(parser_t* parser) {
	ast_t* defvar = init_ast(AST_DEFVAR);
	advance_token_type(parser, DEFVAR);

	ast_t* var = parse_identifier(parser);
	defvar->name = var->name;
	ast_t* func = parse_identifier(parser);
	defvar->value = func->name;

	free(var);
	free(func);

	advance_token_type(parser, RIGHT_PAREN);
	return defvar;
}

ast_t* parse_funcall(parser_t* parser) {
	ast_t* funcall = init_ast(AST_FUNCALL);
	advance_token_type(parser, FUNCALL);

	ast_t* identifier = parse_identifier(parser);
	funcall->name = identifier->name;
	push_args(parser, funcall);

	ast_t* defvar = find_var(funcall->name);
	free(funcall->name);

	if (defvar) {
		funcall->name = defvar->value;
		funcall->value = defvar->value;
		funcall->offset = defvar->offset;
		funcall->defvar = defvar;
	} else {
		go_error_at(parser->location, "undefined reference to '%s'", funcall->name);
	}

	free(identifier);

	advance_token_type(parser, RIGHT_PAREN);
	return funcall;
}

ast_t* parse_stmt(parser_t* parser) {
	token_t* token = peek_token(parser);

	if (token->type == IF) {
		ast_t* ast = init_ast(AST_IF);
		advance_token_type(parser, IF);
		advance_token_type(parser, LEFT_PAREN);
		ast->cond = parse_cond(parser);
		advance_token_type(parser, RIGHT_PAREN);
		ast->body = parse_body(parser);

		token_t* token = peek_token(parser);
		if (token->type == ELSE) {
			advance_token_type(parser, ELSE);
			ast->els = parse_body(parser);
		}
		return ast;
	}

	return NULL;
}

static void set_cond(parser_t* parser, ast_t* lhs) {
	token_t* token = peek_token(parser);
	if (token->type == IDENTIFIER) {
		ast_t* var = find_var(token->value);
		if (var) {
			ast_t* val = var->value;
			lhs->offset = var->offset;
			lhs->value = val->value;
		} else {
			go_error_at(parser->location, "undefined reference to '%s'", token->value);
		}
		lhs->value = token->value;
	} else {
		lhs->value = token->value;
	}
	lhs->type_specifier = token->type;
}

ast_t* parse_cond(parser_t* parser) {
	ast_t* cond = init_ast(AST_COND);

	ast_t* lhs = init_ast(AST_COND);
	set_cond(parser, lhs);
	advance_token(parser);

	if (peek_token(parser)->type == EQUAL) {
		advance_token(parser);
		ast_t* rhs = init_ast(AST_COND);
		set_cond(parser, rhs);
		advance_token(parser);
		cond->lhs = lhs;
		cond->rhs = rhs;
	}

	return cond;
}

ast_t* parse_identifier(parser_t* parser) {
	token_t* token = peek_token(parser);

	ast_t* identifier = init_ast(AST_IDENTIFIER);
	identifier->name = token->value;
	advance_token_type(parser, IDENTIFIER);

	return identifier;
}

ast_t* parse_string(parser_t* parser) {
	token_t* token = peek_token(parser);

	ast_t* string = init_ast(AST_STRING);
	string->value = token->value;
	string->type_specifier = STRING;
	advance_token_type(parser, STRING_LITERAL);

	return string;
}

ast_t* parse_int(parser_t* parser) {
	token_t* token = peek_token(parser);

	ast_t* number = init_ast(AST_INT);
	number->value = token->value;
	number->type_specifier = INT;
	advance_token_type(parser, INT_NUMBER);

	return number;
}

ast_t* parse_compound(parser_t* parser) {
	ast_t* compound = init_ast(AST_COMPOUND);
	advance_token_type(parser, LEFT_PAREN);

	token_t* token = peek_token(parser);
	if (token->type == ASSIGN) {
		advance_token_type(parser, ASSIGN);
		compound->type = AST_ASSIGNMENT;

		ast_t* identifier = parse_expr(parser);
		if (identifier != NULL && identifier->type == AST_IDENTIFIER) {
			compound->name = identifier->name;
		} else {
			go_error_at(parser->location, "expected identifier after '=' token");
		}
		free(identifier);

		ast_t* expr = parse_expr(parser);
		if (expr) {
			if (expr->type == AST_IDENTIFIER) {
				ast_t* var = find_var(expr->name);
				if (var) {
					expr->offset = var->offset;
				} else {
					go_error_at(parser->location, "undefined reference to '%s'", expr->name);
				}
			}
			compound->value = expr;
		} else {
			go_error_at(parser->location, "expected compound expression");
		}

		if (peek_token(parser)->type != RIGHT_PAREN) {
			go_error_at(parser->location, "expected ')' to close compound expression");
		}
	} else if (token->type == IDENTIFIER) {
		ast_t* identifier = parse_identifier(parser);
		compound->name = identifier->name;
		compound->type = AST_CALL;

		push_args(parser, compound);

		free(identifier);
	} else if (token->type == RETURN) {
		free(compound);
		return parse_return(parser);
	} else if (token->type == DEFVAR) {
		free(compound);
		return parse_defvar(parser);
	} else if (token->type == FUNCALL) {
		free(compound);
		return parse_funcall(parser);
	}

	advance_token_type(parser, RIGHT_PAREN);

	if (compound->type == AST_COMPOUND) {
		go_warning_at(parser->location, "compound declaration is empty\n");
	}

	return compound;
}

ast_t* parse_expr(parser_t* parser) {
	token_t* token = peek_token(parser);

	switch(token->type) {
		case FN:		     return parse_func(parser);
		case IF:		     return parse_stmt(parser);
		case IDENTIFIER:	 return parse_identifier(parser);
		case STRING_LITERAL: return parse_string(parser);
		case INT_NUMBER:	 return parse_int(parser);
		case LEFT_PAREN:	 return parse_compound(parser);
		case SINGLE_LINE_COMMENT:
			advance_token_type(parser, SINGLE_LINE_COMMENT);
			break;
		case MULTI_LINE_COMMENT:
			advance_token_type(parser, MULTI_LINE_COMMENT);
			break;
		default:
			break;
	}

	return NULL;
}

void parse_src(char* path, char* src) {
	vec_t asts;
	vec_init(&asts, 1);

	location_t* location = malloc(sizeof(struct location_t));
	location->path = path;
	location->line = 1;

	parser_t* parser = malloc(sizeof(struct parser_t));
	parser->tokens = tokenize(src);
	parser->head = vec_get(&parser->tokens, 0);
	parser->tokens_parsed = 0;
	parser->location = location;

	while (peek_token(parser)->type != END_OF_FILE) {
		ast_t* expr = parse_expr(parser);
		if (expr) {
			vec_push_back(&asts, expr);
		}
	}

	advance_token_type(parser, END_OF_FILE);
	gen_asm(asts);

	for (int i = 0; i < vec_length(&parser->tokens); ++i) {
		token_t* token = vec_get(&parser->tokens, i);
		free(token);
	}

	vec_free(&asts);
	vec_free(&parser->tokens);

	free(parser);
	free(location);
}
