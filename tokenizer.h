#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *next_token(char **str_ptr, const char *delim);
char *expand_var(char *str);

#endif
