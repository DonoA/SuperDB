//
// Created by dallen on 7/5/19.
//

#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>

#include "util.h"

void calloc_strcpy(char ** dest, char * src)
{
    *dest = calloc(strlen(src), sizeof(char));
    strcpy(*dest, src);
}

bool is_number(char c)
{
    if(c >= '0' && c <= '9')
    {
        return true;
    }
    if(c == '.')
    {
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