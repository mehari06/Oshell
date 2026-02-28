#ifndef PARSER_H
#define PARSER_H

// Maximums
#ifndef MAX_COMMANDS
#define MAX_COMMANDS 32
#endif

#ifndef MAX_ARGS
#define MAX_ARGS 64
#endif


typedef enum {
    OP_NONE,
    OP_SEQ,
    OP_AND,
    OP_OR,
    OP_PARALLEL
} operator_t;

typedef struct command {
    char **argv;
    char *redirect;
   int background;
   int pipe_out;
   operator_t op; 
} command_t;


int parse_line(char *line, command_t **cmds, int *count);
void free_commands(command_t *cmds, int count);

#endif

