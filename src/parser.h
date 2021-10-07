#ifndef _PARSER_H
#define _PARSER_H

#include "lexer.h"

typedef enum node_kind_t {
	ND_NULL_EXPR, // Nothing
	ND_EQUAL,	  // ==
	ND_ASSIGN,	  // =
	ND_COND,	  // Condition // UNUSED
	ND_RETURN,	  // "return"
	ND_DEFVAR,    // "defvar" // UNUSED
	ND_FUNCALL,   // "funcall" // UNUSED
	ND_IF,		  // "if" // UNUSED
	ND_NUM,		  // Integer
	ND_VAR,		  // Variable
	ND_EXPR,	  // Expression
	ND_STMT,	  // Statement // UNUSED
	ND_CALL,	  // Function call
	ND_BLOCK,	  // Block
	ND_IDENT	  // Identifier
} node_kind_t;

typedef struct node_t node_t;
typedef struct obj_t obj_t;
struct obj_t {
	char* name;
	token_type_t type;
	token_t* token;

	int offset;

	int is_function;
	int is_definition;

	char* init_data;

	obj_t* params;
	node_t* body;
	obj_t* locals;
	int stack_size; // not using yet.

	obj_t* next;
};

struct node_t {
	node_kind_t kind;
	token_type_t type;
	token_t* token;

	node_t* lhs;
	node_t* rhs;

	node_t* cond;
	node_t* then;
	node_t* els;

	node_t* body;

	char* label; // not using yet.
	char* val;

	token_type_t func_type; // not using yet.
	node_t* args;
	obj_t* return_buffer; // not using yet.

	obj_t* var;
	node_t* next;
};

typedef struct parser_t {
	vec_t tokens;
	token_t* head;
	int tokens_parsed;
} parser_t;

static obj_t* locals;
static obj_t* globals;
static obj_t* current_func;

static token_t* peek(parser_t* parser);
static token_t* peek2(parser_t* parser);
static void consume(parser_t* parser);
static void consume_type(parser_t* parser, token_type_t token_type);

static int is_typename(token_t* token);
static char* make_label();

static obj_t* find_var(char* name);
static obj_t* new_var(char* name, token_type_t type);
static obj_t* new_lvar(char* name, token_type_t type);
static obj_t* new_gvar(char* name, token_type_t type);
static node_t* new_node(node_kind_t kind, token_t* token);
static node_t* new_binary(node_kind_t kind, node_t* lhs, node_t* rhs, token_t* token);
static node_t* new_unary(node_kind_t kind, node_t* expr, token_t* token);
static node_t* declaration(parser_t* parser);
static node_t* stmt(parser_t* parser);
static node_t* compound_stmt(parser_t* parser);
static node_t* read_var(parser_t* parser);
static node_t* read_number(parser_t* parser);
static node_t* read_string(parser_t* parser);
static node_t* expr(parser_t* parser);
static node_t* equality(parser_t* parser);
static node_t* assign(parser_t* parser);
static void func_params(parser_t* parser);
static obj_t* function(parser_t* parser);

obj_t* parse(char* path, char* src);

#endif /* PARSER_H */
