#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "errors.h"

// Split a line into commands and operators
int parse_line(char *line, command_t **cmds, int *count) {
    if (!line || !cmds || !count)
        return -1;

    *count = 0;
    *cmds = malloc(MAX_COMMANDS * sizeof(command_t));
    if (!*cmds) { print_error(); return -1; }

    // Remove comment
    char *comment = strchr(line, '#');
    if (comment) *comment = '\0';

    char *ptr = line;
    while (*ptr) {
        while (isspace(*ptr)) ptr++;
        if (!*ptr) break;

        command_t *cmd = &(*cmds)[*count];
        cmd->argv = malloc((MAX_ARGS + 1) * sizeof(char *));
        cmd->redirect = NULL;
        cmd->background = 0;
        cmd->pipe_out = 0;
        cmd->op = OP_NONE;

        int argc = 0;

        // Parse command tokens until operator or end
        while (*ptr) {
            while (isspace(*ptr)) ptr++;
            if (!*ptr) break;

            // Check for operator (if not quoted)
            if (strchr(";|&", *ptr)) break;

            // Handle Redirection '>'
            if (*ptr == '>') {
                ptr++; // consume '>'
                while (isspace(*ptr)) ptr++;
                
                // Parse filename (respecting quotes? usually filenames don't have quotes but let's be robust or simple)
                // For simplicity: read until space or operator
                char filename[256] = {0};
                int flen = 0;
                while (*ptr && !isspace(*ptr) && !strchr(";|&>", *ptr)) {
                    if (flen < 255)
                        filename[flen++] = *ptr;
                    ptr++;
                }
                filename[flen] = '\0';
                
                if (flen == 0) { print_error(); return -1; }

                // If redirect already exists, create/truncate it as an intermediate file
                if (cmd->redirect) {
                    FILE *fp = fopen(cmd->redirect, "w");
                    if (fp) fclose(fp);
                    free(cmd->redirect);
                }
                cmd->redirect = strdup(filename);
                continue;
            }

            // Regular token parsing with quotes support
            char token[1024] = {0};
            int tlen = 0;
            int in_quote = 0;
            char quote_char = 0;

            while (*ptr) {
                if (in_quote) {
                    if (*ptr == quote_char) {
                        in_quote = 0;
                        ptr++;
                    } else {
                        token[tlen++] = *ptr++;
                    }
                } else {
                    if (isspace(*ptr) || strchr(";|&>", *ptr)) {
                        break; 
                    }
                    if (*ptr == '\'' || *ptr == '"') {
                        in_quote = 1;
                        quote_char = *ptr;
                        ptr++;
                    } else {
                        token[tlen++] = *ptr++;
                    }
                }
                if (tlen >= 1023) break; 
            }
            token[tlen] = '\0';

            if (tlen > 0 && argc < MAX_ARGS) {
                cmd->argv[argc++] = strdup(token);
            }
            
            // If we stopped at > or operator, the outer loop will handle it
        }

        cmd->argv[argc] = NULL;

        // Check operator after command
        while (*ptr && isspace(*ptr)) ptr++;
        if (*ptr) {
            if (*ptr == '&' && *(ptr + 1) == '&') { 
                cmd->op = OP_AND; 
                ptr += 2; 
            }
            else if (*ptr == '|' && *(ptr + 1) == '|') { 
                cmd->op = OP_OR; 
                ptr += 2; 
            }
            else if (*ptr == ';') { 
                cmd->op = OP_SEQ; 
                ptr++; 
            }
            else if (*ptr == '&') { 
                cmd->background = 1; 
                ptr++; 
            }
            else if (*ptr == '|') { 
                cmd->pipe_out = 1; 
                ptr++; 
            }
            else {
                // Unexpected character or malformed operator, just advance to avoid infinite loop
                // or treat as separate command start?
                // The loop condition `while (*ptr)` will catch next command.
                // But we must break this inner loop to increment counts.
                // If it's not an operator, it might be the start of next command if we didn't consume it?
                // But the inner loop "Parse command tokens" consumes everything until operator.
                // So here it must be an operator or syntax error.
                // Let's assume syntax error or skip.
                // Actually, if we hit `>` inside inner loop, we handled it.
                // So here we are at `;`, `&`, `|`...
                // If we are at something else like `)` ?
                if (!strchr(";|&", *ptr)) {
                     // Should not happen if inner loop works correctly
                     ptr++; 
                }
            }
        }

        (*count)++;
        if (*count >= MAX_COMMANDS) break;
    }

    return 0;
}

void free_commands(command_t *cmds, int count) {
    if (!cmds) return;
    for (int i = 0; i < count; i++) {
        if (cmds[i].argv) {
            for (int j = 0; cmds[i].argv[j]; j++)
                free(cmds[i].argv[j]);
            free(cmds[i].argv);
        }
        if (cmds[i].redirect) free(cmds[i].redirect);
    }
    free(cmds);
}

