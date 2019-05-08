#include "tokenizer.h"
#include <string.h>
#include <stdio.h>

/**
 * Retrieves the next token from a string.
 *
 * Parameters:
 * - str_ptr: maintains context in the string, i.e., where the next token in the
 *   string will be. If the function returns token N, then str_ptr will be
 *   updated to point to token N+1. To initialize, declare a char * that points
 *   to the string being tokenized. The pointer will be updated after each
 *   successive call to next_token.
 *
 * - delim: the set of characters to use as delimiters
 *
 * Returns: char pointer to the next token in the string.
 */
char *next_token(char **str_ptr, const char *delim)
{
    if (*str_ptr == NULL) {
        return NULL;
    }

	char quotes[] = "\'\"";

    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);
	size_t quote_start = strcspn(*str_ptr, quotes);

    /* Zero length token. We must be finished. */
    if (tok_end  <= 0) {
        *str_ptr = NULL;
        return NULL;
    }

	/* Remove quotes */
	if (quote_start < tok_end) {
		/* Remove the opposite type of quote */
		quotes[0] = *(*str_ptr + quote_start);
		quotes[1] = '\0';
		size_t offset = quote_start + 1;
		size_t quote_end = strcspn(*str_ptr + offset, quotes) + offset;
		tok_end = strcspn(*str_ptr + quote_end, delim) + quote_end - tok_start;
	}

    /* Take note of the start of the current token. We'll return it later. */
    char *current_ptr = *str_ptr + tok_start;

    /* Shift pointer forward (to the end of the current token) */
    *str_ptr += tok_start + tok_end;

    if (**str_ptr == '\0') {
        /* If the end of the current token is also the end of the string, we
         * must be at the last token. */
        *str_ptr = NULL;
    } else {
        /* Replace the matching delimiter with a NUL character to terminate the
         * token string. */
        **str_ptr = '\0';

        /* Shift forward one character over the newly-placed NUL so that
         * next_pointer now points at the first character of the next token. */
        (*str_ptr)++;
    }

    return current_ptr;
}

/**
 * Expands environment variables (identified by $ prefix, e.g., $SHELL) in a
 * string by resizing and inserting the value of the variable into the string.
 * This function only does one expansion, but can be called multiple times on a
 * string to expand more than one variable.
 * 
 * NOTE: this function allocates memory! The caller is responsible for freeing
 * the memory. 
 *
 * Parameters:
 * - str: The string with variables to expand
 *
 * Returns: char pointer to the newly-expanded and allocated string. Returns
 * NULL if there are no variables to replace or if memory cannot be allocated.
 */
char *expand_var(char *str)
{
    size_t var_start = 0;
    var_start = strcspn(str, "$");
    if (var_start == strlen(str)) {
        /* No variable to replace */
        return NULL;
    }

    size_t var_len = strcspn(str + var_start, " \t\r\n\'\"");

    char *var_name = malloc(sizeof(char) * var_len + 1);
    if (var_name == NULL) {
        return NULL;
    }
    strncpy(var_name, str + var_start, var_len);
    var_name[var_len] = '\0';

    if (strlen(var_name) <= 1) {
        free(var_name);
        return NULL;
    }

    /* Use index 1 to ignore the '$' prefix */
    char *value = getenv(&var_name[1]);
    if (value == NULL) {
        fprintf(stderr, "value was null\n");
        value = "";
    }

    fprintf(stderr, "Replacing variable: %s='%s'\n", var_name, value);
    free(var_name);

    /* Grab the size of the remaining string (after the $var): */
    size_t remain_sz = strlen(str + var_start + var_len);

    /* Our final string contains the prefix data, the value of the variable, the
     * remaining string size, and an extra character for the NUL byte. */
    size_t newstr_sz = var_start + strlen(value) + remain_sz + 1;

    char *newstr = malloc(sizeof(char) * newstr_sz);
    if (newstr == NULL) {
        return NULL;
    }

    strncpy(newstr, str, var_start);
    newstr[var_start] = '\0';
    strcat(newstr, value);
    strcat(newstr, str + var_start + var_len);

    return newstr;
}
