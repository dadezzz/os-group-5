#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdio.h>

size_t read_str_until_char(const char* src, char** dest, char pattern);

size_t read_file_until_char(FILE* file, char** dest, char pattern);

#endif
