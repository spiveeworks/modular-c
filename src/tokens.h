#ifndef TOKENS_H
#define TOKENS_H 1

#include <stdbool.h>

#define TOKEN_WIDTH 16

typedef struct {
	char str[TOKEN_WIDTH];
	bool is_keyword;
} TokenDef;

typedef enum {
	ALPHANUM = -1,
	OPEN_BRACE,
	OPEN_PAREN,
	OPEN_BRACK,
	CLOSE_BRACE,
	CLOSE_PAREN,
	CLOSE_BRACK,
	SEMICOLON,
	STRUCT,

	// number of possible tokens, not to be used as a variant...
	UTOKEN_NUM,
} TokenVariant;

#define FIRST_OPEN OPEN_BRACE
#define LAST_OPEN OPEN_BRACK

#define TOTAL_OPENS (LAST_OPEN - FIRST_OPEN)

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

void destroy_tt(TokenTree tt);

void test_tokenize();

#endif
