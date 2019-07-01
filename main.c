#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <stdbool.h>

#define INIT_TABLES 16
#define INIT_ROWS 16

#define MAX_LIT_SIZE 32

enum type {
    TYPE_INT, TYPE_FLOAT, TYPE_STRING
};

typedef struct
{
    enum type type;
    char * name;
    size_t offset;
} row_head_t;

typedef struct
{
    size_t header_count;
    row_head_t *row_headers;
    size_t row_footprint;
    size_t row_count;
    size_t row_space;
    uint8_t * rows;
    char *name;
} table_t;


typedef struct
{
    size_t table_count;
    size_t table_space;
    table_t *tables;
    char *name;
} database_t;

void calloc_strcpy(char ** dest, char * src)
{
    *dest = calloc(strlen(src), sizeof(char));
    strcpy(*dest, src);
}

database_t *create_database(char *name)
{
    database_t *db = calloc(1, sizeof(database_t));
    calloc_strcpy(&db->name, name);
    db->tables = calloc(INIT_TABLES, sizeof(table_t));
    db->table_count = 0;
    db->table_space = INIT_TABLES;
    return db;
}

size_t size_of_type(enum type type)
{
    switch(type)
    {
        case TYPE_INT:
            return 4;
        case TYPE_FLOAT:
            return 4;
        case TYPE_STRING:
            return 255;
    }
    assert(false);
    return 0;
}

size_t update_offset_get_total(row_head_t *headers, size_t header_count)
{
    size_t offset = 0;
    for(size_t i = 0; i < header_count; i++)
    {
        headers[i].offset = offset;
        offset += size_of_type(headers[i].type);
    }
    return offset;
}

void create_table(database_t *db, char *name, row_head_t *headers, size_t header_count)
{
    assert(db->table_space != (db->table_count + 1));
    table_t *tbl = db->tables + db->table_count;
    db->table_count++;
    calloc_strcpy(&tbl->name, name);
    tbl->header_count = header_count;
    tbl->row_headers = headers;
    tbl->row_footprint = update_offset_get_total(headers, header_count);
    tbl->rows = calloc(INIT_ROWS, tbl->row_footprint);
    tbl->row_count = 0;
    tbl->row_space = INIT_ROWS;
}

table_t * get_table(database_t * db, size_t id)
{
    return db->tables + id;
}

uint8_t * new_row(table_t * tbl)
{
    assert(tbl->row_space != (tbl->row_count + 1));
    uint8_t * row_start = tbl->rows + (tbl->row_count * tbl->row_footprint);
    tbl->row_count++;
    return row_start;
}

uint8_t * get_row(table_t * tbl, size_t row_id)
{
    uint8_t * row_start = tbl->rows + (tbl->row_count * row_id);
    return row_start;
}

void set_value(table_t * tbl, uint8_t * row_offset, size_t col, uint8_t * data)
{
    size_t offset = tbl->row_headers[col].offset;
    size_t data_size = size_of_type(tbl->row_headers[col].type);
    for(size_t i = 0; i < data_size; i++)
    {
        row_offset[offset + i] = data[i];
    }
}

uint8_t * get_value(table_t * tbl, uint8_t * row_offset, size_t col)
{
    size_t offset = tbl->row_headers[col].offset;
    return row_offset + offset;
}

char * value_to_string(enum type type, uint8_t * data)
{
    switch(type)
    {
        case TYPE_INT:
        {
            char * rtn = calloc(16, sizeof(char));
            size_t n = sprintf(rtn, "%i", *((int32_t *) data));
            return rtn;
        }
        case TYPE_FLOAT:
        {
            char *rtn = calloc(32, sizeof(char));
            size_t n = sprintf(rtn, "%f", *((float *) data));
            return rtn;
        }
        case TYPE_STRING:
        {
            char *rtn = calloc(255, sizeof(char));
            strcpy(rtn, data);
            return rtn;
        }

    }
    assert(false);
}

char ** table_to_string(table_t * tbl)
{
    size_t max_elt_len[tbl->header_count];
    char *** all_elts = calloc(tbl->row_count + 1, sizeof(char **));

    for(size_t i = 0; i < tbl->header_count; i++)
    {
        max_elt_len[i] = strlen(tbl->row_headers[i].name);
    }

    for(size_t row_id = 0; row_id < tbl->row_count; row_id++)
    {
        all_elts[row_id] = calloc(tbl->header_count, sizeof(char *));
        for(size_t col = 0; col < tbl->header_count; col++)
        {
            enum type type = tbl->row_headers[col].type;
            uint8_t * data = get_row(tbl, row_id) + tbl->row_headers[col].offset;
            all_elts[row_id][col] = value_to_string(type, data);
            size_t n = strlen(all_elts[row_id][col]);
            if(n > max_elt_len[col])
            {
                max_elt_len[col] = n;
            }
        }
    }

    size_t line_len = 0;
    for (size_t col = 0; col < tbl->header_count; ++col)
    {
        line_len += 3 + max_elt_len[col];
    }

    // final pipe and null char
    line_len += 2;

    char ** all_lines = calloc(tbl->row_count + 1, sizeof(char *));

    all_lines[0] = calloc(line_len, sizeof(char));
    for (int col = 0; col < tbl->header_count; ++col)
    {
        strcat(all_lines[0], "| ");
        strcat(all_lines[0], tbl->row_headers[col].name);
        size_t c_size = strlen(all_lines[0]);
        size_t val_len = strlen(tbl->row_headers[col].name);
        for(size_t j = 0; j < (max_elt_len[col] - val_len + 1); ++j)
        {
            all_lines[0][c_size + j] = ' ';
        }
    }
    strcat(all_lines[0], "|");

    for (int i = 0; i < tbl->row_count; ++i)
    {
        all_lines[i + 1] = calloc(line_len, sizeof(char));
        for (int col = 0; col < tbl->header_count; ++col)
        {
            strcat(all_lines[i + 1], "| ");
            strcat(all_lines[i + 1], all_elts[i][col]);
            size_t c_size = strlen(all_lines[i + 1]);
            size_t val_len = strlen(all_elts[i][col]);
            for(size_t j = 0; j < (max_elt_len[col] - val_len + 1); ++j)
            {
                all_lines[i + 1][c_size + j] = ' ';
            }
        }
        strcat(all_lines[i + 1], "|");
    }

    return all_lines;
}

void print_table(table_t * tbl)
{
    char ** table_lines = table_to_string(tbl);

    size_t len = strlen(table_lines[0]);
    for (int row = 0; row < tbl->row_count + 1; ++row)
    {
        printf("%s\n", table_lines[row]);
        for (int i = 0; i < len; ++i)
        {
            printf("-");
        }
        printf("\n");
    }
}

enum token_type {
    TOKEN_TABLE, TOKEN_INT, TOKEN_STRING, TOKEN_FLOAT, TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE, TOKEN_NAME, TOKEN_SEMICOLON
};

typedef struct {
    enum token_type type;
    char lit[MAX_LIT_SIZE];
} instr_token_t;

typedef struct token_list_node_struct {
    instr_token_t token;
    struct token_list_node_struct * next;
} token_list_node_t;

bool starts_with(char * src, char * starts_with)
{
    size_t i = 0;
    while(true)
    {
        if(starts_with[i] == '\0')
        {
            return true;
        }

        if(src[i] != starts_with[i])
        {
            return false;
        }

        if(src[i] == '\0')
        {
            return false;
        }

        i++;
    }
}

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

bool check_for_token(char * string, size_t * pos, char * token_string, enum token_type type, token_list_node_t ** tokens)
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

bool is_name_char(char c)
{
    if(c >= 'a' && c <= 'z')
    {
        return true;
    }
    if(c >= 'A' && c <= 'Z')
    {
        return true;
    }
    if(c >= '0' && c <= '9')
    {
        return true;
    }
    if(c == '_')
    {
        return true;
    }
    return false;
}

token_list_node_t * tokenize(char * string)
{
    token_list_node_t * tokens = NULL;
    size_t pos = 0;
    while(string[pos] != '\0')
    {
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

        if(string[pos] == '\n' || string[pos] == ' ' || string[pos] == '\t')
        {
            pos++;
            continue;
        }

        size_t i = 0;
        instr_token_t token;
        token.type = TOKEN_NAME;
        memset(token.lit, '\0', MAX_LIT_SIZE);
        while(is_name_char(string[pos]))
        {
            token.lit[i] = string[pos];
            pos++;
            i++;
        }
        add_token(&tokens, token);
    }
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

void execute_table_instr(database_t * db, token_list_node_t * tokens)
{
    token_list_node_t * curr = tokens->next; // table name token
    assert(curr->token.type == TOKEN_NAME);
    char * table_name = curr->token.lit;

    curr = curr->next;
    assert(curr->token.type == TOKEN_LEFT_BRACE);

    curr = curr->next;
    token_list_node_t * token = curr;
    // first pass to count the number of headers
    size_t count = 0;
    while(token->token.type != TOKEN_RIGHT_BRACE) {
        count++;
        while(token->token.type != TOKEN_SEMICOLON)
        {
            token = token->next;
        }
        token = token->next;
    }

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

    create_table(db, table_name, headers, count);
}

void execute_code(database_t * db, char * string)
{
    /*
     * table testTbl {
     *   int id;
     *   int age;
     *   string name;
     * }
     */

    token_list_node_t * tokens = tokenize(string);

    print_token_list(tokens);

    if(tokens->token.type == TOKEN_TABLE)
    {
        execute_table_instr(db, tokens);
        return;
    }
}

int main(int argc, char *argv[])
{
    database_t * db = create_database("testDB");

    execute_code(db, "table testTbl {\n   int id;\n   int age;\n   string name;\n };\n");

    table_t * table = get_table(db, 0);
//
//    uint8_t * row = new_row(table);
//    int id = 0;
//    set_value(table, row, 0, &id);
//    int age = 25;
//    set_value(table, row, 1, &age);
//    char * name = "James Simpson";
//    set_value(table, row, 2, name);
//
    print_table(table);
    printf("Done!\n");
    return 0;
}