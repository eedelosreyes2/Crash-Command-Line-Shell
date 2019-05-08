/**
 * debug.c
 *
 * Debugging using the preprocessor. When DEBUG is set to 0, the LOG statements
 * are completely removed from the binary produced after compliation.
 *
 * Source: https://stackoverflow.com/questions/5765175/macro-to-turn-off-printf-statements
 * 
 * To turn debug on, either:
 * - Add a line like this to your C file:
 *      #define DEBUG 1
 * - Compile as shown below
 *
 * Compile:
 *      gcc -g -Wall -o debug -DDEBUG=1 debug.c
 */

#include <stdio.h>

/* If we haven't passed -DDEBUG=1 to gcc, then this will be set to 0: */
#ifndef DEBUG
#define DEBUG 0
#endif

#define LOGP(str) \
    do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): %s", __FILE__, \
            __LINE__, __func__, str); } while (0)

#define LOG(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
            __LINE__, __func__, __VA_ARGS__); } while (0)

int main(void)
{
    int i = 6;

    LOG("Hello world! i = %d\n", i);

    /* Note: if you just want to print a plain old string, this won't work!
     * BAD: LOG("Yo yo!\n");
     * To get around this, you can use:
     */
    LOG("%s", "Yo yo!\n");

    /* ...or, you can use LOGP: */
    LOGP("Yo yo!\n");

#if DEBUG == 0
    printf("Debug is disabled! "
            "Re-compile with `-DDEBUG=1` to see log messages\n");
#endif

    return 0;
}

