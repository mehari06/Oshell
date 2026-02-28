#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>

int is_interactive(void);
int handle_startup_args(int argc, char *argv[], FILE **input);
char *read_line(FILE *source);
void normalize_whitespace(char *line);

#endif

