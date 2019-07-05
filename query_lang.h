//
// Created by dallen on 7/5/19.
//

#ifndef SUPERDB_QUERY_LANG_H
#define SUPERDB_QUERY_LANG_H

#include "database.h"

#define MAX_LIT_SIZE 32

enum token_type {
    TOKEN_TABLE, TOKEN_INT, TOKEN_STRING, TOKEN_FLOAT, TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE, TOKEN_NAME, TOKEN_SEMICOLON,
    TOKEN_STRING_LIT, TOKEN_NUMBER_LIT, TOKEN_COMMA, TOKEN_DOT, TOKEN_NEW, TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_EQUALS, TOKEN_DOUBLE_EQUALS, TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH, TOKEN_EOF
};

typedef struct {
    enum token_type type;
    char lit[MAX_LIT_SIZE];
} instr_token_t;

typedef struct token_list_node_struct {
    instr_token_t token;
    struct token_list_node_struct * next;
} token_list_node_t;

token_list_node_t * tokenize(char * string);
void print_token_list(token_list_node_t * list);
void execute_code(database_t * db, char * string);

#endif //SUPERDB_QUERY_LANG_H
