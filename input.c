#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "input.h"
#include "errors.h"

int is_interactive(void)
{
    return isatty(STDIN_FILENO);
}

int handle_startup_args(int argc, char *argv[], FILE **input)
{
    if (argc > 2) {
        print_error();
        exit(1);
    }

    if (argc == 2) {
        *input = fopen(argv[1], "r");
        if (!*input) {
            print_error();
            exit(1);
        }
    } else {
        *input = stdin;
    }
    return 0;
}

char *read_line(FILE *source)
{
    char *line = NULL;
    size_t len = 0;

    if (getline(&line, &len, source) == -1) {
        free(line);
        return NULL;
    }
    return line;
}

void normalize_whitespace(char *line)
{
    int i = 0, j = 0, space = 0;

    while (isspace(line[i]))
        i++;

    for (; line[i]; i++) {
        if (isspace(line[i])) {
            if (!space) {
                line[j++] = ' ';
                space = 1;
            }
        } else {
            line[j++] = line[i];
            space = 0;
        }
    }

    if (j > 0 && line[j - 1] == ' ')
        j--;

    line[j] = '\0';
}

