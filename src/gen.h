#ifndef GEN_H
#define GEN_H

#include "parser.h"
#include "file.h"

static FILE* outputfp;

static int count();
static void emit(char* fmt, ...);
static void push_args2(node_t* node);
static int push_args(node_t* node);
static void emit_stmt(node_t* node);
static void emit_expr(node_t* node);
static void assign_lvar_offsets(obj_t* fn);

void codegen(obj_t* globals);

#endif /* GEN_H */
