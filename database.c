//
// Created by dallen on 7/4/19.
//
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <stdbool.h>

#include "util.h"
#include "database.h"

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

table_t * get_table_id(database_t * db, size_t id)
{
    return db->tables + id;
}

table_t * get_table(database_t * db, char * name)
{
    for(size_t i = 0; i < db->table_count; i++)
    {
        if(strcmp(db->tables[i].name, name) == 0)
        {
            return db->tables + i;
        }
    }
    assert(false);
    return NULL;
}

uint8_t * new_row(table_t * tbl)
{
    assert(tbl->row_space != (tbl->row_count + 1));
    uint8_t * row_start = tbl->rows + (tbl->row_count * tbl->row_footprint);
    tbl->row_count++;
    return row_start;
}

uint8_t * get_row_id(table_t * tbl, size_t row_id)
{
    uint8_t * row_start = tbl->rows + (row_id * tbl->row_footprint);
    return row_start;
}

size_t get_col_id(table_t * tbl, char * name)
{
    for(size_t col = 0; col < tbl->header_count; col++)
    {
        if(strcmp(tbl->row_headers[col].name, name) == 0)
        {
            return col;
        }
    }
    assert(false);
    return 0;
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
            uint8_t * data = get_row_id(tbl, row_id) + tbl->row_headers[col].offset;
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
    for (int i = 0; i < len; ++i)
    {
        printf("-");
    }
    printf("\n");
    for (int row = 0; row < tbl->row_count + 1; ++row)
    {
        printf("%s\n", table_lines[row]);
        for (int i = 0; i < len; ++i)
        {
            printf("-");
        }
        printf("\n");
    }
    printf("\n");
}

