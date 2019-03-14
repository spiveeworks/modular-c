#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int utoken_num = 8,
	nt_num = 0,
	rule_num = 0;

#define TOKEN_WIDTH 16

typedef struct {
	bool is_keyword;
	char str[TOKEN_WIDTH];
} TokenDef;

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
	strcpy(utokens[0].str, "{");
	utokens[0].is_keyword = false;
	strcpy(utokens[1].str, "(");
	utokens[1].is_keyword = false;
	strcpy(utokens[2].str, "[");
	utokens[2].is_keyword = false;
	strcpy(utokens[3].str, "}");
	utokens[3].is_keyword = false;
	strcpy(utokens[4].str, ")");
	utokens[4].is_keyword = false;
	strcpy(utokens[5].str, "]");
	utokens[5].is_keyword = false;
	strcpy(utokens[6].str, ";");
	utokens[6].is_keyword = false;
	strcpy(utokens[7].str, "struct");
	utokens[7].is_keyword = true;
	return utokens;
}

ParseTable initialize_parse_table() {
	ParseTable parse_table;
	int size = sizeof(*parse_table) * nt_num * rule_num;
	parse_table = (ParseTable)malloc(size);
	return parse_table;
}

typedef struct {
	int len;
	char *start;
} substr;

struct TokenBranch;

// one day Vec(TokenBranch)
// this raises an important question... when an inner module has access to
// private information of its enclosing module, how is that represented?
// pairwise header files?
// private vs public header file?
// inline the struct directly?
// presumably choose based on size or reuse or something?
typedef struct {
	int branch_num;
	struct TokenBranch *branches;
} TokenTree;

typedef struct TokenBranch {
	int variant;
	union {
		substr substr;
		TokenTree subtree;
	} data;
} TokenBranch;

int prefix_whitespace(const char *input) {
	int result = 0;
	bool done = false;
	while (done) {
		char c = input[result];
		if (c == '\t' || c == '\n' || c == '\r' || c == ' ') {
			result++;
		} else {
			done = true;
		}
	}
	return result;
}

bool is_alphanum(char c) {
	return
		('0' <= c && c <= '9') ||
		('A' <= c && c <= 'Z') ||
		('a' <= c && c <= 'z') ||
		c == '_';
}

int prefix_token(const char *input, TokenDef *utoken) {
	char *str = utoken->str;
	bool is_keyword = utoken->is_keyword;
	int i = 0;
	while (input[i] != '\0' && str[i] != '\0') {
		if (input[i] == '\0' || input[i] != str[i]) {
			return 0;
		}
		i++;
	}
	if(is_keyword && is_alphanum(input[i])) {
		// keywords must be followed by whitespace or an operator
		// in order to tokenize
		return 0;
	}
	return i;
}

substr next_token(char *input) {
	substr result;
	// default to 1 so that tokenizer always moves forward
	// really we should report an error when we reach an unknown operator
	result.len = 1;
	result.start = input;
	bool valid = true;
	while (valid) {
		char c = input[result.len];
		valid = is_alphanum(c);
		if (valid) {
			result.len++;
		}
	}
	return result;
}

// borrows utokens during function
// borrows input for lifetime of result
TokenTree tokenize_flat(TokenDef *utokens, char *input, int input_len) {
	char *end = input + input_len;

	TokenTree result;
	result.branches =
		(TokenBranch*) malloc(sizeof(TokenBranch) * input_len);
	result.branch_num = 0;

	while (input < end) {
		input = input + prefix_whitespace(input);
		int variant = -1;
		for (int ti = 0; ti < utoken_num; ti++) {
			int len = prefix_token(input, &utokens[ti]);
			if (len) {
				variant = ti;
				input += len;
				break;
			}
		}

		TokenBranch *branch = result.branches + result.branch_num;
		result.branch_num++;

		branch->variant = variant;
		if (variant == -1) {
			substr substr = next_token(input);
			branch->data.substr = substr;
			input += substr.len;
		} else {
			// so that when we recursively destroy tt, we don't free garbage
			branch->data.subtree.branches = NULL;
		}
	}

	return result;
}

typedef struct {
	TokenTree tt;
	TokenBranch *remaining;
} groupResult;

groupResult group_tokens(TokenBranch *remaining, TokenBranch *end, int close) {
	TokenTree tt;
	tt.branch_num = 0;
	tt.branches =
		(TokenBranch*)malloc(sizeof(TokenBranch) * (end - remaining));
	TokenBranch *out = tt.branches;
	int variant;
	while (remaining < end) {
		variant = remaining->variant;
		if (variant == close) {
			// so that we return one past the close bracket
			remaining++;
			break;
		}
		// TODO detect bad close braces to prevent "unexpected: }"?
		*out = *remaining;
		// increment before checking open brackets, so that we can recurse
		remaining++;
		if (variant >= 0 && variant < 3) {
			groupResult subtree = group_tokens(remaining, end, variant + 3);
			out->data.subtree = subtree.tt;
			remaining = subtree.remaining;
		}
		tt.branch_num++;
		out++;
	}
	groupResult result;
	result.tt = tt;
	result.remaining = remaining;
	return result;
}

void parse() {
	Rule *rules = initialize_rules();
	TokenDef *utokens = initialize_utokens();
	ParseTable parse_table = initialize_parse_table();

	free(parse_table);
	free(utokens);
	free(rules);
}

TokenTree group_tokens_from(TokenTree ts) {
	TokenBranch *start = ts.branches;
	TokenBranch *end = start + ts.branch_num;
	groupResult tt = group_tokens(start, end, -2);
	return tt.tt;
}

void destroy_tt(TokenTree tt) {

}

int main() {
	TokenDef *utokens = initialize_utokens();
	char *input = "struct point{int x;int y;};";
	int len = strlen(input);
	printf("Tokenizing...\n");
	TokenTree ts = tokenize_flat(utokens, input, len);
	int variants[11] = { 7, -1, 0, -1, -1, 6, -1, -1, 6, 3, 6 };
	if (ts.branch_num != 11) {
		printf("Wrong number of tokens: num == %d != 11\n", ts.branch_num);
	} else {
		for (int i = 0; i < 11; i++) {
			int actual = ts.branches[i].variant;
			if (actual != variants[i]) {
				printf("Wrong variant: expected[%d] != %d\n", i, actual);
			}
		}
	}
	printf("Grouping...\n");
	TokenTree tt = group_tokens_from(ts);

	int actual = tt.branches[2].data.subtree.branch_num;
	if (actual != 6) {
		printf("Wrong subtree size: branches[2].num == %d != 6\n", actual);
	}

	destroy_tt(ts);
	destroy_tt(tt);
	free(utokens);
}
