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
	token_t* token = vec_get(&parser->tokens, parser->tokens_parsed);
	if (token && token->type == token_type) {
		parser->tokens_parsed++;
	} else {
		advance_token(parser);
		go_error_at(parser->location, "token '%s' was expecting '%s'", token->type, token_type);
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
		if (expr->type == AST_ASSIGNMENT) {
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

ast_t* parse_keyword(parser_t* parser) {
	ast_t* keyword = init_ast(AST_KEYWORD);
	token_t* token = peek_token(parser);

	switch(token->type) {
		case RETURN:
			advance_token_type(parser, RETURN);
			keyword->type_specifier = RETURN;

			if (peek_token(parser)->type != RIGHT_PAREN) {
				keyword->value = parse_expr(parser);
			}
			advance_token_type(parser, RIGHT_PAREN);

			break;
		case DEFVAR:
			break;
		case FUNCALL:
			break;
		default:
			break;
	}

	return keyword;
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

	while (peek_token(parser)->type != RIGHT_PAREN) {
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

			ast_t* expr = parse_expr(parser);
			if (expr != NULL) {
				compound->value = expr;
			} else {
				go_error_at(parser->location, "expected compound expression");
			}
		} else if (token->type == IDENTIFIER) {
			ast_t* identifier = parse_identifier(parser);
			compound->name = identifier->name;
			compound->type = AST_CALL;

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

			free(identifier);
		} else if (is_keyword(token->type)) {
			free(compound);
			return parse_keyword(parser);
		}
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
		case FN: return parse_func(parser);
		case IDENTIFIER: return parse_identifier(parser);
		case STRING_LITERAL: return parse_string(parser);
		case INT_NUMBER: return parse_int(parser);
		case LEFT_PAREN: return parse_compound(parser);
		case SINGLE_LINE_COMMENT:
			advance_token_type(parser, SINGLE_LINE_COMMENT);
			break;
		default:
			if (is_keyword(token->type)) {
				return parse_keyword(parser);
			}
	}

	return NULL;
}

int main(int argc, char* argv[]) {
	parser_t* parser = malloc(sizeof(struct parser_t));
	location_t* location = malloc(sizeof(struct location_t));

	char* file_path = argv[1];
	char* file_buff = read_file(file_path);

	parser->tokens = tokenize(file_buff);
	parser->head = vec_get(&parser->tokens, 0);
	parser->location = location;
	parser->tokens_parsed = 0;

	// TODO: change line on 'NEW_LINE' token
	location->file_path = file_path;
	location->line = 1;

	vec_t vec;
	vec_init(&vec, 1);
	while (peek_token(parser)->type != END_OF_FILE) {
		ast_t* expr = parse_expr(parser);
		if (expr) {
			vec_push_back(&vec, expr);
		}
	}
	advance_token_type(parser, END_OF_FILE);

	gen(vec); // test func
	// asm_init(vec);

	for (int i = 0; i < vec_length(&parser->tokens); ++i) {
		token_t* token = vec_get(&parser->tokens, i);
		free(token);
	}

	// TODO: cleanup more allocated memory
	vec_free(&parser->tokens);
	vec_free(&vec);

	free(parser);
	free(location);
	free(file_buff);

	return 0;
}
