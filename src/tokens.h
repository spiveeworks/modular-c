#ifndef TOKENS_H
#define TOKENS_H 1

#include <stdbool.h>

#define TOKEN_WIDTH 16

typedef struct {
	char str[TOKEN_WIDTH];
	bool is_keyword;
} TokenDef;

typedef enum {
	T_ALPHANUM = -1,
	T_OPEN_BRACE,
	T_OPEN_PAREN,
	T_OPEN_BRACK,
	T_CLOSE_BRACE,
	T_CLOSE_PAREN,
	T_CLOSE_BRACK,
	T_SEMICOLON,
	T_PUB,
	T_OPEN,
	T_STRUCT,

	// number of possible tokens, not to be used as a variant...
	UTOKEN_NUM,
} TokenVariant;

#define T_FIRST_OPEN T_OPEN_BRACE
#define T_LAST_OPEN T_OPEN_BRACK

#define T_TOTAL_OPENS (T_LAST_OPEN - T_FIRST_OPEN)

// value TokenDef utokens[UTOKEN_NUM] = 
TokenDef utokens[UTOKEN_NUM] = {
	{"{", false},
	{"(", false},
	{"[", false},
	{"}", false},
	{")", false},
	{"]", false},
	{";", false},
	{"struct", true},
	{"pub", true},
	{"open", true},
};

bool is_alphanum(char c) {
	return
		('0' <= c && c <= '9') ||
		('A' <= c && c <= 'Z') ||
		('a' <= c && c <= 'z') ||
		c == '_';
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
	TokenVariant variant;
	union {
		substr substr;
		TokenTree subtree;
	} data;
} TokenBranch;

TokenTree new_tree(int max_branches);

// borrows utokens during function
// borrows input for lifetime of result
TokenTree tokenize_flat(char *input, int input_len);

TokenTree group_tokens(TokenTree ts);

char *render_token_tree(char*out, TokenTree in);

void destroy_tt(TokenTree tt);

void test_tokenize();

#endif
