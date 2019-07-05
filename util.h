//
// Created by dallen on 7/5/19.
//

#ifndef SUPERDB_UTIL_H
#define SUPERDB_UTIL_H

#include <stdbool.h>

void calloc_strcpy(char ** dest, char * src);
bool is_number(char c);
bool is_name_char(char c);
bool starts_with(char * src, char * starts_with);

#endif //SUPERDB_UTIL_H
