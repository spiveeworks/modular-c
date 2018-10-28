#include <stdlib.h>

int utoken_num = 0,
	nt_num = 0,
	rule_num = 0;

#define TOKEN_WIDTH 16

typedef char TokenDef[TOKEN_WIDTH];

typedef struct {
	int token_num;
	int *tokens;
} Rule;

typedef int *ParseTable;

Rule *initialize_rules() {
	Rule *rules = (Rule *) malloc(sizeof(Rule) * rule_num);
	return rules;
}

TokenDef *initialize_utokens() {
	TokenDef *utokens = (TokenDef *) malloc(sizeof(TokenDef) * utoken_num);
	return utokens;
}

ParseTable initialize_parse_table() {
	ParseTable parse_table;
	int size = sizeof(*parse_table) * nt_num * rule_num;
	parse_table = (ParseTable)malloc(size);
	return parse_table;
}

int main() {
	Rule *rules = initialize_rules();
	TokenDef *utokens = initialize_utokens();
	ParseTable parse_table = initialize_parse_table();

	free(parse_table);
	free(utokens);
	free(rules);
}
