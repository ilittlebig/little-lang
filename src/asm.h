#ifndef ASM_H
#define ASM_H

#include "ast.h"
#include "vec.h"

void asm_call();
char* asm_return(ast_t*);
char* asm_compound(ast_t*);
char* asm_block(ast_t*);
char* asm_function(ast_t*);
void asm_init(vec_t);

#endif /* ASM_H */
