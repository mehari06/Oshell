#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "oshell.h"
#include "input.h"
#include "errors.h"
#include "parser.h"
#include "execute.h"

int last_status = 0;

int main(int argc, char *argv[])
{
    FILE *input = NULL;

    // Ignore SIGINT (Ctrl+C) in the shell itself
    // Child processes will naturally inherit this, so we need to ensure they reset it if needed,
    // explicitly or implicitly (execvp resets caught signals to default, but ignored signals stay ignored).
    // However, usually interactive shells ignore SIGINT so they don't die, but we want the child
    // to potentially receive it. A better approach for a simple shell:
    // When waiting for a child, we want the child to take the signal.
    // For now, let's just ignore it in the parent.
    signal(SIGINT, SIG_IGN);

    // Handle 0 or 1 argument (stdin or batch file)
    handle_startup_args(argc, argv, &input);

    while (1) {
        // Show prompt only in interactive mode
        if (is_interactive())
            write(STDOUT_FILENO, "$ ", 2);

        // Read a line from input
        char *line = read_line(input);
        if (!line)
            break;  // EOF or Ctrl+D

        // Normalize whitespace
        normalize_whitespace(line);

        // Parse the line
        command_t *cmds = NULL;
        int count = 0;

        if (parse_line(line, &cmds, &count) == -1) {
            free(line);
            continue;  // skip invalid line
        }

        // Execute parsed commands (fork + exec + redirection + && / ||)
        execute_commands(cmds, count);

        // Free memory allocated by parser
        free_commands(cmds, count);

        // Free the original line buffer
        free(line);
    }

    return last_status;
}

