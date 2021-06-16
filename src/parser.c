#include "parser.h"

int main() {
	vec_t tokens = tokenize("char = 'hello world'");
	printf("amount of tokens: %i\n", vec_length(&tokens));
	return 0;
}
