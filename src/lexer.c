#include "lexer.h"

bool starts_with(const char* str1, const char* str2, const int pos) {
	for (int i = 0; i < strlen(str2); i++) {
		if (str1[pos + i] != str2[i]) {
			return false;
		}
	}
	return true;
}

bool has_at_least(const tokenizer_t* tokenizer, int n) {
	return tokenizer->pos + n < strlen(tokenizer->input);
}

bool is_eof(const tokenizer_t* tokenizer) {
	return !has_at_least(tokenizer, 0);
}

bool is_valid_ident_start(const tokenizer_t* tokenizer) {
	char c = tokenizer->input[tokenizer->pos];
	if (isalpha(c) || c == '_') {
		return true;
	}
	return false;
}

bool is_valid_ident(const tokenizer_t* tokenizer) {
	char c = tokenizer->input[tokenizer->pos];
	if (isalpha(c) || isdigit(c) || c == '_') {
		return true;
	}
	return false;
}

token_type_t convert_to_token_type(char* word) {
	token_type_t token_type;
	if (strcmp(word, "fn") == 0) {
		token_type = FN;
	} else if (strcmp(word, "int") == 0) {
		token_type = INT;
	} else if (strcmp(word, "float") == 0) {
		token_type = FLOAT;
	} else if (strcmp(word, "char") == 0) {
		token_type = CHAR;
	} else if (strcmp(word, "void") == 0) {
		token_type = VOID;
	} else {
		token_type = UNIDENTIFIED;
	}
	return token_type;
}

char* convert_type_to_string(token_type_t token_type) {
	if (token_type == FN) {
		return "FN";
	} else if (token_type == INT) {
		return "INT";
	} else if (token_type == FLOAT) {
		return "FLOAT";
	} else if (token_type == CHAR) {
		return "CHAR";
	} else if (token_type == VOID) {
		return "VOID";

	} else if (token_type == IDENTIFIER) {
		return "IDENTIFIER";

	} else if (token_type == GREATER_OR_EQUAL) {
		return "GREATER_OR_EQUAL";
	} else if (token_type == EQUAL) {
		return "EQUAL";
	} else if (token_type == LESS_OR_EQUAL) {
		return "LESS_OR_EQUAL";
	} else if (token_type == NOT_EQUAL) {
		return "NOT_EQUAL";

	} else if (token_type == LEFT_PAREN) {
		return "LEFT_PAREN";
	} else if (token_type == RIGHT_PAREN) {
		return "RIGHT_PAREN";
	} else if (token_type == LEFT_BRACKET) {
		return "LEFT_BRACKET";
	} else if (token_type == RIGHT_BRACKET) {
		return "RIGHT_BRACKET";
	} else if (token_type == LEFT_CURLY) {
		return "LEFT_CURLY";
	} else if (token_type == RIGHT_CURLY) {
		return "RIGHT_CURLY";

	} else if (token_type == ADD) {
		return "ADD";
	} else if (token_type == MINUS) {
		return "MINUS";
	} else if (token_type == MUL) {
		return "MUL";
	} else if (token_type == DIV) {
		return "DIV";
	} else if (token_type == MOD) {
		return "MOD";
	} else if (token_type == POW) {
		return "POW";

	} else if (token_type == ASSIGN) {
		return "ASSIGN";
	} else if (token_type == LESS) {
		return "LESS";
	} else if (token_type == GREATER) {
		return "GREATER";
	} else if (token_type == NOT) {
		return "NOT";

	} else if (token_type == COLON) {
		return "COLON";
	} else if (token_type == COMMA) {
		return "COMMA";
	} else if (token_type == SEMICOLON) {
		return "SEMICOLON";
	} else if (token_type == ATTR) {
		return "ATTR";

	} else if (token_type == WHITESPACE) {
		return "WHITESPACE";
	}
	return "UNIDENTIFIED";
}

token_t* read_digit(tokenizer_t* tokenizer) {
	token_t* token = malloc(sizeof(token_t));
	char* digit = calloc(0, sizeof(char));
	char* input = tokenizer->input;

	bool is_int = true;
	int size = 0;

	while (true) {
		const char c = input[tokenizer->pos];
		if (c == '.') {
			is_int = false;
			size++;
			digit = realloc(digit, size*sizeof(char));
			digit[size-1] = input[tokenizer->pos++];
		} else if (isdigit(c)) {
			size++;
			digit = realloc(digit, size*sizeof(char));
			digit[size-1] = input[tokenizer->pos++];
		} else if (c == 'e' || c == 'E') { // TODO: Add support for decimal points
			size++;
			digit = realloc(digit, size*sizeof(char));
			digit[size-1] = input[tokenizer->pos++];

			const char c = input[tokenizer->pos];
			if (c == '-' || c == '+') {
				size++;
				digit = realloc(digit, size*sizeof(char));
				digit[size-1] = input[tokenizer->pos++];
			}
		} else {
			break;
		}
	}

	printf("digit: %s\n", digit);
	if (is_int) {
		token->type = INT;
	} else {
		token->type = FLOAT;
	}

	return token;
}

token_t* read_string(tokenizer_t* tokenizer) {
	token_t* token = malloc(sizeof(token_t));

	char* input = tokenizer->input;
	const char start_char = input[tokenizer->pos++];

	bool is_closed = false;

	while (true) {
		const char c = input[tokenizer->pos];
		if (c == '\\') {
			tokenizer->pos += 2;
		}

		if (c != start_char && has_at_least(tokenizer, 0)) {
			tokenizer->pos++;
		} else if (c == start_char) {
			is_closed = true;
			tokenizer->pos++;
			break;
		} else {
			break;
		}
	}

	if (is_closed) {
		token->type = STRING_LITERAL;
	} else {
		token->type = UNCLOSED_STRING_LITERAL;
	}

	return token;
}

token_t* read_other_tokens(tokenizer_t* tokenizer) {
	token_t* token = malloc(sizeof(token_t));
	char* input = tokenizer->input;
	const char c = input[tokenizer->pos];

	switch(c) {
		case '!':
			if (starts_with(input, "!=", tokenizer->pos)) {
				tokenizer->pos += 2;
				token->type = NOT_EQUAL;
			} else {
				tokenizer->pos++;
				token->type = NOT;
			}
			break;
		case '>':
			if (starts_with(input, ">=", tokenizer->pos)) {
				tokenizer->pos += 2;
				token->type = GREATER_OR_EQUAL;
			} else {
				tokenizer->pos++;
				token->type = GREATER;
			}
			break;
		case '=':
			if (starts_with(input, "==", tokenizer->pos)) {
				tokenizer->pos += 2;
				token->type = EQUAL;
			} else {
				tokenizer->pos++;
				token->type = ASSIGN;
			}
			break;
		case '<':
			if (starts_with(input, "<=", tokenizer->pos)) {
				tokenizer->pos += 2;
				token->type = LESS_OR_EQUAL;
			} else {
				tokenizer->pos++;
				token->type = LESS;
			}
			break;
		default:
			token->type = UNIDENTIFIED;
			break;
	}

	if (is_valid_ident_start(tokenizer)) {
		char* word = calloc(0, sizeof(char));
		int size = 0;

		while (is_valid_ident(tokenizer)) {
			size++;
			word = realloc(word, size*sizeof(char));
			word[size-1] = input[tokenizer->pos++];
		}

		token_type_t token_type = convert_to_token_type(word);
		if (token_type != UNIDENTIFIED) { // KEYWORD
			token->type = token_type;
		} else { // IDENTIFIER
			token->type = IDENTIFIER;
		}

		free(word);
	}

	return token;
}

token_t* next_token(tokenizer_t* tokenizer) {
	token_t* token = malloc(sizeof(token_t));
	const char c = tokenizer->input[tokenizer->pos];

	switch(c) {
		case '\'':
			return read_string(tokenizer);
		case '\"':
			return read_string(tokenizer);

		case '+':
			token->type = ADD;
			break;
		case '-':
			token->type = MINUS;
			break;
		case '*':
			token->type = MUL;
			break;
		case '/':
			token->type = DIV;
			break;
		case '%':
			token->type = MOD;
			break;
		case '^':
			token->type = POW;
			break;

		case '(':
			token->type = LEFT_PAREN;
			break;
		case ')':
			token->type = RIGHT_PAREN;
			break;
		case '[':
			token->type = LEFT_BRACKET;
			break;
		case ']':
			token->type = RIGHT_BRACKET;
			break;
		case '{':
			token->type = LEFT_CURLY;
			break;
		case '}':
			token->type = RIGHT_CURLY;
			break;
		case ':':
			token->type = COLON;
			break;
		case ',':
			token->type = COMMA;
			break;
		case ';':
			token->type = SEMICOLON;
			break;
		case '.':
			token->type = ATTR;
			break;
		default:
			token->type = UNIDENTIFIED;
			break;
	}

	if (isdigit(c)) {
		return read_digit(tokenizer);
	} else if (isspace(c) != 0) {
		token->type = WHITESPACE;
	}

	// Check if there are any tokens with two or more characters
	if (token->type == UNIDENTIFIED) {
		return read_other_tokens(tokenizer);
	} else {
		tokenizer->pos++;
		return token;
	}
}

int main() {
	char* input = "0 0.1 5.5 10E+10 10e-4";

	tokenizer_t* tokenizer = malloc(sizeof(struct tokenizer_t));
	tokenizer->input = strdup(input);
	tokenizer->pos = 0;

	while (!is_eof(tokenizer)) {
		token_t* token = next_token(tokenizer);
		printf("TYPE: %s\n", convert_type_to_string(token->type));
	}
	free(tokenizer->input);

	return 0;
}
