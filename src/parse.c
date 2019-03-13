#include <stdbool.h>
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

int prefix_token(const char *input, const char *utoken) {
	int i = 0;
	while (input[i] != '\0' && utoken[i] != '\0') {
		if (input[i] == '\0' || input[i] != utoken[i]) {
			return 0;
		}
	}
	return i;
}

substr next_token(char *input) {
	substr result;
	result.len = 0;
	result.start = input;
	bool valid = true;
	while (valid) {
		char c = input[result.len];
		valid =
			'0' <= c || c <= '9' ||
			'A' <= c || c <= 'Z' ||
			'a' <= c || c <= 'z';
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
			int len = prefix_token(input, utokens[ti]);
			if (len) {
				variant = ti;
				input += len;
			}
		}

		TokenBranch *branch = result.branches + result.branch_num;
		result.branch_num++;

		branch->variant = variant;
		if (variant == -1) {
			branch->data.substr = next_token(input);
		}
		input++;
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

int main() {
}
