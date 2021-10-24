#ifndef _PARSER_H
#define _PARSER_H

#include "lexer.h"

typedef enum node_kind_t {
	ND_NULL_EXPR,     // Nothing
	ND_EQ,			  // ==
	ND_NOT_EQ,		  // !=
	ND_LESS,		  // <
	ND_LESS_EQ,		  // <=
	ND_GREATER,		  // >
	ND_GREATER_EQ,	  // >=
	ND_ASSIGN,	      // =
	ND_RETURN,	      // 'return'
	ND_DEFVAR,        // 'defvar'
	ND_IF,		      // 'if'
	ND_WHILE,		  // 'while'
	ND_FOR,			  // 'for'
	ND_NUM,		      // Integer
	ND_VAR,		      // Variable
	ND_ARRAY,		  // Array
	ND_EXPR,	      // Expression
	ND_STMT,	      // Statement     UNUSED
	ND_CALL,	      // Function call
	ND_BLOCK,	      // Block
	ND_ADD,		      // '+'
	ND_SUB,		      // '-'
	ND_MUL,		      // '*'
	ND_DIV,		      // '/'
	ND_MOD,		      // '%'
	ND_IDENT	      // Identifier
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
	int is_builtin;
	int is_param;
	int is_array;

	char* init_data;

	obj_t* params;
	node_t* body;
	obj_t* locals;
	int stack_size;

	token_type_t func_type;
	node_t* return_buffer;

	obj_t* next;
};

struct node_t {
	node_kind_t kind;
	token_type_t type;
	token_t* token;

	node_t* lhs;
	node_t* rhs;

	node_t* children;

	node_t* init;
	node_t* cond;
	node_t* loop;
	node_t* then;
	node_t* els;

	node_t* body;
	node_t* args;

	char* val;
	node_t* array_len;

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
static token_t* peekn(parser_t* parser, int n);
static void consume(parser_t* parser);
static void consume_type(parser_t* parser, token_type_t token_type);
static void skip_until_next_brace(parser_t* parser);

static int is_typename(token_t* token);
static char* make_label();
static void initialize_array_size(parser_t* parser, node_t* node);

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
static node_t* read_array_dimensions(parser_t* parser);
static node_t* read_array_initializer(parser_t* parser);
static node_t* read_var(parser_t* parser);
static node_t* read_number(parser_t* parser);
static node_t* read_string(parser_t* parser);
static node_t* expr(parser_t* parser);
static node_t* add(parser_t* parser);
static node_t* mul(parser_t* parser);
static node_t* relational(parser_t* parser);
static node_t* equality(parser_t* parser);
static node_t* assign(parser_t* parser);
static void func_params(parser_t* parser);
static obj_t* function(parser_t* parser);

static void declare_builtin_functions();
obj_t* parse(char* path, char* src);

#endif /* PARSER_H */
