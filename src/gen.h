#ifndef GEN_H
#define GEN_H

#include "ast.h"
#include "file.h"

static FILE* outputfp;

void emit_lvar(ast_t* expr);
void emit_number(ast_t* expr);
void emit_call(ast_t* expr);
void emit_literal(ast_t* literal);
void emit_keyword(ast_t* expr);
void emit_assignment(ast_t* expr);
void emit_body(ast_t* body);
void emit_expr(ast_t* expr);

void gen(vec_t asts);

#endif /* GEN_H */
