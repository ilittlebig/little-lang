#include "parser.h"
#include "error.h"
#include "file.h"

static token_t* peek(parser_t* parser) {
	token_t* token = vec_get(&parser->tokens, parser->tokens_parsed);
	parser->head = token;
	return parser->head;
}

static token_t* peek2(parser_t* parser) {
	token_t* token = vec_get(&parser->tokens, parser->tokens_parsed + 1);
	parser->head = token;
	return parser->head;
}

static token_t* peekn(parser_t* parser, int n) {
	token_t* token = vec_get(&parser->tokens, parser->tokens_parsed + n);
	parser->head = token;
	return parser->head;
}

static void consume(parser_t* parser) {
	token_t* token = vec_get(&parser->tokens, parser->tokens_parsed);
	parser->tokens_parsed++;
}

static void consume_type(parser_t* parser, token_type_t token_type) {
	token_t* token = peek(parser);
	if (token && token->type == token_type) {
		parser->tokens_parsed++;
	} else {
		consume(parser);
		error_at("token '%s' was expecting '%s'\n", token_to_str(token->type), token_to_str(token_type));
	}
}

static int is_typename(token_t* token) {
	token_type_t types[] = {INT, FLOAT, STRING, VOID};
	int len = sizeof(types) / sizeof(*types);
	for (int i = 0; i < len; ++i) {
		if (types[i] == token->type) {
			return 1;
		}
	}
	return 0;
}

static char* make_label() {
	static int c = 0;
	char* buff = calloc(4, sizeof(char));
	sprintf(buff, "LC%d", c++);
	return buff;
}

static obj_t* find_var(char* name) {
	for (obj_t* var = globals; var; var = var->next) {
		if (strcmp(var->name, name) == 0) {
			return var;
		}
	}
	for (obj_t* var = locals; var; var = var->next) {
		if (strcmp(var->name, name) == 0) {
			return var;
		}
	}
	return NULL;
}

static obj_t* new_var(char* name, token_type_t type) {
	obj_t* var = calloc(1, sizeof(obj_t));
	var->name = name;
	var->type = type;
	return var;
}

static obj_t* new_lvar(char* name, token_type_t type) {
	obj_t* var = new_var(name, type);
	var->next = locals;
	locals = var;
	return var;
}

static obj_t* new_gvar(char* name, token_type_t type) {
	obj_t* var = new_var(name, type);
	var->next = globals;
	var->is_definition = true;
	globals = var;
	return var;
}

static node_t* new_node(node_kind_t kind, token_t* token) {
	node_t* node = calloc(1, sizeof(node_t));
	node->kind = kind;
	node->token = token;
	return node;
}

static node_t* new_binary(node_kind_t kind, node_t* lhs, node_t* rhs, token_t* token) {
	node_t* node = new_node(kind, token);
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

static node_t* new_unary(node_kind_t kind, node_t* expr, token_t* token) {
	node_t* node = new_node(kind, token);
	node->lhs = expr;
	return node;
}

static node_t* declaration(parser_t* parser) {
	node_t head = {};
	node_t* cur = &head;

	token_type_t type = peek(parser)->type;
	consume_type(parser, type);
	consume_type(parser, COLON);

	obj_t* var = new_lvar(peekn(parser, 2)->value, type);
	node_t* expr = assign(parser);
	cur = cur->next = new_unary(ND_EXPR, expr, peek(parser));

	node_t* node = new_node(ND_BLOCK, peek(parser));
	node->body = head.next;
	return node;
}

static node_t* stmt(parser_t* parser) {
	if (peek2(parser)->type == RETURN) {
		token_t* token = peek2(parser);
		consume_type(parser, LEFT_PAREN);

		node_t* node = new_node(ND_RETURN, token);
		consume_type(parser, RETURN);

		node->lhs = assign(parser);

		consume_type(parser, RIGHT_PAREN);
		return node;
	}

	if (peek(parser)->type == IF) {
		token_t* token = peek(parser);
		node_t* node = new_node(ND_IF, token);
		consume_type(parser, IF);

		node->cond = assign(parser);
		node->then = stmt(parser);
		if (peek(parser)->type == ELSE) {
			consume_type(parser, ELSE);
			node->els = stmt(parser);
		}

		return node;
	}

	if (peek(parser)->type == WHILE) {
		token_t* token = peek(parser);
		node_t* node = new_node(ND_WHILE, token);
		consume_type(parser, WHILE);

		node->cond = assign(parser);
		node->then = compound_stmt(parser);

		return node;
	}

	if (peek2(parser)->type == DEFVAR) {
		token_t* token = peek2(parser);
		consume_type(parser, LEFT_PAREN);
		node_t* node = new_node(ND_DEFVAR, token);
		consume_type(parser, DEFVAR);

		token = peek(parser);
		consume_type(parser, IDENTIFIER);

		obj_t* var = new_lvar(token->value, token->type);
		node_t* lhs = new_node(ND_VAR, token);
		lhs->var = var;

		node->lhs = lhs;
		node->rhs = stmt(parser);

		consume_type(parser, RIGHT_PAREN);
		return node;
	}

	if (peek2(parser)->type == IDENTIFIER) {
		token_t* token = peek2(parser);
		consume_type(parser, LEFT_PAREN);
		node_t head = {};
		node_t* cur = &head;

		node_t* fn = expr(parser);
		while (peek(parser)->type != RIGHT_PAREN) {
			node_t* arg = assign(parser);
			cur = cur->next = arg;
		}
		consume_type(parser, RIGHT_PAREN);

		node_t* node = new_unary(ND_CALL, fn, token);
		node->args = head.next;
		return node;
	}

	if (peek(parser)->type == LEFT_CURLY) {
		return compound_stmt(parser);
	}

	node_t* node = new_node(ND_EXPR, peek(parser));
	node->lhs = assign(parser);
	return node;
}

static node_t* compound_stmt(parser_t* parser) {
	node_t* node = new_node(ND_BLOCK, peek(parser));
	node_t head = {};
	node_t* cur = &head;

	consume_type(parser, LEFT_CURLY);
	while (peek(parser)->type != RIGHT_CURLY) {
		token_t* token = peek(parser);
		if (is_typename(token)) {
			cur = cur->next = declaration(parser);
		} else {
			cur = cur->next = stmt(parser);
		}
	}
	consume_type(parser, RIGHT_CURLY);

	node->body = head.next;
	return node;
}

static node_t* read_var(parser_t* parser) {
	token_t* token = peek(parser);
	consume_type(parser, IDENTIFIER);

	obj_t* var = find_var(token->value);
	if (var) {
		node_t* node = new_node(ND_VAR, token);
		node->var = var;
		return node;
	}

	error_at("'%s' undeclared (first use in this function)\n", token->value);
	return NULL;
}

static node_t* read_number(parser_t* parser) {
	token_t* token = peek(parser);

	node_t* node = new_node(ND_NUM, token);
	node->val = token->value;

	consume_type(parser, NUMBER);
	return node;
}

static node_t* read_string(parser_t* parser) {
	token_t* token = peek(parser);
	consume_type(parser, STRING_LITERAL);

	obj_t* string_literal = new_gvar(make_label(), token->type);
	string_literal->init_data = token->value;

	node_t* var = new_node(ND_VAR, token);
	var->var = string_literal;
	return var;
}

static node_t* expr(parser_t* parser) {
	token_t* token = peek(parser);
	switch(token->type) {
		case IDENTIFIER:	 return read_var(parser);
		case NUMBER:		 return read_number(parser);
		case STRING_LITERAL: return read_string(parser);
		default:
			error_at("unknown token type: '%s'\n", token_to_str(token->type));
	}
}

static node_t* add(parser_t* parser) {
	node_t* node;

	if (peek2(parser)->type == ADD) { // +
		consume_type(parser, LEFT_PAREN);
		consume_type(parser, ADD);

		node = equality(parser);
		node = new_binary(ND_ADD, node, assign(parser), peek(parser));

		consume_type(parser, RIGHT_PAREN);
		return node;
	} else if (peek2(parser)->type == SUB) { // -
		consume_type(parser, LEFT_PAREN);
		consume_type(parser, SUB);

		node = equality(parser);
		node = new_binary(ND_SUB, node, assign(parser), peek(parser));

		consume_type(parser, RIGHT_PAREN);
		return node;
	}

	node = expr(parser);
	return node;
}

static node_t* mul(parser_t* parser) {
	node_t* node;

	if (peek2(parser)->type == MUL) { // *
		consume_type(parser, LEFT_PAREN);
		consume_type(parser, MUL);

		node = equality(parser);
		node = new_binary(ND_MUL, node, assign(parser), peek(parser));

		consume_type(parser, RIGHT_PAREN);
		return node;
	} else if (peek2(parser)->type == DIV) { // /
		consume_type(parser, LEFT_PAREN);
		consume_type(parser, DIV);

		node = equality(parser);
		node = new_binary(ND_DIV, node, assign(parser), peek(parser));

		consume_type(parser, RIGHT_PAREN);
		return node;
	} else if (peek2(parser)->type == MOD) { // %
		consume_type(parser, LEFT_PAREN);
		consume_type(parser, MOD);

		node = equality(parser);
		node = new_binary(ND_MOD, node, assign(parser), peek(parser));

		consume_type(parser, RIGHT_PAREN);
		return node;
	}

	node = add(parser);
	return node;
}
static node_t* relational(parser_t* parser) {
	node_t* node;

	if (peek2(parser)->type == LESS) { // <
		consume_type(parser, LEFT_PAREN);
		consume_type(parser, LESS);

		node = equality(parser);
		node = new_binary(ND_LESS, node, assign(parser), peek(parser));

		consume_type(parser, RIGHT_PAREN);
		return node;
	} else if (peek2(parser)->type == LESS_OR_EQUAL) { // <=
		consume_type(parser, LEFT_PAREN);
		consume_type(parser, LESS_OR_EQUAL);

		node = equality(parser);
		node = new_binary(ND_LESS_EQUAL, node, assign(parser), peek(parser));

		consume_type(parser, RIGHT_PAREN);
		return node;
	} else if (peek2(parser)->type == GREATER) { // >
		consume_type(parser, LEFT_PAREN);
		consume_type(parser, GREATER);

		node = equality(parser);
		node = new_binary(ND_GREATER, node, assign(parser), peek(parser));

		consume_type(parser, RIGHT_PAREN);
		return node;
	} else if (peek2(parser)->type == GREATER_OR_EQUAL) { // >=
		consume_type(parser, LEFT_PAREN);
		consume_type(parser, GREATER_OR_EQUAL);

		node = equality(parser);
		node = new_binary(ND_GREATER_EQUAL, node, assign(parser), peek(parser));

		consume_type(parser, RIGHT_PAREN);
		return node;
	}

	node = mul(parser);
	return node;
}

static node_t* equality(parser_t* parser) {
	node_t* node;

	if (peek2(parser)->type == EQUAL) { // ==
		consume_type(parser, LEFT_PAREN);
		consume_type(parser, EQUAL);

		node = equality(parser);
		node = new_binary(ND_EQUAL, node, assign(parser), peek(parser));

		consume_type(parser, RIGHT_PAREN);
		return node;
	} else if (peek2(parser)->type == NOT_EQUAL) { // !=
		consume_type(parser, LEFT_PAREN);
		consume_type(parser, NOT_EQUAL);

		node = equality(parser);
		node = new_binary(ND_NOT_EQUAL, node, assign(parser), peek(parser));

		consume_type(parser, RIGHT_PAREN);
		return node;
	}

	node = relational(parser);
	return node;
}

static node_t* assign(parser_t* parser) {
	token_t* token = peek2(parser);

	switch(token->type) {
		case ASSIGN:
			consume_type(parser, LEFT_PAREN);
			consume_type(parser, ASSIGN);

			node_t* var = read_var(parser);
			node_t* rhs = assign(parser);
			node_t* node = new_binary(ND_ASSIGN, var, rhs, token);

			consume_type(parser, RIGHT_PAREN);
			return node;
	}

	node_t* node = equality(parser);
	return node;
}

static void func_params(parser_t* parser) {
	consume_type(parser, LEFT_PAREN);
	while (peek(parser)->type != RIGHT_PAREN) {
		token_type_t type = peek(parser)->type;
		consume_type(parser, type);

		char* name = peek(parser)->value;
		consume_type(parser, IDENTIFIER);

		new_lvar(name, type);
		if (peek(parser)->type != RIGHT_PAREN) {
			consume_type(parser, COMMA);
		}
	}
	consume_type(parser, RIGHT_PAREN);
}

static obj_t* function(parser_t* parser) {
	consume_type(parser, FN);
	char* name = peek(parser)->value;
	consume(parser);

	locals = NULL;
	func_params(parser);

	consume_type(parser, COLON);
	token_type_t type = peek(parser)->type;
	consume(parser);

	obj_t* func = new_gvar(name, type);
	current_func = func;

	func->is_function = true;
	func->params = locals;
	func->body = compound_stmt(parser);
	func->locals = locals;

	return func;
}

static void declare_builtin_functions() {
	obj_t* printfn = new_gvar("print", VOID);
	printfn->is_definition = false;
	obj_t* printifn = new_gvar("printi", VOID);
	printifn->is_definition = false;
	obj_t* strlenfn = new_gvar("strlen", INT);
	strlenfn->is_definition = false;
}

obj_t* parse(char* path, char* src) {
	globals = NULL;
	declare_builtin_functions();

	parser_t* parser = malloc(sizeof(struct parser_t));
	parser->tokens = tokenize(src);
	parser->head = vec_get(&parser->tokens, 0);
	parser->tokens_parsed = 0;

	while (peek(parser)->type != END_OF_FILE) {
		if (peek(parser)->type == FN) {
			obj_t* func = function(parser);
			continue;
		}
	}
	consume_type(parser, END_OF_FILE);

	return globals;
}
