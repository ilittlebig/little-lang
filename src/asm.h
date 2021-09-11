#ifndef ASM_H
#define ASM_H

#include "ast.h"
#include "vec.h"

void asm_string(ast_t*);
char* asm_call(ast_t*);
char* asm_return(ast_t*, vec_t*);
char* asm_assignment(ast_t*, vec_t* vars);
char* asm_block(ast_t*, vec_t*);
char* asm_function(ast_t*);
void asm_init(vec_t);

#endif /* ASM_H */
