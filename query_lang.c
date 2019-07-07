//
// Created by dallen on 7/5/19.
//

#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#include "util.h"
#include "query_lang.h"
#include "database.h"

void add_token(token_list_node_t ** list, instr_token_t token)
{
    token_list_node_t * new_node = calloc(1, sizeof(token_list_node_t));
    new_node->token = token;
    new_node->next = NULL;

    if(*list == NULL)
    {
        *list = new_node;
        return;
    }
    token_list_node_t * curr = *list;
    while(curr->next != NULL)
    {
        curr = curr->next;
    }
    curr->next = new_node;
}

void free_token_list(token_list_node_t * head)
{
    token_list_node_t * last = head;
    while(last != NULL)
    {
        head = head->next;
        free(last);
        last = head;
    }
}

bool check_for_token(char * string, size_t * pos, char * token_string, enum token_type type,
        token_list_node_t ** tokens)
{
    if(starts_with(string + *pos, token_string))
    {
        instr_token_t token;
        token.type = type;
        memset(token.lit, '\0', MAX_LIT_SIZE);
        strcpy(token.lit, token_string);
        add_token(tokens, token);
        *pos += strlen(token_string);
        return true;
    }
    return false;
}

void tokenize_string_literal(char * string, size_t * pos, token_list_node_t ** tokens)
{
    size_t i = 0;
    instr_token_t token;
    token.type = TOKEN_STRING_LIT;
    memset(token.lit, '\0', MAX_LIT_SIZE);
    (*pos)++;
    while(string[*pos] != '"')
    {
        token.lit[i] = string[*pos];
        (*pos)++;
        i++;
    }
    add_token(tokens, token);
    (*pos)++;
}

void tokenize_number_literal(char * string, size_t * pos, token_list_node_t ** tokens)
{
    size_t i = 0;
    instr_token_t token;
    token.type = TOKEN_NUMBER_LIT;
    memset(token.lit, '\0', MAX_LIT_SIZE);
    while(is_number(string[*pos]))
    {
        token.lit[i] = string[*pos];
        (*pos)++;
        i++;
    }
    add_token(tokens, token);
}

void tokenize_name(char * string, size_t * pos, token_list_node_t ** tokens)
{
    size_t i = 0;
    instr_token_t token;
    token.type = TOKEN_NAME;
    memset(token.lit, '\0', MAX_LIT_SIZE);
    while(is_name_char(string[*pos]))
    {
        token.lit[i] = string[*pos];
        (*pos)++;
        i++;
    }
    add_token(tokens, token);
}

token_list_node_t * tokenize(char * string)
{
    token_list_node_t * tokens = NULL;
    size_t pos = 0;
    while(string[pos] != '\0')
    {
        if(string[pos] == '\n' || string[pos] == ' ' || string[pos] == '\t')
        {
            pos++;
            continue;
        }

        // non literal tokens
        if(check_for_token(string, &pos, "table", TOKEN_TABLE, &tokens))
        {
            continue;
        }
        if(check_for_token(string, &pos, "int", TOKEN_INT, &tokens))
        {
            continue;
        }
        if(check_for_token(string, &pos, "float", TOKEN_FLOAT, &tokens))
        {
            continue;
        }
        if(check_for_token(string, &pos, "string", TOKEN_STRING, &tokens))
        {
            continue;
        }
        if(check_for_token(string, &pos, "{", TOKEN_LEFT_BRACE, &tokens))
        {
            continue;
        }
        if(check_for_token(string, &pos, "}", TOKEN_RIGHT_BRACE, &tokens))
        {
            continue;
        }
        if(check_for_token(string, &pos, ";", TOKEN_SEMICOLON, &tokens))
        {
            continue;
        }
        if(check_for_token(string, &pos, ",", TOKEN_COMMA, &tokens))
        {
            continue;
        }
        if(check_for_token(string, &pos, ".", TOKEN_DOT, &tokens))
        {
            continue;
        }
        if(check_for_token(string, &pos, "new", TOKEN_NEW, &tokens))
        {
            continue;
        }
        if(check_for_token(string, &pos, "(", TOKEN_LEFT_PAREN, &tokens))
        {
            continue;
        }
        if(check_for_token(string, &pos, ")", TOKEN_RIGHT_PAREN, &tokens))
        {
            continue;
        }
        if(check_for_token(string, &pos, "==", TOKEN_DOUBLE_EQUALS, &tokens))
        {
            continue;
        }
        if(check_for_token(string, &pos, "=", TOKEN_EQUALS, &tokens))
        {
            continue;
        }
        if(check_for_token(string, &pos, "+", TOKEN_PLUS, &tokens))
        {
            continue;
        }
        if(check_for_token(string, &pos, "-", TOKEN_MINUS, &tokens))
        {
            continue;
        }
        if(check_for_token(string, &pos, "*", TOKEN_STAR, &tokens))
        {
            continue;
        }
        if(check_for_token(string, &pos, "/", TOKEN_SLASH, &tokens))
        {
            continue;
        }


        if(string[pos] == '"')
        {
            tokenize_string_literal(string, &pos, &tokens);
            continue;
        }

        if(is_number(string[pos]))
        {
            tokenize_number_literal(string, &pos, &tokens);
            continue;
        }

        tokenize_name(string, &pos, &tokens);
    }
    instr_token_t eof_token;
    eof_token.type = TOKEN_EOF;
    memset(eof_token.lit, '\0', MAX_LIT_SIZE);
    add_token(&tokens, eof_token);
    return tokens;
}

void print_token_list(token_list_node_t * list)
{
    for(; list != NULL; list = list->next)
    {
        printf("%i(\"%s\") ", list->token.type, list->token.lit);
    }
    printf("\n");
}

enum type translate_type(enum token_type type)
{
    switch(type)
    {
        case TOKEN_INT:
            return TYPE_INT;
        case TOKEN_FLOAT:
            return TYPE_FLOAT;
        case TOKEN_STRING:
            return TYPE_STRING;
    }
    assert(false);
    return 0;
}

size_t count_until(token_list_node_t ** start, enum token_type to_count, enum token_type until)
{
    token_list_node_t * curr = (*start);
    size_t count = 0;
    while(curr->token.type != until) {
        if(curr->token.type == to_count)
        {
            count++;
        }
        curr = curr->next;
    }

    *start = curr;
    return count;
}

table_t exit_code_table(int i)
{
    table_t result;
    result.header_count = 1;
    result.row_headers = calloc(1, sizeof(row_head_t));
    result.row_space = result.row_count = 1;
    result.row_headers[0].type = TYPE_INT;
    result.row_headers[0].name = "EXIT CODE";
    result.row_headers[0].offset = 0;

    result.row_footprint = size_of_type(TYPE_INT);
    result.rows = calloc(size_of_type(TYPE_INT), 1);

    set_value(&result, result.rows, 0, (uint8_t *) &i);

    return result;
}

table_t execute_table_instr(database_t * db, token_list_node_t ** tokens)
{
    token_list_node_t * curr = (*tokens)->next; // table name token
    assert(curr->token.type == TOKEN_NAME);
    char * table_name = curr->token.lit;

    curr = curr->next;
    assert(curr->token.type == TOKEN_LEFT_BRACE);

    curr = curr->next;
    // first pass to count the number of headers
    token_list_node_t * lookahead = curr;
    size_t count = count_until(&lookahead, TOKEN_SEMICOLON, TOKEN_RIGHT_BRACE);

    // second pass to collect values
    row_head_t * headers = calloc(count, sizeof(row_head_t));
    for(size_t i = 0; i < count; i++)
    {
        headers[i].type = translate_type(curr->token.type);
        curr = curr->next;
        calloc_strcpy(&headers[i].name, curr->token.lit);
        curr = curr->next;
        curr = curr->next;
    }

    assert(curr->token.type == TOKEN_RIGHT_BRACE);
    curr = curr->next;
    assert(curr->token.type == TOKEN_SEMICOLON);
    curr = curr->next;

    create_table(db, table_name, headers, count);

    *tokens = curr;

    return exit_code_table(0);
}

table_t execute_new_instr(database_t * db, token_list_node_t ** tokens)
{
    token_list_node_t * curr = (*tokens)->next; // table name token
    assert(curr->token.type == TOKEN_NAME);
    char * table_name = curr->token.lit;

    table_t * tbl = get_table(db, table_name);

    curr = curr->next;
    assert(curr->token.type == TOKEN_LEFT_BRACE);

    curr = curr->next;
    uint8_t * row = new_row(tbl);
    for (size_t j = 0; j < tbl->header_count; ++j)
    {
        if(tbl->row_headers[j].type == TYPE_INT)
        {
            int i = atoi(curr->token.lit);
            set_value(tbl, row, j, (uint8_t *) &i);
        }
        else if(tbl->row_headers[j].type == TYPE_FLOAT)
        {
            float f = atof(curr->token.lit);
            set_value(tbl, row, j, (uint8_t *) &f);
        }
        else if(tbl->row_headers[j].type == TYPE_STRING)
        {
            set_value(tbl, row, j, (uint8_t *) curr->token.lit);
        }
        curr = curr->next;
        if(curr->token.type == TOKEN_COMMA)
        {
            curr = curr->next;
        }
    }

    assert(curr->token.type == TOKEN_RIGHT_BRACE);
    curr = curr->next;
    assert(curr->token.type == TOKEN_SEMICOLON);
    curr = curr->next;

    *tokens = curr;

    return exit_code_table(0);
}

// execution consists of:
// - an action on an expression, or some fields
// - any number of inner joins to other tables, each with an on statement following it
// - a selector like where/top/unique used to narrow the action set

table_t execute_table_query(database_t *db, token_list_node_t **tokens)
{
    token_list_node_t * curr = *tokens;
    table_t * tbl = get_table(db, curr->token.lit);

    curr = curr->next;
    assert(curr->token.type == TOKEN_DOT);

    curr = curr->next;
    char * action = curr->token.lit;

    curr = curr->next;
    assert(curr->token.type == TOKEN_LEFT_PAREN);

    curr = curr->next;
    token_list_node_t * action_params = curr;
    token_list_node_t * lookahead = curr;
    size_t action_arg_count = count_until(&lookahead, TOKEN_COMMA, TOKEN_RIGHT_PAREN) + 1;

    if(strcmp(action, "select") == 0)
    {
        // build return table
        table_t result;
        result.header_count = action_arg_count;
        result.row_headers = calloc(action_arg_count, sizeof(row_head_t));
        size_t * col_ids = calloc(action_arg_count, sizeof(size_t));

        for(size_t i = 0; i < action_arg_count; i++)
        {
            assert(action_params->token.type == TOKEN_NAME);
            char * field = action_params->token.lit;
            col_ids[i] = get_col_id(tbl, field);

            result.row_headers[i].type = tbl->row_headers[col_ids[i]].type;
            result.row_headers[i].name = field;
            result.row_headers[i].offset = 0;

            action_params = action_params->next->next;
        }

        result.row_footprint = update_offset_get_total(result.row_headers, action_arg_count);

        result.row_space = INIT_ROWS;
        result.row_count = 0;
        result.rows = calloc(result.row_space, result.row_footprint);

        for(size_t row_id = 0; row_id < tbl->row_count; row_id++)
        {
            uint8_t * source_row_offset = get_row_id(tbl, row_id);
            uint8_t * result_row_offset = new_row(&result);
            for(size_t i = 0; i < action_arg_count; i++)
            {
                uint8_t * data = get_value(tbl, source_row_offset, col_ids[i]);
                set_value(&result, result_row_offset, i, data);
            }
        }

        *tokens = action_params->next;
        return result;
    }

    if(strcmp(action, "update") == 0)
    {

    }

    *tokens = curr;

    return exit_code_table(1);
}

table_t execute_joined_table_query(database_t *db, token_list_node_t *tokens)
{
    return exit_code_table(1);
}

void execute_code(database_t * db, char * string)
{
    token_list_node_t * tokens = tokenize(string);

    token_list_node_t * curr = tokens;
    print_token_list(tokens);

    while(curr->token.type != TOKEN_EOF)
    {
        table_t result;
        if (curr->token.type == TOKEN_TABLE)
        {
            result = execute_table_instr(db, &curr);
        }
        else if (curr->token.type == TOKEN_NEW)
        {
            result = execute_new_instr(db, &curr);
        }
        else if (curr->token.type == TOKEN_NAME)
        {
            result = execute_table_query(db, &curr);
        }
        print_table(&result);
    }
    free_token_list(tokens);
}
