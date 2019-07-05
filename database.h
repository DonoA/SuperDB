//
// Created by dallen on 7/4/19.
//

#ifndef SUPERDB_DATABASE_H
#define SUPERDB_DATABASE_H

#define INIT_TABLES 16
#define INIT_ROWS 16

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


database_t *create_database(char *name);
size_t size_of_type(enum type type);
size_t update_offset_get_total(row_head_t *headers, size_t header_count);
void create_table(database_t *db, char *name, row_head_t *headers, size_t header_count);
table_t * get_table_id(database_t * db, size_t id);
table_t * get_table(database_t * db, char * name);
uint8_t * new_row(table_t * tbl);
uint8_t * get_row_id(table_t * tbl, size_t row_id);
size_t get_col_id(table_t * tbl, char * name);
void set_value(table_t * tbl, uint8_t * row_offset, size_t col, uint8_t * data);
uint8_t * get_value(table_t * tbl, uint8_t * row_offset, size_t col);
char * value_to_string(enum type type, uint8_t * data);
char ** table_to_string(table_t * tbl);
void print_table(table_t * tbl);

#endif //SUPERDB_DATABASE_H
